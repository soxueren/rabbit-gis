#ifndef __RREGISTER_H__
#define __RREGISTER_H__

#include "Base/RDefine.h"
#include <string>
using namespace std;

// ����ע��,�����ļ�����
class BASE_API RRegister
{
public:

	RRegister();
	~RRegister();

	//RRegister(const string& strFileName);

	//void SetFileName(const string& strFileName);

	// �õ�Ӧ�ó����µ��ļ�
	static string GetFileFromApp(const string& strFileName);

	// ��¼�ϴη��ʹ���Ŀ¼
	static string GetLastVisitedDirectory();
	static void SetLastVisitedDirectory(const string& strNewDir);

private:
	static string m_strConfigFileName;

};


#endif