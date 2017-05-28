#include <cstdio>
#include <mysql.h>
#include <iostream>
#include <mysql.h>
#include <memory.h>

using namespace std;

int findURL(MYSQL *mysql, char* list, char* string);
int insertURL(MYSQL *mysql, char* list, char* serverIP);
int deleteURL(MYSQL *mysql, char* list, char* serverIP); 
