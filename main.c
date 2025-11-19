#include "include/crossword.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void show_title() {
    printf(
" _______  _______  _______  __   __  _______  _______  ___      ___      _______ \n"
"|       ||       ||       ||  | |  ||       ||       ||   |    |   |    |       |\n"
"|    _  ||    ___||  _____||  | |  ||____   ||____   ||   |    |   |    |    ___|\n"
"|   |_| ||   |___ | |_____ |  |_|  | ____|  | ____|  ||   |    |   |    |   |___ \n"
"|    ___||    ___||_____  ||       || ______|| ______||   |___ |   |___ |    ___|\n"
"|   |    |   |___  _____| ||       || |_____ | |_____ |       ||       ||   |___ \n"
"|___|    |_______||_______||_______||_______||_______||_______||_______||_______|\n\n"
    );
}


int main(void) {
    Puzzle *p = puzzle_create();
    if (!p) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    srand((unsigned)time(NULL));

    /* Default words only */
    char default_words[][MAX_WORD_LENGTH] = {
        "QUEUE", "STACK", "GRAPH", "ALGORITHM", "SEARCH", "SORT",
        "TREE", "NODE", "ARRAY", "DATA", "PAINT", "ROBOT",
        "NOISE", "OFFER", "ASSET", "COURT", "STEEP", "PYTHON"
    };
    int default_count = sizeof(default_words) / sizeof(default_words[0]);

    /* Convert them to dynamic pointers list */
    char *word_ptrs[default_count];
    for (int i = 0; i < default_count; ++i) {
        word_ptrs[i] = default_words[i];
        to_upper_inplace(word_ptrs[i]);
    }

    clear_screen();
    show_title();

    printf("%s\n=== CROSSWORD GENERATOR & SOLVER ===%s\n", BOLD, RESET);
    printf("Using default word list (%d words)...\n", default_count);

    if (!puzzle_generate(p, word_ptrs, default_count)) {
        printf("Failed to generate puzzle.\n");
        puzzle_free(p);
        return 1;
    }

    printf("%sGenerated with %d placed words.%s\n", GREEN, p->word_count, RESET);

    char buf[256];

    for (;;) {
        printf("%s\n--- MENU ---\n%s", BOLD, RESET);
        printf("1. View puzzle (game view)\n");
        printf("2. View clues\n");
        printf("3. Input answer for a clue\n");
        printf("4. Hint (reveal one letter)\n");
        printf("5. Check progress\n");
        printf("6. Show solution (boxed)\n");
        printf("7. Show timer\n");
        printf("8. Quit\n");
        printf("Choice: ");

        safe_gets(buf, sizeof(buf));
        int opt = atoi(buf);

        if (opt == 1) {
            draw_grid(p, false);

        } else if (opt == 2) {
            show_clues(p);

        } else if (opt == 3) {
            show_clues(p);
            printf("Clue number: ");
            safe_gets(buf, sizeof(buf));
            int clue = atoi(buf);

            printf("Direction (A/D): ");
            safe_gets(buf, sizeof(buf));
            char d = (toupper((unsigned char)buf[0]) == 'A') ? 'A' : 'D';

            printf("Your answer: ");
            char ans[MAX_WORD_LENGTH];
            safe_gets(ans, sizeof(ans));
            to_upper_inplace(ans);

            input_answer(p, clue, d, ans);

            if (puzzle_solved(p)) {
                printf("%s\nPuzzle solved! Congratulations!\n%s", GREEN, RESET);
                draw_grid(p, true);
                show_timer(p);
                break;
            }

        } else if (opt == 4) {
            show_clues(p);
            printf("Clue number for hint: ");
            safe_gets(buf, sizeof(buf));
            int clue = atoi(buf);

            printf("Direction (A/D): ");
            safe_gets(buf, sizeof(buf));
            char d = (toupper((unsigned char)buf[0]) == 'A') ? 'A' : 'D';

            give_hint(p, clue, d);

            if (puzzle_solved(p)) {
                printf("%s\nPuzzle solved! Congratulations!\n%s", GREEN, RESET);
                draw_grid(p, true);
                show_timer(p);
                break;
            }

        } else if (opt == 5) {
            printf("%sCompletion: %.1f%%%s\n", BOLD, puzzle_completion(p), RESET);
            draw_grid(p, false);

        } else if (opt == 6) {
            draw_grid(p, true);

        } else if (opt == 7) {
            show_timer(p);

        } else if (opt == 8) {
            printf("%sGoodbye!\n%s", CYAN, RESET);
            break;

        } else {
            printf("%sInvalid option. Try again.%s\n", RED, RESET);
        }
    }

    puzzle_free(p);
    return 0;
}
