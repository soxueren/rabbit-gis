#ifndef __RREGISTER_H__
#define __RREGISTER_H__

#include "Base/RDefine.h"
#include <string>
using namespace std;

// 负责注册,配置文件操作
class BASE_API RRegister
{
public:

	RRegister();
	~RRegister();

	//RRegister(const string& strFileName);

	//void SetFileName(const string& strFileName);

	// 得到应用程序下的文件
	static string GetFileFromApp(const string& strFileName);

	// 记录上次访问过的目录
	static string GetLastVisitedDirectory();
	static void SetLastVisitedDirectory(const string& strNewDir);

private:
	static string m_strConfigFileName;

};


#endif