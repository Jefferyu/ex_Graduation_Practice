#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	pid_t pid, ppid,pgid;
	pid = getpid();
	printf("pid = %d\n", pid);
		ppid = getppid();
	printf("ppid = %d\n", ppid);
	pgid = getpgid(pid);
	printf("pgid = %d\n",pgid);
	return 0;
}