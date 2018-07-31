/*************************************************************************
	> File Name: out.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2018年07月05日 星期四 15时48分41秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
int main()
{

//	int fd=open("error",O_WRONLY|O_CREAT,0666);
	char method[100];
//	dup2(2,1);
//	write(fd,"call out.cgi",12);
	strcpy(method,getenv("METHOD"));
//	write(fd,method,strlen(method));
	char buf[1024]={
		0
	};
	int first;
	int second;
	if(strcasecmp(method,"GET")==0)
	{
		perror("GET");
		strcpy(buf,getenv("QUERY_STRING"));
	//	write(fd,buf,strlen(buf));
	}
	else
	{
		int count=atoi(getenv("CONTENT_LENGTH"));
		perror("%d",count);
		read(0,buf,count);
	}
	sscanf(buf,"first=%d&second=%d",&first,&second);
	int finally=first+second;
	printf("<html>\n<body>\n<p>first+second=%d\n</p>\n</body>\n</html>\n",finally);
	perror("cgi finally");
//	write(fd,"cgi finally",11);
//	close(fd);
}
