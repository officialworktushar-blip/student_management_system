/* test_student.c — assert-based unit tests for student.c / storage.c.
 * No external framework: standard <assert.h> aborts on first failure.
 * Build & run via:  make test
 * Concepts: unit testing, assertions, test fixtures, setup/teardown. */

#include <assert.h>   /* assert(): aborts with file/line on failure */
#include <stdio.h>    /* remove(): cleanup of the temp storage file */
#include <string.h>   /* strcmp, memset */

#include "../src/student.h"
#include "../src/storage.h"

/* ---------- helpers ---------- */

/* Build a Student with sensible defaults; caller overrides fields. */
static Student make_student(int roll, const char *name, int age,
                            const char *branch, const float marks[SUBJECT_COUNT])
{
    Student s;
    memset(&s, 0, sizeof s);
    s.roll_no = roll;
    strncpy(s.name, name, MAX_NAME_LEN - 1);
    s.age = age;
    strncpy(s.branch, branch, MAX_BRANCH_LEN - 1);
    memcpy(s.marks, marks, sizeof s.marks);
    return s;
}

static int approx(float a, float b)
{
    float d = a - b;
    return d < 0 ? -d : d;
}

/* ---------- tests ---------- */

/* student_calculate_result: total, percentage and grade thresholds. */
static void test_calculate_result(void)
{
    Student s;
    memset(&s, 0, sizeof s);

    /* A+ boundary: 90% -> '+' */
    float m1[SUBJECT_COUNT] = {90, 90, 90, 90, 90};
    s = make_student(1, "Top", 20, "CSE", m1);
    student_calculate_result(&s);
    assert(approx(s.total, 450.0f) < 0.001f);
    assert(approx(s.percentage, 90.0f) < 0.001f);
    assert(s.grade == '+');

    /* just below A+ -> 'A' (80-89) */
    float m2[SUBJECT_COUNT] = {80, 80, 80, 80, 80};
    s = make_student(2, "A", 20, "CSE", m2);
    student_calculate_result(&s);
    assert(s.grade == 'A');
    assert(approx(s.percentage, 80.0f) < 0.001f);

    /* B band: 70-79 */
    float m3[SUBJECT_COUNT] = {70, 70, 70, 70, 70};
    s = make_student(3, "B", 20, "CSE", m3);
    student_calculate_result(&s);
    assert(s.grade == 'B');

    /* C band: 60-69 */
    float m4[SUBJECT_COUNT] = {60, 60, 60, 60, 60};
    s = make_student(4, "C", 20, "CSE", m4);
    student_calculate_result(&s);
    assert(s.grade == 'C');

    /* D band: 50-59 */
    float m5[SUBJECT_COUNT] = {50, 50, 50, 50, 50};
    s = make_student(5, "D", 20, "CSE", m5);
    student_calculate_result(&s);
    assert(s.grade == 'D');

    /* F band: below 50; also checks total/percentage math */
    float m6[SUBJECT_COUNT] = {40, 30, 20, 10, 0};
    s = make_student(6, "F", 20, "CSE", m6);
    student_calculate_result(&s);
    assert(s.grade == 'F');
    assert(approx(s.total, 100.0f) < 0.001f);
    assert(approx(s.percentage, 20.0f) < 0.001f);

    printf("PASS test_calculate_result\n");
}

/* add / duplicate rejection / search / update / delete / sort. */
static void test_list_operations(void)
{
    StudentList list;
    assert(student_list_init(&list) == STUDENT_OK);
    assert(list.count == 0);

    float mA[SUBJECT_COUNT] = {90, 85, 80, 75, 95};   /* 85% -> A */
    float mB[SUBJECT_COUNT] = {40, 50, 60, 70, 80};   /* 60% -> C */
    float mC[SUBJECT_COUNT] = {95, 95, 95, 95, 95};   /* 95% -> A+ */

    Student a = make_student(101, "Alice", 20, "CSE", mA);
    Student b = make_student(102, "Bob",   19, "ECE", mB);
    Student c = make_student(103, "Carol", 21, "ME",  mC);

    /* add three records */
    assert(student_list_add(&list, &a) == STUDENT_OK);
    assert(student_list_add(&list, &b) == STUDENT_OK);
    assert(student_list_add(&list, &c) == STUDENT_OK);
    assert(list.count == 3);

    /* duplicate roll number must be rejected */
    Student dup = make_student(101, "Alice2", 20, "CSE", mA);
    assert(student_list_add(&list, &dup) == STUDENT_ERR_DUPLICATE);
    assert(list.count == 3);

    /* add auto-computes results */
    Student *found = student_find(&list, 103);
    assert(found != NULL);
    assert(strcmp(found->name, "Carol") == 0);
    assert(approx(found->total, 475.0f) < 0.001f);
    assert(approx(found->percentage, 95.0f) < 0.001f);
    assert(found->grade == '+');

    /* search for a missing roll returns NULL */
    assert(student_find(&list, 999) == NULL);

    /* update: change name, age, branch and marks; result recomputed */
    float mNew[SUBJECT_COUNT] = {88, 77, 66, 55, 44}; /* 330 -> 66% -> C */
    Student upd = make_student(102, "Bobby", 20, "ECE-Up", mNew);
    assert(student_list_update(&list, 102, &upd) == STUDENT_OK);

    found = student_find(&list, 102);
    assert(found != NULL);
    assert(strcmp(found->name, "Bobby") == 0);
    assert(found->age == 20);
    assert(strcmp(found->branch, "ECE-Up") == 0);
    assert(approx(found->total, 330.0f) < 0.001f);
    assert(approx(found->percentage, 66.0f) < 0.001f);
    assert(found->grade == 'C');

    /* update of a missing roll fails */
    assert(student_list_update(&list, 999, &upd) == STUDENT_ERR_NOT_FOUND);

    /* sort by marks descending: Carol(95) > Alice(85) > Bobby(66) */
    student_sort_by_marks_desc(&list);
    assert(list.items[0].roll_no == 103);
    assert(list.items[1].roll_no == 101);
    assert(list.items[2].roll_no == 102);

    /* sort by roll ascending */
    student_sort_by_roll_asc(&list);
    assert(list.items[0].roll_no == 101);
    assert(list.items[1].roll_no == 102);
    assert(list.items[2].roll_no == 103);

    /* delete existing record */
    assert(student_list_delete(&list, 102) == STUDENT_OK);
    assert(list.count == 2);
    assert(student_find(&list, 102) == NULL);

    /* deleting it again fails */
    assert(student_list_delete(&list, 102) == STUDENT_ERR_NOT_FOUND);

    student_list_free(&list);   /* free the heap array */
    printf("PASS test_list_operations\n");
}

/* Exercise realloc growth: add more than INITIAL_CAPACITY records. */
static void test_capacity_growth(void)
{
    StudentList list;
    assert(student_list_init(&list) == STUDENT_OK);

    float m[SUBJECT_COUNT] = {50, 50, 50, 50, 50};
    for (int roll = 1; roll <= INITIAL_CAPACITY + 10; roll++) {
        Student s = make_student(roll, "Growth", 18, "IT", m);
        assert(student_list_add(&list, &s) == STUDENT_OK);
    }
    assert(list.count == INITIAL_CAPACITY + 10);
    assert(list.capacity >= list.count);

    student_list_free(&list);
    printf("PASS test_capacity_growth\n");
}

/* storage round-trip: save then load into a fresh list. */
static void test_storage_roundtrip(void)
{
    const char *path = "test_students.tmp";
    float m[SUBJECT_COUNT] = {91, 82, 73, 64, 55};

    StudentList src, dst;
    assert(student_list_init(&src) == STUDENT_OK);
    assert(student_list_init(&dst) == STUDENT_OK);

    Student s = make_student(42, "Persist", 22, "AI", m);
    assert(student_list_add(&src, &s) == STUDENT_OK);

    assert(storage_save(path, &src) == STUDENT_OK);
    assert(storage_load(path, &dst) == STUDENT_OK);
    assert(dst.count == 1);

    Student *loaded = student_find(&dst, 42);
    assert(loaded != NULL);
    assert(strcmp(loaded->name, "Persist") == 0);
    assert(approx(loaded->total, 365.0f) < 0.001f);

    /* loading a missing file is not an error: returns OK, empty list */
    StudentList empty;
    assert(student_list_init(&empty) == STUDENT_OK);
    assert(storage_load("no_such_file.tmp", &empty) == STUDENT_OK);
    assert(empty.count == 0);
    student_list_free(&empty);

    remove(path);               /* delete the temp file */
    student_list_free(&src);
    student_list_free(&dst);
    printf("PASS test_storage_roundtrip\n");
}

int main(void)
{
    test_calculate_result();
    test_list_operations();
    test_capacity_growth();
    test_storage_roundtrip();
    printf("\nAll tests passed.\n");
    return 0;
}
