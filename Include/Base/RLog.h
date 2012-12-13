#ifndef __RLOG_H__
#define __RLOG_H__

#pragma once

#include "Base/RDefine.h"
#include <iostream>
#include <string>
using namespace std;

// 向设备输出的回调函数
typedef void (__stdcall * CallBackForHandle) (unsigned long pHandle, const string& strMsg);

/*// 日之类
可打印日志到文件
可打印日志到指定设备句柄
*/
class BASE_API RLog
{
public:

	typedef enum RLevel
	{
		Info=0,
		Warning,
		Error,
	};

	RLog(){};
	~RLog(){};

	static void SetFile(FILE* pf);
	static void SetHandle(unsigned long pHandle);

	static void SetCallBackFunc(CallBackForHandle pCallBackFunc);

	static void Print(int iLevel, const string& strMsg);

	

private:

	// 文件句柄
	static FILE* m_pFile;

	static CallBackForHandle m_pCallBackFunc;

	// 可以是窗口句柄
	static unsigned long m_pHandle;

};


#endif //__RLOG_H__