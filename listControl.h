#ifndef __LIST_CONTROL_H__

#define __LIST_CONTROL_H__

#include <stdio.h>
#include <mysql.h>
//#include <iostream>
#include <mysql.h>
#include <memory.h>
#include <string>

using namespace std;

#define CHARMAX 255

using namespace std;

int findURL(MYSQL *mysql, char* list, char* string);
int insertURL(MYSQL *mysql, char* list, char* serverIP);
int deleteURL(MYSQL *mysql, char* list, char* serverIP);


struct listItem
{
        int idx;
        char url[CHARMAX];
        unsigned int date;
        char status[8];
	string reason;
};

#endif
