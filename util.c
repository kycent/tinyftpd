/*

 * util.c
 *
 *  Created on: 2017-3-12
 *      Author: gaokai
 */

#include <string.h>
#include "util.h"

int trim(char * old_str, char * new_str)
{
	while(IS_BLANK_OR_CTRL_CHAR(*old_str)){
		old_str++;
	}

	int tail = strlen(old_str);

	if (tail == 0){
		return 0;
	}

	tail--;
	while(IS_BLANK_OR_CTRL_CHAR(old_str[tail])){
		tail--;
	}

	strncpy(new_str, old_str, tail+1);
	//new_str[tail] = '\0';
	return 0;
}

int get_cmd_name(char * request, char * cmd)
{
	char cmd_temp[128] = {0};
	trim(request, cmd_temp);

	while(*request != ' ' && *request != '\0'){
		request++;
		*cmd = *request;
		cmd++;
	}
	return 0;
}

int get_cmd_detail_1(char * request, char * name, char * para1)
{
	char actemp[256] = {0};
	char * ctemp = actemp;
	trim(request, ctemp);

	while(*ctemp != ' ' && *ctemp != '\0'){
		*name = *ctemp;
		name++;
		ctemp++;
	}

	if (*ctemp == '\0'){
		return -1;
	}

	while(*ctemp == ' ' && *ctemp != '\0'){
		ctemp++;
	}

	if (*ctemp == '\0'){
		return -1;
	}

	while (*ctemp != ' ' && * ctemp != '\0'){
		*para1 = *ctemp;
		para1++;
		ctemp++;
	}

	return 0;
}

int get_cmd_detail_2(char * request, char * name, char * para1, char * para2)
{
	char ctemp[256] = {0};
	trim(request, ctemp);

	while(*request != ' ' && *request != '\0'){
		request++;
		*name = *request;
		name++;
	}

	if (*request == '\0'){
		return -1;
	}

	while(*request == ' ' && *request != '\0'){
		request++;
	}

	if (*request == '\0'){
		return -1;
	}

	while (*request != ' ' && * request != '\0'){
		*para1 = *request;
		para1++;
	}

	if (*request == '\0'){
		return -1;
	}

	while(*request == ' ' && *request != '\0'){
		request++;
	}

	if (*request == '\0'){
		return -1;
	}

	while (*request != ' ' && * request != '\0'){
		*para2 = *request;
		para2++;
	}

	return 0;
}
