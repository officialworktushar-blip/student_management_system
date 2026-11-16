#ifndef STUDENT_H
#define STUDENT_H

/* ---- compile-time constants (preprocessor macros) ---- */
#define SUBJECT_COUNT 5          /* marks[] array size */
#define MAX_NAME_LEN 50          /* char name[50]  -> fixed-size string buffer */
#define MAX_BRANCH_LEN 30        /* char branch[30] -> fixed-size string buffer */
#define INITIAL_CAPACITY 16      /* starting heap capacity of the dynamic array */

/* ---- struct: a single student record ---- */
typedef struct {
    int roll_no;
    char name[MAX_NAME_LEN];       /* C string: fixed-size char array */
    int age;
    char branch[MAX_BRANCH_LEN];   /* C string */
    float marks[SUBJECT_COUNT];    /* array of 5 subject marks */
    float total;                   /* computed: sum of marks */
    float percentage;              /* computed: total / SUBJECT_COUNT */
    char grade;                    /* computed: 'A+', 'A', 'B', 'C', 'D', 'F' */
} Student;

/* ---- struct: dynamic array (heap-allocated via malloc/realloc) ----
 * Wraps a Student* pointer plus count/capacity so core logic never
 * touches stdio — this keeps the module portable to WebAssembly. */
typedef struct {
    Student *items;   /* pointer to heap-allocated array of Student structs */
    int count;        /* number of valid records currently stored */
    int capacity;     /* allocated capacity of items[] */
} StudentList;

/* ---- status codes returned by logic functions (no printf/scanf here) ---- */
enum {
    STUDENT_OK = 0,
    STUDENT_ERR_ALLOC = -1,      /* malloc/realloc failed */
    STUDENT_ERR_DUPLICATE = -2,  /* roll number already exists */
    STUDENT_ERR_NOT_FOUND = -3,  /* roll number not in list */
    STUDENT_ERR_INVALID = -4     /* failed validation */
};

/* ---- core logic prototypes (implemented in student.c) ---- */

/* Compute total, percentage and grade for one record (pointer to struct). */
void student_calculate_result(Student *s);

/* Allocate the backing array. Call once before using a StudentList. */
int student_list_init(StudentList *list);

/* Free the heap array. Call on exit to avoid memory leaks. */
void student_list_free(StudentList *list);

/* Append a record; rejects duplicate roll numbers; auto-computes result. */
int student_list_add(StudentList *list, const Student *s);

/* Linear search by roll number; returns pointer to record or NULL. */
Student *student_find(StudentList *list, int roll_no);

/* Overwrite fields of an existing record; auto-computes result. */
int student_list_update(StudentList *list, int roll_no, const Student *new_data);

/* Remove a record, shifting the rest down (memmove). */
int student_list_delete(StudentList *list, int roll_no);

/* Sort in place with qsort(): by percentage desc, then roll asc as tiebreak. */
void student_sort_by_marks_desc(StudentList *list);

/* Sort in place with qsort(): by roll number ascending. */
void student_sort_by_roll_asc(StudentList *list);

#endif /* STUDENT_H */
