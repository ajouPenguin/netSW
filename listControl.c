/*
 * 아주대 소프트웨어학과
 * Network software term project.
 * 3조 teamPUB
 * 2017-06-14
 */

#include "listControl.h"
/* #define __TEST__ */
MYSQL *conn;          // MYSQL handler

const char *set_charset = "SET CHARSET utf8;";

const char *create_table = "CREATE TABLE IF NOT EXISTS `graylist` ("
"`idx` INT NOT NULL AUTO_INCREMENT PRIMARY KEY ,"
"`url` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,"
"`cookie` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,"
"`date` DATETIME NOT NULL,"
"`status` VARCHAR( 8 ) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,"
"`reason` TEXT CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL"
") ENGINE = MYISAM CHARACTER SET utf8 COLLATE utf8_general_ci;";


/* DB에 query 삽입 및 실행 */
#define MYSQL_QUERY(q) \
if (mysql_query(conn, (q))) { \
    printf("Query failed: %s\n", mysql_error(conn)); \
    return 0; \
}

/* DB handler가 DB에 연결되어 있는지 확인 */
#define CHECK_MYSQL_CONNECT \
if (conn == NULL) { \
    puts("mysql is not connected"); \
    return NULL; \
}

/* handler에 DB를 연결, charset 설정, 테이블 생성 */
int connect_db() {

    conn = mysql_init(NULL);        // handler 초기화

    /* raspberry pi의 MYSQL에 연결
     * handler, ip, user id, password, db 이름, port, socket, flag
     */
    if (mysql_real_connect(conn, "localhost", "root", "123123", "rss", 3306, (char *)NULL, 0) == NULL) {
      printf("%s\n", mysql_error(conn));
      printf("mysql_real_connect error\n");
      return 0;
    }
    MYSQL_QUERY(set_charset);       // DB의 charset을 UTF-8로 설정
    MYSQL_QUERY(create_table);      // 테이블 생성
    return 1;
}

/* parameter로 주어진 url과 cookie를 이용하여 DB에 있는 인스턴스를 얻어옴 */
struct list_item_t *select_item(char *url, char *cookie) {
    char *query, *r;
    MYSQL_RES *result;              // 결과 저장 구조체
    MYSQL_ROW row;                  // result로부터 온 한 인스턴스를 저장하는 구조체
    struct list_item_t *ret = NULL; // row로부터 받은 data를 재가공하여 저장
    int i, fields;

    CHECK_MYSQL_CONNECT;

    /* query 생성 및 수행 */
    query = (char *) malloc((112 + strlen(url) + strlen(cookie)) * sizeof(char));
    sprintf(query, "SELECT * FROM `graylist` WHERE url='%s' AND cookie='%s' AND unix_timestamp()-6*60*60<unix_timestamp(date);", url, cookie);
    MYSQL_QUERY(query);

    /* select문 결과를 얻어옴 */
    if ((result = mysql_store_result(conn)) == NULL) {
        printf("%s\n", mysql_error(conn));
        return 0;
    }

    /* 얻어온 결과를 재가공하여 list_item_t에 저장 */
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

/* url과 cookie를 DB에 저장 */
int insert_item(char *url, char *cookie) {
    char *query;
    CHECK_MYSQL_CONNECT;

    /* isert문 query 생성 및 수행 */
    query = (char *) malloc((68 + strlen(url) + strlen(cookie)) * sizeof(char));
    sprintf(query, "INSERT INTO `graylist` VALUES(NULL, '%s', '%s', NOW(), 'PENDDING', '');", url, cookie);
    MYSQL_QUERY(query);
    return 1;
}

/* a의 자리수 return */
int digit(int a) {
    int i;
    for (i = 0; a != 0; a /= 10, ++i);
    return i;
}

/* 해당 인덱스를 가진 인스턴스에 col = val를 업데이트 */
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

/*
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
*/
