#include "listControl.h"

int findURL(MYSQL *mysql, char* list, char* string)
{
  char query[255];

  sprintf(query, "select * from %s where %s", list, string);
  printf("%s\n", query);
  if(mysql_query(mysql, query))
  {
     printf("%s\n", mysql_error(mysql));
     return 0;
  }

  return 1;
}

int insertURL(MYSQL *mysql, char* list, char* serverIP)
{
    char query[255];

    sprintf(query, "insert into %s values('%s')",list, serverIP);
    printf("%s\n", query);
    if(mysql_query(mysql, query))
    {
        printf("%s\n", mysql_error(mysql));
        return 0;
    }

    return 1;
}

int deleteURL(MYSQL *mysql, char* list, char* serverIP)
{
    char query[255];
    MYSQL_RES *sql_result;
    MYSQL_ROW row;

    sprintf(query, "url=%s",serverIP);

    if(!findURL(mysql, list, query))
    {
        return 0;
    }
    sql_result = mysql_store_result(mysql);

    memset(query, 0, sizeof(query));

    if(sql_result == NULL)
        printf("No Result.\n");
    else
    {
        while((row = mysql_fetch_row(sql_result)))
        {
            if(strcmp(serverIP, row[1]))
            {
                sprintf(query,"delete from %s where url=%s",list, serverIP);
                if(mysql_query(mysql, query))
		{
		    return 0;
		}
		return 1;
            }
        }
    }
    return 0;
}
