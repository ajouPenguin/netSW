#include <stdio.h>
#include <mysql.h>
#include <mysql.h>
#include <memory.h>

#define CHARMAX 255
#define REASON_SIZE 500

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
	char reason[REASON_SIZE];
};
