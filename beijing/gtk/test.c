#include <gtk/gtk.h>

int main(int argc, char  *argv[])
{
	gtk_init(&argc,&argv);
	//gtk环境 初始化
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk控件指针 window gtk窗口新建      置顶有边框
	gtk_window_set_title((GtkWindow*)window,"haha");
	//gtk_window_set_title(GTK_WINDOW(window),"haha");
	gtk_widget_set_size_request(window,700,500);
	gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
	
	GtkWidget* table = gtk_table_new(7,5,TRUE);
	gtk_container_add(GTK_CONTAINER(window),table);//将table放入window中

	//创建其他的控件，放入窗口中
	GtkWidget* label_title = gtk_label_new("音乐");
	gtk_table_attach_defaults(GTK_TABLE(table),label_title,0,5,0,1);

	GtkWidget *image = gtk_image_new_from_file("1.jpg"); 
	gtk_table_attach_defaults(GTK_TABLE(table),image,0,2,1,4);

	//创建一个带有标题的分栏列表
	char* clist_titles[3] = {"歌曲名","歌手","流派"};
	GtkWidget* clist = gtk_clist_new_with_titles(3,clist_titles);
	gtk_clist_set_column_width(GTK_CLIST(clist),0,100);
	gtk_clist_set_column_width(GTK_CLIST(clist),1,100);
	gtk_clist_set_column_width(GTK_CLIST(clist),2,100);
	char * text[3] = {"荷塘月色","凤凰传奇","民族"};
	gtk_clist_append(GTK_CLIST(clist),text);
	gtk_table_attach_defaults(GTK_TABLE(table),clist,2,5,1,5);

	GtkWidget* button_pause = gtk_button_new_with_label("暂停/播放");
	gtk_table_attach_defaults(GTK_TABLE(table),button_pause,2,3,5,7);	


	gtk_widget_show_all(window);
	//gtk 控件 显示
	g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	gtk_main();//主事件循环
	return 0;
}