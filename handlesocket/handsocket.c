#include"handsocket.h"
int Getline(int socket,char*buf,int size)
{

	char c=0;
	int i=0;
	while(c!='\n'&&i<size-1)
	{
		if(recv(socket,&c,1,0)>0)
		{
			if(c=='\r')
			{
				if(recv(socket,&c,1,MSG_PEEK)>0)
				{

					if(c=='\n')
						recv(socket,&c,1,0);
					else
						c='\n';
				}
			}
			buf[i++]=c;
		}
		else
			break;
	}
	buf[i]=0;
	printf("%s\n",buf);
	return i;
}
void clear_sock(int sock)
{
	char buf[MAX];
	for(;;)
	{
		int size=Getline(sock,buf,MAX-1);
		if(size==1&&strcmp(buf,"\n")==0)
			break;
	}
	//	printf("clean finish\n");
}
void bad_request(const char*path,const char*errorstr,int sock)
{
	printf("call bad_request \n");
	int fd=open(path,O_RDONLY);
	printf("path=%s\n",path);
	if(fd<0)
	{
		printf("open failer\n");
		perror("open");
		return ;
	}
	//响应报头加空行
	//	printf("响应报头加空行\n");	
	char line[MAX];
	sprintf(line,"%s",errorstr);
	send(sock,line,strlen(line),0);
	const char*content_type="Content-Type:text/html;charset=utf-8\r\n";
	send(sock,content_type,strlen(content_type),0);
	send(sock,"\r\n",2,0);
	struct stat st;
	if(fstat(fd,&st)<0)
	{
		printf("fstat failer \n");
		perror("fstat");
		return ;
	}
	int size =st.st_size;
	printf("%s size=%d\n",path,size);
	sendfile(sock,fd,0,size);
	close(fd);
}
void echo_errno(int code,int socket)
{
	switch(code)
	{
		case 404:
			{

				bad_request("wwwroot/404.html","HTTP/1.0 404 Not found\r\n",socket);
				break;
			}
		case 503:
			bad_request("wwwroot/503.html","HTTP/1.0 503 Not found\r\n",socket);
			break;
		default:
			break;
	}
}
int excu_cgi(int socket,char*method,char*path,char*query_string)
{
	//	char Method[MAX];
	char Content_Length[MAX>>3];
	char buf[MAX];
	int size=0;
	int content_length=-1;
	char Path[20];
	// sprintf(Method,"METHOD=%s",method);
	if(strcasecmp(method,"POST")==0)
	{
		//	strcpy(Method,"POST");
		do
		{
			size=Getline(socket,buf,sizeof(buf));
			if(strncmp(buf,"Content-Length: ",16)==0)
			{
				//	 	获得正文长度
				content_length=atoi(buf+16);
			}
		}while(size!=1&&strcmp(buf,"\n")!=0);
		sprintf(Content_Length,"%d",content_length);
	}
	else
	{
		//		strcpy(Method,"GET");
		clear_sock(socket);
		printf("%s\n",query_string);
		//		char Query[MAX];
		//	sprintf(Query,"%s",query_string);
		//	putenv(Query);
	}
	int outfd[2];
	int infd[2];
	if(pipe(outfd)<0)
	{
		return 404;
	}

	if(pipe(infd)<0)
	{
		return 404;
	}

	char line[MAX];
	sprintf(line,"HTTP/1.0 200 OK\n");
	send(socket,line,strlen(line),0);
	const char*content_type="Content-Type:text/html;charset=utf-8\r\n";
	send(socket,content_type,strlen(content_type),0);
	send(socket,"\r\n",2,0);
	pid_t pid=fork();
	if(pid==0)
	{
		setenv("METHOD",method,1);
		//		printf("%s\n",getenv("METHOD"));
		//		printf("开辟子进程\n");

		if(strcasecmp(method,"GET")==0)
		{
			setenv("QUERY_STRING",query_string,1);
		}
		else
		{
			setenv("CONTENT_LENGTH",Content_Length,1);
			printf("CONTENT_LENGTH=%s\n",Content_Length);
		}
		printf("path=%s\n",path);
		close(outfd[1]);
		close(infd[0]);
		dup2(outfd[0],0);
		dup2(infd[1],1);
		execl(path,path,NULL);
		perror("call cgi faile");
		return 404;
	}else if(pid>0)
	{
		close(outfd[0]);
		close(infd[1]);	
		int i=0;
		char c;
		if(strcasecmp(method,"POST")==0)
		{

			printf("content_length=%d\n",content_length);
			while(i<content_length)
			{
				recv(socket,&c,1,0);
				write(outfd[1],&c,1);
				//			printf("%c",c);
				++i;
			}}
		//		printf("\n");
		wait(NULL);
		//		printf("read infd[0]\n");
		//		int k=0;
		while(read(infd[0],&c,1)>0)
		{
			//			k++;
			send(socket,&c,1,0);
			//	printf("%c",c);
		}
		close(outfd[1]);
		close(infd[0]);
		printf("excgi,ok\n");

	}else
	{
		perror("fork");
		return 404;
	}
	return 200;
}
int echo_www(int  sock,char*path,int size)
{
	printf("clear_sock start\n");
	clear_sock(sock);
	printf("clear_sock end\n");
	int fd=open(path,O_RDONLY);
	printf("文件的大小为:%d\n",size);
	//清理剩余报文信息，避免粘包问题
	//	printf("clean socket\n");
	//发送响应报头
	//	printf("send http\n");
	char line[MAX>>3];
	sprintf(line,"HTTP/1.0 200 OK\n");
	send(sock,line,strlen(line),0);
	const char*content_type="Content-Type:text/html;charset=utf-8\r\n";
	send(sock,content_type,strlen(content_type),0);
	send(sock,"\n",1,0);
	// snedfile效率高，因为它少了从内核区向用户区拷贝，然后再从用户区拷贝到内核区
	if(sendfile(sock,fd,0,size)<0)
	{
		printf("%s发送文件失败\n",path);
		return 404;
	}
	close(fd);
	return 200;
}

void*handle(void*arg)
{
//	server_t *s=(server_t*)arg;
//	int socket=s->socket;
//	int efd=s->efd;
	int socket=(int)arg;
	char buf[MAX];
	char method[MAX];
	char url[MAX];
	char path[MAX];
	char* query_string=0;
	int cgi=0;
	int erroncode=200;
	//获得http的请求行
	int size = Getline(socket,buf,sizeof(buf));
	printf("size=%d  buf=%s\n",size,buf);
//	if(size==0)
//	{
//		epoll_ctl(efd,EPOLL_CTL_DEL,socket,NULL);
//		printf("delete fd %d\n",socket);
//		goto end;
//	}
	//	printf("size=%d\n",size);
	int i=0;
	int j=0;
	//从请求行解析出请求方法
	while(buf[i]&&i<size)
	{
		if(isspace(buf[i]))
		{
			break;
		}
		method[j++]=buf[i++];
	}
	method[j]=0;
	//	printf("method=%s\n",method);
	while(i<size&&isspace(buf[i]))
		++i;
	//获得url
	j=0;
	while(buf[i]&&i<size)
	{
		if(isspace(buf[i]))
		{
			break;
		}
		url[j++]=buf[i++];
	}
	url[j]=0;
	//	printf("url=%s\n",url);
	//判断请求方法
	if(strcasecmp(method,"POST")==0)
		cgi=1;
	else if(strcasecmp(method,"GET")==0)
	{
		j=0;
		while(url[j]&&j<MAX)
		{
			if(url[j]=='?')
			{
				//	 printf("GET 方法带参数\n");
				cgi=1;
				query_string=url+j+1;
				url[j]=0;
				break;
			}
			++j;
		}
		//	printf("path=%s\n",path);
	}
	else
	{
		erroncode=404;
		goto end;
	}

	sprintf(path,"wwwroot%s",url);
	//如果访问的是目录，就返回目录下的主页
	if(path[strlen(path)-1]=='/')
	{
		strcat(path,"index.html");
	}
	//判断path资源是否存在
	struct stat state;
	if(stat(path,&state)<0)
	{
		printf("no file path=%s\n",path);
		printf("文件不存在\n");
		clear_sock(socket);
		erroncode=404;
		goto end;
	}
	if(S_ISDIR(state.st_mode))
	{
		strcat(path,"/index.html");
	}
	else if(S_ISREG(state.st_mode))
	{
		if((state.st_mode&S_IXUSR)||\
				(state.st_mode&S_IXGRP)||(state.st_mode&S_IXOTH))
		{
			cgi=1;
		}
	}
	else
	{
	}
	if(stat(path,&state)<0)
	{
		printf("no file path=%s\n",path);
		printf("文件不存在\n");
		clear_sock(socket);
		erroncode=404;
		goto end;
	}
	printf("path=%s\n",path);
	if(cgi==1)
	{
		printf("call excu_cgi\n");
		erroncode=excu_cgi(socket,method,path,query_string);
	}
	else
	{
		//	printf("call echo_www\n");
		erroncode=echo_www(socket,path,state.st_size);
	}
end:
	if(erroncode!=200)
	{
		printf("call echo_errno\n");	
		echo_errno(erroncode,socket);
	}
	printf("send ok\n");
	close(socket);
	return NULL;
}
