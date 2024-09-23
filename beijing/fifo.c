#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>

#define FIFO_A "/tmp/fifoA"
#define FIFO_B "/tmp/fifoB"


// 父进程
void parent_process() 
{
    int fd_write, fd_read;
    char parent_send[200]="";
    char parent_rec[200]="";
    

    // 打开fifoA用于写，fifoB用于读
    fd_write = open(FIFO_A, O_WRONLY);
    fd_read = open(FIFO_B, O_RDONLY);

    while (1) 
    {
        printf("发：父进程：");
        //fgets(buffer, 200, stdin);
        scanf("%s",parent_send);
        write(fd_write, parent_send, strlen(parent_send)+1);          // 发送消息到子进程
        ssize_t bytes_read = read(fd_read, parent_rec, 200);        // 接收子进程的消息
        if (bytes_read > 0) );
    }


    close(fd_write);
    close(fd_read);
}

// 子进程
void child_process() 
{
    int fd_read, fd_write;
    char child_send[200]="";
    char child_rec[200]="";

    fd_read = open(FIFO_A, O_RDONLY);   // 读fifoA
    fd_write = open(FIFO_B, O_WRONLY);  // 写fifoB

    while (1) 
    {
        ssize_t bytes_read = read(fd_read, child_rec, 200);              // 接收父进程的消息
        if (bytes_read > 0) printf("收：父进程say：%s\n", child_rec);     // 发送消息到父进程
        printf("发：子进程：");
        scanf("%s",child_send);
        write(fd_write, child_send, strlen(child_send)+1);
    }
    close(fd_read);
    close(fd_write);
}


int main(int argc, char const *argv[])
{
    mkfifo(FIFO_A, 0666);
    mkfifo(FIFO_B, 0666);

    pid_t pid = fork();
    if (pid == 0) child_process();
    else parent_process();
    wait(NULL);

    unlink(FIFO_A);
    unlink(FIFO_B);
    return 0;
}
