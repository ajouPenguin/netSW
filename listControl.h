#ifndef __LIST_CONTROL_H__

#define __LIST_CONTROL_H__

#include <stdio.h>
#include <mysql.h>
#include <mysql.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

struct list_item_t {    // DB 스키마
    int idx;            // 인덱스
    char *url;          // 검증된 url
    char *cookie;       // url에 맞는 cookie
    char *date;         // 검증이 된 날짜와 시간
    char *status;       // BLACK, WHITE, PENDDING 중 하나
    char *reason;       // BLACK일 때, 그 이유
};

int connect_db();                                           // DB와 handler를 연결하고 테이블을 생성
struct list_item_t *select_item(char *url, char *cookie);   // url과 cookie가 일치하는 인스턴스를 얻어옴
int insert_item(char *url, char *cookie);                   // parameter로 들어온 url과 cookie를  가지는 인스턴스를 테이블에 삽입
int update_status(int idx, const char *status);             // 해당 인덱스를 가진 인스턴스에 status를 업데이트
int update_reason(int idx, const char *reason);             // 해단 인덱스를 가진 인스턴스에 reason을 업데이트

#endif
