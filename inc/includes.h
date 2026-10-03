
#ifndef __INCLUDES_H_
#define __INCLUDES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdarg.h>
#include <getopt.h>
#include <error.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <time.h>
#include <assert.h>
#include <signal.h>



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <syslog.h>
#include <termios.h>
#include <pthread.h>


#include <linux/rtc.h>

#include 	<sys/signal.h>
#include    <sys/mman.h>
#include 	<math.h>

#include "localLib.h"

#include "tk_net_lib.h"

typedef   unsigned char  UINT8 ;
typedef   unsigned  int UINT32;
typedef   unsigned  short int UINT16;

typedef   unsigned char  U8 ;
typedef   char      INT8 ;
typedef   unsigned  int U32;
typedef   int INT32;
typedef   short  int INT16;

#define TK_ERROR -1
#define TK_OK     0



#define TK_TIMEOUT   -2 
#define TK_SYS_ERR   -3
#define TK_SOCKET_EOF 0 // 2025-10-10 ¿ÀÈÄ 2:04:27 



#define TK_DNP_PROTOCOL_ERR  -5
#define TK_DNP_LINK_HEADER_SIZE  10
#define TK_DNP_BROADCAST   0x1FF


#define TK_AI_ERR  -6



#define  ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))

//#ifdef _PROJECT_
//#include "project.h"
//#endif 

#endif
