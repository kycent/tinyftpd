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


void * client_thread_main(void * pClient){
	struct client * client_info = (struct client *)pClient;
	char buff[1024];
	char *pucResp = malloc(1024);
	pthread_t   tid = pthread_self();
	client_info->tid = tid;

	char welcome_msg[256] = {0};
	sprintf(welcome_msg, "220 %s %s\r\n",server_config.welcome_msg, server_config.version);

	send(client_info->fd, welcome_msg, strlen(welcome_msg) + 1, 0);

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
			printf("close connection. [fd=%d,tid:%d]", client_info->fd, (int)tid);
			goto CLOSE;
		}

		handle_request(client_info,buff, pucResp);
		//sprintf(pucResp,"Recevied msg[fd=%d,tid:%d]:%s\r\n",client_info->fd, tid, buff);
		send(client_info->fd, pucResp, strlen(pucResp) + 1, 0);
	}

CLOSE:close(client_info->fd);
	free(pucResp);
	free(pClient);
	return NULL;
}


int serve_forever(struct server * pSrv)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0){
		perror("Failed to create socket.");
		exit(-1);
	}

	struct sockaddr_in addr, peer_addr;
	socklen_t peer_addr_size = 0;
	int cfd = 0;

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
		strcpy(client_info->pwd, server_config.root_path);

		pthread_t tid;

		/* create a thread per client */
		pthread_create(&tid, NULL, client_thread_main, client_info);
	}
}


struct server server_config = {
		.port = 8009,
		.backlog = 500,
		.bind_ip = "0.0.0.0",
		.root_path = "/home/gaokai/ftpd",
		.welcome_msg = "TinyFtpd written by gaokai",
		.version = "v0.1.1",
};


void print_usage()
{
	printf("Usage:\n"
			"tinyftpd -p <listen_port> -r <root_path_of_ftp_service> -m <max_client_num_same_time>"
			"Example: tinyftpd -p 8000 -r /home/gaokai/ftpd -m 500");
}


int main(int argc, char *argv[]){

	/* read arguments from cli. set the server_config variable */
	/*
	 * usage: -p port -r root_path -m max_client_num
	 * */
	int op = 0;
	char * cli_arg;
	while ((op = getopt(argc, argv, "prm:")) != -1){
		switch(op){
			case 'p':
			{
				cli_arg = optarg;

				server_config.port = atoi(cli_arg);
				break;
			}
			case 'r':
			{
				cli_arg = optarg;
				server_config.root_path = cli_arg;
				break;
			}
			case 'm':
			{
				cli_arg = optarg;
				server_config.backlog = atof(cli_arg);
				break;
			}
			case '?':
			{
				printf("Invalid argment.\r\n");
				print_usage();
				break;
			}
		}
	}

	printf("server config:\nport:%d\nroot_path:%s\nbacklog:%d\nversion:%s\nbind_ip:%s\n",
		server_config.port, server_config.root_path, server_config.backlog, server_config.version, server_config.bind_ip);

	printf("start ftp service...");
	serve_forever(&server_config);
}
