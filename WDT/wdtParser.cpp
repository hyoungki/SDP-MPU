//------------------------------------------------------------------------------
//
// Project:
// Target:		TEST
// Filename:	Parser.cpp
// Version:		1.0
// History:		Date		By		Content
//				----------	------	--------------------------------------------
// 
//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------
#include	"localLib.h"
//#include    "external.h"

#include    "Parser.h"

extern  "C"   { void Debug_CPP(CONSOLE_INFO *debug, ...);};

extern  SDP_DEVICE      *deviceCFG[MAX_DEVICE];         // 계전기/장치- 전자식배전반 (GiPAM, HiMAP...)
extern  OPR_MSG         *opr;
extern  CONSOLE_INFO	*console;

//------------------------------------------------------------------------------
// Local
//------------------------------------------------------------------------------
int		m_cntToken;
int		m_posStack;
char	*m_nextToken;
char	*m_token;

bool	m_online;
double	m_d1;
double	m_d2;
double	m_stack[MAX_CALC_STACK];
string	m_infix;
string	m_postfix;
string	m_odInfix;
string	m_digitSet;
string	m_digitSet1;
string	m_oprSet1;
string	m_oprSet2;
string	m_oprSet12;
string	m_listToken[MAX_TOKEN];
const char	*m_pSeperator;
const char	*m_pOperator;
LIST_STR	m_list;
LIST_STR	m_listPostfix;
//------------------------------------------------------------------------------
// parserInit
//------------------------------------------------------------------------------
extern "C" void parserInit(void)
{
    //printf("=========>>>>>>>  parserInit ....\n");
    
	m_pSeperator = " ";
	m_pOperator = "(+-*/)<>|&!=";
	m_digitSet = STR_DIGIT_SET;
	m_digitSet1 = STR_DIGIT_SET1;
	m_oprSet1 = STR_OPR_SET1;
	m_oprSet2 = STR_OPR_SET2;
	m_oprSet12 = STR_OPR_SET12;
}
//------------------------------------------------------------------------------
// parserAddString
//------------------------------------------------------------------------------
void parserAddString(string *pTarget, string addStr, bool addSpace)
{
	if (addSpace && pTarget->length())
		*pTarget += " ";
	*pTarget += addStr;
}
//------------------------------------------------------------------------------
// parserGetStack
//------------------------------------------------------------------------------
bool parserGetStack(double *pValue)
{
	if (m_posStack <= 0)
		return(false);

	*pValue = m_stack[--m_posStack];
	return(true);
}
//------------------------------------------------------------------------------
// parserDoCalculate
//------------------------------------------------------------------------------
bool parserDoCalculate(void)
{
	if (!parserGetStack(&m_d2))
		return(false);
	if (!parserGetStack(&m_d1))
		return(false);

//	printf("Pop Stack => %d:%f:%f\n", m_posStack, m_d1, m_d2);
	if (strcmp(m_token, "+") == 0)
		m_stack[m_posStack++] = m_d1 + m_d2;
	else if (strcmp(m_token, "-") == 0)
		m_stack[m_posStack++] = m_d1 - m_d2;
	else if (strcmp(m_token, "*") == 0)
		m_stack[m_posStack++] = m_d1 * m_d2;
	else if (strcmp(m_token, "/") == 0)
	{
		if (m_d2 == 0)
			return(false);
		m_stack[m_posStack++] = m_d1 / m_d2;
	}
	else if (strcmp(m_token, "<") == 0)
		m_stack[m_posStack++] = m_d1 < m_d2;
	else if (strcmp(m_token, ">") == 0)
		m_stack[m_posStack++] = m_d1 > m_d2;
	else if (strcmp(m_token, "<=") == 0)
		m_stack[m_posStack++] = m_d1 <= m_d2;
	else if (strcmp(m_token, "||") == 0)
		m_stack[m_posStack++] = m_d1 || m_d2;
	else if (strcmp(m_token, "&&") == 0)
		m_stack[m_posStack++] = m_d1 && m_d2;
	else if (strcmp(m_token, "!=") == 0)
		m_stack[m_posStack++] = m_d1 != m_d2;
	else if (strcmp(m_token, "==") == 0)
		m_stack[m_posStack++] = m_d1 == m_d2;
	else
	{
		printf("Invalid operator [%s]\n", m_token);
		return(false);
	}
//	printf("Calculate => %d:%f:%f:%f\n", m_posStack, m_d1, m_d2, m_stack[m_posStack-1]);
	return(true);
}
//------------------------------------------------------------------------------
// parserGetPoint
//------------------------------------------------------------------------------
int parserGetPoint(char *pToken, int *ptType)
{
	int		count = 0;
	char	point[SHORTBUF_LEN];
	
	if(*pToken == 'A')
    {
        //printf("Analog Point...");
        *ptType = 2;
        pToken++;
    }
    else if(*pToken == 'S')
    {
        //printf("Status Point...");
        *ptType = 1;
        pToken++;
    }
    else
    {
        *ptType = 0;
        //printf("Default Point...");
    }
                  	    
	memset(point, 0, SHORTBUF_LEN);
	for (int idx = 0; idx < SHORTBUF_LEN; idx++, pToken++)
	{
		if (m_digitSet1.find(*pToken) != string::npos)
		{
			point[count++] = *pToken;
			continue;
		}
		if (*pToken == ']')
			break;

		return(-1);
	}
	//printf("GetPoint => %s\n", point);
	return(atoi(point));
}
//------------------------------------------------------------------------------
// parserGetModule
//------------------------------------------------------------------------------
int parserGetModule(char *pToken)
{
	int		count = 0;
	char	module[SHORTBUF_LEN];
	
	if (toupper(*pToken) != 'M')
		return(-1);

	pToken++;
	memset(module, 0, SHORTBUF_LEN);
	for (int idx = 0; idx < SHORTBUF_LEN; idx++, pToken++)
	{
		if (m_digitSet1.find(*pToken) != string::npos)
		{
			module[count++] = *pToken;
			continue;
		}
		if (*pToken == ':')
			break;

		return(-1);
	}
	//printf("GetModule => %s\n", module);
	return(atoi(module));
}
//------------------------------------------------------------------------------
// parserGetValue
//------------------------------------------------------------------------------
bool parserGetValue(double *pValue, int calPoint)
{
	int		module, point, ptType;
	char	*ptr;
    
    SDP_DEVICE  *dev;
    POINT_BUF   *ptBuf;
    
	if ((ptr = strstr(m_token, ":")) == NULL)
		return(false);
	if ((module = parserGetModule(m_token+1)) < 0)
		return(false);
	if ((point = parserGetPoint(ptr+1, &ptType)) < 0)
		return(false);

	//
	// /DEVICE / Point 정보 확인
	//
	dev = (SDP_DEVICE *) deviceCFG[module-1];
	
	if(ptType == 1)     // 상태 포인트 
    {
        ptBuf = (POINT_BUF *) &dev->diPtBuf[point - 1];
        *pValue = ptBuf->status;
    }
    else if(ptType == 2)
    {
        ptBuf = (POINT_BUF *) &dev->aiPtBuf[point - 1];
        *pValue = ptBuf->floatData;
    }
         	    
	//*pValue = 99;
	if(opr->calDebug == calPoint)
	Debug_CPP(console, ">> CAL (%d) GetValue => Mod %d:point(%d) %d:val %f\n", calPoint+1, module, ptType, point, *pValue);
	
	return(true);
}

//------------------------------------------------------------------------------
// parserPopToken
//------------------------------------------------------------------------------
bool parserPopToken(int mode)
{
	string	token;

	while (m_list.size())
	{
		token = m_list.front();
		switch (mode)
		{
		case 1:
			if (token.find("*") != 0 && token.find("/") != 0)
				return(true);
			break;
		default:
			if (token.find("(") == 0)
				return(true);
			break;
		}
		m_listPostfix.push_back(token);
//		printf("push_back m_listPostfix [%s]\n", token.c_str());
		m_list.pop_front();
	}
	return(true);
}
//------------------------------------------------------------------------------
// parserIsOperator
//------------------------------------------------------------------------------
int parserIsOperator(string token)
{
	int		idx;
	string	strOperator[] = { "(", "+", "-", "*", "/", ")", "<", ">", "<=", ">=", "||", "&&", "!=", "==" };

	for (idx = 0; idx < 14; idx++)
	{
		if (strOperator[idx] == token)
			return(idx);
	}
	return(-1);
}		
//------------------------------------------------------------------------------
// parserConvert
//------------------------------------------------------------------------------
bool parserConvert(void)
{
	int		opIndex;
	LS_IT	it;
	string	fToken, token;

	m_postfix = "";
	m_list.clear();
	m_listPostfix.clear();
	for (int idx = 0; idx < m_cntToken; idx++)
	{
		// 연산자가 아닌 경우 패스
		token = m_listToken[idx];
		if (token.find("[") == 0 || (opIndex = parserIsOperator(token)) < 0)
		{
			m_listPostfix.push_back(token);
//			printf("push_back m_listPostfix [%s]\n", token.c_str());
			continue;
		}
		// 연산자 처리
		switch (opIndex)
		{
		case 0:			// '('
			m_list.push_front(token);
//			printf("push_front m_list [%s]\n", token.c_str());
			break;
		case 5:			// ')'
			parserPopToken(0);
			if (m_list.size() <= 0)
				return(false);
			
			m_list.pop_front();
			break;
		case 1:			// '+'
		case 2:			// '-'
		case 6:			// '<'
		case 7:			// '>'
		case 8:			// '<='
		case 9:			// '>='
		case 10:		// '||'
		case 11:		// '&&'
		case 12:		// '!='
		case 13:		// '=='
			parserPopToken(0);
			m_list.push_front(token);
//			printf("push_front m_list [%s]\n", token.c_str());
			break;
		case 3:			// '*'
		case 4:			// '/'
			parserPopToken(1);
			m_list.push_front(token);
//			printf("push_front m_list [%s]\n", token.c_str());
			break;
		default:	return(false);
		}
	}
	while (m_list.size())
	{
		fToken = m_list.front();
		m_listPostfix.push_back(fToken);
//		printf("push_back m_listPostfix [%s]\n", fToken.c_str());
		m_list.pop_front();
	}
	for (it = m_listPostfix.begin(); it != m_listPostfix.end(); it++)
		parserAddString(&m_postfix, *it, true);
//	printf("postfix => %s\n", m_postfix.c_str());
	return(true);
}
//------------------------------------------------------------------------------
// parserGetIFtoken
//------------------------------------------------------------------------------
int parserGetToken(void)
{
	char	input[MAX_INFIX_SIZE];

	m_cntToken = 0;
	
	sprintf(input, m_odInfix.c_str(), sizeof(input));
	m_token = strtok_r(input, m_pSeperator, &m_nextToken);
	while (m_token != NULL)
	{
		m_listToken[m_cntToken] = m_token;
		m_cntToken++;
		m_token = strtok_r(NULL, m_pSeperator, &m_nextToken);	
	}
	return(m_cntToken);
}
//------------------------------------------------------------------------------
// parserOrderInfix
//------------------------------------------------------------------------------
bool parserOrderInfix(int point)
{
	bool	isPrevOpr, isVarStart;
	string	cPrevOpr, cCur;
	int     last;

	m_odInfix = "";
	cPrevOpr = "";
	isPrevOpr = false;
	isVarStart = false;

    last = m_infix.length();
    //printf("paserOrderInfix : last = %d\n", last);
    
	//for (int idx = 0; idx < m_infix.length(); idx++)
	for (int idx = 0; idx < last; idx++)
	{
		cCur = m_infix[idx];
//  printf("[%d] %s:%s\n", idx, cCur.c_str(), m_oprSet1.c_str());
		if (cCur == " " || cCur == "\t" || cCur == "\n" || cCur == "\r")
			continue;

		if (cCur == "[")
		{
			isVarStart = true;
			parserAddString(&m_odInfix, " ", false);
		}
		if (isVarStart)
		{
			parserAddString(&m_odInfix, cCur, false);
			if (cCur.find("]") != string::npos)
				isVarStart = false;
			continue;
		}
		if (m_digitSet.find(cCur) != string::npos)
		{
			if (isPrevOpr)
				parserAddString(&m_odInfix, " ", false);
			parserAddString(&m_odInfix, cCur, false);
			cPrevOpr = "";
			isPrevOpr = false;
			continue;
		}
		else if (m_oprSet1.find(cCur) != string::npos)
		{
			parserAddString(&m_odInfix, cCur, true);
			cPrevOpr = "";
			isPrevOpr = true;
			continue;
		}
		else if (m_oprSet12.find(cCur) != string::npos)
		{
			if (cPrevOpr != "")
				return(false);
				
			parserAddString(&m_odInfix, cCur, true);
			cPrevOpr = cCur;
			isPrevOpr = true;
			continue;
		}
		else if (m_oprSet2.find(cCur) != string::npos)
		{
			if (cPrevOpr == "")
			{
				parserAddString(&m_odInfix, cCur, true);
				cPrevOpr = cCur;
			}
			else if ((cPrevOpr == "<" && cCur == "=") || (cPrevOpr == ">" && cCur == "=")
				|| (cPrevOpr == "=" && cCur == "=") || (cPrevOpr == "!" && cCur == "=")
				|| (cPrevOpr == "|" && cCur == "|") || (cPrevOpr == "&" && cCur == "&"))
			{
				parserAddString(&m_odInfix, cCur, false);
				cPrevOpr = "";
			}
			else	return(false);
				
			isPrevOpr = true;
			continue;
		}
		else	return(false);
	}
	
	if(opr->calDebug == point)
    Debug_CPP(console, ">> CAL (%d) odInfix => [%s]\n", point+1, m_odInfix.c_str());
    
	return(true);
}
//------------------------------------------------------------------------------
// parserInitData
//------------------------------------------------------------------------------
bool parserInitData(const char *pInfix)
{
	// Infix 확인
	if (strlen(pInfix) <= 0)
		return(false);

	m_online = true;
	m_token = NULL;
	m_nextToken = NULL;
	m_infix = pInfix;
	return(true);
}
//------------------------------------------------------------------------------
// parserToPostfix
//------------------------------------------------------------------------------
bool parserToPostfix(const char *pInfix, int point)
{
	if (!parserInitData(pInfix))
		return(false);
	
	if (!parserOrderInfix(point))
		return(false);
	
	if (parserGetToken() <= 0)
		return(false);
	
	if (!parserConvert())
		return(false);
		
	return(true);
}
//------------------------------------------------------------------------------
// parserCalculate
//------------------------------------------------------------------------------
extern "C" bool parserCalculate(const char *pInfix, double *pValue, bool *pOnline, int point)
{
	char	input[MAX_INFIX_SIZE];
	double	dTemp;

	if (!parserToPostfix(pInfix, point))
		return(false);

	m_posStack = 0;
	sprintf(input, m_postfix.c_str(), sizeof(input));
	m_token = strtok_r(input, m_pSeperator, &m_nextToken);
	while (m_token != NULL)
	{
		if (strchr(m_pOperator, m_token[0]) == NULL)
		{
			if (m_token[0] == '[')
			{
				if (!parserGetValue(&dTemp, point))
					return(false);
			}
			else	dTemp = atof(m_token);
			m_stack[m_posStack++] = dTemp;
//			printf("Push stack => %d:%f\n", m_posStack, m_stack[m_posStack-1]);
		}
		else
		{
			if (!parserDoCalculate())
				return(false);
		}			
		m_token = strtok_r(NULL, m_pSeperator, &m_nextToken);
	}
	*pValue = m_stack[--m_posStack];
	*pOnline = m_online;
	
	if(opr->calDebug == point)
    Debug_CPP(console, ">> CAL (%d) Result => online=%d, value= %4.2f\n", point+1, *pOnline, *pValue);

	return(true);
}
//------------------------------------------------------------------------------
// End of Parser.cpp
//------------------------------------------------------------------------------