
#ifndef RMAP_INIT_H
#define RMAP_INIT_H

#include <string>
#include <vector>

/************************************************************************/
/* xml配置文件，xml编码为ANSI                                                                     */
/************************************************************************/

class RMapInit
{
public:
	RMapInit();
	~RMapInit();

	bool get_drawbyblock(); // 是否使用分块显示模式
	void set_drawbyblock(bool byblock=true);

	bool get_last_opened_directory(std::string& strdir);
	void set_last_opened_directory(const std::string& newdir);

	void get_filelists(std::vector<std::string>& arrfiles, std::vector<std::string>& arrexts);

protected:
private:
	
	std::string m_strname;
	
};

const std::string m_init = 
"<?xml version=\"1.0\"?>\n"
"<RabbitMap>\n"
"<DrawByBlock>false</DrawByBlock>\n"
"<LastOpenedDirectory>.</LastOpenedDirectory>\n"
"</RabbitMap>";



#endif //RMAP_INIT_H