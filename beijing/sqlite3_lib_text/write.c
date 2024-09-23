#include <stdio.h>
#include "sqlite3.h"

int main(int argc, char const *argv[])
{
	sqlite3* db;
	sqlite3_open("haha.db",&db);

	char * sql1 = "create table persons (id int,name text,age int);";

	char * errmsg = NULL;
	sqlite3_exec(db,sql1,NULL,NULL,&errmsg);

	if(errmsg!=NULL) printf("%s\n",errmsg);

	sqlite3_close(db);

	return 0;
}