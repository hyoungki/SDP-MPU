/*
 * ==============================================================
 * System TARGET : ACE Control SDP-2000 Ver 1.0   
 * Target CPU    : MPC8248, VME6U
 * Main Factors  : 스마트급전용 SDP 자장치 
 *     - MPU 이중화 구성
 *     - ESOP/SIO 연계 : 전력감시, 원격진단, 전력품질, 고장점 등
 *     - 통합 시뮬레이터 : 2013/07/08 V8.2 적용 
 * --------------------------------------------------------------
 * System DESIGN : SANE-SYSTEM   .... by  Lee Ho-Sang
 * Initial-DATA  : 2016,03,25
 * Last Updated  : 2016,03,25
 * ==============================================================
 */
extern  int    	ExecCommand(char *format, ...);
extern  int		ShmCheck(SHM_DESC *sp);
extern  int		ShmCreate(SHM_DESC *sp);
extern  void	ShmDelete(SHM_DESC *sp);
extern  void	ShmDetach(SHM_DESC *sp);

extern  void    memcpy8248(volatile byte *des, volatile byte *src, int size);
extern  void    bzero8248(byte *buf, int size);

extern  void	UpdateProcessInfo(TASK_INFO *prcPtr, int state, pid_t pid, int arg);
extern  int    	StartProcess(char *name, char *argument);
extern  void    KillProcess(pid_t pid, int wait);
extern  int     IsProcessActive(int pid);

extern  int     dbFileWrite(OPR_MSG *opr, RTU_DATABASE *localDB);
extern  int     dbFileRead(OPR_MSG *opr, RTU_DATABASE *localDB);

extern  int     vmeChanWrite(int chid, VME_SIODCB *vmeSio, VME_CHAN_DCB *vmeChan, byte *txbuf, int count);
extern  int     vmeChanRead(int chid, VME_SIODCB *vmeSio, VME_CHAN_DCB *vmeChan, byte *rxbuf, int reqCount);

extern  word    B013_ckdnpcrc(byte *buff, int count);
extern  word    B013_mkdnpcrc( byte *buff,  int count);
extern  void    DumpDNP_snd(CONSOLE_INFO *debug, ...);
extern  void    DumpDNP_rcv(CONSOLE_INFO *debug, ...);

extern  int     tkWriteTCP(int sockfd, byte *txBuffer, int size,int timeout);
extern  int     TKread(int fd, char *buf, int size , int tm);
extern  int     TKreadn (int fd, char *buf, int size , int timeout);

extern  void    logEvent_MPU(SHM_MEMORY *pShm, int logid, int ioid, int point, int state, int hostid, struct timeval *soeTime);

//extern  int     LogFile_MPU (int code, char *outstr, int osize);
extern	int  	LogFile_MPU (SHM_MEMORY *shmPtr, int code, char *outstr, int osize);
extern  int     LogFile_CONSOLE (char *outstr, int osize);

extern  void    DumpBuff(CONSOLE_INFO *debug, ...);
extern  void    Debug(CONSOLE_INFO *debug, ...);
extern  void    prndat(const char *msg, char *str, int  len);

extern  void    PortClose(TTY_DESC *tp);
extern  void    rtsControl(TTY_DESC *tp, int   mode);
extern  int     checkCTS(TTY_DESC *tp);
extern  int     PortAttribute(TTY_DESC *tp);
extern  int     PortOpen(TTY_DESC *tp);
extern  int     PortWrite(TTY_DESC *ttyDesc, byte *buffer, int length);
extern  int     PortRead(TTY_DESC *tp, byte *bfptr, int count);
extern  int	    asyncRead( OPR_MSG *opr, TTY_DESC	*tp,  byte *bfptr, int count);
extern  int     harrisPortInitial(TTY_DESC *ttyDesc);
extern  int     PortInitial(TTY_DESC *ttyDesc);

extern  int     FileOpen(FILE_DESC *fp);
extern  int     FileWrite(FILE_DESC *fp, char *buffer, int length);
extern  int     FileRead(FILE_DESC *fp, char *buffer, int length);
extern  void    FileClose(FILE_DESC *fp);

extern  byte    GenLrc(byte *buffer, int length);
extern  byte    genlrc(byte *buf, int bfcnt);
extern  word    gensum(byte *buf, int bfcnt);
extern  word    genCrc_modbus (byte *ptr, int len);
extern  word    GenCrc(char *buffer, int length);


extern  int    makeDnpTimeRTC( PDPDTIME_DATE_TIME   *pDateTime, PDPDTIME_MS_SINCE_70 *pMsSince70);
extern  int    makeDnpTimeByte( PDPDTIME_MS_SINCE_70 *pMsSince70, PDPDTIME_DATE_TIME   *pDateTime);

/* lib_scanShm.c */
extern  int     curStore_BIT_MPU( SHM_MEMORY *shmPtr, int devid, byte *stsbuf, int dataSize);
extern  int     storeAI_FLOAT_MPU(SHM_MEMORY *shmPtr, int devid, int point, float fdata, byte *lowdata);

extern  int     store_MPU_SOE(SHM_MEMORY *shmPtr, MPU_SOEQ_ENTRY *rcvEvent);
extern  int     store_MPU_Device(SHM_MEMORY *shmPtr, MPU_SOEQ_ENTRY *rcvEvent);

extern  int     update_ICCP_info( SHM_MEMORY *shmPtr, POINT_BUF *ptBuf, float value, int online);

extern  int     set_Online_Device(SHM_MEMORY *shmPtr, MPU_SOEQ_ENTRY *event);
extern  int     set_Offline_Device(SHM_MEMORY *shmPtr, MPU_SOEQ_ENTRY *event);
extern  int     Create_Device_SOE(SHM_MEMORY *shmPtr, MPU_SOEQ_ENTRY *event);
//extern  int     rcv_Device_SOE(SHM_MEMORY *shmPtr, MPU_SOEQ_ENTRY *rcvEvent);

extern  int     controlInfo_MPU (SHM_MEMORY *shmPtr, int cntrDev, int cntrPoint, int cntrTCF, int cntrPass, struct timeval *ctime);