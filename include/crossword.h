#ifndef CROSSWORD_H
#define CROSSWORD_H

#include <stdbool.h>
#include <time.h>

#define MAX_WORD_LENGTH 24
#define GRID_SIZE 15

/* ANSI color macros (optional) */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

/* Owner bitmask flags */
#define OWNER_ACROSS 1
#define OWNER_DOWN   2

/* Word position metadata (one per placed word) */
typedef struct {
    char word[MAX_WORD_LENGTH];
    int row, col;
    char direction; /* 'A' or 'D' */
    int clue_num;
    bool hint_used;
} WordPos;

/* Linked list node for placed words */
typedef struct WordNode {
    WordPos data;
    struct WordNode *next;
} WordNode;

/* Move (for undo stack) */
typedef struct {
    int row;
    int col;
    char prev;  /* previous character in user grid */
    char now;   /* new character (for reference) */
} Move;

/* Simple stack for moves */
typedef struct MoveStackNode {
    Move mv;
    struct MoveStackNode *next;
} MoveStackNode;

typedef struct {
    MoveStackNode *top;
    int size;
} MoveStack;

/* BST node for storing dictionary words */
typedef struct BSTNode {
    char word[MAX_WORD_LENGTH];
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

/* Puzzle object */
typedef struct {
    char sol[GRID_SIZE][GRID_SIZE];     /* solution letters */
    char user[GRID_SIZE][GRID_SIZE];    /* user view */
    unsigned char owner[GRID_SIZE][GRID_SIZE]; /* ownership bits */
    WordNode *positions_head;           /* linked list head for WordPos */
    int word_count;
    int clue_counter;
    time_t start_time;

    /* data-structures: undo stack and BST dictionary root */
    MoveStack undo_stack;
    BSTNode *dict_root;
} Puzzle;

/* Platform helpers */
void clear_screen(void);

/* Utilities */
void safe_gets(char *buf, int size);
void to_upper_inplace(char *s);

/* Puzzle lifecycle */
Puzzle *puzzle_create(void);
void puzzle_free(Puzzle *p);
bool puzzle_init(Puzzle *p);
void puzzle_create_user_grid(Puzzle *p);

/* Dictionary BST */
BSTNode *bst_insert(BSTNode *root, const char *word);
void bst_free(BSTNode *root);
int bst_count(BSTNode *root);
void bst_inorder_collect(BSTNode *root, char **out, int *idx, int max);

/* Generation */
bool puzzle_generate_from_bst(Puzzle *p); /* new: uses BST as source */
bool puzzle_generate(Puzzle *p, char **words, int count);

/* Placement */
bool puzzle_can_place(Puzzle *p, const char *w, int r, int c, char d);
bool puzzle_place_word_record(Puzzle *p, const char *w, int r, int c, char d);
int puzzle_find_intersection(Puzzle *p, const char *w, int *out_r, int *out_c, char *out_d);

/* Rendering */
void draw_grid(const Puzzle *p, bool solution_view);
void show_clues(const Puzzle *p);

/* Interaction */
bool input_answer(Puzzle *p, int clue, char d, const char *ans);
bool give_hint(Puzzle *p, int clue, char d);

/* Undo stack */
void push_move(MoveStack *s, Move mv);
bool pop_move(MoveStack *s, Move *out);
void undo_last_move(Puzzle *p);

/* Progress / timer */
bool puzzle_solved(const Puzzle *p);
float puzzle_completion(const Puzzle *p);
void show_timer(const Puzzle *p);

/* Extra helpers for BST population (default words) */
void populate_default_dictionary(Puzzle *p);

#endif /* CROSSWORD_H */
