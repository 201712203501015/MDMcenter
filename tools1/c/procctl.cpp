#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // 这是 execl 所需的头文件
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char *argv[])
{
	// ./procctl 5 /usr/bin/ls -ls /tmp/aa.cpp
	if(argc < 3)
	{
		printf("Using:./procctl timetvl program argv ...\n");
		printf("Example:/project/tools1/bin/procctl 5 /usr/bin/tar zcvf /tmp/tmp.tgz /usr/include\n\n");
		
		printf("本程序是服务程序的调度程序，周期性启动服务程序或shell脚本。\n");
		printf("timetvl 运行周期，单位：秒。被调度的程序运行结束后，在timetvl秒后会被procctl重新启动。\n");
		printf("program 被调度的程序名，必须使用全路径。\n");
		printf("argvs   被调度的程序参数。\n");
		printf("注意，本程序不会被kill杀死，但可以用kill -9强行杀死。\n\n\n");
		
		return -1;
	}

	// Note：服务程序要关闭全部的信号，关闭全部的IO；原因：本程序不希望被干扰
	for(int ii=0;ii<64;ii++)
	{
		signal(ii,SIG_IGN);
		close(ii);
	}	

	// 生成子进程，父进程退出，让程序运行在后台，由系统1号进程托管
	if(fork()!=0) exit(0);

	// 启用SIGCHLD信号，让父进程可以wait子进程退出的状态
	signal(SIGCHLD,SIG_DFL);

	char *pargv[argc];
	for (int ii=2;ii<argc;ii++) // 这里从ii = 2开始，因为./procctl 是执行指令，5代表参数长度，ii = 2之后的位置才是具体参数列表
		pargv[ii-2] = argv[ii];
	pargv[argc-2]=NULL; // 数组结尾是NULL

	while(true)
	{
		if(fork()==0) // 子进程
		{
			execv(argv[2],pargv); // argv[2]是路径名，pargv是参数列表
			// exit(0)在execv执行成功时不会被执行，execv执行失败会被执行退出进程
			exit(0); // 这里要添加退出执行，因为如果execv失败，还会再次调用fork(0)，但execv的进程映像并没有被释放，会无限占用内存资源直至内存崩溃
		}
		else // 父进程
		{
			int status;
			wait(&status); // 父进程等待子进程执行完，防止出现僵尸进程
			sleep(atoi(argv[1]));
		}
	}
	return 0;
}
