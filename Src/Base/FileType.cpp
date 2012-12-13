
#include "Base/FileType.h"
#include "cpl_conv.h"
//#include "cpl_string.h"

#ifdef WIN32

#include <Windows.h>

#endif


int CRFile::GetTypeByExtName(const string& strExt)
{
	for(int i=0; gfileDetail[i].chExt!=NULL; ++i){
		if(strcmp(strExt.c_str(), gfileDetail[i].chExt) ==0)
			return (int)gfileDetail[i].iType;
	}
	
	return 0;

}

string CRFile::GetDriverNameByType(int iType)
{
	for(int i=0; gfileDetail[i].iType != CRFile::None; ++i){
		if(iType == (int)gfileDetail[i].iType)
			return gfileDetail[i].chDri;
	}

	return "";

}


string CRFile::GetExtNameByType(int iType)
{
	return "";

}

string CRFile::GetExtNameFromPath(const string& strPath)
{
	const char* chext = CPLGetExtension(strPath.c_str());
	if(strlen(chext) >0 )
		return chext;

	return "";
}

string CRFile::GetTitleNameFromPath(const string& strPath)
{
	const char* chname = CPLGetBasename(strPath.c_str());
	if(strlen(chname) >0 )
		return chname;

	return "";

}


CRFile::FClass CRFile::GetClassByName(const string& strPath)
{
	string strExt = CRFile::GetExtNameFromPath(strPath);
	int iType = CRFile::GetTypeByExtName(strExt);

	if(CRFile::GetClass(iType) == CRFile::fcVector){
		return CRFile::fcVector;
	}
	else if(CRFile::GetClass(iType) == CRFile::fcRaster){
		return CRFile::fcRaster;
	}	
}

bool CRFile::IsExist(const string& strFileName)
{
#ifdef WIN32
	
	bool bFind=false;
	WIN32_FIND_DATA myFindData;
	HANDLE hFind = FindFirstFile(strFileName.c_str(), &myFindData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		bFind = true;
	}

	FindClose(hFind);
	return bFind;

#endif

}

string CRFile::GetDirName(const string& strFileName)
{
	const char* pchDirName = CPLGetDirname(strFileName.c_str());
	if(strlen(pchDirName) > 0)
		return pchDirName;

	return "";
}

void CRFile::MkDir(const string& strDir)
{
	string strDIR = strDir;
	int iPos = strDIR.find('\\');
	while(iPos != string::npos)
	{
		strDIR = strDIR.replace(iPos, 1, "/");
		iPos = strDIR.find('\\');
	} // �滻\\, Ϊ/

	iPos = strDIR.find('/');
	string dirCreate;
	while(iPos != string::npos)
	{
		dirCreate += strDIR.substr(0, iPos+1);
		mkdir(dirCreate.c_str());
		strDIR.erase(0, iPos+1);
		iPos = strDIR.find('/');
	}

	if(strDIR.size() > 0)
	{
		dirCreate += strDIR;
		mkdir(dirCreate.c_str());
	}




	//strDIR.find();
	

}




CRFile::FClass CRFile::GetClass(int iType)
{
	if(0<=iType && iType <=100){
		return fcVector;
	}
	else if(100<iType<=200){
		return fcRaster;
	}

	return (CRFile::FClass)0;

}
