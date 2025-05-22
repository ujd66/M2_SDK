/*************************************************************************************
@company: WHEELTEC (Dongguan) Co., Ltd
@product: 4/6mic
@filename: com_test.c
@brief:
@version:       date:       author:            comments:
@v1.0           22-4-16      choi                com
*************************************************************************************/

#include "com_test.h"
#include "record.h"

#define FRAME_HEADER      0XA5
#define USER_ID      0X01

unsigned char Receive_Data[1024] = {0};
int angle_int = 0;
int if_awake = 0;


int deal_with(unsigned char buffer)
{
	static int count=0, frame_len=0, msg_id=0;
	Receive_Data[count] = buffer;
    if(Receive_Data[0] != FRAME_HEADER || (count == 1 && Receive_Data[1] != USER_ID))  //frame header and user id
      count = 0,frame_len = 0, msg_id = 0;
    else 
      count++;
	if (count == 7){  //length and msg id
      msg_id = data_trans(Receive_Data[6], Receive_Data[5]);
      frame_len = data_trans(Receive_Data[4], Receive_Data[3]) + 7 + 1;
	}
	if(count == frame_len){
		unsigned char str[1024] = {0};
		cJSON *json1;
		cJSON *json2;
		char  *out;
		cJSON *content;
		cJSON *info;
		cJSON *ivw;
		cJSON *angle;
		switch(Receive_Data[2]){
			case 0X01:
			/*
				if(check_sum(frame_len-1) == Receive_Data[frame_len-1]){
					for(int i=0; i<frame_len; i++){
						printf("%x ", Receive_Data[i]);
					}
					printf("\n");
				}
				else{
					printf("check failed !\n");
				}
				*/
				break;
			
			case 0X04:
				if(check_sum(frame_len-1) == Receive_Data[frame_len-1]){
					if_awake = 1;
                    // record_finish = 1; // Removed to prevent premature session end
					for(int i=0; i<frame_len-8; i++){
						str[i] = Receive_Data[i+7];
					}
					//printf("%s\n", str);
					//printf("check = %x \n", check_sum(frame_len-1));
					json1 = cJSON_Parse(str);
					//out = cJSON_Print(json1);
					content = cJSON_GetObjectItem(json1,"content");
					info = cJSON_GetObjectItemCaseSensitive(content,"info");
					json2 = cJSON_Parse(info->valuestring);
					out = cJSON_Print(json2);
					//printf("%s\n", out);
					ivw = cJSON_GetObjectItem(json2,"ivw");
					angle = cJSON_GetObjectItem(ivw,"angle");
					angle_int = angle->valueint;
					//printf("angle : [%d]\n",angle_int);
					cJSON_Delete(json1);
					cJSON_Delete(json2);
				}
				else{
					printf("check failed !\n");
				}
				break;
				
			default:
				break;
		}
		
		count = 0,frame_len = 0, msg_id = 0;
		memset(Receive_Data, 0, 1024);
	}
	return 0;
}

unsigned char check_sum(int count_num)
{
	unsigned char check_sum = 0;
	for(int i=0; i<count_num; i++){
		check_sum = check_sum + Receive_Data[i];
	}
	return ~check_sum+1;
}


short data_trans(unsigned char data_high, unsigned char data_low)
{
	short transition_16;
	transition_16 = 0;
	transition_16 |=  data_high<<8;   
	transition_16 |=  data_low;
	return transition_16;
}

int open_port(char* uartname)
{
    int fd = open(uartname, O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return(-1);
    }
     /*恢复串口为阻塞状态*/
     if(fcntl(fd, F_SETFL, 0)<0)
     {
            printf("fcntl failed!\n");
     }else{
        //printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
     }
     /*测试是否为终端设备*/
     if(isatty(STDIN_FILENO)==0)
     {
        printf("standard input is not a terminal device\n");
     }
     else
     {
        //printf("isatty success!\n");
     }
     //printf("fd-open=%d\n",fd);
     return fd;
}


int set_opt(int fd,int nSpeed, int nBits, unsigned char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0) {
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    case 921600:
        printf("B921600\n");
        cfsetispeed(&newtio, B921600);
                cfsetospeed(&newtio, B921600);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if( nStop == 1 )
        newtio.c_cflag &=  ~CSTOPB;
    else if ( nStop == 2 )
    newtio.c_cflag |=  CSTOPB;
    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
  //printf("set done!\n\r");
    return 0;
}
