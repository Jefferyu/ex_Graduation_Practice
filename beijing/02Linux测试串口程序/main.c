#include <stdio.h>
#include "uart.h"

int main()
{
	//初始化语音识别串口
	int fd1 = uart_init("/dev/ttyUSB0");
	while(1)
	{
		char text[128] = "";
		uart_readline(fd1,text,sizeof(text),1000);
		
		if(text[0] != '\0')
		{
			printf("text = %s\n", text);
			
		}		
	}
}
