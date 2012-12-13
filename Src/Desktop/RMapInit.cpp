
#include "Desktop/RMapInit.h"
#include "Desktop/Markup.h"

//{{ 用这种方式（实际上是ANSI编码），避免markup将xml强制存储为utf-8，导致乱码
void load_xml(const std::string& filename, std::string& strdoc){
	FILE* pf = fopen(filename.c_str(), "r+");
	if(pf!=NULL){
		fseek(pf, 0, SEEK_END);
		long ilen = ftell(pf);
		fseek(pf, 0, SEEK_SET);
		char* pdoc=new char[ilen+1];
		memset(pdoc, 0, ilen+1);
		pdoc[ilen]='\0';
		fread(pdoc, ilen, 1, pf);
		strdoc=pdoc;
		delete [] pdoc;
		fclose(pf);
		pf=NULL;
	}
}


void save_xml(const std::string& filename, const std::string& strdoc){
	FILE* pf = fopen(filename.c_str(), "w+");
	if(pf!=NULL){
		fwrite(strdoc.c_str(), strdoc.length(), 1, pf);
		fclose(pf);
		pf=NULL;
	}
}

//}}

RMapInit::RMapInit()
{
	if(m_strname.empty()){
		m_strname = "config/RabbitMap.xml";
	}

	CMarkup mk;
	if(!mk.Load(m_strname))	{
		save_xml(m_strname, m_init);
	}
	else{
	}

}

RMapInit::~RMapInit()
{

}


bool RMapInit::get_drawbyblock() // 是否使用分块
{
	std::string strdoc;
	load_xml(m_strname, strdoc);

	CMarkup mk;
	if(mk.Load(m_strname))
		if(mk.FindElem("RabbitMap")){
			if(mk.FindChildElem("DrawByBlock")){
			std::string strvalue = mk.GetElemContent();
			if(strvalue.find("true") != std::string::npos)
				return true;		
			}
		}
		return false;
}


void RMapInit::set_drawbyblock(bool byblock/*=true*/)
{
	CMarkup mk;
	if(mk.Load(m_strname))
		if(mk.FindElem("RabbitMap")){
			if(mk.FindChildElem("DrawByBlock")){
				if(byblock)
					mk.SetElemContent("true");
				else
					mk.SetElemContent("false");

				mk.Save(m_strname);
			}
		}
}


bool RMapInit::get_last_opened_directory(std::string& strdir)
{
	std::string strdoc;
	load_xml(m_strname, strdoc);
	CMarkup mk(strdoc);
	//if(mk.Load(m_strname))
		if(mk.FindElem("RabbitMap")){
			if(mk.FindChildElem("LastOpenedDirectory")){
				std::string strvalue = mk.GetChildData();// .GetElemContent();
				if(!strvalue.empty()){
					strdir=strvalue;
					return true;	
				}
			}
		}
		return false;
}

void RMapInit::set_last_opened_directory(const std::string& newdir)
{
	if(newdir.empty())
		return;

	std::string strdoc;
	load_xml(m_strname, strdoc);

	CMarkup mk(strdoc);
	//if(mk.Load(m_strname))
		if(mk.FindElem("RabbitMap")){
			if(mk.FindChildElem("LastOpenedDirectory")){
				mk.IntoElem();
				mk.SetElemContent(newdir);
				//CMarkup::WriteTextFile(m_strname, mk.GetDoc());
				save_xml(m_strname, mk.GetDoc());
			}
		}
}

void RMapInit::get_filelists(std::vector<std::string>& arrfiles, std::vector<std::string>& arrexts)
{
	CMarkup mk;
	std::string filename = "config/FileFormat.xml";
	if(mk.Load(filename)){
		if(mk.FindElem("FileFormat")){
			std::string strext, strname;
			for(;mk.FindChildElem("FileItem");){
				mk.IntoElem();
				while(mk.FindChildElem("File")){
					strname = mk.GetChildAttrib("name");
					if(strname.find("ext") != std::string::npos){
						strext = mk.GetChildData();
						arrexts.push_back(strext);
					}
					else if(strname.find("type") != std::string::npos){
						strname = mk.GetChildData();
						arrfiles.push_back(strname);
					}
				}
				mk.OutOfElem();
			}
		}
	}
}

