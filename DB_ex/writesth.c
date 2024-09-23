#include <stdio.h>
#include "sqlite3.h"//数据库的头文件

int main(int argc, char const *argv[])
{
	//定义一个db用于存储数据库打开得到的句柄
	sqlite3* db;
	sqlite3_open("haha.db",&db);
	char * sql1 = "create table persons (id int,name text,age int);";

	char * errmsg = NULL;
	sqlite3_exec(db,sql1,NULL,NULL,&errmsg);
	if (errmsg!=NULL)
	{
		printf("%s\n", errmsg);
	}

	char name[20] = "";
	char id[20] = "";
	int age = 0;
	printf("请输入一个id：");
	scanf("%s",id);
	printf("请输入一个name：");
	scanf("%s",name);

	printf("请输入一个age：\n");
	scanf("%d",&age);
	char sql2[100] = "";
	sprintf(sql2,"insert into persons values (\'%s\',\'%s\',%d);",id,name,age);
	errmsg = NULL;
	sqlite3_exec(db,sql2,NULL,NULL,&errmsg);
	

	sqlite3_close(db);
	
	return 0;
}