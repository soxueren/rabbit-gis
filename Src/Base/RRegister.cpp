#include "Base/RRegister.h"
#include "Base/Markup.h"

#ifdef _WIN32
#include "Windows.h"
#include "io.h"
#endif

string RRegister::m_strConfigFileName="";

RRegister::RRegister()
{

}

RRegister::~RRegister()
{

}

string RRegister::GetFileFromApp(const string& strFileName)
{
	char chName[1024]={0};
	if(GetModuleFileName(NULL, chName, 1024))
	{
		char path_buffer[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];

		_splitpath( chName, drive, dir, fname, ext ); // C4996

		string strConfigFile;
		if(strFileName.empty())
		{
			 strConfigFile= string(drive)+string(dir)+string(fname)+".xml";
		}
		else
		{
			strConfigFile= string(drive)+string(dir)+strFileName+".xml";
		}
		return strConfigFile;
	}

	return "";
}


string RRegister::GetLastVisitedDirectory()
{
	string strConfigFile = GetFileFromApp(m_strConfigFileName);
	if(_access(strConfigFile.c_str(), 0) != -1)
	{		
		CMarkup mkFile;
		if(mkFile.Load(strConfigFile.c_str()))
		{
			mkFile.ResetMainPos();
			while (mkFile.FindChildElem("LastVistedDirectory"))
			{
				//此时接点还是父接点
				string strTagName;
				string strData;

				strTagName = mkFile.GetChildTagName();
				strData = mkFile.GetChildData();
				return strData;
			}
		}
	}
	return "";
}

void RRegister::SetLastVisitedDirectory(const string& strNewDir)
{
	string strConfigFile = GetFileFromApp(m_strConfigFileName);
	if(_access(strConfigFile.c_str(), 0) != -1)
	{
		CMarkup mkfile;
		if(mkfile.Load(strConfigFile.c_str()))
		{
			mkfile.ResetMainPos();
			while (mkfile.FindChildElem("LastVistedDirectory"))
			{
				mkfile.SetChildData(strNewDir);
				break;
			}

			mkfile.Save(strConfigFile);
		}
	}
	else
	{
		CMarkup mkfile;
		mkfile.SetDoc("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
		mkfile.AddElem("RegisterInfo");
		mkfile.IntoElem();
		mkfile.AddElem("LastVistedDirectory", strNewDir);
		mkfile.OutOfElem();
		mkfile.Save(strConfigFile);
	}
}