#include <stdio.h>
#include "sqlite3.h"

sqlite3* db;
char * errmsg = NULL;
char **table = NULL;
int row=0,col=0;

void menu();

void initialDB()
{
	sqlite3_open("contacts.db",&db);
	char * sql = "create table if not exists contacts (id int,name text,phonenumber text);";
	sqlite3_exec(db,sql,NULL,NULL,&errmsg);
	if (errmsg!=NULL)
	{
		printf("创建数据库失败，错误代码：%s\n", errmsg);
	}
	printf("数据库初始化成功\n");
}

void addContact()
{
	char name[20] = "";
	int id = 0;
	char phonenumber[20] = "";
	char sql[100] = "";
	printf("==========================\n");
	printf("1.新增联系人\n");
	printf("==========================\n");
	printf("请输入新增id：");
    scanf("%d",&id);
    printf("请输入新增姓名：");
    scanf("%s",name);
    printf("请输入电话号码：");
    scanf("%s",phonenumber);

    sprintf(sql,"insert into contacts values (\'%d\',\'%s\',\'%s\');",id,name,phonenumber);
    errmsg = NULL;
    sqlite3_exec(db,sql,NULL,NULL,&errmsg);
    printf("新增成功，按任意键返回主菜单。\n");
    getchar();
    getchar();
    menu();
}

void deleteContact()
{	
	int id = 0;
	char sql[100] = "";
	printf("==========================\n");
	printf("2.删除联系人\n");
	printf("==========================\n");
	printf("请输入要删除联系人的id：");
    scanf("%d",&id);
    sprintf(sql,"delete from contacts where id = %d;",id);
	errmsg = NULL;
    sqlite3_exec(db,sql,NULL,NULL,&errmsg);
    printf("删除ID为 %d 的联系人成功，按任意键返回主菜单。\n",id);
    getchar();
    getchar();
    menu();
}

void modifyContact()
{
	int id = 0;
	char name[20] = "";
	char phonenumber[20] = "";
	char sql[100] = "";
	printf("==========================\n");
	printf("3.修改联系人\n");
	printf("==========================\n");
	printf("请输入要修改联系人的id：");
    scanf("%d",&id);
    printf("请输入修改后的姓名：");
    scanf("%s",name);
    printf("请输入修改后的电话号码：");
    scanf("%s",phonenumber);
    sprintf(sql, "UPDATE contacts SET name = '%s', phonenumber = '%s' WHERE id = %d;", name, phonenumber, id);
    errmsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("修改联系人失败：%s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("修改ID为 %d 的联系人成功，修改后姓名为 %s ,修改后的电话号码为 %s 。\n", id, name, phonenumber);
    }
    getchar();
    getchar();
    menu();
}

void findContact()
{
	int id = 0;
	char name[20] = "";
	char phonenumber[20] = "";
	char sql[100] = "";
	int select;
	int row=0,col=0;
	printf("==========================\n");
	printf("4.查询联系人\n");
	printf("==========================\n");
	printf("请输入查询条件 1.ID查询 2.姓名查询 3.电话号码查询 0.返回主菜单：");
	scanf("%d",&select);
	switch (select) {
        case 1:
            printf("请输入ID:");
            scanf("%d",&id);
            sprintf(sql,"select name, phonenumber from contacts where id = \'%d\';",id);
    		errmsg = NULL;
    		sqlite3_get_table(db,sql,&table,&row,&col,&errmsg);
    		if (row!=0)
    		{
        		printf("ID %d 所对应的姓名为 %s，电话号码为 %s。\n",id,table[2],table[3]);
    		}
    		getchar();
    		getchar();
    		findContact();
    		break;
        case 2:
            printf("请输入姓名:");
            scanf("%s",name);
            sprintf(sql,"select id, phonenumber from contacts where name = \'%s\';",name);
    		errmsg = NULL;
    		sqlite3_get_table(db,sql,&table,&row,&col,&errmsg);
    		if (row!=0)
    		{
        		printf("姓名 %s 所对应的ID为 %s，电话号码为 %s。\n",name,table[2],table[3]);
    		}
    		getchar();
    		getchar();
    		findContact();
    		break;
        case 3:
            printf("请输入电话号码:");
            scanf("%s",phonenumber);
            sprintf(sql,"select id, name from contacts where phonenumber = \'%s\';",phonenumber);
    		errmsg = NULL;
    		sqlite3_get_table(db,sql,&table,&row,&col,&errmsg);
    		if (row!=0)
    		{
        		printf("电话号码 %s 所对应的ID为 %s，姓名为 %s。\n",phonenumber,table[2],table[3]);
    		}
    		getchar();
    		getchar();
    		findContact();
    		break;
    	case 0:
    		menu();
    		break;
        default:
            printf("无效选项，请重新输入。\n");
            findContact();
    }
}

void showContact()
{
	char *sql = "select * from contacts;";
	row = col =0;
    table = NULL;
   	errmsg = NULL;
	printf("==========================\n");
	printf("5.显示所有联系人\n");
	printf("==========================\n");
    sqlite3_get_table(db,sql,&table,&row,&col,&errmsg);
    if(row!=0)
    {
        for (int i = 0; i < row+1; ++i)
        {
            for (int k = 0; k < col; ++k)
            {
                printf("[%s] ", table[i*col+k]);
            }
            printf("\n");
        }
        printf("\n");
    }
    printf("显示成功，按任意键返回主菜单。\n");
    getchar();
    getchar();
    menu();
}

int exitSystem()
{
	sqlite3_close(db);
	printf("数据库已保存\n");
	return 0;
}



void menu()
{
	//printf("\033[2J"); // 清屏
	
	int select;
	printf("==========================\n");
	printf("通讯录管理系统\n");
	printf("1.新增联系人\n");
	printf("2.删除联系人\n");
	printf("3.修改联系人\n");
	printf("4.查找联系人\n");
	printf("5.显示所有联系人\n");
	printf("0.退出系统\n");
	printf("==========================\n");
	printf("请输入选项：");
	scanf("%d",&select);
	switch (select) {
        case 1:
            addContact();
            break;
        case 2:
            deleteContact();
            break;
        case 3:
            modifyContact();
            break;
        case 4:
            findContact();
            break;
        case 5:
            showContact();
            break;
        case 0:
            exitSystem();
            break;
        default:
            printf("无效选项，请重新输入。\n");
            menu();
            break;
    }
}

int main(int argc, char const *argv[])
{
	initialDB();
	menu();
	return 0;
}