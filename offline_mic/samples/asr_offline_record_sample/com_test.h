/*************************************************************************************
@company: WHEELTEC (Dongguan) Co., Ltd
@product: 4/6mic
@filename: com_test.h
@brief:
@version:       date:       author:            comments:
@v1.0           22-4-16      choi                com
*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <cjson/cJSON.h>
#include <pthread.h>

int set_opt(int,int,int,unsigned char,int);
int open_port(char*);
int deal_with(unsigned char);
short data_trans(unsigned char, unsigned char);
unsigned char check_sum(int);
