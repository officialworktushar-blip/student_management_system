/* main.c — menu loop and ALL user interaction (printf/scanf live here only).
 * Core logic in student.c/storage.c stays stdio-free for WebAssembly reuse.
 * Concepts: console I/O, fgets+sscanf/strtol input validation, formatted
 * table output, switch-based menu dispatch. */

#include <stdio.h>    /* printf, fgets, sscanf */
#include <stdlib.h>   /* strtol, strtof, malloc-free via student.c */
#include <string.h>   /* strlen, strchr */

#include "student.h"
#include "storage.h"

#define DATA_PATH "data/students.dat"
#define INPUT_BUF 128

static StudentList students;   /* global list; initialised in main() */

/* ---------- input helpers (robust against bad/non-numeric input) ---------- */

/* Read one line, strip trailing newline. Returns 1 if something was typed. */
static int read_line(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL)
        return 0;

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[--len] = '\0';          /* strip newline */
    if (len > 0 && buf[len - 1] == '\r')
        buf[--len] = '\0';          /* strip CR (Windows line endings) */

    /* flush any leftover characters if the line was longer than the buffer */
    if (len == size - 1 && buf[len - 1] != '\0') {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
    return 1;
}

/* Parse a whole line as a long; rejects junk and out-of-range values. */
static int read_long(const char *prompt, long min, long max, long *out)
{
    char buf[INPUT_BUF];
    for (;;) {
        if (!read_line(prompt, buf, sizeof buf))
            return 0;               /* EOF */

        char *end = NULL;
        long val = strtol(buf, &end, 10);   /* strtol: string -> long */
        /* valid only if the whole trimmed line was consumed and in range */
        while (end && (*end == ' ' || *end == '\t')) end++;
        if (end != buf && *end == '\0' && val >= min && val <= max) {
            *out = val;
            return 1;
        }
        printf("  Invalid input. Enter a whole number between %ld and %ld.\n",
               min, max);
    }
}

static int read_float(const char *prompt, float min, float max, float *out)
{
    char buf[INPUT_BUF];
    for (;;) {
        if (!read_line(prompt, buf, sizeof buf))
            return 0;

        char *end = NULL;
        float val = strtof(buf, &end);      /* strtof: string -> float */
        while (end && (*end == ' ' || *end == '\t')) end++;
        if (end != buf && *end == '\0' && val >= min && val <= max) {
            *out = val;
            return 1;
        }
        printf("  Invalid input. Enter a number between %.0f and %.0f.\n",
               (double)min, (double)max);
    }
}

/* Read a non-empty string into buf (bounded, NUL-terminated). */
static int read_name(const char *prompt, char *buf, size_t size)
{
    for (;;) {
        if (!read_line(prompt, buf, size))
            return 0;
        /* trim leading/trailing spaces */
        char *start = buf;
        while (*start == ' ' || *start == '\t') start++;
        if (*start == '\0') {
            printf("  Name cannot be empty.\n");
            continue;
        }
        if (start != buf)
            memmove(buf, start, strlen(start) + 1);
        return 1;
    }
}

/* Read an optional string: empty input keeps the current value.
 * Returns 1 if buf was updated, 0 on EOF. */
static int read_optional(const char *prompt, char *buf, size_t size)
{
    char tmp[INPUT_BUF];
    if (!read_line(prompt, tmp, sizeof tmp))
        return 0;
    /* trim */
    char *start = tmp;
    while (*start == ' ' || *start == '\t') start++;
    size_t len = strlen(start);
    while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\t'))
        start[--len] = '\0';
    if (len > 0) {
        size_t copy = len < size - 1 ? len : size - 1;  /* bounded copy */
        memcpy(buf, start, copy);
        buf[copy] = '\0';
    }
    return 1;
}

/* ---------- display helpers ---------- */

/* Map internal grade code to its display string. */
static const char *grade_str(char grade)
{
    switch (grade) {
    case '+': return "A+";
    case 'A': return "A";
    case 'B': return "B";
    case 'C': return "C";
    case 'D': return "D";
    default:  return "F";
    }
}

static void print_header(void)
{
    printf("\n");
    printf("================================================================\n");
    printf("%-6s %-20s %-4s %-15s", "Roll", "Name", "Age", "Branch");
    for (int i = 1; i <= SUBJECT_COUNT; i++)
        printf(" %4s%d", "M", i);   /* column headers M1..M5 */
    printf(" %7s %6s %5s\n", "Total", "Pct%", "Grade");
    printf("----------------------------------------------------------------\n");
}

static void print_student(const Student *s)
{
    printf("%-6d %-20.20s %-4d %-15.15s", s->roll_no, s->name, s->age,
           s->branch);
    for (int i = 0; i < SUBJECT_COUNT; i++)
        printf(" %5.0f", (double)s->marks[i]);
    printf(" %7.1f %5.1f%% %5s\n", (double)s->total, (double)s->percentage,
           grade_str(s->grade));
}

static void display_all(void)
{
    if (students.count == 0) {
        printf("\nNo students found.\n");
        return;
    }
    print_header();              /* formatted table header */
    for (int i = 0; i < students.count; i++)  /* loop over array */
        print_student(&students.items[i]);    /* pass pointer to struct */
    printf("----------------------------------------------------------------\n");
    printf("Total: %d student(s)\n", students.count);
}

/* ---------- menu actions ---------- */

static void menu_add(void)
{
    Student s;
    memset(&s, 0, sizeof s);    /* zero the whole struct first */

    long roll;
    if (!read_long("Enter roll number (>0): ", 1, 1000000, &roll))
        return;
    s.roll_no = (int)roll;

    if (!read_name("Enter name: ", s.name, sizeof s.name))
        return;

    long age;
    if (!read_long("Enter age (5-100): ", 5, 100, &age))
        return;
    s.age = (int)age;

    /* branch is optional-checked: reuse read_name but it must be non-empty;
     * spec requires rejecting empty name only, branch may be any text */
    char branch[INPUT_BUF];
    for (;;) {
        if (!read_line("Enter branch: ", branch, sizeof branch))
            return;
        char *start = branch;
        while (*start == ' ' || *start == '\t') start++;
        if (*start != '\0') {
            strncpy(s.branch, start, sizeof s.branch - 1);
            s.branch[sizeof s.branch - 1] = '\0';
            break;
        }
        printf("  Branch cannot be empty.\n");
    }

    for (int i = 0; i < SUBJECT_COUNT; i++) {
        char prompt[64];
        snprintf(prompt, sizeof prompt, "Enter marks for subject %d (0-100): ",
                 i + 1);
        if (!read_float(prompt, 0.0f, 100.0f, &s.marks[i]))
            return;
    }

    int rc = student_list_add(&students, &s);   /* core logic, status code */
    switch (rc) {
    case STUDENT_OK:
        printf("\nStudent added. Saving...\n");
        if (storage_save(DATA_PATH, &students) != STUDENT_OK)
            printf("Warning: could not save to %s\n", DATA_PATH);
        break;
    case STUDENT_ERR_DUPLICATE:
        printf("\nError: roll number %d already exists.\n", s.roll_no);
        break;
    case STUDENT_ERR_ALLOC:
        printf("\nError: out of memory.\n");
        break;
    default:
        printf("\nError: could not add student.\n");
        break;
    }
}

static void menu_search(void)
{
    long roll;
    if (!read_long("Enter roll number to search: ", 1, 1000000, &roll))
        return;

    Student *s = student_find(&students, (int)roll);   /* pointer into array */
    if (s == NULL) {
        printf("\nNo student with roll number %ld.\n", roll);
        return;
    }
    printf("\n");
    print_header();
    print_student(s);
    printf("----------------------------------------------------------------\n");
}

static void menu_update(void)
{
    long roll;
    if (!read_long("Enter roll number to update: ", 1, 1000000, &roll))
        return;

    Student *s = student_find(&students, (int)roll);
    if (s == NULL) {
        printf("\nNo student with roll number %ld.\n", roll);
        return;
    }

    printf("\nCurrent record:\n");
    print_header();
    print_student(s);
    printf("----------------------------------------------------------------\n");
    printf("(Press Enter to keep the current value)\n");

    Student upd;
    memset(&upd, 0, sizeof upd);
    upd.roll_no = s->roll_no;

    /* name */
    char tmp[INPUT_BUF];
    snprintf(tmp, sizeof tmp, "New name [%s]: ", s->name);
    if (!read_optional(tmp, upd.name, sizeof upd.name))
        return;
    if (upd.name[0] == '\0')
        strcpy(upd.name, s->name);          /* keep old name */

    /* age: blank keeps current, otherwise must be 5-100 */
    char prompt[64];
    snprintf(prompt, sizeof prompt, "New age [%d] (5-100, blank=keep): ", s->age);
    char line[INPUT_BUF];
    for (;;) {
        if (!read_line(prompt, line, sizeof line))
            return;
        if (line[0] == '\0') { upd.age = s->age; break; }
        char *end = NULL;
        long v = strtol(line, &end, 10);
        while (end && (*end == ' ' || *end == '\t')) end++;
        if (end != line && *end == '\0' && v >= 5 && v <= 100) {
            upd.age = (int)v;
            break;
        }
        printf("  Invalid age.\n");
    }

    /* branch */
    snprintf(tmp, sizeof tmp, "New branch [%s]: ", s->branch);
    if (!read_optional(tmp, upd.branch, sizeof upd.branch))
        return;
    if (upd.branch[0] == '\0')
        strcpy(upd.branch, s->branch);

    /* marks */
    for (int i = 0; i < SUBJECT_COUNT; i++) {
        snprintf(prompt, sizeof prompt, "New mark %d [%.0f] (0-100, blank=keep): ",
                 i + 1, (double)s->marks[i]);
        for (;;) {
            if (!read_line(prompt, line, sizeof line))
                return;
            if (line[0] == '\0') { upd.marks[i] = s->marks[i]; break; }
            char *end = NULL;
            float v = strtof(line, &end);
            while (end && (*end == ' ' || *end == '\t')) end++;
            if (end != line && *end == '\0' && v >= 0.0f && v <= 100.0f) {
                upd.marks[i] = v;
                break;
            }
            printf("  Invalid mark.\n");
        }
    }

    int rc = student_list_update(&students, (int)roll, &upd);
    if (rc == STUDENT_OK) {
        printf("\nStudent updated. Saving...\n");
        if (storage_save(DATA_PATH, &students) != STUDENT_OK)
            printf("Warning: could not save to %s\n", DATA_PATH);
    } else {
        printf("\nError: update failed.\n");
    }
}

static void menu_delete(void)
{
    long roll;
    if (!read_long("Enter roll number to delete: ", 1, 1000000, &roll))
        return;

    int rc = student_list_delete(&students, (int)roll);
    if (rc == STUDENT_OK) {
        printf("\nStudent %ld deleted. Saving...\n", roll);
        if (storage_save(DATA_PATH, &students) != STUDENT_OK)
            printf("Warning: could not save to %s\n", DATA_PATH);
    } else {
        printf("\nNo student with roll number %ld.\n", roll);
    }
}

static void menu_result(void)
{
    /* recompute total/percentage/grade for every record */
    for (int i = 0; i < students.count; i++)
        student_calculate_result(&students.items[i]);

    printf("\nResults recalculated for %d student(s).\n", students.count);
    storage_save(DATA_PATH, &students);
}

static void menu_sort(void)
{
    printf("\nSort by:\n");
    printf("  1. Marks (highest first)\n");
    printf("  2. Roll number (ascending)\n");
    printf("  0. Cancel\n");

    long choice;
    if (!read_long("Choice: ", 0, 2, &choice))
        return;

    if (choice == 1)
        student_sort_by_marks_desc(&students);   /* qsort descending marks */
    else if (choice == 2)
        student_sort_by_roll_asc(&students);     /* qsort ascending roll */
    else
        return;

    printf("\nSorted. Displaying:\n");
    display_all();
}

static void print_menu(void)
{
    printf("\n===============================================================\n");
    printf("           STUDENT MANAGEMENT SYSTEM\n");
    printf("===============================================================\n");
    printf("  1. Add Student\n");
    printf("  2. Display All Students\n");
    printf("  3. Search Student (by roll number)\n");
    printf("  4. Update Student (name, age, branch, marks)\n");
    printf("  5. Delete Student\n");
    printf("  6. Calculate Result (recompute totals)\n");
    printf("  7. Sort Students\n");
    printf("  8. Exit\n");
    printf("===============================================================\n");
}

int main(void)
{
    /* heap-allocate the dynamic array */
    if (student_list_init(&students) != STUDENT_OK) {
        printf("Fatal: could not allocate memory.\n");
        return 1;
    }

    /* load persisted records (missing file -> empty list, no error) */
    if (storage_load(DATA_PATH, &students) != STUDENT_OK)
        printf("Warning: could not load %s (starting empty)\n", DATA_PATH);

    int running = 1;
    while (running) {                       /* menu loop until Exit */
        print_menu();

        long choice;
        if (!read_long("Enter your choice: ", 0, 8, &choice))
            break;                      /* EOF on stdin -> exit cleanly */

        switch ((int)choice) {              /* switch dispatch on menu item */
        case 1: menu_add();     break;
        case 2: display_all();  break;
        case 3: menu_search();  break;
        case 4: menu_update();  break;
        case 5: menu_delete();  break;
        case 6: menu_result();  break;
        case 7: menu_sort();    break;
        case 8:
            running = 0;
            break;
        default:
            printf("\nInvalid choice.\n");
            break;
        }
    }

    /* persist one final time, then free the heap array (no leaks) */
    storage_save(DATA_PATH, &students);
    student_list_free(&students);

    printf("\nGoodbye! Data saved to %s\n", DATA_PATH);
    return 0;
}
