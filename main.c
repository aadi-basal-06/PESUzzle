#include "include/crossword.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

void show_title(void) {
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
    srand((unsigned)time(NULL));
    Puzzle *p = puzzle_create();
    if (!p) { fprintf(stderr, "Failed to allocate puzzle\n"); return 1; }

    clear_screen();
    show_title();

    /* Generate puzzle from BST dictionary (uses BST -> array -> generator) */
    if (!puzzle_generate_from_bst(p)) {
        fprintf(stderr, "Failed to generate puzzle\n");
        puzzle_free(p);
        return 1;
    }

    printf("%sGenerated with %d placed words.%s\n", GREEN, p->word_count, RESET);

    char buf[256];
    for (;;) {
        clear_screen();
        show_title();
        printf("%s\n--- MENU ---\n%s", BOLD, RESET);
        printf("1. View puzzle (game view)\n");
        printf("2. View clues\n");
        printf("3. Input answer for a clue\n");
        printf("4. Hint (reveal one letter)\n");
        printf("5. Undo last move\n");
        printf("6. Check progress\n");
        printf("7. Show solution (boxed)\n");
        printf("8. Show timer\n");
        printf("9. Quit\n");
        printf("Choice: ");

        safe_gets(buf, sizeof(buf));
        int opt = atoi(buf);

        if (opt == 1) {
            draw_grid(p, false);
            printf("Press ENTER to return...");
            getchar();
        } else if (opt == 2) {
            show_clues(p);
            printf("Press ENTER to return...");
            getchar();
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
            printf("Press ENTER to continue...");
            getchar();
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
            printf("Press ENTER to continue...");
            getchar();
        } else if (opt == 5) {
            undo_last_move(p);
            printf("Press ENTER to continue...");
            getchar();
        } else if (opt == 6) {
            printf("%sCompletion: %.1f%%%s\n", BOLD, puzzle_completion(p), RESET);
            draw_grid(p, false);
            printf("Press ENTER to continue...");
            getchar();
        } else if (opt == 7) {
            draw_grid(p, true);
            printf("Press ENTER to continue...");
            getchar();
        } else if (opt == 8) {
            show_timer(p);
        } else if (opt == 9) {
            printf("%sGoodbye!\n%s", CYAN, RESET);
            break;
        } else {
            printf("%sInvalid option. Try again.%s\n", RED, RESET);
            printf("Press ENTER to continue...");
            getchar();
        }
    }

    puzzle_free(p);
    return 0;
}
