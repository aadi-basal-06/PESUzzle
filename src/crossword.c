/* src/crossword.c
 *
 * Rewritten crossword implementation (ASCII Option A).
 * Assumes header is available at ../include/crossword.h when compiling from src/.
 *
 * All state is stored in Puzzle; positions[] is dynamically grown with realloc.
 */

#include "../include/crossword.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ----------------- Platform helper ----------------- */

/* Cross-platform clear screen */
void clear_screen(void) {
#ifdef _WIN32
    (void)system("cls");
#else
    (void)system("clear");
#endif
}

/* ----------------- Utilities ----------------- */

/* safe_gets defined in header; implement here */
void safe_gets(char *buf, int size) {
    if (!buf || size <= 0) return;
    if (!fgets(buf, size, stdin)) {
        buf[0] = '\0';
        clearerr(stdin);
        return;
    }
    size_t ln = strcspn(buf, "\n");
    if (ln < (size_t)size) buf[ln] = '\0';
}

/* uppercase in-place */
void to_upper_inplace(char *s) {
    if (!s) return;
    for (size_t i = 0; s[i]; ++i) s[i] = (char) toupper((unsigned char)s[i]);
}

/* ----------------- Puzzle lifecycle ----------------- */

/* Allocate puzzle instance */
Puzzle *puzzle_create(void) {
    Puzzle *p = (Puzzle *) calloc(1, sizeof(Puzzle));
    if (!p) return NULL;
    p->positions = NULL;
    p->word_count = 0;
    p->clue_counter = 1;
    p->start_time = time(NULL);
    return p;
}

/* Free puzzle instance */
void puzzle_free(Puzzle *p) {
    if (!p) return;
    free(p->positions);
    free(p);
}

/* Initialize/reset puzzle grid and counters */
bool puzzle_init(Puzzle *p) {
    if (!p) return false;
    for (int r = 0; r < GRID_SIZE; ++r)
        for (int c = 0; c < GRID_SIZE; ++c) {
            p->sol[r][c] = ' ';
            p->user[r][c] = ' ';
            p->owner[r][c] = 0;
        }
    free(p->positions);
    p->positions = NULL;
    p->word_count = 0;
    p->clue_counter = 1;
    p->start_time = time(NULL);
    return true;
}

/* Create user grid with '_' placeholders for letter cells */
void puzzle_create_user_grid(Puzzle *p) {
    if (!p) return;
    for (int r = 0; r < GRID_SIZE; ++r)
        for (int c = 0; c < GRID_SIZE; ++c)
            p->user[r][c] = (p->sol[r][c] != ' ') ? '_' : ' ';
}

/* ----------------- Placement logic ----------------- */

/* comparator for qsort by descending length */
static int cmp_len_desc(const void *a, const void *b) {
    const char * const *pa = (const char * const *)a;
    const char * const *pb = (const char * const *)b;
    size_t la = strlen(*pa), lb = strlen(*pb);
    if (la < lb) return 1;
    if (la > lb) return -1;
    return 0;
}

/* Check whether a word can be placed at r,c in direction d ('A' or 'D') */
bool puzzle_can_place(Puzzle *p, const char *w, int r, int c, char d) {
    if (!p || !w) return false;
    int L = (int)strlen(w);
    if (L <= 0 || L >= MAX_WORD_LENGTH) return false;

    if (d == 'A') {
        if (r < 0 || r >= GRID_SIZE || c < 0 || c + L > GRID_SIZE) return false;
        if (c > 0 && p->sol[r][c-1] != ' ') return false;
        if (c + L < GRID_SIZE && p->sol[r][c+L] != ' ') return false;
        for (int i = 0; i < L; ++i) {
            char cur = p->sol[r][c+i];
            if (cur != ' ' && cur != w[i]) return false;
            if (cur == ' ') {
                if (r > 0 && p->sol[r-1][c+i] != ' ') return false;
                if (r + 1 < GRID_SIZE && p->sol[r+1][c+i] != ' ') return false;
            }
        }
    } else if (d == 'D') {
        if (c < 0 || c >= GRID_SIZE || r < 0 || r + L > GRID_SIZE) return false;
        if (r > 0 && p->sol[r-1][c] != ' ') return false;
        if (r + L < GRID_SIZE && p->sol[r+L][c] != ' ') return false;
        for (int i = 0; i < L; ++i) {
            char cur = p->sol[r+i][c];
            if (cur != ' ' && cur != w[i]) return false;
            if (cur == ' ') {
                if (c > 0 && p->sol[r+i][c-1] != ' ') return false;
                if (c + 1 < GRID_SIZE && p->sol[r+i][c+1] != ' ') return false;
            }
        }
    } else {
        return false;
    }

    return true;
}

/* Place the word into grid and append a WordPos record; returns true on success */
bool puzzle_place_word_record(Puzzle *p, const char *w, int r, int c, char d) {
    if (!p || !w) return false;
    if (!puzzle_can_place(p, w, r, c, d)) return false;

    WordPos *new_arr = (WordPos *) realloc(p->positions, (size_t)(p->word_count + 1) * sizeof(WordPos));
    if (!new_arr) return false;
    p->positions = new_arr;

    int L = (int)strlen(w);
    if (d == 'A') {
        for (int i = 0; i < L; ++i) {
            p->sol[r][c+i] = w[i];
            p->owner[r][c+i] |= OWNER_ACROSS;
        }
    } else { /* 'D' */
        for (int i = 0; i < L; ++i) {
            p->sol[r+i][c] = w[i];
            p->owner[r+i][c] |= OWNER_DOWN;
        }
    }

    WordPos *wp = &p->positions[p->word_count];
    strncpy(wp->word, w, MAX_WORD_LENGTH - 1);
    wp->word[MAX_WORD_LENGTH - 1] = '\0';
    wp->row = r;
    wp->col = c;
    wp->direction = d;
    wp->clue_num = p->clue_counter++;
    wp->hint_used = false;

    p->word_count++;
    return true;
}

/* Find an intersection placement for w; if found set out_r/out_c/out_d and return 1 */
int puzzle_find_intersection(Puzzle *p, const char *w, int *out_r, int *out_c, char *out_d) {
    if (!p || !w) return 0;
    int Lw = (int)strlen(w);
    for (int k = 0; k < p->word_count; ++k) {
        WordPos *wp = &p->positions[k];
        int L2 = (int)strlen(wp->word);
        for (int i = 0; i < Lw; ++i) {
            for (int j = 0; j < L2; ++j) {
                if (w[i] != wp->word[j]) continue;
                int nr, nc; char nd;
                if (wp->direction == 'A') {
                    nr = wp->row - i;
                    nc = wp->col + j;
                    nd = 'D';
                } else {
                    nr = wp->row + j;
                    nc = wp->col - i;
                    nd = 'A';
                }
                if (nr < 0 || nc < 0 || nr >= GRID_SIZE || nc >= GRID_SIZE) continue;
                if (puzzle_can_place(p, w, nr, nc, nd)) {
                    if (out_r) *out_r = nr;
                    if (out_c) *out_c = nc;
                    if (out_d) *out_d = nd;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* ----------------- Puzzle generation ----------------- */

/* Generate puzzle: words[] are NUL-terminated uppercase strings, count is number of pointers.
   Returns true if at least one word placed successfully. */
bool puzzle_generate(Puzzle *p, char **words, int count) {
    if (!p || !words || count <= 0) return false;

    /* gather valid words pointers */
    char **tmp = (char **) malloc((size_t)count * sizeof(char *));
    if (!tmp) return false;
    int wc = 0;
    for (int i = 0; i < count && wc < MAX_WORDS; ++i) {
        if (words[i] && words[i][0] != '\0') tmp[wc++] = words[i];
    }
    if (wc == 0) { free(tmp); return false; }

    /* sort by length descending */
    qsort(tmp, (size_t)wc, sizeof(char *), cmp_len_desc);

    if (!puzzle_init(p)) { free(tmp); return false; }

    /* place the longest horizontally near center if possible */
    int L0 = (int)strlen(tmp[0]);
    int sr = GRID_SIZE / 2;
    int sc = (GRID_SIZE - L0) / 2;
    if (sc < 0) sc = 0;
    if (!puzzle_place_word_record(p, tmp[0], sr, sc, 'A')) {
        bool placed = false;
        for (int r = 0; r < GRID_SIZE && !placed; ++r)
            for (int c = 0; c < GRID_SIZE && !placed; ++c)
                if (puzzle_place_word_record(p, tmp[0], r, c, 'A')) placed = true;
        (void)placed;
    }

    /* place remaining words: try intersection first */
    for (int i = 1; i < wc; ++i) {
        int r, c; char d;
        if (puzzle_find_intersection(p, tmp[i], &r, &c, &d)) {
            (void)puzzle_place_word_record(p, tmp[i], r, c, d);
            continue;
        }
        bool placed = false;
        for (int rr = 0; rr < GRID_SIZE && !placed; ++rr) {
            for (int cc = 0; cc < GRID_SIZE && !placed; ++cc) {
                if (puzzle_place_word_record(p, tmp[i], rr, cc, 'A')) placed = true;
                else if (puzzle_place_word_record(p, tmp[i], rr, cc, 'D')) placed = true;
            }
        }
        (void)placed;
    }

    puzzle_create_user_grid(p);
    p->start_time = time(NULL);
    free(tmp);
    return p->word_count > 0;
}

/* ----------------- Rendering helpers ----------------- */

static void print_centered_str(const char *s, int width) {
    if (!s || width <= 0) {
        for (int i = 0; i < width; ++i) putchar(' ');
        return;
    }
    int len = (int)strlen(s);
    if (len >= width) {
        for (int i = 0; i < width; ++i) putchar(s[i]);
        return;
    }
    int pad = width - len;
    int left = pad / 2;
    int right = pad - left;
    for (int i = 0; i < left; ++i) putchar(' ');
    fputs(s, stdout);
    for (int i = 0; i < right; ++i) putchar(' ');
}

/* ----------------- Drawing (ASCII Option A) ----------------- */

void draw_grid(const Puzzle *p, bool solution_view) {
    if (!p) return;

    const int ROW_LABEL_WIDTH = 4;  /* space reserved for row numbers */
    const int CELL_WIDTH = 5;       /* internal width of each cell */

    const char *border_col = BOLD;
    const char *blank_col  = CYAN;

    /* refresh screen */
    clear_screen();

    /* column header: align with ROW_LABEL_WIDTH */
    for (int i = 0; i < ROW_LABEL_WIDTH; ++i) putchar(' ');
    for (int c = 0; c < GRID_SIZE; ++c) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%2d", c);
        print_centered_str(tmp, CELL_WIDTH);
    }
    putchar('\n');

    /* top border */
    for (int i = 0; i < ROW_LABEL_WIDTH; ++i) putchar(' ');
    for (int c = 0; c < GRID_SIZE; ++c) {
        printf("%s+%s", border_col, RESET);
        for (int k = 0; k < CELL_WIDTH; ++k) printf("%s=%s", border_col, RESET);
    }
    printf("%s+%s\n", border_col, RESET);


    /* rows */
    for (int r = 0; r < GRID_SIZE; ++r) {
        /* row label */
        char rowlabel[16];
        snprintf(rowlabel, sizeof(rowlabel), "%3d", r);
        print_centered_str(rowlabel, ROW_LABEL_WIDTH);

        /* cells */
        for (int c = 0; c < GRID_SIZE; ++c) {
            putchar('|');
            char ch = solution_view ? p->sol[r][c] : p->user[r][c];

            char content[8] = " ";
            if (ch == ' ') content[0] = ' ', content[1] = '\0';
            else if (!solution_view && ch == '_') content[0] = '_', content[1] = '\0';
            else { content[0] = ch; content[1] = '\0'; }

            int len = (int)strlen(content);
            if (len >= CELL_WIDTH) {
                /* truncated print */
                for (int i = 0; i < CELL_WIDTH && content[i]; ++i) putchar(content[i]);
            } else {
                int pad = CELL_WIDTH - len;
                int left = pad / 2;
                int right = pad - left;

                /* left padding */
                for (int i = 0; i < left; ++i) putchar(' ');

                /* choose color for letters (print color codes around content only) */
                if (content[0] != ' ' && content[0] != '_') {
                    unsigned char own = p->owner[r][c];
                    const char *cell_col = GREEN;
                    if ((own & OWNER_ACROSS) && (own & OWNER_DOWN)) cell_col = MAGENTA;
                    else if (own & OWNER_ACROSS) cell_col = YELLOW;
                    else if (own & OWNER_DOWN) cell_col = RED;
                    fputs(cell_col, stdout);
                    fputs(content, stdout);
                    fputs(RESET, stdout);
                } else if (content[0] == '_') {
                    fputs(blank_col, stdout);
                    fputs(content, stdout);
                    fputs(RESET, stdout);
                } else {
                    putchar(' ');
                }

                /* right padding */
                for (int i = 0; i < right; ++i) putchar(' ');
            }
        }
        putchar('|');
        putchar('\n');

        /* separator line */
        for (int i = 0; i < ROW_LABEL_WIDTH; ++i) putchar(' ');
        for (int c = 0; c < GRID_SIZE; ++c) {
            putchar('+');
            for (int k = 0; k < CELL_WIDTH; ++k) putchar('=');
        }
        putchar('+');
        putchar('\n');
    }

    putchar('\n');
}

/* ----------------- Clues ----------------- */

void show_clues(const Puzzle *p) {
    if (!p) return;
    printf("\n%sACROSS:%s\n", BOLD, RESET);
    for (int i = 0; i < p->word_count; ++i) {
        if (p->positions[i].direction == 'A') {
            const char *w = p->positions[i].word;
            printf("%2d. %c...%c (%ld) at [%d,%d]%s\n",
                   p->positions[i].clue_num,
                   w[0], w[strlen(w)-1],
                   (long) strlen(w),
                   p->positions[i].row, p->positions[i].col,
                   p->positions[i].hint_used ? " (hint used)" : "");
        }
    }

    printf("\n%sDOWN:%s\n", BOLD, RESET);
    for (int i = 0; i < p->word_count; ++i) {
        if (p->positions[i].direction == 'D') {
            const char *w = p->positions[i].word;
            printf("%2d. %c...%c (%ld) at [%d,%d]%s\n",
                   p->positions[i].clue_num,
                   w[0], w[strlen(w)-1],
                   (long) strlen(w),
                   p->positions[i].row, p->positions[i].col,
                   p->positions[i].hint_used ? " (hint used)" : "");
        }
    }
    putchar('\n');
}

/* ----------------- Interaction ----------------- */

bool input_answer(Puzzle *p, int clue, char d, const char *ans) {
    if (!p || !ans) return false;
    for (int i = 0; i < p->word_count; ++i) {
        if (p->positions[i].clue_num == clue && p->positions[i].direction == d) {
            WordPos *wp = &p->positions[i];
            int L = (int)strlen(wp->word);
            if ((int)strlen(ans) != L) {
                printf("%sWrong length! Expected %d letters.%s\n", RED, L, RESET);
                return false;
            }
            if (d == 'A') {
                for (int k = 0; k < L; ++k) p->user[wp->row][wp->col + k] = ans[k];
            } else {
                for (int k = 0; k < L; ++k) p->user[wp->row + k][wp->col] = ans[k];
            }
            printf("%sPlaced answer for clue %d %c.%s\n", GREEN, clue, d, RESET);
            return true;
        }
    }
    printf("%sInvalid clue number/direction.%s\n", RED, RESET);
    return false;
}

bool give_hint(Puzzle *p, int clue, char d) {
    if (!p) return false;
    for (int i = 0; i < p->word_count; ++i) {
        if (p->positions[i].clue_num == clue && p->positions[i].direction == d) {
            WordPos *wp = &p->positions[i];
            int L = (int)strlen(wp->word);
            int choices[MAX_WORD_LENGTH];
            int cc = 0;
            for (int k = 0; k < L; ++k) {
                int rr = wp->row + (d == 'D' ? k : 0);
                int cc2 = wp->col + (d == 'A' ? k : 0);
                if (p->user[rr][cc2] != p->sol[rr][cc2]) choices[cc++] = k;
            }
            if (cc == 0) {
                printf("%sAll letters already revealed for that clue.%s\n", YELLOW, RESET);
                return true;
            }
            int pick = choices[rand() % cc];
            int rr = wp->row + (d == 'D' ? pick : 0);
            int cc2 = wp->col + (d == 'A' ? pick : 0);
            p->user[rr][cc2] = p->sol[rr][cc2];
            wp->hint_used = true;
            printf("%sHint: revealed letter %d -> %c%s\n", CYAN, pick + 1, p->sol[rr][cc2], RESET);
            return true;
        }
    }
    printf("%sInvalid clue for hint.%s\n", RED, RESET);
    return false;
}

/* ----------------- Progress / Timer ----------------- */

bool puzzle_solved(const Puzzle *p) {
    if (!p) return false;
    for (int r = 0; r < GRID_SIZE; ++r)
        for (int c = 0; c < GRID_SIZE; ++c)
            if (p->sol[r][c] != ' ')
                if (p->user[r][c] != p->sol[r][c]) return false;
    return true;
}

float puzzle_completion(const Puzzle *p) {
    if (!p) return 0.0f;
    int total = 0, good = 0;
    for (int r = 0; r < GRID_SIZE; ++r)
        for (int c = 0; c < GRID_SIZE; ++c)
            if (p->sol[r][c] != ' ') {
                total++;
                if (p->user[r][c] == p->sol[r][c]) good++;
            }
    return total ? (float)good * 100.0f / (float)total : 0.0f;
}

void show_timer(const Puzzle *p) {
    if (!p) return;

    clear_screen();   // <--- refresh screen BEFORE printing timer

    int sec = (int) difftime(time(NULL), p->start_time);
    printf("%sElapsed time: %02d:%02d%s\n", CYAN, sec / 60, sec % 60, RESET);

    printf("\nPress ENTER to return to menu...");
    getchar();
}