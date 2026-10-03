/* 
 *****************************************************************************
 * SYSTEM   : ACE-VMECU-NET-V5 (철도청 VMECU : 공통제어부 )
 * FileName : CONFIG.C  
 * File 내용: 데이터베이스 관련 저장 및 초기화 관련 함수
 * Designed : TRATEK ... by LHS
 *****************************************************************************
*/   
#include	"localLib.h"
#include	"external.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//#include <time.h>
//#include <string.h>


extern  MPC860IO_DESC    ledPort;
extern  MPC860IO_DESC    rtcPort;

extern  int	            termExec;

#define DEFAULT_TIMEOUT_SEC   2 

char *default_server_ip="121.190.154.173";

int  ntpdate(char *server_ip);


 
int debug =1; 
 
#define JAN_1970  0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT  (123)

#define NTPFRAC(x) ( 4294*(x) + ( (1981*(x))>>11 ) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC-1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ( (x) * 15.2587890625 )

struct ntptime {
	unsigned int coarse;
	unsigned int fine;
} xtim_time;


//ntptime  xtim_time ;

/*
*
*/
static int set_time(struct ntptime *new)
{
    int retVal;
    time_t  now,new_time  ; 
    struct tm cTime;
    
	/* Traditional Linux way to set the system clock
	 */
	struct timeval tv_set;
	/* it would be even better to subtract half the slop */
	tv_set.tv_sec  = new->coarse - JAN_1970;
	/* divide xmttime.fine by 4294.967296 */
	tv_set.tv_usec = USEC(new->fine);

    now = ( time_t)tv_set.tv_sec ;
		
	if (settimeofday(&tv_set,NULL)<0) 
    {
		perror("settimeofday");
		//exit(1);
        termExec = 0;
        return (0);
	}
	
	if(opr->ntpDebug)
	{
		Debug(console, "==> set time to %lu.%.6lu\n", tv_set.tv_sec, tv_set.tv_usec);
	}
	
    new_time=time(NULL);
    
    //if(opr->ntpDebug)
    {      
        Debug(console, ">> NTP time is %s ", ctime(&now) );
        Debug(console, ">> NEW NTP time is %s", ctime(&new_time) );
        Debug(console, ">> System time is %ld seconds off \n",  (new_time-now));
    }
    
    /* -------------------------------------------- */
    /* 수신 Time Sync 시간 추출                    */
    /* -------------------------------------------- */
    localtime_r (&tv_set.tv_sec, &cTime);
    
    //if(opr->ntpDebug)
    Debug(console, ">> rcv SET-TIME ...[%4d/%02d/%02d-%d-%02d:%02d:%02d]\n", 
        cTime.tm_year + 1900, cTime.tm_mon +1, cTime.tm_mday, cTime.tm_wday, cTime.tm_hour, cTime.tm_min, cTime.tm_sec);
    
    retVal = ioctl(rtcPort.id, RTC_SET_TIME, &cTime);
	
	if (retVal == -1) 
	{
	    if(opr->ntpDebug)
    	Debug(console,"wdt> *ioctl RTC_SET_TIME  %s\n",strerror(errno)) ;
    }
    
    return (1);
 // new_time=time(NULL);



}

int usage(void)
{
	
	printf("Usage  : prog [ip]\r\n");
	//exit(1);
    termExec = 0;
    return (0);
}

#if 0
int main(int argc, char *argv[])
{
    int rc; 
	if ( argc > 1 )
			 rc = ntpdate( argv[1] );
	else 
			rc = ntpdate( default_server_ip);

    if   ( rc < 0 ) 
        printf("Error : no server response\r\n");
    return 0;
}
#endif
 
/*
*/ 
int  ntpdate( char *hostname) 
{
// can be any timing server
// you might have to change the IP if the server is no longer available

    //char *hostname=(char *)"200.20.186.76";
    time_t now ;
    // ntp uses port 123
    int portno=123;
    int maxlen=1024;
    int i;
    // buffer for the socket request
    unsigned char msg[48]={010,0,0,0,0,0,0,0,0};
    // buffer for the reply
    unsigned long buf[maxlen];
    //struct in_addr ipaddr;
    struct protoent *proto; //
    struct sockaddr_in server_addr;
    int s; // socket
    //long tmit; // the time -- This is a time_t sort of
 
 
    fd_set fds;
    struct timeval to;
 
    // open a UDP socket
    proto=getprotobyname("udp");
    s=socket(PF_INET, SOCK_DGRAM, proto->p_proto);
 
    //here you can convert hostname to ipaddress if needed
    //$ipaddr = inet_aton($HOSTNAME);
 
    memset( &server_addr, 0, sizeof( server_addr ));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);
    server_addr.sin_port=htons(portno);
 
    /*
    * build a message. Our message is all zeros except for a one in the
    * protocol version field
    * msg[] in binary is 00 001 000 00000000
    * it should be a total of 48 bytes long
    */
 
    // send the data to the timing server
    i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
    // get the data back
    struct sockaddr saddr;
    socklen_t saddr_l = sizeof (saddr);
    // here we wait for the reply and fill it into our buffer

    now =time(NULL); 
    
    if(opr->ntpDebug)
    Debug(console, "ntp>> Current time is %s \r\n", ctime(&now) );

    to.tv_sec=1;
    to.tv_usec=0;
	
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    i = select(s+1, &fds, NULL, NULL, &to);  /* Wait on read or error */

    if(i > 0) 
    {
        i=recvfrom(s,buf,48,0,&saddr,&saddr_l);
    }
    else if ( i == 0)
    {
        if(opr->ntpDebug)
        Debug(console, "ntp> timeout ...\r\n");
        return -1 ; 
    }    
    else 
    {
        if(opr->ntpDebug)
        Debug(console, "ntp> select error ...\r\n");
        return -1 ;             
    }    

    //We get 12 long words back in Network order
 
    /*
    * The high word of transmit time is the 4th word we get back
    * tmit is the time in seconds not accounting for network delays which
    * should be way less than a second if this is a local NTP server
    */
 

    xtim_time.coarse  = buf[10];
    xtim_time.fine    = buf[11]; 
 
    set_time ( &xtim_time);
 

    
    return 0;

}
 
 
/*
*   NTP 서버로부터 시각동기화 호출...
*/
int     get_NTP_Info()
{
    //int     i;
    int     retVal;
    
    MPU_NET_ENTRY   *serverNet;
    
    /* ------------------------------------------------ */
    /*  MPU-A/B 간 링크용 Network ... eth0 사용         */
    /* ------------------------------------------------ */
    if(opr->cpuMode == MPU_A)   serverNet = (MPU_NET_ENTRY *) &mpuCFG->slave_netCfg[0];
    else                        serverNet = (MPU_NET_ENTRY *) &mpuCFG->master_netCfg[0];
                
    if(opr->ntpDebug)
    Debug(console, "==> NTP Sync-REQ... %s\n", serverNet->ipAddr);
    retVal = ntpdate( serverNet->ipAddr);
    
    if(retVal < 0)
    {
        Debug(console, "ntp> *NTP Error : no server response \n"); 
        return (0);
    }
    
    return (1);      
}   