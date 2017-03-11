/*
 * ftp_protocol.c

 *
 *  Created on: 2017-3-11
 *      Author: gaokai
 */

#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ftp_server.h"


struct ftp_cmd cmds[] = {
	{"USER", handle_cmd_request_user},
	{"SYST", handle_cmd_request_syst},
	{"TYPE", handle_cmd_request_type},
	{"PORT", handle_cmd_request_port},
	{"RETR", handle_cmd_request_retr},
	{"STOR", handle_cmd_request_stor},
};


int handle_cmd_request_user(struct client * client, char * cmd, char * response)
{
	sprintf(response, "230 login successfully\r\n");
	return 0;
}

int handle_cmd_request_syst(struct client * client, char * cmd, char * response)
{
	sprintf(response, "UNIX Type: L8\r\n");
	return 0;
}

int handle_cmd_request_type(struct client * client, char * cmd, char * response)
{
	sprintf(response, "200 type set to binary\r\n");
	return 0;
}

int handle_cmd_request_port(struct client * client, char * cmd, char * response)
{
	struct sockaddr_in addr;
	if (-1 == parse_port_cmd(cmd, &addr)){
		sprintf(response, "451 Internal error on server\r\n");
		return 0;
	}
	if (-1 == create_port_mode_data_channel(client,&addr)){
		sprintf(response, "451 Internal error on server\r\n");
		return 0;
	}
	sprintf(response, "220 successfully\r\n");
	return 0;
}

int handle_cmd_request_retr(struct client * client, char * cmd, char * response)
{
	if (-1 == push_file(client, cmd)){
		sprintf(response, "450 Invalid file.\r\n");
		return 0;
	}
	sprintf(response, "226 finish transferred file.\r\n");
	return 0;
}

int handle_cmd_request_stor(struct client * client, char * cmd, char * response)
{
	if (-1 == store_file(client, cmd)){
		sprintf(response, "450 Invalid file.\r\n");
		return 0;
	}
	sprintf(response, "226 finish transferred file.\r\n");
	return 0;
}

int parse_port_cmd(char * cmd, struct sockaddr_in * client_addr)
{
	//char strPort[32] = {0};
	char strIp[32] = {0};
	if(!IS_CMD(cmd, "PORT")){
		return -1;
	}

	cmd += strlen("PORT");

	while(*cmd == ' ' && *cmd != '\0'){
		cmd++;
	}

	if (*cmd == '\0'){
		return -1;
	}

	/* the left format is 127,0,0,1,1234,56 */
	int comma_cnt = 0; //the count of comma
	int cursor = 0; //loop cursor
	int seperator = 0; //seperator of ip and port
	int port1_str_index = 0;
	int port2_str_index = 0;
	char str_port1[16] = {0};
	char str_port2[16] = {0};
	while (cursor < strlen(cmd)){
		if (cmd[cursor] == ','){
			comma_cnt++;
			if (comma_cnt == 4){
				seperator = cursor;
			}
			if (comma_cnt < 4){
				cmd[cursor] = '.';
			}
		}
		else{
			if (comma_cnt == 4 && cmd[cursor] >= '0' && cmd[cursor] <= '9'){
				str_port1[port1_str_index++] = cmd[cursor];
			}
			if (comma_cnt == 5 && cmd[cursor] >= '0' && cmd[cursor] <= '9'){
				str_port2[port2_str_index++] = cmd[cursor];
			}
		}
		cursor++;
	}

	if (comma_cnt != 5){
		return -1;
	}

	int port1 = atoi(str_port1);
	int port2 = atoi(str_port2);

	int port = port1 * 256 + port2;

	strncpy(strIp, cmd, seperator);

	printf("parsed client address is %s:%d", strIp, port);

	memset(client_addr, 0, sizeof(struct sockaddr_in));
	client_addr->sin_family = AF_INET;
	client_addr->sin_port = htons(port);
	client_addr->sin_addr.s_addr = inet_addr(strIp);
	return 0;
}


int create_port_mode_data_channel(struct client * client,  struct sockaddr_in * addr){
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0){
		perror("Failed to create socket.");
		client->data_conn.status = FTP_DATA_CHANNEL_UNAVAILABLE;
		exit(-1);
	}

	/*  connect to the client address and port */
	if (-1 == connect(fd, (struct sockaddr *)addr, sizeof(struct sockaddr_in))){
		perror("failed to connect the client");
		close(fd);
		client->data_conn.status = FTP_DATA_CHANNEL_UNAVAILABLE;
		return -1;
	}

	client->data_conn.fd = fd;
	client->data_conn.mode = 0; //port
	client->data_conn.status = FTP_DATA_CHANNEL_IDLE; //idle

	return 0;
}


int push_file(struct client * client, char * cmd){
	if (client->data_conn.status != 0){
		return -1;
	}

	if (!(IS_CMD(cmd, "RETR"))){
		return -1;
	}

	client->data_conn.status = FTP_DATA_CHANNEL_RUNNING;

	/* get the filename */
	cmd += strlen("RETR");
	while(*cmd == ' '){
		cmd++;
	}

	int end_index = 0;
	while(cmd[end_index] != ' ' && cmd[end_index] != '\r' &&
			cmd[end_index] != '\n' &&
			cmd[end_index] != '\0'){
		end_index++;
	}

	char file_name[256] = {0};
	char file_basename[128] = {0};
	strncpy(file_basename, cmd, end_index);

	sprintf(file_name ,"%s/%s", FTP_ROOT_DIR, file_basename);

	char * str_ctrl_notice_msg = "150 opened data channel connection.\r\n";
	send(client->fd, str_ctrl_notice_msg, strlen(str_ctrl_notice_msg) + 1, 0);

	char  * buff = malloc(10240);
	FILE * f = fopen(file_name, "r+");
	if (NULL == f){
		perror("Failed to open file");
		close(client->data_conn.fd);
		return -1;
	}

	memset(buff,0,10240);
	size_t read_bytes = fread(buff, 1, 10240, f);
	while (read_bytes != 0){
		send(client->data_conn.fd, buff, read_bytes, 0);

		usleep(20);
		//memset(buff,0,10240);
		read_bytes = fread(buff, 1, 10240, f);
	}

	free(buff);
	client->data_conn.status = FTP_DATA_CHANNEL_UNAVAILABLE;
	close(client->data_conn.fd);
	fclose(f);
	return 0;
}


int store_file(struct client * client, char * cmd){
	if (client->data_conn.status != 0){
		return -1;
	}

	if (!(IS_CMD(cmd, "STOR"))){
		return -1;
	}

	client->data_conn.status = FTP_DATA_CHANNEL_RUNNING;

	/* get the filename */
	cmd += strlen("STOR");
	while(*cmd == ' '){
		cmd++;
	}

	int end_index = strlen(cmd);
	int last_index = 0;
	while(cmd[end_index] != ' '){
		end_index--;
		if (last_index == 0 && cmd[end_index] != '\r' && cmd[end_index] != '\n'){
			last_index = end_index;
		}
	}

	if (last_index == 0)
	{
		return -1;
	}

	char file_name[256] = {0};
	char file_basename[128] = {0};
	strncpy(file_basename, cmd + end_index + 1, last_index - end_index);

	sprintf(file_name ,"%s/%s", FTP_ROOT_DIR, file_basename);

	char * str_ctrl_notice_msg = "150 opened data channel connection.\r\n";
	send(client->fd, str_ctrl_notice_msg, strlen(str_ctrl_notice_msg) + 1, 0);

	char * buff = malloc(10240);
	FILE * f = fopen(file_name, "w+");
	if (NULL == f){
		perror("Failed to open file");
		close(client->data_conn.fd);
		return -1;
	}

	memset(buff,0,10240);
	size_t read_bytes = recv(client->data_conn.fd, buff, 10240, 0);
	while (read_bytes != 0){
		//send(client->data_conn.fd, buff, read_bytes, 0);
		fwrite(buff, 1, read_bytes, f);
		fflush(f);

		usleep(20);
		//memset(buff,0,10240);
		read_bytes = recv(client->data_conn.fd, buff, 10240, 0);
	}

	free(buff);
	client->data_conn.status = FTP_DATA_CHANNEL_UNAVAILABLE;
	close(client->data_conn.fd);
	fclose(f);
	return 0;
}



int handle_request(struct client * client, char * request, char * response)
{
	int cmd_size = sizeof(cmds)/sizeof(cmds[0]);

	int loop = 0;

	for (loop = 0; loop < cmd_size; loop++)
	{
		if (IS_CMD(request, cmds[loop].cmd))
		{
			if (NULL == cmds[loop].handle_cbk)
			{
				sprintf(response, "500 unimplemented command");
				return 0;
			}
			if (0 != cmds[loop].handle_cbk(client, request, response))
			{
				printf("failed to run cmd %s", request);
				return 0;
			}
			return 0;
		}
	}

	sprintf(response, "500 unimplemented command");
	return 0;

}
