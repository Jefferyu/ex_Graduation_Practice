#include <stdio.h>

int main(int argc, char const *argv[]) {
    // 打印程序接收到的命令行参数数量
    printf("Number of arguments: %d\n", argc);

    // 遍历并打印每个命令行参数
    for (int i = 0; i < argc; ++i) {
        printf("Argument %d: %s\n", i, argv[i]);
    }

    return 0;
}