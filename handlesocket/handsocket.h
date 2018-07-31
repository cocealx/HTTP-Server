#ifndef HTTP_SERVER_H_
#define  HTTP_SERVER_H_
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<signal.h>
#include<sys/epoll.h>
#define MAX 1024
typedef struct
{
	int efd;
	int socket;
}server_t;
int Getline(int socket,char*buf,int size);
void clear_sock(int sock);
void bad_request(const char*path,const char*errorstr,int sock);
int excu_cgi(int socket,char*method,char*path,char*query_string);
int echo_www(int  sock,char*path,int size);
void*handle(void*arg);
#endif

