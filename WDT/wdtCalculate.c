
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


#include	"localLib.h"
#include    "external.h"
#

typedef unsigned char   bool;           // wdtParser.cpp 참조 

extern bool 	parserCalculate(const char *, double *, bool *, int);


/*
*   SDP 연산포인트 처리...
*/
void    sdp_Calculate_Point()
{
    int devNo, devPt;
    int point;
    int hostid, dnpIndex;
    
    SDP_DEVICE      *dev;
    CAL_POINT_BUF   *calPt;
    POINT_BUF       *ptBuf;
    HOST_DCB        *host;
    DNP_ANA_INPUT   *anaPoint;
    
    bool    online;
	double	value;
	byte    status;
	byte    *lowdata;

    /* -------------------------------------------------------- */
    /*  전체 연산 포인트 대상                                   */
    /* -------------------------------------------------------- */
    for(point=0; point < MAX_CAL_POINT; point++)
    {
        calPt = (CAL_POINT_BUF *) calPtBuf[point];// calPtBuf 이 놈이 공유 메모리를 가리킨다...
        
        /* 포인트 DB 에서 사용영역을 설정... */
        if(calPt->useFlag == RESET) continue;           // CAL 포인트 정의여부
        if(calPt->config == RESET)  continue;           // Matching 포인트 정의여부
            
        if(opr->calDebug == point)  
        Debug(console, "\n----------------------------------------------------\n"); 
    
        /* ------------------------------------------------ */
        /*  연산포인트 : 연산식 계산...                     */
        /* ------------------------------------------------ */
        if (!parserCalculate(calPt->calString, &value, &online, point))
        {   
            if(opr->calDebug == point)    
	            Debug(console, ">> CAL (%02d) : *** Parser failed\n", point+1);    
	        
	        continue;
	    }
	    
	    /* ------------------------------------------------ */
        /*  연산포인트 : 계전기 정보 Uppdate...             */
        /* ------------------------------------------------ */
	    if(calPt->pointType == CAL_STATUS)           // STATUS
	    {
	        status = (int ) value;
	        if(opr->calDebug == point)  
	        Debug(console, ">> CAL-STATUS(%02d) : online=%d, status=%d\n", point+1, online, status); 
	        
	        devNo = calPt->devNo - 1;
	        devPt = calPt->devPt - 1;
	        
	        if((devNo < 0) || (devNo >= MAX_DEVICE))        continue;
	        if((devPt < 0) || (devPt >= MAX_DEV_DI_POINT))  continue;    
	             
	        /* ------------------------------------ */
	        /* 연산포인트 계전기 정보 Update...     */
	        /* ------------------------------------ */
	        dev   = (SDP_DEVICE *) deviceCFG[calPt->devNo-1];
	        ptBuf = (POINT_BUF *) &dev->diPtBuf[calPt->devPt-1];
	        ptBuf->status = status;
	        
	        gettimeofday(&ptBuf->updateTime, NULL);       //상태정보 갱신시간 추출...timeval  Form          
	        
	        /* ------------------------------------ */ 
            /* DNP HOST Data Assign...              */
            /*  => DNP-HOST 별 상태포인트 정보 저장 */
            /* ------------------------------------ */ 
            if(opr->dnpHostEnb == SET)
            {
                for(hostid = 0; hostid < MAX_HOST; hostid++)
                {
                    host = (HOST_DCB *) &shmPtr->hostDCB[hostid];
                
                    if(host->hostDualMode == HOST_NOT_USE)	continue;
                    if(host->hostProtocol != HOST_DNP)      continue;    
                        
                    dnpIndex = ptBuf->hostIndex[hostid];             // 1'st DNP-HOST
                
                    if((dnpIndex > 0) && (dnpIndex <= MAX_DNP_DI_POINT))
                    {
                        dnpIndex = dnpIndex - 1;
                    
                        /* ---------------------------------------- */
                        /* HOST 별 상태포인트 정보                  */
                        /*  MULTI Point 처리 :                      */
                        /*    - 상태에 따라서 복합처리              */
                        /* ---------------------------------------- */
                        if (ptBuf->status)    
                        {
                            host->sts_pointData[dnpIndex] = PT_FLAG_STS_ON | PT_FLAG_ONLINE;
                        }
                        else                    
                        {
                            host->sts_pointData[dnpIndex] = PT_FLAG_STS_OFF| PT_FLAG_ONLINE;
                        }
                    
                        /* Device Offline : 상태정보 표기 */
                        if(dev->online != 1)   host->sts_pointData[dnpIndex] &= (~PT_FLAG_ONLINE);      // 포인트 상태를 Offline으로...
                    }
                }                           
            }
            
	    }
	    else if(calPt->pointType == 2)      // ANALOG
	    {
	        if(opr->calDebug == point)  
                Debug(console, ">> CAL-ANALOG(%02d) : online=%d, value=%4.2f\n", point+1, online, value); 
	        
	        /* ------------------------------------ */
	        /* 연산포인트 계전기 정보 Update...     */
	        /* ------------------------------------ */
	        dev   = (SDP_DEVICE *) deviceCFG[calPt->devNo-1];
	        ptBuf = (POINT_BUF *) &dev->aiPtBuf[calPt->devPt-1];
	        ptBuf->floatData = value;
	        
	        gettimeofday(&ptBuf->updateTime, NULL);       //상태정보 갱신시간 추출...timeval  Form      
	        
	        /* ------------------------------------ */ 
            /* DNP HOST Data Assign...              */
            /*  => DNP-HOST 별 계측포인트 정보 저장 */
            /* ------------------------------------ */
	        if(opr->dnpHostEnb == SET)
            {	            
                for(hostid = 0; hostid < MAX_HOST; hostid++)
                {
                    host = (HOST_DCB *) &shmPtr->hostDCB[hostid];

                    if(host->hostDualMode == HOST_NOT_USE)	continue;
                    if(host->hostProtocol != HOST_DNP)      continue;    
                        
            		/* -------------------------------------------- */
                	/*  DNP Data Formattiong...           */
                	/* -------------------------------------------- */
                    dnpIndex = ptBuf->hostIndex[hostid];             // 1'st DNP-HOST

                    if((dnpIndex > 0) && (dnpIndex <= MAX_DNP_AI_POINT))
                    {
                        dnpIndex = dnpIndex - 1;
                        anaPoint = (DNP_ANA_INPUT *) &host->ana_pointData[dnpIndex];

                        anaPoint->flag = PT_FLAG_ONLINE;

                        /* Device Offline : 아날로그 정보 표기 */
                        if(dev->online != 1)    anaPoint->flag = 0;
                        else                    anaPoint->flag = PT_FLAG_ONLINE;        

//hkkim 2026-02-07 오후 4:28:30
                        if ( opr->endian == ROAD_BIG_ENDIAN)       // 사장님                 
                        {
                            anaPoint->pointData = (int) ptBuf->floatData;                         
                            // 아래는 ptBuf를 host로 옮기는데 . . host에서 lowdata[0은] LSB
                            lowdata = (byte *) &ptBuf->floatData;                                                                        
                            anaPoint->lowdata[0] = lowdata[3];
                            anaPoint->lowdata[1] = lowdata[2];
                            anaPoint->lowdata[2] = lowdata[1];
                            anaPoint->lowdata[3] = lowdata[0];
                        }
                        else
                        {
                             anaPoint->pointData = (int) ptBuf->floatData;                         

                            // 아래는 ptBuf를 host로 옮기는데 . . host에서 lowdata[0은] LSB
                            lowdata = (byte *) &ptBuf->floatData; 
#if 0                                                                       
                            anaPoint->lowdata[0] = lowdata[0];
                            anaPoint->lowdata[1] = lowdata[1];
                            anaPoint->lowdata[2] = lowdata[2];
                            anaPoint->lowdata[3] = lowdata[3];                           
#else 
                            memcpy(anaPoint->lowdata, lowdata, 4) ;
#endif                             
                        }    
                        
                    
                    
                    
                    }
                }     
            }
	    }
    }
    
#if 0	
    printf("------------------------------------\n");
    if (!parserCalculate("([M01:S001] + 34) * 56", &value, &online))
	printf("Parser failed\n");
		
	printf(" value = %f, online = %d\n", value, online);
		
	if (!parserCalculate("([M03:A001] + 100) * 2", &value, &online))
	printf("Parser failed\n");
		
	printf(" value = %f, online = %d\n", value, online);
#endif
		
		    
}

