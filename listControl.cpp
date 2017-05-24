#include <cstdio>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <mysql.h>

using namespace std;

int main(int argc, char* args[])
{
  MYSQL mysql;

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, NULL, "root", "tiger1343", NULL, 3306, (char *)NULL, 0))
  {
    printf("%s\n", mysql_error(&mysql));
    exit(0);
  }

  printf("MYSQL connection is successful");

  mysql_close(&mysql);
  return 0;
}
