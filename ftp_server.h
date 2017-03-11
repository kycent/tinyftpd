/*
 * ftp_server.h
 *
 *  Created on: 2017-3-11
 *      Author: gaokai
 */

#ifndef FTP_SERVER_H_
#define FTP_SERVER_H_

#define FTP_PATH_NAME_MAX_LEN (256)

enum FTP_DATA_CHANNEL_STAT{
	FTP_DATA_CHANNEL_IDLE = 0,
	FTP_DATA_CHANNEL_RUNNING = 1,
	FTP_DATA_CHANNEL_UNAVAILABLE = 2,
};

enum FTP_TRANS_MODE{
	FTP_TRANS_PORT = 0,
	FTP_TRANS_PASSIVE = 1,
};

struct server{
	unsigned short port;
	unsigned short backlog;
	int sockfd;
	char * root_path;
	char * bind_ip;
};

struct response{
	void * pMsg;
	size_t len;
};

#define IS_CMD(input, cmd) (0 == strncmp(input, cmd, strlen(cmd)))

#define FTP_ROOT_DIR "/home/gaokai/ftpd"

struct data_connection{
	int fd;
	int mode; //see FTP_TRANS_MODE
	int status; //see FTP_DATA_CHANNEL_STAT
};

struct client{
	int fd;
	pthread_t tid;
	struct data_connection data_conn;
};

typedef int (*handle_func)(struct client* client, char * cmd, char * response);

struct ftp_cmd{
	char * cmd;
	handle_func handle_cbk;
};
int handle_request(struct client * client, char * request, char * response);


int parse_port_cmd(char * cmd, struct sockaddr_in * client_addr);

int create_port_mode_data_channel(struct client * client,  struct sockaddr_in * addr);

int push_file(struct client * client, char * cmd);


int handle_cmd_request_user(struct client * client, char * cmd, char * response);

int handle_cmd_request_syst(struct client * client, char * cmd, char * response);

int handle_cmd_request_type(struct client * client, char * cmd, char * response);

int handle_cmd_request_port(struct client * client, char * cmd, char * response);

int handle_cmd_request_retr(struct client * client, char * cmd, char * response);

int handle_cmd_request_stor(struct client * client, char * cmd, char * response);
#endif /* FTP_SERVER_H_ */
