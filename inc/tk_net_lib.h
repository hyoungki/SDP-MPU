#ifndef TK_NET_LIB_H_
#define TK_NET_LIB_H_

#ifndef TK_NET_LIB_C
#define TK_EXTERN extern
#else
#define TK_EXTERN 
#endif 



TK_EXTERN void tk_print_curTime(char *buffer,int size) ;
TK_EXTERN void tk_print_curTime_onthefly(void);

TK_EXTERN int  elapsed_msec(struct timeval past_time) ;
TK_EXTERN void  msleep( int mSec) ;

TK_EXTERN int tk_net_readn (int fd, char *buf, int size , int timeout);
TK_EXTERN int tk_net_read(int fd, char *buf, int size , int tm) ;

TK_EXTERN  void  msleep( int mSec) ;
TK_EXTERN  int tk_connect ( int domain, int type, int protocol,
                            const struct sockaddr *addr, socklen_t addrlen,
                             int timeout_msec, int max_retries, int retry_delay_msec,
                             int *sockfd_out) ;


TK_EXTERN int tk_connect_once (int domain, int type, int protocol,
                              const struct sockaddr *addr, socklen_t addrlen,
                              int timeout_msec,  int *sockfd_out) ;



TK_EXTERN int tk_readn (int fd, char *buf, int size , int timeout);
TK_EXTERN int tk_read(int fd, char *buf, int size , int tm) ;

TK_EXTERN  int tk_accept_with_timeout( int listen_fd, 
                         int timeout_ms,   // accpet 대기 시간
                         struct sockaddr_in *cli_addr_out, // 상대편 주소 담는 곳 
                         int debug    // 개발시 debug enable 
                         ) ;
                         


//TK_EXTERN  int open_serial(TTY_DESC *tp) ;
#endif
