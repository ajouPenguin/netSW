#include "listControl.h"
/* #define __TEST__ */
MYSQL *conn;

const char *set_charset = "SET CHARSET utf8;";

const char *create_table = "CREATE TABLE IF NOT EXISTS `graylist` ("
"`idx` INT NOT NULL AUTO_INCREMENT PRIMARY KEY ,"
"`url` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,"
"`cookie` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,"
"`date` DATETIME NOT NULL,"
"`status` VARCHAR( 8 ) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,"
"`reason` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL"
") ENGINE = MYISAM CHARACTER SET utf8 COLLATE utf8_general_ci;";

#define MYSQL_QUERY(q) \
if (mysql_query(conn, (q))) { \
    printf("Query failed: %s\n", mysql_error(conn)); \
    return 0; \
}

#define CHECK_MYSQL_CONNECT \
if (conn == NULL) { \
    puts("mysql is not connected"); \
    return NULL; \
}

int connect_db() {

    conn = mysql_init(NULL);

    if (mysql_real_connect(conn, "localhost", "root", "123123", "rss", 3306, (char *)NULL, 0) == NULL) {
      printf("%s\n", mysql_error(conn));
      printf("mysql_real_connect error\n");
      return 0;
    }
    MYSQL_QUERY(set_charset);
    MYSQL_QUERY(create_table);
    return 1;
}

struct list_item_t *select_item(char *url, char *cookie) {
    char *query, *r;
    MYSQL_RES *result;
    MYSQL_ROW row;
    struct list_item_t *ret = NULL;
    int i, fields;

    CHECK_MYSQL_CONNECT;

    query = (char *) malloc((112 + strlen(url) + strlen(cookie)) * sizeof(char));
    sprintf(query, "SELECT * FROM `graylist` WHERE url='%s' AND cookie='%s' AND unix_timestamp()-6*60*60<unix_timestamp(date);", url, cookie);

    MYSQL_QUERY(query);

    if ((result = mysql_store_result(conn)) == NULL) {
        printf("%s\n", mysql_error(conn));
        return 0;
    }

    if ((row = mysql_fetch_row(result)) != NULL) {
        ret = (struct list_item_t *) malloc(1 * sizeof(struct list_item_t));
        ret->idx = row[0]? atoi(row[0]): -1;
        ret->url = row[1]? strdup(row[1]): NULL;
        ret->cookie = row[2]? strdup(row[2]): NULL;
        ret->date = row[3]? strdup(row[3]): NULL;
        ret->status = row[4]? strdup(row[4]): NULL;
        ret->reason = row[5]? strdup(row[5]): NULL;
    }

    return ret;
}

int insert_item(char *url, char *cookie) {
    char *query;
    CHECK_MYSQL_CONNECT;
    query = (char *) malloc((68 + strlen(url) + strlen(cookie)) * sizeof(char));
    sprintf(query, "INSERT INTO `graylist` VALUES(NULL, '%s', '%s', NOW(), 'PENDDING', '');", url, cookie);
    MYSQL_QUERY(query);
    return 1;
}

int digit(int a) {
    int i;
    for (i = 0; a != 0; a /= 10, ++i);
    return i;
}

int update_item(int idx, const char *col, const char *val) {
    char *query;
    CHECK_MYSQL_CONNECT;
    query = (char *) malloc((42 + strlen(col) + strlen(val) + digit(idx)) * sizeof(char));
    sprintf(query, "UPDATE `graylist` SET `%s`='%s' WHERE `idx`=%d;", col, val, idx);
    MYSQL_QUERY(query);
    return 1;
}

int update_status(int idx, const char *status) {
    return update_item(idx, "status", status);
}

int update_reason(int idx, const char *reason) {
    return update_item(idx, "reason", reason);
}


#ifdef __TEST__
int main(int argc, char *argv[]) {
    struct list_item_t *x = NULL;
    connect_db();

    x = select_item(argv[1], argv[2]);

    if (x == NULL) {
        insert_item(argv[1], argv[2]);

        x = select_item(argv[1], argv[2]);
        printf("%d %s\n", x->idx, x->status);

        update_status(x->idx, "WHITE");
        free(x);

        x = select_item(argv[1], argv[2]);
        printf("%d %s\n", x->idx, x->status);
    } else {
        printf("%s\n", x->status);
    }

    free(x);


    mysql_close(conn);
    return 0;
}
#endif
