#ifndef TEST_STUB_MYSQL_H
#define TEST_STUB_MYSQL_H

#include <stddef.h>
typedef struct MYSQL MYSQL;
typedef struct MYSQL_RES MYSQL_RES;
typedef char **MYSQL_ROW;
typedef unsigned long long my_ulonglong;

MYSQL *mysql_init(MYSQL *);
MYSQL *mysql_real_connect(MYSQL *, const char *, const char *, const char *,
                          const char *, unsigned int, const char *,
                          unsigned long);
const char *mysql_error(MYSQL *);
void mysql_close(MYSQL *);
int mysql_set_character_set(MYSQL *, const char *);
unsigned long mysql_real_escape_string(MYSQL *, char *, const char *,
                                       unsigned long);
int mysql_query(MYSQL *, const char *);
MYSQL_RES *mysql_store_result(MYSQL *);
my_ulonglong mysql_insert_id(MYSQL *);
MYSQL_ROW mysql_fetch_row(MYSQL_RES *);
void mysql_free_result(MYSQL_RES *);
my_ulonglong mysql_affected_rows(MYSQL *);

#endif
