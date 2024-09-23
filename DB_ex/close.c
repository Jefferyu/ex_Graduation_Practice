#include <stdio.h>
#include "sqlite3.h"//数据库的头文件

int main(int argc, char const *argv[])
{
	//定义一个db用于存储数据库打开得到的句柄
	sqlite3* db;
	int ret  = sqlite3_open("haha.db",&db);
	if (ret !=SQLITE_OK)
	{
		perror("sqlite3_open:");
		return 0;
	}
	else
		printf("数据库打开成功\n");
	ret = sqlite3_close(db);
	if (ret !=SQLITE_OK)
	{
		perror("sqlite3_close:");
		return 0;
	}
	else
		printf("数据库关闭成功\n");
	return 0;
}