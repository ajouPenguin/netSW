#ifndef __LIST_CONTROL_H__

#define __LIST_CONTROL_H__

#include <stdio.h>
#include <mysql.h>
#include <mysql.h>
#include <memory.h>

#define CHARMAX 255

int findURL(MYSQL *mysql, char* list, char* string);
int insertURL(MYSQL *mysql, char* list, char* serverIP);
int deleteURL(MYSQL *mysql, char* list, char* serverIP);


struct listItem
{
    int idx;
    char url[CHARMAX];
    struct MYSQL_TIME date;
    char status[8];
	  char reason[255];
};

#endif
