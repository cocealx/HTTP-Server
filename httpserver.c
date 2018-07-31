#include"pthread_pool.h"
#include"handsocket.h"
#define MAX_EVENTS 1024
int startup(int port)
{

	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
	}
	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(port);
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
	{
		perror("bind");
		exit(3);
	}
	if(listen(sock,5)<0)
	{
		perror("listen");
		exit(4);
	}
	return sock;
}
static pthread_pool_t*pool=NULL;
void handler(int arg)
{
	pthreadpool_destory(pool);
	printf("process exit\n");
	exit(0);
}

int main(int argc,char*argv[])
{
	//	if(argc!=2)
	//	{
	//		printf("argc=%d\n",argc);
	//		printf("usage:[%s] [%s]\n",argv[0],"port");
	//	//	exit(1);
	//	}	
	struct sigaction act;
	act.sa_handler=handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGINT,&act,NULL);
	//创建一个监听套接字
	int listen_socket=startup(9999);
	////创建一个线程池
	pool=pthreadpool_create();
	//任务封装
	task_t task;
	task.function=handle;
	struct sockaddr_in clientaddr;
	size_t len =sizeof(clientaddr);
	//////创建一个epool句柄
//	int epfd=epoll_create(MAX_EVENTS);
//	struct epoll_event event;
//	event.events=EPOLLIN;
//	event.data.fd=listen_socket;
//	struct epoll_event retevents[MAX_EVENTS];
//	epoll_ctl(epfd,EPOLL_CTL_ADD,listen_socket,&event);
//	for(;;){
//		int count = epoll_wait(epfd,retevents,MAX_EVENTS,-1);
//		int i=0;
//		printf("read_count=%d\n",count);
//		for(;i<count;i++)
//		{
//			if(!retevents[i].events&EPOLLIN)
//				continue;
//			if(retevents[i].data.fd==listen_socket)
//			{
//				int client_socket=accept(listen_socket,(struct sockaddr*)&clientaddr,&len);
//				event.data.fd=client_socket;
//				event.events=EPOLLIN;
//				printf("Get a fd %d\n",retevents[i].data.fd);
//				epoll_ctl(epfd,EPOLL_CTL_ADD,client_socket,&event);
//			}
//			else 
//			{
//				server_t *s=(server_t*)malloc(sizeof(server_t));
//				s->efd=epfd;
//				s->socket=retevents[i].data.fd;
//				task.arg=(void*)s;
//				pthreadpool_addtask(pool,task);
//				printf("add a fd %d\n",retevents[i].data.fd);
//			}
//		}
//
//	}
	for(;;)
	{

		int client_socket=accept(listen_socket,(struct sockaddr*)&clientaddr,&len);

		task.arg=(void*)client_socket;
		printf("Get a fd %d\n",client_socket);
		pthreadpool_addtask(pool,task);
		//printf("Get a fd %d\n",client_socket);
  	//pthread_t  tid;
  	//pthread_create(&tid,NULL,handle,(void*)client_socket);
  	//pthread_detach(tid);pthreadpool_addtask(pool,task);
	}
	return 0;
}
