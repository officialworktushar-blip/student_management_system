#ifndef STORAGE_H
#define STORAGE_H

#include "student.h"

/* File format (binary, data/students.dat):
 *   int32 count, followed by count * sizeof(Student) records.
 * Written/read with fwrite/fread — see storage.c. */

/* Load all records into list (which must already be initialised).
 * Returns STUDENT_OK on success; a missing file is NOT an error —
 * it just means the database is empty (first run). */
int storage_load(const char *path, StudentList *list);

/* Save all records to path. Returns STUDENT_OK or STUDENT_ERR_ALLOC/IO. */
int storage_save(const char *path, const StudentList *list);

#endif /* STORAGE_H */
