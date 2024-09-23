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

#define MAX_NUM 4  // 歌曲数量

#define FIFO "/tmp/fifo"
#define TTS "/dev/ttyUSB0"

static struct termios option_old;
static int now_music = 0;

char * music_name[MAX_NUM][2] = {
    {"Aimer", "LASTSTARDUST"},
    {"曲婉婷", "最好的安排"},
    {"宮本浩次", "冬の花"},
    {"杜宣达", "指纹"}
};

GtkWidget *image_widget = NULL;
GtkWidget *window = NULL;

int uart_fd;  // 串口文件描述符
void close_uart() {
    if (uart_fd >= 0) {
        close(uart_fd);  // 关闭串口
        printf("串口设备已关闭\n");
    }
}

int uart_init(char *devname)
{
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

    if ((fd = open(FIFO, O_WRONLY)) < 0) {
        perror("open");
        exit(1);
    }
    write(fd, buffer, strlen(buffer));
    close(fd);
}

void close_window(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    close_uart();
	send_mplayer_command("quit");   //退出mplayer
	unlink(FIFO);    //销毁fifo
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

void add_vol(GtkButton* button,gpointer user_data)
{
	send_mplayer_command("volume +5");
}

void red_vol(GtkButton* button,gpointer user_data)
{
	send_mplayer_command("volume -5");
}

gboolean do_next(GtkButton* button,gpointer user_data)
{
    char cmd[200];
    char song_image[200];
    char title[200];
    //printf("%s\n",music_name[now_music]);
    now_music++;
    if (now_music >= MAX_NUM) {
        now_music = 0;  // 如果超出歌曲数量，则循环回到第一首
    }
    sprintf(cmd,"loadfile %s-%s.mp3",music_name[now_music][0],music_name[now_music][1]);
    send_mplayer_command(cmd);

    //封面
    sprintf(song_image,"%s-%s.jpg",music_name[now_music][0],music_name[now_music][1]);
    gtk_image_set_from_file(GTK_IMAGE(image_widget), song_image);
    //标题
    sprintf(title,"网易云 音乐？%s - %s",music_name[now_music][0],music_name[now_music][1]);
    gtk_window_set_title((GtkWindow*)window, title);

    return FALSE;
}

gboolean do_prev(GtkButton* button,gpointer user_data)
{
    char cmd[200];
    char song_image[200];
    char title[200];
    //printf("%s\n",music_name[now_music]);
    now_music--;
    if (now_music < 0) {
        now_music = MAX_NUM - 1;  // 如果当前是第一首歌ss，切换到最后一首
    }
    sprintf(cmd,"loadfile %s-%s.mp3",music_name[now_music][0],music_name[now_music][1]);
    send_mplayer_command(cmd);

    //封面
    sprintf(song_image,"%s-%s.jpg",music_name[now_music][0],music_name[now_music][1]);
    gtk_image_set_from_file(GTK_IMAGE(image_widget), song_image);
    //标题
    sprintf(title,"网易云 音乐？%s - %s",music_name[now_music][0],music_name[now_music][1]);
    gtk_window_set_title((GtkWindow*)window, title);
    
    return FALSE;
}

void selection_made(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	gchar *text;
	char cmd[200];
    char song_image[200];
    char title[200];
	/* 取得存储在被选中的行和列的单元格上的文本
	 * 当鼠标点击时，我们用text参数接收一个指针
	 */
	gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);
	//打印一些关于选中了哪一行的信息
	g_print("第%d行，第%d列的内容为%s\n", row, column, text);

    sprintf(cmd,"loadfile %s-%s.mp3",music_name[row][0],music_name[row][1]);
    send_mplayer_command(cmd);

    //封面
    sprintf(song_image,"%s-%s.jpg",music_name[row][0],music_name[row][1]);
    gtk_image_set_from_file(GTK_IMAGE(image_widget), song_image);
    //标题
    sprintf(title,"网易云 音乐？%s - %s",music_name[row][0],music_name[row][1]);
    gtk_window_set_title((GtkWindow*)window, title);

    now_music = row;

}


void *thread1_fun()
{
    
    //初始化语音识别串口
    int fd1 = uart_init(TTS);
    while(1)
    {
        char text[128] = "";
        int len = uart_readline(fd1,text,sizeof(text),1000);
        //判断len的长度进行识别数据
        if(len > 0)
        {
            printf("len =%d, text=%s\n",len, text);
            
            if(strncmp(text, "bofang", strlen("bofang")) == 0 || strncmp(text, "zanting", strlen("zanting")) == 0)
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
                g_idle_add((GSourceFunc)do_next, NULL);
            }
            else if(strncmp(text, "shangyiqu", strlen("shangyiqu")) == 0)
            {
                g_idle_add((GSourceFunc)do_prev, NULL);
            }
            else if(strncmp(text, "jiadayinliang", strlen("jiadayinliang")) == 0)
            {
                send_mplayer_command("volume +20");
            }
            else if(strncmp(text, "jianxiaoyinliang", strlen("jianxiaoyinliang")) == 0)
            {
                send_mplayer_command("volume -20");
            }
        }
    }
}

void *thread2_fun()
{
    while(1)
    {
        system("echo get_percent_pos >/tmp/fifo");
        usleep(500000);  
    }
}

int main(int argc, char *argv[])
{

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);	//顶层窗口

	gtk_window_set_title((GtkWindow*)window, "网易云 音乐？Aimer - LASTSTARDUST");	//标题

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window, 1024, 768);	//设置大小
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);	//设置不可伸缩

	gtk_container_set_border_width(GTK_CONTAINER(window), 30);	//控件与窗口边框间隔为15

	GtkWidget *table = gtk_table_new(5,5,TRUE);
	gtk_container_add(GTK_CONTAINER(window),table);

	GtkWidget *hbox = gtk_hbox_new(FALSE, 5); // 创建水平盒子
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 5, 4, 5);

   	//创建一个带有标题的分栏列表
	char* clist_titles[2] = {"歌曲名","歌手"};
	GtkWidget* clist = gtk_clist_new_with_titles(2,clist_titles);
	gtk_clist_set_column_width(GTK_CLIST(clist),0,300);
	gtk_clist_set_column_width(GTK_CLIST(clist),1,150);
	// 遍历 music_name 数组，将歌手和歌曲名添加到列表中
    for (int i = 0; i < MAX_NUM; i++) {
        char *text[2] = {music_name[i][1], music_name[i][0]}; // 交换顺序以匹配列表标题
        gtk_clist_append(GTK_CLIST(clist), text);
    }
	gtk_table_attach_defaults(GTK_TABLE(table),clist,0,3,0,2);

	//封面
	image_widget = gtk_image_new_from_file("Aimer-LASTSTARDUST.jpg"); 
	gtk_table_attach_defaults(GTK_TABLE(table),image_widget,3, 5, 0, 2);

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
    GtkWidget *ff_button = gtk_button_new_with_label("快进");
    GtkWidget *next_button = gtk_button_new_with_label("下一曲");
    GtkWidget *vol_up = gtk_button_new_with_label("Vol Up");
    GtkWidget *vol_down = gtk_button_new_with_label("Vol Down");

    GtkWidget *vol_hbox = gtk_hbox_new(FALSE, 2); // 创建水平盒子
    gtk_table_attach_defaults(GTK_TABLE(table), vol_hbox, 0, 3, 2, 3);
    gtk_box_pack_start(GTK_BOX(vol_hbox), vol_up, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vol_hbox), vol_down, TRUE, TRUE, 0);

    // 将控制按钮添加到水平盒子中
    gtk_box_pack_start(GTK_BOX(hbox), prev_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), rew_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), play_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ff_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), next_button, TRUE, TRUE, 0);

    gtk_button_set_relief(GTK_BUTTON(play_button), GTK_RELIEF_NONE);	// 按钮背景色透明

    // 创建进度条
    GtkWidget *progressbar = gtk_progress_bar_new();
    gtk_table_attach(GTK_TABLE(table), progressbar, 0, 5, 3, 4, GTK_FILL, GTK_SHRINK, 0, 10);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), 0.3); 	//设置进度条30%
	gtk_widget_set_size_request(progressbar, -1, 30); // 宽度为100像素，高度不限制
    
    g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);	//关闭窗口信号量
	gtk_widget_show_all(window);	//显示所有控件
	//start_progress_timer(progressbar);

    g_signal_connect(prev_button,"pressed",G_CALLBACK(do_prev),NULL);
    g_signal_connect(next_button,"pressed",G_CALLBACK(do_next),NULL);
    g_signal_connect(ff_button,"pressed",G_CALLBACK(do_ff),NULL);
    g_signal_connect(rew_button,"pressed",G_CALLBACK(do_rew),NULL);
	g_signal_connect(play_button,"pressed",G_CALLBACK(do_pause),NULL);
    g_signal_connect(vol_up,"pressed",G_CALLBACK(add_vol),NULL);
    g_signal_connect(vol_down,"pressed",G_CALLBACK(red_vol),NULL);
	// 选择某一行时触发selection_made回调函数
	g_signal_connect(clist, "select-row", G_CALLBACK(selection_made), NULL);

    pid_t pid;//创建进程号
    mkfifo(FIFO, 0666);

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
		pthread_t thread1, thread2; // 创建线程号  
		pthread_create(&thread1, NULL, thread1_fun, NULL);   //创建线程
        pthread_create(&thread2, NULL, thread2_fun, NULL); // 创建第二个线程
		gtk_main();
	}

	return 0;
}