
#ifndef RMAP_INIT_H
#define RMAP_INIT_H

#include <string>
#include <vector>

/************************************************************************/
/* xml�����ļ���xml����ΪANSI                                                                     */
/************************************************************************/

class RMapInit
{
public:
	RMapInit();
	~RMapInit();

	bool get_drawbyblock(); // �Ƿ�ʹ�÷ֿ���ʾģʽ
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