#ifndef RTREEVIEWMAPWORLD_H_INLCUDE_
#define RTREEVIEWMAPWORLD_H_INLCUDE_

#include <QTreeWidget>
#include <QDialog>

#include <vector>
using namespace std;



class RTreeViewMapWorld : public QTreeWidget
{
	Q_OBJECT

public:
	explicit RTreeViewMapWorld(QWidget *parent = 0);
	~RTreeViewMapWorld();
};

//////////////////////////////////////////////////////////////////////////
class RTreeViewDownload : public QTreeWidget
{
	Q_OBJECT
public:
	explicit RTreeViewDownload(QWidget *parent = 0);
	~RTreeViewDownload();

};

//////////////////////////////////////////////////////////////////////////
class RTreeViewDownloadTask : public QTreeWidget
{
	Q_OBJECT
public:
	explicit RTreeViewDownloadTask(QWidget *parent = 0);
	~RTreeViewDownloadTask();

};

//////////////////////////////////////////////////////////////////////////


class RDialogDownload : public QDialog
{
	Q_OBJECT

public:
	explicit RDialogDownload(QWidget *parent = 0);
	//RDialogDownload();

	protected slots:
		void OnPushBtnOutPathClicked();
		void OnPushBtnSelectAllClicked();
		void OnPushBtnSelectNoneClicked(); // ��ѡ
		void OnPushBtnSelectClearClicked();
		void LineEditingFinished(); // �������ҷ�Χ�༭���¼�

public:

	double m_dLeft;
	double m_dTop;
	double m_dRight;
	double m_dBottom;

	QString m_strOutPath;
	QString m_strTaskName;
	vector<int> arrLevels; // �û�ѡ��Ҫ���صļ���	

private:

	QLineEdit* m_pLineEditOutPath;//= new QLineEdit;
	QPushButton* m_pPushBtnOutPath;// = new QPushButton("...");
	QTreeWidget* m_pTreeWidgetLeves;

	QLineEdit* m_pLineEditLeft;
	QLineEdit* m_pLineEditTop;
	QLineEdit* m_pLineEditRight;
	QLineEdit* m_pLineEditBottom;




};



#endif //RTREEVIEWMAPWORLD_H_INLCUDE_