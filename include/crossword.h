#ifndef CROSSWORD_H
#define CROSSWORD_H

#include <stdbool.h>
#include <time.h>

#define GRID_SIZE 15
#define MAX_WORD_LENGTH 24   /* includes room for terminating NUL */
#define MAX_WORDS 50

/* ANSI color macros (optional) */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

/* owner bitmask */
#define OWNER_ACROSS 1
#define OWNER_DOWN   2

/* Word placement record */
typedef struct {
    char word[MAX_WORD_LENGTH]; /* stored uppercase, NUL-terminated */
    int row;
    int col;
    char direction;            /* 'A' or 'D' */
    int clue_num;
    bool hint_used;
} WordPos;

/* Puzzle state - kept as a POD structure; the Puzzle instance itself
   will be allocated on the heap by the caller (dynamic allocation). */
typedef struct {
    char sol[GRID_SIZE][GRID_SIZE];    /* solution letters, ' ' empty */
    char user[GRID_SIZE][GRID_SIZE];   /* user view: '_' for empty cell containing a letter */
    unsigned char owner[GRID_SIZE][GRID_SIZE];
    WordPos *positions;    /* dynamic array of length word_count (allocated at generate time) */
    int word_count;
    int clue_counter;
    time_t start_time;
} Puzzle;


void clear_screen(void);
/* utils.c style helpers */
void safe_gets(char *buf, int size);
void to_upper_inplace(char *s);

/* puzzle operations (crossword.c) */
Puzzle *puzzle_create(void);
void puzzle_free(Puzzle *p);
bool puzzle_init(Puzzle *p);
void puzzle_create_user_grid(Puzzle *p);
bool puzzle_can_place(Puzzle *p, const char *w, int r, int c, char d);
bool puzzle_place_word_record(Puzzle *p, const char *w, int r, int c, char d);
int puzzle_find_intersection(Puzzle *p, const char *w, int *out_r, int *out_c, char *out_d);
bool puzzle_generate(Puzzle *p, char **words, int count);

/* rendering and interaction */
void draw_grid(const Puzzle *p, bool solution_view);
void show_clues(const Puzzle *p);
bool input_answer(Puzzle *p, int clue, char d, const char *ans);
bool give_hint(Puzzle *p, int clue, char d);
bool puzzle_solved(const Puzzle *p);
float puzzle_completion(const Puzzle *p);
void show_timer(const Puzzle *p);

#endif /* CROSSWORD_H */
