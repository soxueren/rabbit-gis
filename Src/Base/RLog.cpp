#include "Base/RLog.h"
#include <time.h>


// 注意!!!类的静态变量定义与初始化
FILE* RLog::m_pFile=NULL;
unsigned long RLog::m_pHandle=NULL;
CallBackForHandle RLog::m_pCallBackFunc=NULL;
// 
// RLog::RLog()
// {
// }
// 
// RLog::~RLog()
// {
// 
// }


void RLog::SetFile(FILE* pf)
{
	m_pFile=pf;

}

void RLog::SetHandle(unsigned long pHandle)
{
	m_pHandle=pHandle;
}

void RLog::Print(int iLevel, const string& strMsg)
{
	char tbuffer [9];
	tbuffer[8]='\0';
	_strtime( tbuffer ); // C4996
	string strNow = tbuffer;
	string strLog = strNow+" | "+strMsg;
	

	if(m_pCallBackFunc!=NULL && m_pHandle!=NULL)
	{
		m_pCallBackFunc(m_pHandle, strLog);
	}

}

void RLog::SetCallBackFunc(CallBackForHandle pCallBackFunc)
{
	m_pCallBackFunc = pCallBackFunc;
}

