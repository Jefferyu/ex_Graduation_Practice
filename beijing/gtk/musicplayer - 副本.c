#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <gdk/gdkx.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <termios.h>

static struct termios option_old;
static int now_music = 0;
char * music_name[2] = {"Aimer-LASTSTARDUST","曲婉婷-最好的安排"};

typedef struct {
    FILE *pp;
    double progress;
} MPlayer;

MPlayer init_mplayer() {
    MPlayer mp;
    mp.pp = popen("mplayer -slave -quiet -idle -input file=/tmp/fifo Aimer-LASTSTARDUST.mp3", "r");
    if (mp.pp == NULL) {
        perror("popen failed");
        exit(1);
    }
    mp.progress = 0.0;
    return mp;
}


int uart_init(char *devname)
{
    int uart_fd;
    struct termios option_new;
    
    uart_fd = open(devname, O_RDWR);
    if(uart_fd < 0)
    {
        perror("open_dev");
        _exit(-1);
    }
    tcgetattr(uart_fd, &option_old);        //保存串口属性
    tcgetattr(uart_fd, &option_new);

    cfsetispeed(&option_new, B9600);        //波特率为9600
    cfsetospeed(&option_new, B9600);        //波特率为9600
    option_new.c_cflag &= ~CSIZE;           //设置数据位时先关闭历史设置
    option_new.c_cflag |= CS8;              //数据位为8位
    option_new.c_cflag &= ~CSTOPB;          //1位停止位
    option_new.c_cflag &= ~PARENB;          //无奇偶校验位
    option_new.c_lflag &= ~(ICANON);        //非标准模式
    option_new.c_lflag &= ~ECHO;            //关回显，在使用GPRS模组时需关回显
    //option_new.c_lflag |= ECHO;               //开回显
    tcsetattr(uart_fd, TCSANOW, &option_new);
    return uart_fd;
}

void uart_uninit(int uart_fd)
{
    /*还原串口属性*/
    tcsetattr(uart_fd, TCSANOW, &option_old);
    
    /*关闭串口*/
    close(uart_fd);
}

void uart_send_str(int uart_fd, char *str)
{
    int ret;
    
    ret = write(uart_fd, str, strlen(str));
    if(ret < 0)
    {
        perror("write");
    }
}

int uart_readline(int uart_fd, char *buffer, int len, int timeout_ms)
{
    char c = '\0';
    fd_set fds;
    struct timeval tv;
    int i;
    int ret;

    memset(buffer, 0, len);
    for(i=0; i<len && c != '\n'; i++){
        tv.tv_sec = 0;
        tv.tv_usec = timeout_ms*1000;
        FD_ZERO(&fds);
        FD_SET(uart_fd, &fds);
        ret = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
        if(ret < 0){
            perror("seclect");
            return -1;
        }else if(ret > 0){
            ret = read(uart_fd, &c, 1);
            if(ret < 0)
            {
                perror("read");
            }
        }else{
            return -1;
        }
        *buffer++ = c;
        //printf("c=%c\n", c);
    }  
    return i;
}

// 向mplayer发送命令的函数
void send_mplayer_command(const char *cmd) {
    int fd;
    char buffer[256];

    sprintf(buffer, "%s\n", cmd);
    //printf("%s",buffer);
    if ((fd = open("/tmp/fifo", O_WRONLY)) < 0) {
        perror("open");
        exit(1);
    }
    write(fd, buffer, strlen(buffer));
    close(fd);
}

void close_window(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    int fd1 = uart_init("/dev/ttyUSB0");
    uart_uninit(fd1);
	send_mplayer_command("quit");   //退出mplayer
	unlink("/tmp/fifo");    //销毁fifo
	gtk_main_quit();
}

void do_pause(GtkButton* button,gpointer user_data)
{
	send_mplayer_command("pause");
}

void do_rew(GtkButton* button,gpointer user_data)
{
	send_mplayer_command("seek -5");
}

void do_ff(GtkButton* button,gpointer user_data)
{
	send_mplayer_command("seek 5");
}

void do_next(GtkButton* button,gpointer user_data)
{
    char cmd[200];
    printf("%s\n",music_name[now_music]);
    now_music++;
    if (now_music >= 2) {
        now_music = 0;  // 如果超出歌曲数量，则循环回到第一首
    }
    sprintf(cmd,"loadfile %s.mp3",music_name[now_music]);
    send_mplayer_command(cmd);
    
}

void do_prev(GtkButton* button,gpointer user_data)
{
    char cmd[200];
    printf("%s\n",music_name[now_music]);
    now_music--;
    if (now_music < 0) {
        now_music = 1;  // 如果当前是第一首歌，切换到最后一首
    }
    sprintf(cmd,"loadfile %s.mp3",music_name[now_music]);
    send_mplayer_command(cmd);
    
}

void selection_made(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	gchar *text;
	
	/* 取得存储在被选中的行和列的单元格上的文本
	 * 当鼠标点击时，我们用text参数接收一个指针
	 */
	gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);
	//打印一些关于选中了哪一行的信息
	g_print("第%d行，第%d列的内容为%s\n", row, column, text);

}


// gboolean update_progress(gpointer progressbar) {
//     // 向 mplayer 发送命令获取播放进度的百分比
//     system("echo get_percent_pos >/tmp/fifo");

    
//     printf("timer is running\n");
//     return TRUE; // 返回 TRUE 让定时器继续运行
// }


gboolean update_progress(gpointer progressbar) {
    printf("timer\n");

    // 返回 TRUE 让定时器继续运行
    return TRUE;
}


void start_progress_timer(GtkWidget *progressbar) {
    // 设置每秒更新一次进度条
    g_timeout_add(1000, update_progress, progressbar);
}



void *thread_fun()
{
    
    //初始化语音识别串口
    int fd1 = uart_init("/dev/ttyUSB0");
    while(1)
    {
        char text[128] = "";
        int len = uart_readline(fd1,text,sizeof(text),1000);
        //判断len的长度进行识别数据
        if(len > 0)
        {
            printf("len =%d, text=%s\n",len, text);
            
            if(strncmp(text, "bofang", strlen("bofang")) == 0)
            {
                send_mplayer_command("pause");
            }
            else if(strncmp(text, "zanting", strlen("zanting")) == 0)
            {
                send_mplayer_command("pause");
            }
            else if(strncmp(text, "kuaijin", strlen("kuaijin")) == 0)
            {
    			send_mplayer_command("seek 5");
            }
            else if(strncmp(text, "kuaitui", strlen("kuaitui")) == 0)
            {
    			send_mplayer_command("seek -5");
            }
            else if(strncmp(text, "xiayiqu", strlen("xiayiqu")) == 0)
            {

            }
            else if(strncmp(text, "shangyiqu", strlen("shangyiqu")) == 0)
            {

            }
        }
    }
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);	//顶层窗口

	gtk_window_set_title((GtkWindow*)window, "网易云 音乐？");	//标题

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window, 1024, 768);	//设置大小
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);	//设置不可伸缩

	gtk_container_set_border_width(GTK_CONTAINER(window), 30);	//控件与窗口边框间隔为15

	GtkWidget *table = gtk_table_new(5,5,TRUE);
	gtk_container_add(GTK_CONTAINER(window),table);

	GtkWidget *hbox = gtk_hbox_new(FALSE, 5); // 创建水平盒子
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 5, 4, 5);

   	//创建一个带有标题的分栏列表
	char* clist_titles[3] = {"歌曲名","歌手","流派"};
	GtkWidget* clist = gtk_clist_new_with_titles(3,clist_titles);
	gtk_clist_set_column_width(GTK_CLIST(clist),0,100);
	gtk_clist_set_column_width(GTK_CLIST(clist),1,100);
	gtk_clist_set_column_width(GTK_CLIST(clist),2,100);
	char * text[3] = {"LAST STARDUST","Aimer","流行"};
	gtk_clist_append(GTK_CLIST(clist),text);
	gtk_table_attach_defaults(GTK_TABLE(table),clist,0,3,0,3);

	//封面
	GtkWidget *image = gtk_image_new_from_file("Aimer-LASTSTARDUST.jpg"); 
	gtk_table_attach_defaults(GTK_TABLE(table),image,3, 5, 0, 2);

	//歌词
	GtkWidget *LRC_Title = gtk_label_new("歌词");
    GtkWidget *LRC = gtk_label_new("Dust to Dust, Earth to Earth");
    gtk_misc_set_alignment(GTK_MISC(LRC_Title), 0.5, 0.0);
    gtk_misc_set_alignment(GTK_MISC(LRC), 0.5, 0.5);
    gtk_table_attach(GTK_TABLE(table), LRC_Title, 3, 5, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), LRC, 3, 5, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

    // 带图标按钮
	GtkWidget *play_button = gtk_button_new(); // 先创建空按钮
	GtkWidget *playimage = gtk_image_new_from_file("play.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(play_button), playimage);

    // 创建控制按钮
    GtkWidget *prev_button = gtk_button_new_with_label("上一曲");
    GtkWidget *rew_button = gtk_button_new_with_label("快退");
    //GtkWidget *play_button = gtk_button_new_with_label("播放/暂停");
    GtkWidget *ff_button = gtk_button_new_with_label("快进");
    GtkWidget *next_button = gtk_button_new_with_label("下一曲");

    // 将控制按钮添加到水平盒子中
    gtk_box_pack_start(GTK_BOX(hbox), prev_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), rew_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), play_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ff_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), next_button, TRUE, TRUE, 0);

    gtk_button_set_relief(GTK_BUTTON(play_button), GTK_RELIEF_NONE);	// 按钮背景色透明

    // 创建进度条
    GtkWidget *progressbar = gtk_progress_bar_new();
    gtk_table_attach_defaults(GTK_TABLE(table), progressbar, 0, 5, 3, 4);
	//gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), 0.3); 	//设置进度条30%

	g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);	//关闭窗口信号量
	gtk_widget_show_all(window);	//显示所有控件
	start_progress_timer(progressbar);

    g_signal_connect(prev_button,"pressed",G_CALLBACK(do_prev),NULL);
    g_signal_connect(next_button,"pressed",G_CALLBACK(do_next),NULL);
    g_signal_connect(ff_button,"pressed",G_CALLBACK(do_ff),NULL);
    g_signal_connect(rew_button,"pressed",G_CALLBACK(do_rew),NULL);
	g_signal_connect(play_button,"pressed",G_CALLBACK(do_pause),NULL);
	// 选择某一行时触发selection_made回调函数
	g_signal_connect(clist, "select-row", G_CALLBACK(selection_made), NULL);

    pid_t pid;//创建进程号
    mkfifo("/tmp/fifo", 0666);

	//创建进程
	pid = fork();//创建多进程（fork、vfork的区别）
	if (pid<0)//说明失败了
	{
		perror("fork/n");
		return 0;//进程失败就不做事情直接退出，说明程序系统有问题
	}
	else if(pid==0)//子进程
	{
		execlp("mplayer","mplayer","-slave","-quiet","-idle","-input", "file=/tmp/fifo","Aimer-LASTSTARDUST.mp3",NULL);
		
        _exit(0);//结束进程
	}
	else//父进程
	{
		pthread_t thread;    //创建线程号   
		pthread_create(&thread, NULL, thread_fun, NULL);   //创建线程
		gtk_main();
	}

	return 0;
}