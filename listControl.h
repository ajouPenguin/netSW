#ifndef __LIST_CONTROL_H__

#define __LIST_CONTROL_H__

#include <stdio.h>
#include <mysql.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

struct list_item_t {
    int idx;
    char *url;
    char *cookie;
    char *date;
    char *status;
    char *reason;
};

int connect_db();
struct list_item_t *select_item(char *url, char *cookie);
int insert_item(char *url, char *cookie);
int update_status(int idx, const char *status);
int update_reason(int idx, const char *reason);

#endif
