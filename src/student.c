/* student.c — core logic only: no printf/scanf, no file I/O.
 * Concepts used: structs, pointers, dynamic arrays (malloc/realloc/free),
 * loops, string.h (memcpy/memmove/strlen), qsort comparators, status codes. */

#include "student.h"

#include <stdlib.h>   /* malloc, realloc, free, qsort */
#include <string.h>   /* memcpy, memmove, strlen */

/* Internal grade codes stored in Student.grade (single char field):
 *   '+' -> A+   'A' -> A   'B' -> B   'C' -> C   'D' -> D   'F' -> F
 * main.c maps these codes to display strings. */

/* ---- internal helpers ---- */

/* Grow the backing array when full (realloc = resize heap allocation). */
static int ensure_capacity(StudentList *list)
{
    if (list->count < list->capacity)
        return STUDENT_OK;

    int new_cap = list->capacity * 2;   /* double capacity (amortized growth) */
    Student *new_items = realloc(list->items,
                                 (size_t)new_cap * sizeof(Student));
    if (new_items == NULL)
        return STUDENT_ERR_ALLOC;

    list->items = new_items;
    list->capacity = new_cap;
    return STUDENT_OK;
}

/* qsort comparators: plain functions, no captured state allowed. */
static int cmp_marks_desc(const void *a, const void *b)
{
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;

    /* sort by percentage descending (highest marks first) */
    if (sa->percentage > sb->percentage) return -1;
    if (sa->percentage < sb->percentage) return  1;
    return sa->roll_no - sb->roll_no;   /* tie-break: roll ascending */
}

static int cmp_roll_asc(const void *a, const void *b)
{
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return sa->roll_no - sb->roll_no;
}

/* ---- public API ---- */

void student_calculate_result(Student *s)
{
    float sum = 0.0f;
    /* loop over the marks[] array to total the 5 subjects */
    for (int i = 0; i < SUBJECT_COUNT; i++)
        sum += s->marks[i];

    s->total = sum;
    s->percentage = sum / (float)SUBJECT_COUNT;

    /* grade thresholds: >=90 A+, >=80 A, >=70 B, >=60 C, >=50 D, else F */
    if      (s->percentage >= 90.0f) s->grade = '+';
    else if (s->percentage >= 80.0f) s->grade = 'A';
    else if (s->percentage >= 70.0f) s->grade = 'B';
    else if (s->percentage >= 60.0f) s->grade = 'C';
    else if (s->percentage >= 50.0f) s->grade = 'D';
    else                             s->grade = 'F';
}

int student_list_init(StudentList *list)
{
    /* malloc: allocate the initial backing array on the heap */
    list->items = malloc((size_t)INITIAL_CAPACITY * sizeof(Student));
    if (list->items == NULL)
        return STUDENT_ERR_ALLOC;

    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    return STUDENT_OK;
}

void student_list_free(StudentList *list)
{
    /* free: release the heap array to avoid memory leaks */
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

int student_list_add(StudentList *list, const Student *s)
{
    /* duplicate check: linear search through existing records */
    if (student_find(list, s->roll_no) != NULL)
        return STUDENT_ERR_DUPLICATE;

    int rc = ensure_capacity(list);
    if (rc != STUDENT_OK)
        return rc;

    /* copy the struct into the dynamic array (struct assignment) */
    list->items[list->count] = *s;
    student_calculate_result(&list->items[list->count]);  /* auto-compute */
    list->count++;
    return STUDENT_OK;
}

Student *student_find(StudentList *list, int roll_no)
{
    /* linear search loop; returns a pointer into the array */
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].roll_no == roll_no)
            return &list->items[i];
    }
    return NULL;
}

int student_list_update(StudentList *list, int roll_no, const Student *new_data)
{
    Student *s = student_find(list, roll_no);
    if (s == NULL)
        return STUDENT_ERR_NOT_FOUND;

    /* copy editable fields; roll_no stays the key */
    s->age = new_data->age;
    /* strings: bounded copy so name/branch never overflow their buffers */
    strncpy(s->name, new_data->name, MAX_NAME_LEN - 1);
    s->name[MAX_NAME_LEN - 1] = '\0';
    strncpy(s->branch, new_data->branch, MAX_BRANCH_LEN - 1);
    s->branch[MAX_BRANCH_LEN - 1] = '\0';

    /* marks[] array copy, then recompute total/percentage/grade */
    memcpy(s->marks, new_data->marks, sizeof(s->marks));
    student_calculate_result(s);
    return STUDENT_OK;
}

int student_list_delete(StudentList *list, int roll_no)
{
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].roll_no == roll_no) {
            /* memmove: shift every record after i one slot left */
            memmove(&list->items[i], &list->items[i + 1],
                    (size_t)(list->count - i - 1) * sizeof(Student));
            list->count--;
            return STUDENT_OK;
        }
    }
    return STUDENT_ERR_NOT_FOUND;
}

void student_sort_by_marks_desc(StudentList *list)
{
    /* qsort: generic sorting via function pointer + comparators */
    qsort(list->items, (size_t)list->count, sizeof(Student), cmp_marks_desc);
}

void student_sort_by_roll_asc(StudentList *list)
{
    qsort(list->items, (size_t)list->count, sizeof(Student), cmp_roll_asc);
}
