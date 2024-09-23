#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void chlid() {
    int fd, fd1;
    char size[50];

    fd = open("fifo_a", O_RDONLY);
    if (fd == -1) {
        perror("open failed for fifo_a");
        exit(1);
    }

    fd1 = open("fifo_b", O_WRONLY);
    if (fd1 == -1) {
        perror("open failed for fifo_b");
        close(fd);
        exit(1);
    }

    printf("输入: ");
    scanf("%s", size);
    write(fd1, size, strlen(size));
    
    close(fd);
    close(fd1);
}

void father() {
    int fd, fd1;
    char k[20];

    fd = open("fifo_b", O_RDONLY);
    if (fd == -1) {
        perror("open failed for fifo_b");
        exit(1);
    }

    fd1 = open("fifo_a", O_WRONLY);
    if (fd1 == -1) {
        perror("open failed for fifo_a");
        close(fd);
        exit(1);
    }

    read(fd, k, sizeof(k) - 1);
    k[sizeof(k) - 1] = '\0'; 
    printf("输出: %s\n", k);
    
    close(fd);
    close(fd1);
}

int main(int argc, char const *argv[]) {
    int FLFO_A, FLFO_B;
    pid_t pid;

    FLFO_A = mkfifo("fifo_a", 0666);
    if (FLFO_A == -1) {
        perror("mkfifo failed for fifo_a");
        return 1;
    }

    FLFO_B = mkfifo("fifo_b", 0666);
    if (FLFO_B == -1) {
        perror("mkfifo failed for fifo_b");
        return 1;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        chlid();
    } else {
        father();
    }

    unlink("fifo_a");
    unlink("fifo_b");
    return 0;
}
