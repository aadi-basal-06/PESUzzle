# 🧩 PESUzzle – Crossword Generator & Solver  
*A Mini-Project for PES University (C Programming – Multi-File Project)*

PESUzzle is a complete **ASCII-based crossword generator and solver** built in C using a strict **multi-file architecture**.  
It generates a fully playable 15×15 crossword puzzle using a predefined word list, renders it in an ANSI-styled terminal UI, and supports solving, hints, timing, and interactive gameplay.

---

## 🚀 Features

### ✔ Crossword Generation
- Automatic placement of words using:
  - Intersection detection  
  - Priority placement by word length  
  - Boundary and adjacency validation  
- Ensures all placed words follow crossword constraints.

### ✔ ANSI Terminal UI (Fully ASCII – Works on Windows PowerShell & CMD)
- Clean **ASCII box grid** using:  

+=====+=====+=====+
| A | _ | C |
+=====+=====+=====+

- Colored letters:
- **Yellow** → Across  
- **Red** → Down  
- **Magenta** → Intersection  
- **Cyan** → Blanks / underscores  
- Automatic screen refresh (cls/clear) for smooth transitions.
- Intro ASCII art title displayed at startup.

### ✔ Gameplay
- View puzzle (game mode)
- View clues (Across/Down)
- Enter answers for each clue
- Reveal **hints** (one letter at a random position)
- Check live completion %
- Show full solution
- Timer showing gameplay duration

---

## 📁 Project Structure (Multi-File)

crossword/
│── include/
│ └── crossword.h # Function prototypes, macros, structs
│
│── src/
│ └── crossword.c # Core logic: generator, rendering, hints, timer
│
│── main.c # Main menu & user interaction
│── README.md # Project documentation
│── .gitignore # Optional
│── crossword.exe # Build output (not tracked)

---

## 🔧 Compilation & Execution

### **Compile**
```bash
gcc -Wall -Iinclude main.c src/crossword.c -o crossword
crossword.exe


🎮 How It Works

1. Program Startup

Screen clears
PESUzzle ASCII art is displayed
Puzzle is automatically generated using the default word list



2. Menu Options

1. View puzzle (game view)
2. View clues
3. Input answer for a clue
4. Hint (reveal one letter)
5. Check progress
6. Show solution (boxed)
7. Show timer
8. Quit

3. Crosswords

The grid uses + = and | for perfect alignment.
Letters are colored based on the owning direction.
Blanks appear as _ and fill when user answers.


🧠 Data Structures Used

--WordPos (for each word placed)--
typedef struct {
    char word[MAX_WORD_LENGTH];
    int row, col;
    char direction;
    int clue_num;
    bool hint_used;
} WordPos;

--Puzzle (entire crossword state)--
typedef struct {
    char sol[GRID_SIZE][GRID_SIZE];
    char user[GRID_SIZE][GRID_SIZE];
    unsigned char owner[GRID_SIZE][GRID_SIZE];
    WordPos *positions;
    int word_count;
    int clue_counter;
    time_t start_time;
} Puzzle;

Why these structures?

Efficient grid-based lookups
Easy to track positions, owners, and intersections
Dynamic array (positions via realloc) avoids fixed-limit assumptions

🔍 Memory & Error Handling

PESUzzle follows proper coding practices:

Dynamic memory allocation with calloc and realloc
No global variables
Boundary checks for grid placement
Return value validation
Memory cleanup via puzzle_free()
Cross-platform screen clearing

🏫 About

This project was created as part of the Mini-Project for the
Department of Computer Science & Engineering, PES University (PESU).

It demonstrates:

Modular C programming
Multi-file compilation
Data structures & algorithms
Interactive terminal UI
Puzzle generation logic

👥 Authors

Aadi Basal
Abhijna Marathe
Abhirup Roy