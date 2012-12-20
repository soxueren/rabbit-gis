#include "Desktop/Rabbit.h"
#include <QPainter>
 #include <QMouseEvent> //鼠标事件
#include <QBitmap>
#include <QPixmap>
#include <QLayout>
#include <QSettings>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include "QMessageBox"

#include <QDialog>
#include <QTabWidget>
#include <QLabel>
#include <QTableView>
#include <QMdiArea>
#include <QMdiSubWindow>

#include <QProgressBar>
#include <QTextEdit>
#include <QDockWidget>
 #include <QApplication>


#include "Desktop/RMapInit.h"
#include "Base/RMapWnd.h"

#include "Desktop/RFileInfo.h"
#include "Base/FileType.h"
#include "QTCore/RLayerControl.h"
#include "Desktop/RDialogDB.h"
#include "QTCore/RMapWidget.h"
#include "Base/RDataSource.h"

#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include "Base/RLog.h"

QProgressBar* Rabbit::m_pProgressBar=NULL;

// 向设备输出的回调函数
 void __stdcall PrintLog(unsigned long pHandle, const string& strMsg)
{
	if(pHandle!=NULL)
	{
		QTextEdit* pTextEdit=(QTextEdit *)pHandle;
		QString strLog = QString::fromLocal8Bit(strMsg.c_str());
		pTextEdit->append(strLog);
	}
}

Rabbit::Rabbit(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	//m_centralWidget=new QWidget;	
	m_pMdiArea = new QMdiArea;

	m_pMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_pMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	//m_centralWidget->setMouseTracking(true); // 启动不用单击的 鼠标移动事件
	//setCentralWidget(m_centralWidget);
	setCentralWidget(m_pMdiArea);
	connect(m_pMdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMenus()));


	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createDockWindows();
	updateMenus();


	setWindowState(Qt::WindowMaximized);


	//m_iwin_width=m_centralWidget->geometry().width(); // 显示区域宽度
	//m_iwin_height=m_centralWidget->geometry().height();//0; // 显示区域高度

	m_pfileinfor=NULL;
	m_layerControl=NULL;
	m_pWnd = new RMap;
	m_pWkspc=new RWorkspace;
	
	OGRRegisterAll();
	GDALAllRegister();
}


Rabbit::~Rabbit()
{
	if(m_pWnd){
		delete m_pWnd;
		m_pWnd=NULL;
	}

	for(int ir=0; ir<m_arrFileRasters.size(); ir++){
		RFileRaster* pDataset = m_arrFileRasters[ir];
		if(pDataset != NULL){
			pDataset->Close();
			delete pDataset;
			pDataset=NULL;
		}
	}
	m_arrFileRasters.clear();

	for(int idsw=0; idsw<m_arrDataSourceWebs.size(); idsw++)
	{
		RDataSource* pDataSourceWeb = m_arrDataSourceWebs[idsw];
		if(pDataSourceWeb != NULL)
		{
			delete pDataSourceWeb;
			pDataSourceWeb=NULL;
		}
	}

	m_arrDataSourceWebs.clear();
	if(m_pWkspc)
	{
		delete m_pWkspc;
		m_pWkspc=NULL;
	}

}



void Rabbit::createActions()
{
	m_act_layControl = new QAction(tr("Layer Control"), this);
	//m_act_layControl->setStatusTip(tr("Layer Control..."));
	connect(m_act_layControl, SIGNAL(triggered()), this, SLOT(action_layerControl()));

	m_act_dbDialog = new QAction(QString::fromLocal8Bit("Oracle Spatial"), this);
	connect(m_act_dbDialog, SIGNAL(triggered()), this, SLOT(actionDBDialog()));

	m_act_dsMgr = new QAction(tr("DataSource"), this);
	connect(m_act_dsMgr, SIGNAL(triggered()), this, SLOT(actionDSDialog()));
	
	m_actOpen = new QAction(QIcon("images/fileopen.png"), QString::fromLocal8Bit("打开"), this);
	m_actOpen->setShortcut(QKeySequence::Open);
	//m_actOpen->setStatusTip(tr("Open an existing file"));
	connect(m_actOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));

	m_actClose = new QAction(QString::fromLocal8Bit("关闭"), this);
	m_actClose->setShortcuts(QKeySequence::Close);
	connect(m_actClose, SIGNAL(triggered()), this, SLOT(actionClose()));

	m_actExit = new QAction(QIcon("images/button-close.png"), QString::fromLocal8Bit("退出"), this);
	m_actExit->setShortcuts(QKeySequence::Quit);
	//m_actExit->setStatusTip(tr("Exit the Rabbit"));
	connect(m_actExit, SIGNAL(triggered()), this, SLOT(close()));

	m_actSelect = new QAction(QIcon("images/cursor-arrow.png"), QString::fromLocal8Bit("无选择"), this);
	//m_act_select->setShortcuts()
	//m_act_select->setStatusTip("Select");
	connect(m_actSelect, SIGNAL(triggered()), this, SLOT(actionSelect())); // 选择
	
	m_actSelectPoint = new QAction(QString::fromLocal8Bit("点选"), this);
	connect(m_actSelectPoint, SIGNAL(triggered()), this, SLOT(actionSelectPoint())); // 点选择


	m_actSelectRect = new QAction(QString::fromLocal8Bit("框选"), this);
	connect(m_actSelectRect, SIGNAL(triggered()), this, SLOT(actionSelectRect())); // 框选择

	m_actPan = new QAction(QIcon("images/cursor-openhand.png"), QString::fromLocal8Bit("平移"), this);
	//m_act_pan->setStatusTip("Pan");
	connect(m_actPan, SIGNAL(triggered()), this, SLOT(actionPan())); // 平移 

	m_actZoomIn = new QAction(QIcon("images/zoomin.png"), QString::fromLocal8Bit("放大"), this);
	//m_actZoomIn->setStatusTip("ZoomIn");
	connect(m_actZoomIn, SIGNAL(triggered()), this, SLOT(actionZoomIn())); // 放大

	m_actZoomOut = new QAction(QIcon("images/zoomout.png"), QString::fromLocal8Bit("缩小"), this);	
	//m_actZoomOut->setStatusTip("ZoomOut");
	connect(m_actZoomOut, SIGNAL(triggered()), this, SLOT(actionZoomOut()));

	m_actZoomEntire = new QAction(QIcon("images/fit-page-32.png"), QString::fromLocal8Bit("全副"), this);
	//m_actZoomEntire->setStatusTip("ZoomEntire");
	connect(m_actZoomEntire, SIGNAL(triggered()), this, SLOT(actionZoomEntire()));

	m_act_file_info = new QAction(QIcon("images/fileinfo-32.png"), tr("&FileInfo"), this);
	//m_act_file_info->setStatusTip("Infomation");
	connect(m_act_file_info, SIGNAL(triggered()), this, SLOT(action_fileinfo()));

	m_act_about = new QAction(tr("&About"), this);
	//m_act_about->setStatusTip("About");
	connect(m_act_about, SIGNAL(triggered()), this, SLOT(action_about()));

	m_actAdd2NewMap = new QAction(QString::fromLocal8Bit("添加到新地图"), this);
	connect(m_actAdd2NewMap, SIGNAL(triggered()), this, SLOT(actionAdd2NewMap()));

	m_actAdd2CurrMap = new QAction(QString::fromLocal8Bit("添加到当前地图"), this);
	connect(m_actAdd2CurrMap, SIGNAL(triggered()), this, SLOT(actionAdd2CurrMap()));
	
	m_actCloseOne = new QAction(QString::fromLocal8Bit("关闭"), this);
	m_actCloseOne->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_W));
	connect(m_actCloseOne, SIGNAL(triggered()), m_pMdiArea, SLOT(closeActiveSubWindow()));
	//bool btest = connect(m_actCloseOne, SIGNAL(activated()), m_pMdiArea, SLOT(closeActiveSubWindow()));

	m_actCloseAll = new QAction(QString::fromLocal8Bit("关闭所有"), this);
	connect(m_actCloseAll, SIGNAL(triggered()), m_pMdiArea, SLOT(closeAllSubWindows()));

	m_actTitle = new QAction(QString::fromLocal8Bit("平铺"), this);
	connect(m_actTitle, SIGNAL(triggered()), m_pMdiArea, SLOT(tileSubWindows()));


	m_actCascade = new QAction(QString::fromLocal8Bit("层叠"), this);
	connect(m_actCascade, SIGNAL(triggered()), m_pMdiArea, SLOT(cascadeSubWindows()));

}
void Rabbit::createMenus()
{
	m_menuFile = menuBar()->addMenu(QString::fromLocal8Bit("文件"));
	m_menuFile->addAction(m_actOpen);
	m_menuFile->addAction(m_actClose);
	m_menuFile->addAction(m_act_dbDialog);
	m_menuFile->addSeparator();
	m_menuFile->addAction(m_actExit);

	m_menuMapCtrl = menuBar()->addMenu(QString::fromLocal8Bit("操作"));
	QMenu* menuSelect = m_menuMapCtrl->addMenu(QString::fromLocal8Bit("选择"));
	menuSelect->addAction(m_actSelect);
	menuSelect->addAction(m_actSelectPoint);
	menuSelect->addAction(m_actSelectRect);


	m_menuMapCtrl->addAction(m_actPan);
	m_menuMapCtrl->addAction(m_actZoomIn);
	m_menuMapCtrl->addAction(m_actZoomOut);
	m_menuMapCtrl->addAction(m_actZoomEntire);
	
	m_menuView = menuBar()->addMenu(QString::fromLocal8Bit("查看"));

	m_menuWindow = menuBar()->addMenu(QString::fromLocal8Bit("窗口"));
	updateWindowMenu();

	m_menuHelp = menuBar()->addMenu(QString::fromLocal8Bit("帮助"));
	m_menuHelp->addAction(m_act_about);

	m_menuRightBtn =  new QMenu;
	m_menuRightBtn->addSeparator();

	//m_menuRightBtn->addAction(m_act_layControl);
	//m_menuRightBtn->addAction(m_act_dsMgr);
	m_menuRightBtn->addAction(m_actSelect);
	m_menuRightBtn->addAction(m_actPan);
	m_menuRightBtn->addAction(m_actZoomIn);
	m_menuRightBtn->addAction(m_actZoomOut);
	m_menuRightBtn->addAction(m_actZoomEntire);

	m_menuRightBtnDataset = new QMenu;
	m_menuRightBtnDataset->addAction(m_actAdd2NewMap);
	m_menuRightBtnDataset->addAction(m_actAdd2CurrMap);

}

void Rabbit::updateWindowMenu()
{
	m_menuWindow->clear();
	m_menuWindow->addAction(m_actCloseOne);
	m_menuWindow->addAction(m_actCloseAll);
	m_menuWindow->addSeparator();

	m_menuWindow->addAction(m_actTitle);
	m_menuWindow->addAction(m_actCascade);


}

void Rabbit::updateMenus()
{
	bool bHasMapWnd=false;
	if(activeMap()!=NULL){
		bHasMapWnd=true;
	}

	m_actCloseOne->setEnabled(bHasMapWnd);
	m_actCloseAll->setEnabled(bHasMapWnd);
	m_actTitle->setEnabled(bHasMapWnd);
	m_actCascade->setEnabled(bHasMapWnd);
	m_actAdd2CurrMap->setEnabled(bHasMapWnd);
}


void Rabbit::actionAdd2NewMap()
{
	QString strDsName, strDtName;
	GetRightBtnSelectDsAndDtNames(strDsName, strDtName);

	NewMap(strDsName, strDtName);
	RMapWidget* pActiveMap = activeMap();
	if(pActiveMap!=NULL){
		pActiveMap->AddLayer(strDsName, strDtName);
	}
}

void Rabbit::actionAdd2CurrMap()
{
	QString strDsName, strDtName;
	GetRightBtnSelectDsAndDtNames(strDsName, strDtName);

	RMapWidget* pActiveMap = activeMap();
	if(pActiveMap!=NULL)
	{
		pActiveMap->AddLayer(strDsName, strDtName);
		pActiveMap->GetMap()->SetRedrawFlag(true); // 新增了图层，需要刷新地图
		pActiveMap->update();
	}

}

void Rabbit::createToolBars()
{
	m_fileToolBar = addToolBar(QString::fromLocal8Bit("文件操作工具"));
	m_fileToolBar->addAction(m_actOpen);
	//m_fileToolBar->addAction(m_actExit);
	m_menuView->addAction(m_fileToolBar->toggleViewAction());

	m_mapToolBar = addToolBar(QString::fromLocal8Bit("地图操作工具"));
	m_mapToolBar->addAction(m_actSelect);
	m_mapToolBar->addAction(m_actPan);
	m_mapToolBar->addAction(m_actZoomIn);
	m_mapToolBar->addAction(m_actZoomOut);
	m_mapToolBar->addAction(m_actZoomEntire);
	m_mapToolBar->addAction(m_act_file_info);
	m_menuView->addAction(m_mapToolBar->toggleViewAction());

	//connect(m_fileToolBar, SIGNAL(movableChanged(bool)), this, SLOT(set_redraw_flag(bool)));
}

void Rabbit::createDockWindows()
{
	QDockWidget *dock = new QDockWidget(QString::fromLocal8Bit("数据源"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	m_pDsTreeViewCtrl = new RDataSourceCtrl(dock);
	dock->setWidget(m_pDsTreeViewCtrl);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	m_menuView->addAction(dock->toggleViewAction());

	// 数据集双击事件注册
	bool bt = connect(m_pDsTreeViewCtrl, SIGNAL(doubleClicked(const QModelIndex)), 
		this, SLOT(OnTreeViewDoubleClicked(const QModelIndex)));

	bt = connect(m_pDsTreeViewCtrl, SIGNAL(pressed(const QModelIndex)), this, SLOT(OnTreeViewRightBntDown(const QModelIndex)));

	//////////////////////////////////////////////////////////////////////////

	dock = new QDockWidget(QString::fromLocal8Bit("输出窗口"), this);
	dock->setAllowedAreas(Qt::BottomDockWidgetArea);
	m_pOutPutWnd = new QTextEdit(dock);
	dock->setWidget(m_pOutPutWnd);
	dock->close(); // 默认关闭输出窗口
	RLog::SetHandle((unsigned long) m_pOutPutWnd);
	RLog::SetCallBackFunc(&PrintLog); // 设置日志输出到输出窗口
	addDockWidget(Qt::BottomDockWidgetArea, dock);
	m_menuView->addAction(dock->toggleViewAction());
}


void Rabbit::OnTreeViewDoubleClicked(const QModelIndex& modeIndex)
{
	QModelIndex miParent = modeIndex.parent();
	if(!miParent.isValid()){ // 在数据源上双击什么都不做
		return;
	}

	QAbstractItemModel* m=(QAbstractItemModel*)modeIndex.model();  

	int iRowCount = m->rowCount();
	int iColCount = m->columnCount();

	int iRow = modeIndex.row();
	QModelIndex indexDs = m->index(miParent.row(), 0);  
	QModelIndex indexDt = indexDs.child(iRow, 0);
	QString strDtName = indexDt.data().toString();  
	QString strDsName = indexDs.data().toString();
	NewMap(strDsName, strDtName);

	RMapWidget* pActiveMap = activeMap();
	if(pActiveMap!=NULL){
		pActiveMap->AddLayer(strDsName, strDtName);
	}
}

void Rabbit::OnTreeViewRightBntDown(const QModelIndex& modelIndex)
{
	Qt::MouseButtons mbState = QApplication::mouseButtons();
	if(mbState == Qt::RightButton) // 鼠标右键
	{
		QModelIndex miParent = modelIndex.parent();
		if(!miParent.isValid()){ // 在数据源上操作，什么都不响应
			return;
		}

		QAbstractItemModel* m=(QAbstractItemModel*)modelIndex.model();  
		int iRowCount = m->rowCount();
		int iColCount = m->columnCount();

		int iRow = modelIndex.row();
		QModelIndex indexDs = m->index(miParent.row(), 0);  
		QModelIndex indexDt = indexDs.child(iRow, 0);
		QString strDtName = indexDt.data().toString();  
		QString strDsName = indexDs.data().toString();
		SetRightBtnSelectDsAndDtNames(strDsName, strDtName);

		QPoint Pos = QCursor::pos();
		m_menuRightBtnDataset->exec(Pos);
	}
}

void __stdcall StautsBarCallBackFunc(unsigned long pHandle, const string& strMsg)
{
	Rabbit* pMainWnd = (Rabbit* )pHandle;
	if(pMainWnd!=NULL)
	{
		QString strTmp = QString::fromLocal8Bit(strMsg.c_str());
		QLabel* pLabel = pMainWnd->GetStatusBarLabel();
		pLabel->setText(strTmp);
		//pMainWnd->statusBar()->clearMessage();
		//pMainWnd->statusBar()->showMessage(strTmp);
	}
}

void Rabbit::NewMap(const QString& strDsName, const QString& strName)
{
	RMapWidget* pNewMap = new RMapWidget;
	m_pMdiArea->addSubWindow(pNewMap);
	pNewMap->setWindowTitle(strName);
	pNewMap->showMaximized();
	pNewMap->GetMap()->SetMapStatusCallBackFun(&StautsBarCallBackFunc); 
	pNewMap->GetMap()->SetMapStatusHandle((unsigned long)this);

	RDataSource* pDs = m_pWkspc->GetDataSource(strDsName.toLocal8Bit().data());
	pNewMap->GetMap()->SetDataSource(pDs);

}

RMapWidget* Rabbit::activeMap()//; // 活动地图窗口
{
	if (QMdiSubWindow *activeSubWindow = m_pMdiArea->activeSubWindow())
		return qobject_cast<RMapWidget *>(activeSubWindow->widget());
	
	return NULL;
}



void Rabbit::createStatusBar()
{

	m_pLabelStatusBar = new QLabel;
	//m_pLabelPosition->setFrameStyle(QFrame::Panel | QFrame::Raised);
	//m_pLabelPosition->setFrameShape(QFrame::Panel);

	//m_pLabelPosition->setMinimumWidth (100);
	//statusBar()->addWidget(m_pLabelPosition);
	statusBar()->insertWidget(0, m_pLabelStatusBar);

	//m_pProgressBar =new QProgressBar;
	//m_pProgressBar->setTextVisible(false);
	//statusBar()->addWidget(m_pProgressBar);
	//statusBar()->insertWidget(1, m_pProgressBar);
	
}

#include "QFileDialog"


void Rabbit::paintEvent(QPaintEvent *event)
{
	// 主窗体不应该在绘制了
	// 地图子窗体负责绘制
	// 下面的代码可以废弃了
	return; 


	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd==NULL)
		return;

	int iWndWidth = pMapWnd->geometry().width();//m_centralWidget->geometry().width();
	int iWndHeight = pMapWnd->geometry().height();//m_centralWidget->geometry().height();


	QSize winSize(iWndWidth, iWndHeight); 

	// 先在内存位图中将要素渲染一遍
	QImage image(winSize, QImage::Format_RGB888);	

	int iBytesPerLine = iWndWidth*3; // RGB一行字节数
	if (iBytesPerLine%4)
		iBytesPerLine += 4-iBytesPerLine%4;

	unsigned char* pbuf = new unsigned char[iBytesPerLine*iWndHeight];
	memset(pbuf, 205, iBytesPerLine*iWndHeight); // 这个颜色决定了屏幕可视范围默认背景色
	pMapWnd->GetMap()->Draw(iWndWidth, iWndHeight, pbuf);
	unsigned char* pwinbuf = (unsigned char*)image.bits();

	/*
	for(int irow=0; irow<ih; ++irow){
		memcpy(pwinbuf+ibytes_per_line*irow, pbuf+ibytes_per_line*irow, ibytes_per_line);
	}*/
	memcpy(pwinbuf, pbuf, iBytesPerLine*iWndHeight); // 这样加快一下速度 

	// Qt中贴出结果位图
	QPainter winPainter(this);
	winPainter.drawImage(pMapWnd->geometry().x(), pMapWnd->geometry().y(), image);

	delete [] pbuf;
	pbuf=NULL;

	return;
}




void Rabbit::keyReleaseEvent ( QKeyEvent * event )
{
	if(event->key() == Qt::Key_Escape){
		m_pWnd->GetDrawParms()->SetCancel(true);
	}

}

void Rabbit::closeEvent ( QCloseEvent * event )
{
	if(m_pfileinfor!=NULL)
		m_pfileinfor->close();

	if(m_layerControl!=NULL)
		m_layerControl->close();
}




// 打开文件
void Rabbit::actionOpen()
{
	QString strDir;
	QString strFilter = "All Files (";

	QString strSingleFilter;
	for(int i=0; gfileDetail[i].iType!=CRFile::None; ++i){
		strFilter += "*.";
		strFilter += gfileDetail[i].chExt;
		strFilter += " ";	

		strSingleFilter += gfileDetail[i].chDri;
		strSingleFilter += " (*.";
		strSingleFilter += gfileDetail[i].chExt;
		strSingleFilter += ");;";
	}
	strFilter = strFilter.trimmed();
	strFilter += ");;";

	strFilter += strSingleFilter;


	QString strfile = QFileDialog::getOpenFileName(this,
		tr("Open File"), "./", strFilter);

	if(strfile.isEmpty()){
		return ;
	}

	string strfileName = strfile.toLocal8Bit().data();
	// 打开文件，获取到数据源
	RDataSource* pDs = m_pWkspc->AddDataSource(strfileName);
	if(pDs != NULL)
	{
		if(pDs->GetType() == RDataSource::RDsFileVector)
		{
			RDataSourceVector* pDsVector = (RDataSourceVector*)pDs;
			OGRDataSource* poDs = pDsVector->GetOGRDataSource();
			int iCount = poDs->GetLayerCount();
			vector<string> arrDtNames;
			for(int i=0; i<iCount; i++)
			{
				string strName = poDs->GetLayer(i)->GetName();
				arrDtNames.push_back(strName);
			}

			m_pDsTreeViewCtrl->AddItem(strfileName, arrDtNames);
		}
		else if(pDs->GetType() == RDataSource::RDsFileRaster)
		{
			RDataSourceRaster* pDsRaster = (RDataSourceRaster*)pDs;
			RFileRaster* pFileRaster = pDsRaster->GetFileRaster();
			string strDtName = CRFile::GetTitleNameFromPath(strfileName);
			vector<string> arrDtNames;
			arrDtNames.push_back(strDtName);
			m_pDsTreeViewCtrl->AddItem(strfileName, arrDtNames);
		}
	}


	m_pWnd->SetRedrawFlag();
	string strLog = "打开文件成功.";
	RLog::Print(RLog::Info, strLog);
	//update();
	return;
}

void Rabbit::actionClose()
{
	QItemSelectionModel* pModel = m_pDsTreeViewCtrl->selectionModel();
	if(pModel!=NULL)
	{
		QModelIndex modeIndex = pModel->currentIndex();
		QModelIndex miParent = modeIndex.parent();
		if(!miParent.isValid()) // 没有父节点,说明是数据源
		{ 
			QString s= modeIndex.data().toString();  
			//QMessageBox::about(this,s,s);  
			string strDsName = s.toLocal8Bit().data();
			m_pDsTreeViewCtrl->RemoveItem(strDsName);
			m_pWkspc->RemoveDataSource(strDsName);
			return;
		}
	}
}

void Rabbit::actionSelect()
{
	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd != NULL)
	{
		pMapWnd->ActionSelect();
	}
}


void Rabbit::actionSelectPoint()
{
	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd != NULL)
	{
		pMapWnd->ActionSelectPoint();
	}
}

void Rabbit::actionSelectRect()
{
	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd != NULL)
	{
		pMapWnd->ActionSelectRect();
	}
}

void Rabbit::actionPan()
{
	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd != NULL)
	{
		pMapWnd->ActionPan();
	}
}

void Rabbit::actionZoomIn() // 放大
{
	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd != NULL)
	{
		pMapWnd->ActionZoomIn();
	}
}

void Rabbit::actionZoomOut() // 放大
{
	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd != NULL)
	{
		pMapWnd->ActionZoomOut();
	}
}

void Rabbit::actionZoomEntire()
{
	RMapWidget* pMapWnd = activeMap();
	if(pMapWnd != NULL)
	{
		pMapWnd->GetMap()->ZoomEntire();
		pMapWnd->update();
	}

}


void Rabbit::action_layerControl() // 图层控制处理 
{
	//////////////////////////////////////////////////////////////////////////
	RLayerControl layCtrl(this);
	if(layCtrl.GetParent()==NULL)
		layCtrl.SetParent(this);

	layCtrl.SetMap(m_pWnd);
	layCtrl.RefreshLayers();
	QRect rcLayerCtrl = layCtrl.geometry();
	QPoint iPos; // 窗口定位到父窗口中间
	//iPos.setX( m_centralWidget->geometry().x() + m_iwin_width*0.5 - rcLayerCtrl.width()*0.5);
	//iPos.setY( m_centralWidget->geometry().y() + m_iwin_height*0.5 - rcLayerCtrl.height()*0.5);

	layCtrl.exec();
	int iResult = layCtrl.result();
	if(iResult == QDialog::Accepted)
		update();

	return;
	//////////////////////////////////////////////////////////////////////////

	if(m_layerControl==NULL)
		m_layerControl = new RLayerControl(this);

	//m_layerControl->clear(); // 先清除里面的图层
	m_layerControl->SetMap(m_pWnd);
	if(m_layerControl->GetParent()==NULL)
		m_layerControl->SetParent(this);

	m_layerControl->RefreshLayers();

	QRect rcLayCtl = m_layerControl->geometry();
	QPoint ipos; // 窗口定位到父窗口中间
	//ipos.setX( m_centralWidget->geometry().x() + m_iwin_width*0.5 - rcLayCtl.width()*0.5);
	//ipos.setY( m_centralWidget->geometry().y() + m_iwin_height*0.5 - rcLayCtl.height()*0.5);

	m_layerControl->move(ipos);
	m_layerControl->show();
	m_layerControl->raise();
	//m_layerControl->activateWindow();
}

void Rabbit::actionDBDialog()
{
	RDialogDB dbDlg(this);
	dbDlg.setWindowTitle(QString::fromLocal8Bit("打开Oracle Spatial"));
	dbDlg.exec();

	bool bNeedRefresh=false;
	if(dbDlg.result() == QDialog::Accepted){
		string strServer = dbDlg.GetServer();
		string strUser = dbDlg.GetUser();
		string strPwd = dbDlg.GetPwd();

		string strConn = "OCI:" + strUser + "/" + strPwd + "@" + strServer;
		
		OGRDataSource* poDs = OGRSFDriverRegistrar::Open(strConn.c_str());
		if(poDs !=NULL ){
///			m_pWnd->AddDataSource(poDs);
			bNeedRefresh=true;
			//m_pDsTreeViewCtrl->SetDataSource(*(m_pWnd->GetDataSources()));
		}
	}
	if(bNeedRefresh && m_pDsTreeViewCtrl!=NULL){
		//m_pDsTreeViewCtrl->SetDataSource(*(m_pWnd->GetDataSources()));
	}
}

void Rabbit::actionDSDialog()//; // 数据源管理器
{
	QDialog dlgDsMgr;
	RDataSourceCtrl dsCtrl;
// 	if(m_pWnd!=NULL)
// 		dsCtrl.SetDataSource(*m_pWnd->GetDataSources());

	QVBoxLayout* dlgLayout = new QVBoxLayout(&dlgDsMgr);
	dlgLayout->addWidget(&dsCtrl);
	dlgDsMgr.exec();

	return;


}



void Rabbit::action_about()
{
	QString strabout = QString::fromLocal8Bit("<p><b>Rabbit Map </b>基于个人兴趣开发，作者乐于分享：）</p>"
		"<p>若需要源码，请联系：<br	>wenyulin.lin@gmail.com"
		"<br><a href=\"http://www.atolin.net\">www.atolin.net</a></p>");
	QMessageBox::about(this, tr("About RabbitMap"), strabout);
}




