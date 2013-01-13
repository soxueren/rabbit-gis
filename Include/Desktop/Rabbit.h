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

	//typedef void (Rabbit::*CallBackProgress) (int iMin, int iMax, int iValue); // 进度信息回调


	QMenu* GetRightBtnMenu() {return m_menuRightBtn;}

protected:
	
	/************************************************************************/
	/* 必须重载父类paintEvent函数 ，实现本地绘制
	*/
	/************************************************************************/
	virtual void paintEvent(QPaintEvent *event);


	virtual void keyReleaseEvent ( QKeyEvent * event );
	
	/************************************************************************/
	/* 窗口关闭                                                                     */
	/************************************************************************/
	virtual void closeEvent ( QCloseEvent * event );

	private slots: // 事件插槽
		void actionOpen();
		void actionClose();
		void actionSelect();
		void actionSelectPoint();
		void actionSelectRect();
		void actionPan();
		void actionZoomIn(); // 放大
		void actionZoomOut(); // 缩小
		void actionZoomEntire();
		void action_about();
		void updateWindowMenu();
		void updateMenus();
		void actionAdd2NewMap();
		void actionAdd2CurrMap();
		void actionViewAttribute();

		void action_layerControl(); // 图层控制处理 
		void actionDBDialog();  // 数据库联系
		void actionDSDialog(); // 数据源管理器

		// 双击数据集事件
		void OnTreeViewDoubleClicked(const QModelIndex& modeIndex);
		void OnTreeViewRightBntDown(const QModelIndex& modelIndex);

private:

	RMap* m_pWnd;

	// 文件信息窗口
	RFileInfo *m_pfileinfor;// = new QWidget;
	RLayerControl* m_layerControl;
	
	RWorkspace* m_pWkspc;

public:
	//vector<OGRDataSource*> m_arrDataSources; // 矢量
	vector<RFileRaster*> m_arrFileRasters; // 栅格
	vector<RDataSource*> m_arrDataSourceWebs;// 网络

	RMapWidget* activeMap(); // 活动地图窗口
	QLabel* GetStatusBarLabel(){ return m_pLabelStatusBar;}

	//////////////////////////////////////////////////////////////////////////
	// 窗体各个部件
	//////////////////////////////////////////////////////////////////////////
private:

	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createDockWindows();

	// 新建地图
	void NewMap(const QString& strDsName, const QString& strName);

	void SetRightBtnSelectDsAndDtNames(const QString& strDsName, const QString& strDtName){m_strSelectDsName=strDsName; m_strSelectDtName=strDtName;}
	void GetRightBtnSelectDsAndDtNames(QString& strDsName, QString& strDtName){strDsName=m_strSelectDsName; strDtName=m_strSelectDtName;}

	QString m_strSelectDsName, m_strSelectDtName;


	QWidget* m_centralWidget;
	QMdiArea *m_pMdiArea;


	QMenu *m_menuFile;
	QMenu *m_menuMapCtrl; // 地图操作
	QMenu *m_menuHelp;
	QMenu *m_menuView;
	QMenu *m_menuWindow;
	QMenu *m_menuRightBtn; // 右键菜单
	QMenu* m_menuRightBtnDataset; // 数据集右键

	QToolBar *m_fileToolBar;
	QToolBar *m_mapToolBar; // 地图操作工具

	QAction* m_actOpen;
	QAction* m_actClose;
	QAction* m_actExit;
	QAction* m_actSelect;
	QAction* m_actSelectPoint; //  点选
	QAction* m_actSelectRect; // 框选

	QAction* m_actPan;
	QAction* m_actZoomIn;
	QAction* m_actZoomOut;
	QAction* m_actZoomEntire;
	QAction* m_act_file_info;
	QAction* m_act_about;
	QAction* m_act_layControl; // 图层控制
	QAction* m_act_dbDialog; // 数据库连接
	QAction* m_act_dsMgr; // 数据源管理器事件
	QAction* m_actAdd2NewMap; // 添加到新地图
	QAction* m_actAdd2CurrMap; // 添加到当前地图
	QAction* m_actViewAttributeTable; // 浏览属性表

	QAction* m_actCloseOne; // 关闭当前地图 
	QAction* m_actCloseAll;
	QAction* m_actTitle;
	QAction* m_actCascade;

	static QProgressBar *m_pProgressBar;
	QTextEdit* m_pOutPutWnd; // 输出窗口

	RDataSourceCtrl* m_pDsTreeViewCtrl;// 数据源管理器控件

	public:

	QLabel * m_pLabelStatusBar; // 状态栏上的消息
};

#endif // RABBIT_H
