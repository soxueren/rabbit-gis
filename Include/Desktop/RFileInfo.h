
#ifndef RFILE_INFO_H
#define RFILE_INFO_H

#include <QWidget>
class QTabWidget;
class QTableView;
class QStandardItemModel;
class QTextEdit;

class RFileInfo : public  QWidget
{
	Q_OBJECT

public:
	RFileInfo(QWidget *parent = 0, Qt::WFlags flags = 0);
	~RFileInfo();

	void set_filename(const std::string& filename);

private:

	QTableView* create_imginfo_tableview();

	QTabWidget* m_centralWidget;
	QStandardItemModel* m_pItemModel;
	QTextEdit* m_pgeoInfo;

	//QAction* m_actok;
};

#endif