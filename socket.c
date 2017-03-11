#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "ftp_server.h"


void client_thread_main(void * pClient){
	struct client * client_info = (struct client *)pClient;
	char buff[1024];
	char *pucResp = malloc(1024);
	pthread_t   tid = pthread_self();
	client_info->tid = tid;

	send(client_info->fd, "220 welcome\r\n", strlen("220 welcome\r\n") + 1, 0);

	while (1){
		memset(buff, 0, sizeof(buff));
		memset(pucResp, 0, 1024);
		/* get the input msg content */
		int size = recv(client_info->fd, buff, sizeof(buff) - 1, 0);
		if (size < 0){
			perror("Receive content");
			break;
		}


		printf("Recevied msg is: %s", buff);
		if (0 == strncmp(buff, "QUIT", strlen("QUIT"))){
			printf("close connection. [fd=%d,tid:%d]", client_info->fd, tid);
			goto CLOSE;
		}

		handle_request(client_info,buff, pucResp);
		//sprintf(pucResp,"Recevied msg[fd=%d,tid:%d]:%s\r\n",client_info->fd, tid, buff);
		send(client_info->fd, pucResp, strlen(pucResp) + 1, 0);
	}

CLOSE:close(client_info->fd);
	free(pucResp);
	free(pClient);
	return;
}


int serve_forever(struct server * pSrv)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0){
		perror("Failed to create socket.");
		exit(-1);
	}

	//fcntl(fd, F_SETFL, O_NONBLOCK);
	//struct sockaddr addr = {0};
	struct sockaddr_in addr, peer_addr;
	socklen_t peer_addr_size = 0;
	int cfd = 0;
	char buff[1024] = {0};

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pSrv->port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (-1 == bind(fd, (struct sockaddr *)&addr, sizeof(addr))){
		perror("Failed to bind socket.");
		close(fd);
		exit(-1);
	}

	if (-1 == listen(fd, pSrv->backlog)){
		perror("Failed to listen socket");
		close(fd);
		exit(-1);
	}

	while(1){

		peer_addr_size = sizeof(peer_addr);
		cfd = accept(fd, (struct sockaddr *)&peer_addr, &peer_addr_size);
		if (-1 == cfd){
			perror("Accept");
			close(fd);
		}

		struct client *client_info = malloc(sizeof(struct client));
		client_info->fd = cfd;

		pthread_t tid;

		/* create a thread per client */
		pthread_create(&tid, NULL, client_thread_main, client_info);
	}
}


struct server srv = {
		.port = 8001,
		.backlog = 500,
		.bind_ip = "0.0.0.0",
		.root_path = "/home/gaokai/ftpd",
};


int main(){
	serve_forever(&srv);
}
