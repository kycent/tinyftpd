/*
 * util.h
 *
 *  Created on: 2017-3-12
 *      Author: gaokai
 */

#ifndef UTIL_H_
#define UTIL_H_


#define _FILE_OFFSET_BITS 64

#define IS_BLANK_OR_CTRL_CHAR(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')


int trim(char * old_str, char * new_str);

int get_cmd_name(char * request, char * cmd);

int get_cmd_detail_1(char * request, char * name, char * para1);

int get_cmd_detail_2(char * request, char * name, char * para1, char * para2);
#endif /* UTIL_H_ */
