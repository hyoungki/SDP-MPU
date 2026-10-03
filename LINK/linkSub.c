#include	"localLib.h"
#include    "external.h"

extern  ICCP_SOE_DELETE_QUEUE   *iccpDelSOEQ;     // ICCP-HOST Delete SOEQ


/*
*/
int check_ICCP_deleteSOEQ()
{
    byte    front, rear;
    byte    Qnum;

#if 0    
    /* ICCP-HOST 통신 금지시...	*/
    if(opr->iccpEnbFlag == RESET)
  	{
  		linkCfg->iccpDelSOE = 0;
  		return (0);
  	}
  	  	
    front = iccpDelSOEQ->front & 0xff;
    rear  = iccpDelSOEQ->rear & 0xff;
    
    Qnum = (front - rear + 256) % 256;
    
    linkCfg->iccpDelSOE = Qnum;

    if(Qnum > 0)
    {
        if(opr->dualCpuSts == SET)
        Debug(console, "link> ICCP: delete SOE-Q count ... %d\n", Qnum);
    }
#endif
           
    return (0);
}

/* 
*   LOCAL-SLAVE : DNP HOST 용 정보 초기화 
*   Active MPU로부터 수신한 DEVICE 포인트들에 대한 DNP-HOST 상태정보 Update...
*/
int  update_Device_DNP()
{
    int index;
    int	devNo, devPt;
    int hostid, dnpPoint;
    
    HOST_DCB    *host;
    POINT_BUF   *devPoint;
    POINT_BUF   *diptBuf;
    SDP_DEVICE	*dev;
    
    /* -------------------------------------------- */
    /*  DEVICE 포인트 정보 Update...                */
    /* -------------------------------------------- */
    for(index = INDEX_ALL_DEVICE; index < MAX_DEV_POINT; index++)
    {
        devPoint = (POINT_BUF *)   devPtBuf[index];     // SDP 포인트 Config 정보   
        if(devPoint->config != SET) continue;

		/* ------------------------------------ */
        /* ICCP TIME-SYNC 포인트 .... skip      */
        /* INDEX_CNTR_CHANGE 포인트 .... skip      */
        /* ------------------------------------ */
        if(index == INDEX_SDP_TIMESYNC) continue;       // SDP 시각동기 명령
        if(index == INDEX_CNTR_CHANGE) continue;        // SDP 이중화 절체

        devPoint->oldsts = devPoint->status; 

        /* 상태포인트 정보...추출 */
      	devNo = devPoint->devNo - 1;
      	devPt = devPoint->devPt - 1;
      	
      	if((devNo < 0) || (devNo >= MAX_DEVICE))		continue;
   		if((devPt < 0) || (devPt >= MAX_DEV_DI_POINT))	continue;
      		
      	dev      = (SDP_DEVICE *) deviceCFG[devNo];
      	diptBuf  = (POINT_BUF *) &dev->diPtBuf[devPt];

      	if(diptBuf->config != SET)	continue;
		diptBuf->status = devPoint->status;
		
        /* ------------------------------------ */ 
        /* DNP HOST Data Assign...              */
        /*  => DNP-HOST 별 상태포인트 정보 저장 */
        /* ------------------------------------ */ 
        if(opr->dnpHostEnb == SET)
        {
	        /* ------------------------------------ */
   	     	/* DNP HOST : Device 포인트 Update ...  */
        	/* ------------------------------------ */
	        for(hostid=0; hostid < MAX_HOST; hostid++)
    	    {
        	    host = (HOST_DCB *) hostDCB[hostid];
	            if(host->hostDualMode == HOST_NOT_USE)	continue;
    	      	if(host->hostProtocol != HOST_DNP)      continue;  
        
        	    dnpPoint = devPoint->hostIndex[hostid] - 1;         // SDP POINT : 상위 호스트#1 Index 번호 [1..1024]
            	if((dnpPoint < 0) || (dnpPoint >= MAX_DNP_DI_POINT))    continue;         

	            /* DNP HOST - 0: 이상, 1: 정상 */
    	        if(devPoint->status == 1)   host->sts_pointData[dnpPoint] = PT_FLAG_STS_ON  | PT_FLAG_ONLINE;
        	    else                        host->sts_pointData[dnpPoint] = PT_FLAG_STS_OFF | PT_FLAG_ONLINE;        
        	}
		}
		
    }
 
    return (0);
}

            
