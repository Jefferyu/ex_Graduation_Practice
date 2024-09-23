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

#define UART "/dev/ttyUSB2"
#define WEB_FIFO "/tmp/web_fifo"

static struct termios option_old;

GtkWidget *window = NULL;   //新窗口
GtkWidget *temp_label;
GtkWidget *humid_label;

float humid,temp;

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


void *thread1_fun()
{
    
    //初始化STM32串口
    int fd1 = uart_init(UART);
    while(1)
    {
        char text[128] = "";
        int len = uart_readline(fd1,text,sizeof(text),1000); //判断len的长度进行识别数据
        if(len > 0)
        {
            printf("len =%d, text=%s\n",len, text);
            
            if(strncmp(text, "dht11_temp:", strlen("dht11_temp:")) == 0)        //DHT11 温度
            {
                temp = atof(text + strlen("dht11_temp:"));
                printf("Temperature: %f\n", temp);
            }
            else if(strncmp(text, "dht11_humid:", strlen("dht11_humid:")) == 0)    //DHT11 湿度
            {
                humid = atof(text + strlen("dht11_humid:"));
                printf("Humidity: %f\n", humid);
            }
        }
    }
}

// 定义update_values函数，用于更新界面上的值
gboolean update_values(gpointer data) {
    char temp_text[200],humid_text[200];


    // 这里应该是获取实时数据的代码，暂时用静态值代替
    sprintf(temp_text, "Temperature: %f",temp);
    sprintf(humid_text, "Humidity: %f",humid);

    // 更新标签的文本
    gtk_label_set_text(GTK_LABEL(temp_label), temp_text);
    gtk_label_set_text(GTK_LABEL(humid_label), humid_text);

    // 返回TRUE以继续接收定时器事件
    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv); //GTK初始化
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);	//顶层窗口
    gtk_window_set_title((GtkWindow*)window, "STM32上位机");	//标题
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window, 1024, 768);	//设置大小
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);	//设置不可伸缩
    gtk_container_set_border_width(GTK_CONTAINER(window), 30);  // 设置窗口边框的宽度 
    
    GtkWidget *table = gtk_table_new(7,9,TRUE);
    gtk_container_add(GTK_CONTAINER(window),table);
    
    //温度湿度
    temp_label = gtk_label_new("Temperature: --");
    humid_label = gtk_label_new("Humidity: --");	  
    gtk_misc_set_alignment(GTK_MISC(temp_label), 0.5, 0.0);
    gtk_misc_set_alignment(GTK_MISC(humid_label), 0.5, 0.5);
    gtk_table_attach(GTK_TABLE(table), temp_label, 1, 5, 1, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), humid_label, 1, 5, 1, 3, GTK_FILL, GTK_FILL, 0, 0);

    // 显示所有窗口组件
    gtk_widget_show_all(window);

    // 设置定时器，每1000毫秒（1秒）调用一次update_values函数
    g_timeout_add(1000, update_values, NULL);

    // 连接destroy事件，当关闭窗口时退出GTK主循环
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    pid_t pid;//创建进程号
	pid = fork();//创建多进程（fork、vfork的区别）
	if (pid<0)//说明失败了
	{
		perror("fork/n");
		return 0;//进程失败就不做事情直接退出，说明程序系统有问题
	}
    else if(pid==0)//子进程
	{
		//execlp("mplayer","mplayer","-slave","-quiet","-idle","-input", "file=/tmp/fifo","Aimer-LASTSTARDUST.mp3",NULL);
		
        _exit(0);//结束进程
	}
	else//父进程
	{
		pthread_t thread1; // 创建线程号  
		pthread_create(&thread1, NULL, thread1_fun, NULL);   //创建线程
		gtk_main();
	}

    return 0;
}