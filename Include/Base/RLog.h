#ifndef __RLOG_H__
#define __RLOG_H__

#pragma once

#include "Base/RDefine.h"
#include <iostream>
#include <string>
using namespace std;

// ���豸����Ļص�����
typedef void (__stdcall * CallBackForHandle) (unsigned long pHandle, const string& strMsg);

/*// ��֮��
�ɴ�ӡ��־���ļ�
�ɴ�ӡ��־��ָ���豸���
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

	// �ļ����
	static FILE* m_pFile;

	static CallBackForHandle m_pCallBackFunc;

	// �����Ǵ��ھ��
	static unsigned long m_pHandle;

};


#endif //__RLOG_H__