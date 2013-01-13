#ifndef RABBIT_H
#define RABBIT_H

#pragma once 

#include <QtGui/QMainWindow>
#include <QStandardItemModel>

#include <vector>
using namespace std;


class QAction;
class QToolBar;
class QProgressBar;
class QLabel;
class QMdiArea;
class QTextEdit;
class RFileInfo;
class RLayerControl;
class RMap;
class RDataSourceCtrl;
class RMapWidget;
class OGRDataSource;
class GDALDataset;
class RFileRaster;
class RDataSource;
class RWorkspace;


class Rabbit : public QMainWindow
{
	Q_OBJECT
	
public:
	Rabbit(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Rabbit();

	//typedef void (Rabbit::*CallBackProgress) (int iMin, int iMax, int iValue); // ������Ϣ�ص�


	QMenu* GetRightBtnMenu() {return m_menuRightBtn;}

protected:
	
	/************************************************************************/
	/* �������ظ���paintEvent���� ��ʵ�ֱ��ػ���
	*/
	/************************************************************************/
	virtual void paintEvent(QPaintEvent *event);


	virtual void keyReleaseEvent ( QKeyEvent * event );
	
	/************************************************************************/
	/* ���ڹر�                                                                     */
	/************************************************************************/
	virtual void closeEvent ( QCloseEvent * event );

	private slots: // �¼����
		void actionOpen();
		void actionClose();
		void actionSelect();
		void actionSelectPoint();
		void actionSelectRect();
		void actionPan();
		void actionZoomIn(); // �Ŵ�
		void actionZoomOut(); // ��С
		void actionZoomEntire();
		void action_about();
		void updateWindowMenu();
		void updateMenus();
		void actionAdd2NewMap();
		void actionAdd2CurrMap();
		void actionViewAttribute();

		void action_layerControl(); // ͼ����ƴ��� 
		void actionDBDialog();  // ���ݿ���ϵ
		void actionDSDialog(); // ����Դ������

		// ˫�����ݼ��¼�
		void OnTreeViewDoubleClicked(const QModelIndex& modeIndex);
		void OnTreeViewRightBntDown(const QModelIndex& modelIndex);

private:

	RMap* m_pWnd;

	// �ļ���Ϣ����
	RFileInfo *m_pfileinfor;// = new QWidget;
	RLayerControl* m_layerControl;
	
	RWorkspace* m_pWkspc;

public:
	//vector<OGRDataSource*> m_arrDataSources; // ʸ��
	vector<RFileRaster*> m_arrFileRasters; // դ��
	vector<RDataSource*> m_arrDataSourceWebs;// ����

	RMapWidget* activeMap(); // ���ͼ����
	QLabel* GetStatusBarLabel(){ return m_pLabelStatusBar;}

	//////////////////////////////////////////////////////////////////////////
	// �����������
	//////////////////////////////////////////////////////////////////////////
private:

	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createDockWindows();

	// �½���ͼ
	void NewMap(const QString& strDsName, const QString& strName);

	void SetRightBtnSelectDsAndDtNames(const QString& strDsName, const QString& strDtName){m_strSelectDsName=strDsName; m_strSelectDtName=strDtName;}
	void GetRightBtnSelectDsAndDtNames(QString& strDsName, QString& strDtName){strDsName=m_strSelectDsName; strDtName=m_strSelectDtName;}

	QString m_strSelectDsName, m_strSelectDtName;


	QWidget* m_centralWidget;
	QMdiArea *m_pMdiArea;


	QMenu *m_menuFile;
	QMenu *m_menuMapCtrl; // ��ͼ����
	QMenu *m_menuHelp;
	QMenu *m_menuView;
	QMenu *m_menuWindow;
	QMenu *m_menuRightBtn; // �Ҽ��˵�
	QMenu* m_menuRightBtnDataset; // ���ݼ��Ҽ�

	QToolBar *m_fileToolBar;
	QToolBar *m_mapToolBar; // ��ͼ��������

	QAction* m_actOpen;
	QAction* m_actClose;
	QAction* m_actExit;
	QAction* m_actSelect;
	QAction* m_actSelectPoint; //  ��ѡ
	QAction* m_actSelectRect; // ��ѡ

	QAction* m_actPan;
	QAction* m_actZoomIn;
	QAction* m_actZoomOut;
	QAction* m_actZoomEntire;
	QAction* m_act_file_info;
	QAction* m_act_about;
	QAction* m_act_layControl; // ͼ�����
	QAction* m_act_dbDialog; // ���ݿ�����
	QAction* m_act_dsMgr; // ����Դ�������¼�
	QAction* m_actAdd2NewMap; // ��ӵ��µ�ͼ
	QAction* m_actAdd2CurrMap; // ��ӵ���ǰ��ͼ
	QAction* m_actViewAttributeTable; // ������Ա�

	QAction* m_actCloseOne; // �رյ�ǰ��ͼ 
	QAction* m_actCloseAll;
	QAction* m_actTitle;
	QAction* m_actCascade;

	static QProgressBar *m_pProgressBar;
	QTextEdit* m_pOutPutWnd; // �������

	RDataSourceCtrl* m_pDsTreeViewCtrl;// ����Դ�������ؼ�

	public:

	QLabel * m_pLabelStatusBar; // ״̬���ϵ���Ϣ
};

#endif // RABBIT_H
