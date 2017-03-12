/*
 * ftp_file.c
 *
 *  Created on: 2017-3-12
 *      Author: gaokai
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

int list_dir(char * path, char * output){
	DIR * dir = opendir(path);
	if (NULL == dir){
		return -1;
	}

	struct dirent * dir_node = NULL;

	sprintf(output, "Size\tName:\n");
	while ((dir_node = readdir(dir)) != NULL){
		sprintf(output + strlen(output), "%d\t%s\n", dir_node->d_reclen, dir_node->d_name);
	}

	closedir(dir);
	return 0;
}


int get_file_size(char *path){
	struct stat stat_info = {0};
	if (0 > stat(path, &stat_info)){
		perror("failed to get file state");
		return -1;
	}

	return stat_info.st_size;
}
