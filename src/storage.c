/* storage.c — binary file persistence only (fopen/fread/fwrite/fclose).
 * No printf/scanf: callers inspect the returned status code.
 * Concepts: file handling, binary I/O, fread/fwrite, error codes. */

#include "storage.h"

#include <stdio.h>    /* FILE, fopen, fread, fwrite, fclose */

int storage_load(const char *path, StudentList *list)
{
    /* fopen mode "rb" = read binary */
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return STUDENT_OK;   /* missing file -> empty database, not fatal */

    int count = 0;
    /* file header: number of records (written by storage_save) */
    if (fread(&count, sizeof(int), 1, fp) != 1 || count < 0) {
        fclose(fp);
        return STUDENT_ERR_INVALID;
    }

    /* sanity cap so a corrupt file can't make us loop forever */
    if (count > 1000000) {
        fclose(fp);
        return STUDENT_ERR_INVALID;
    }

    /* read each record and append via student_list_add (reuses validation,
     * capacity growth and result computation). Pointers keep it lean. */
    for (int i = 0; i < count; i++) {
        Student s;
        if (fread(&s, sizeof(Student), 1, fp) != 1) {
            fclose(fp);
            return STUDENT_ERR_INVALID;
        }
        int rc = student_list_add(list, &s);
        if (rc != STUDENT_OK) {
            fclose(fp);
            return rc;
        }
    }

    fclose(fp);
    return STUDENT_OK;
}

int storage_save(const char *path, const StudentList *list)
{
    /* fopen mode "wb" = write binary (truncates/creates the file) */
    FILE *fp = fopen(path, "wb");
    if (fp == NULL)
        return STUDENT_ERR_INVALID;

    /* header: record count */
    if (fwrite(&list->count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return STUDENT_ERR_INVALID;
    }

    /* body: write the whole records array in one call */
    if (list->count > 0 &&
        fwrite(list->items, sizeof(Student), (size_t)list->count, fp)
            != (size_t)list->count) {
        fclose(fp);
        return STUDENT_ERR_INVALID;
    }

    fclose(fp);
    return STUDENT_OK;
}
