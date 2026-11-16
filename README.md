# Student Management System

A terminal-based Student Management System written in **C11**. It manages
student records — roll number, name, age, branch and marks in 5 subjects —
with automatic total, percentage and grade calculation, binary file
persistence, and a clean interactive menu.

The core logic (`student.c`, `storage.c`) contains **no `printf`/`scanf`** —
it only takes parameters and returns status codes. All user interaction
lives in `main.c`, which keeps the logic modules cleanly separable for a
future **WebAssembly** build.

## Features

- Add / display / search / update / delete students
- Marks in 5 subjects; **total, percentage and grade computed automatically**
- Grades: `A+` (>=90), `A` (>=80), `B` (>=70), `C` (>=60), `D` (>=50), `F`
- Duplicate roll-number rejection
- Input validation: roll > 0, age 5–100, marks 0–100, non-empty name/branch;
  invalid input reprompts instead of looping forever
- Sort by marks (descending) or roll number (ascending)
- Formatted table display
- Binary persistence to `data/students.dat` — loaded at startup, saved after
  every add / update / delete; missing file handled gracefully
- Dynamically allocated record array (`malloc`/`realloc`), freed on exit
- Assert-based unit tests (`make test`) and a seed script (`make seed`)

## C concepts used

| Concept | Where |
|---|---|
| Structs | `Student`, `StudentList` in `student.h` |
| Fixed-size string buffers | `char name[50]`, `char branch[30]` |
| Arrays | `float marks[5]`, dynamic `Student *items[]` |
| Pointers | `Student *student_find()`, struct assignment/copy |
| Dynamic memory | `malloc` / `realloc` / `free` in `student.c` |
| Memory move | `memmove` on delete, `memcpy`/`strncpy` for strings |
| Loops | `for`/`while` for search, input, display, totals |
| Functions & prototypes | split across `student.c`, `storage.c`, `main.c` |
| Function pointers / callbacks | `qsort` comparators for sorting |
| File handling | `fopen`/`fread`/`fwrite`/`fclose` in `storage.c` |
| String parsing | `strtol`, `strtof`, `fgets`, `strlen`, `snprintf` |
| Preprocessor | header guards, `#define` constants |
| Enum / status codes | `STUDENT_OK`, `STUDENT_ERR_*` returns |
| `switch` dispatch | menu loop in `main.c` |
| Unit testing | `<assert.h>` based tests in `tests/` |

## Project structure

```
student_management_system/
├── Makefile               # all, run, test, seed, clean
├── README.md
├── .gitignore             # binaries, *.o, data/students.dat
├── .vscode/
│   ├── tasks.json         # build / build-tests / test tasks
│   └── launch.json        # F5 debug configuration (gdb)
├── scripts/
│   └── seed_input.txt     # scripted input for 10 sample students
├── src/
│   ├── main.c             # menu loop + all printf/scanf (user I/O only)
│   ├── student.h          # struct definitions + prototypes
│   ├── student.c          # core logic: add/search/update/delete/result/sort
│   ├── storage.h          # storage prototypes
│   └── storage.c          # binary file I/O (fread/fwrite)
├── tests/
│   └── test_student.c     # assert-based unit tests
└── data/
    ├── .gitkeep
    └── students.dat       # created at runtime (gitignored)
```

## Build & run

Requires a C11 compiler (`gcc`). `make` is optional but recommended.

### Windows (MinGW / MSYS2)

```bat
# one-time: install MSYS2, then in the "MSYS2 MinGW 64-bit" shell:
pacman -S mingw-w64-ucrt-x86_64-gcc make

# build & run
make
make run

# or without make:
gcc -Wall -Wextra -std=c11 -O2 -o sms.exe src/main.c src/student.c src/storage.c
sms.exe
```

### Linux

```bash
sudo apt install build-essential   # gcc + make
git clone <repo-url> && cd student_management_system
make
make run
```

### macOS

```bash
xcode-select --install            # provides clang (gcc-compatible)
git clone <repo-url> && cd student_management_system
make        # uses cc/gcc; clang works identically
make run
```

> On macOS the binary is named `sms` (no `.exe`); the Makefile detects the
> platform automatically.

## Usage

```
===============================================================
           STUDENT MANAGEMENT SYSTEM
===============================================================
  1. Add Student
  2. Display All Students
  3. Search Student (by roll number)
  4. Update Student (name, age, branch, marks)
  5. Delete Student
  6. Calculate Result (recompute totals)
  7. Sort Students
  8. Exit
===============================================================
Enter your choice:
```

### Sample session

Adding a student and displaying the table:

```
Enter your choice: 1
Enter roll number (>0): 101
Enter name: Alice
Enter age (5-100): 20
Enter branch: CSE
Enter marks for subject 1 (0-100): 90
Enter marks for subject 2 (0-100): 85
Enter marks for subject 3 (0-100): 80
Enter marks for subject 4 (0-100): 75
Enter marks for subject 5 (0-100): 95
Student added. Saving...
```

```
================================================================
Roll   Name                 Age  Branch             M1    M2    M3    M4    M5   Total   Pct% Grade
----------------------------------------------------------------
101    Alice                20   CSE                90    85    80    75    95   425.0  85.0%     A
103    Carol                21   ME                 95    95    95    95    95   475.0  95.0%    A+
102    Bobby                19   ECE                88    77    66    55    44   330.0  66.0%     C
----------------------------------------------------------------
Total: 3 student(s)
```

Sorting by marks (highest first):

```
Sort by:
  1. Marks (highest first)
  2. Roll number (ascending)
  0. Cancel
Choice: 1
Sorted. Displaying:

================================================================
Roll   Name                 Age  Branch             M1    M2    M3    M4    M5   Total   Pct% Grade
----------------------------------------------------------------
103    Carol                21   ME                 95    95    95    95    95   475.0  95.0%    A+
101    Alice                20   CSE                90    85    80    75    95   425.0  85.0%     A
102    Bobby                19   ECE                88    77    66    55    44   330.0  66.0%     C
----------------------------------------------------------------
Total: 3 student(s)
```

Searching and updating (blank input keeps the current value):

```
Enter your choice: 3
Enter roll number to search: 102

================================================================
Roll   Name                 Age  Branch             M1    M2    M3    M4    M5   Total   Pct% Grade
----------------------------------------------------------------
102    Bobby                19   ECE                88    77    66    55    44   330.0  66.0%     C
----------------------------------------------------------------

Enter your choice: 4
Enter roll number to update: 102
Current record:
... (record shown) ...
(Press Enter to keep the current value)
New name [Bobby]: Robert
New age [19] (5-100, blank=keep):
New branch [ECE]:
New mark 1 [88] (0-100, blank=keep): 95
... (remaining marks) ...
Student updated. Saving...
```

Invalid input is rejected and reprompted — no infinite loops:

```
Enter your choice: abc
  Invalid input. Enter a whole number between 0 and 8.
Enter your choice: 1
Enter roll number (>0): -5
  Invalid input. Enter a whole number between 1 and 1000000.
Enter roll number (>0): 101
Enter name: Alice
Enter age (5-100): 20
Enter branch: CSE
Enter marks for subject 1 (0-100): 150
  Invalid input. Enter a number between 0 and 100.
Enter marks for subject 1 (0-100):
```

## Tests

```bash
make test        # builds and runs tests/test_student.c
```

Covers: grade/percentage calculation (all bands + boundaries), add,
duplicate roll rejection, search, update (with result recompute),
delete, sorting both ways, dynamic-array growth past initial capacity,
and a storage save/load round-trip including the missing-file case.

## Seeding sample data

```bash
make seed        # feeds scripts/seed_input.txt to the program
```

Adds 10 sample students (rolls 1–10). Re-running is safe — duplicate roll
numbers are rejected automatically.

## VS Code

`.vscode/tasks.json` and `.vscode/launch.json` are included:

- **Build:** `Ctrl+Shift+B` → `build` task (`gcc -Wall -Wextra -std=c11 -g`)
- **Debug/Run:** `F5` → builds then launches `sms.exe` under `gdb`
- **Tests:** run the `test` task to build and run `test_runner.exe`

Requirements: the
[Microsoft C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
extension and `gdb` on `PATH` (included with MinGW/MSYS2 on Windows;
`xcode-select --install` on macOS; `sudo apt install gdb` on Linux).
`launch.json` points at `C:/msys64/ucrt64/bin/gdb.exe` — adjust
`miDebuggerPath` if your gdb lives elsewhere.

## Design notes

- **Logic/UI separation:** `student.c` and `storage.c` never call
  `printf`/`scanf`; they return status codes (`STUDENT_OK`,
  `STUDENT_ERR_DUPLICATE`, `STUDENT_ERR_NOT_FOUND`, …). This makes the
  modules directly reusable from a WebAssembly build where console I/O
  works differently.
- **Dynamic array:** records live in a heap array that doubles via
  `realloc` when full; `student_list_free()` releases it on exit.
- **File format:** `int32 count` header followed by `count` raw
  `Student` structs, written with a single `fwrite`.

## Future improvements

- Search by name / branch, not just roll number
- Subject-wise grade reports and class toppers
- CSV/JSON import & export
- Password-protected or per-user data files
- WebAssembly + browser UI front-end over the existing logic modules
- More granular update (change roll number, individual subject names)
