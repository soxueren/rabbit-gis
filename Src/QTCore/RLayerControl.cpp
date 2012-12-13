
#include "QTCore/RLayerControl.h"

#include <QColorDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include "Base/RMapWnd.h"
//#include "Desktop/Rabbit.h"

#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include <QMessageBox>


//////////////////////////////////////////////////////////////////////////


RDataSourceCtrl::RDataSourceCtrl(QWidget *parent /*= 0*/)
{
	//m_tree = new QTreeView(this);
	//m_tree->setHeaderHidden(true);
	setHeaderHidden(true);
	//QVBoxLayout* mainLayout = new QVBoxLayout;
	//mainLayout->addWidget(m_tree);

	m_pModel = new QStandardItemModel();
	QStandardItem *itemHeader = new QStandardItem("DataSource");
	m_pModel->setHorizontalHeaderItem(0, itemHeader);

	setModel(m_pModel);


}

RDataSourceCtrl::~RDataSourceCtrl()
{

}

/*void RDataSourceCtrl::SetDataSource(vector<OGRDataSource*> arrDs)
{
	int iCount=arrDs.size();
	QStandardItemModel *model=new QStandardItemModel(iCount,1);
	QStandardItem *itemHeader = new QStandardItem("DataSource");
	model->setHorizontalHeaderItem(0, itemHeader);

	for(int i=0; i<iCount; ++i){
		OGRDataSource* pDs = arrDs[i];
		if(pDs!=NULL){
			QString strDsName = QString::fromLocal8Bit(pDs->GetName());
			QStandardItem *item = new QStandardItem(strDsName);
			item->setEditable(false);
			int ilayCount = pDs->GetLayerCount();
			for(int ilay=0; ilay<ilayCount; ++ilay){
				OGRLayer* pLayer = pDs->GetLayer(ilay);
				QString strLayName = QString::fromLocal8Bit(pLayer->GetName());
				QStandardItem *itemLay = new QStandardItem(strLayName);
				itemLay->setEditable(false);
				item->appendRow(itemLay);
			}
			model->setItem(i, 0, item);	
		}

	}
	//m_tree->setModel(model);
	setModel(model);
}*/

void RDataSourceCtrl::AddItem(OGRDataSource* pDataSource)
{
	QString strDsName = QString::fromLocal8Bit(pDataSource->GetName());
	QStandardItem *item = new QStandardItem(strDsName);
	item->setEditable(false);
	int ilayCount = pDataSource->GetLayerCount();
	for(int ilay=0; ilay<ilayCount; ++ilay){
		OGRLayer* pLayer = pDataSource->GetLayer(ilay);
		QString strLayName = QString::fromLocal8Bit(pLayer->GetName());
		QStandardItem *itemLay = new QStandardItem(strLayName);
		itemLay->setEditable(false);
		item->appendRow(itemLay);
	}
	m_pModel->appendRow(item);	
}


void RDataSourceCtrl::AddDataSource(OGRDataSource* pDataSource)
{
	if(pDataSource==NULL)
		return;

	string strDsName = pDataSource->GetName();
	bool bFind=false;
	for(int i=0; i<m_arrDataSources.size(); ++i){
		OGRDataSource* pDataSource = m_arrDataSources[i];
		if(pDataSource!=NULL){
			if(strcmp(pDataSource->GetName(), strDsName.c_str()) == 0)
			{
				bFind=true;
				break;
			}
		}
	}

	if(!bFind){
		m_arrDataSources.push_back(pDataSource);
		AddItem(pDataSource);
	}

}

void RDataSourceCtrl::AddItem(const string& strDsName, const vector<string>& arrDtNames)
{
	QString strDataSource = QString::fromLocal8Bit(strDsName.c_str());
	QStandardItem *item = new QStandardItem(strDataSource);
	item->setEditable(false);
	int ilayCount = arrDtNames.size();
	for(int ilay=0; ilay<ilayCount; ++ilay){
		string strDtName = arrDtNames[ilay];
		QString strLayName = QString::fromLocal8Bit(strDtName.c_str());
		QStandardItem *itemLay = new QStandardItem(strLayName);
		itemLay->setEditable(false);
		item->appendRow(itemLay);
	}
	m_pModel->appendRow(item);
}

void RDataSourceCtrl::RemoveItem(const string& strDsName)
{
	QString strDataSource = QString::fromLocal8Bit(strDsName.c_str());
	int iCount = m_pModel->rowCount();
	for(int i=0; i<iCount; i++)
	{
		QModelIndex mIndex = m_pModel->index(i, 0);
		{
			QString s = mIndex.data().toString();
			if(s == strDataSource)
			{
				m_pModel->removeRow(i);
				break;
			}
		}
	}
}





void RDataSourceCtrl::OnTreeViewDoubleClicked(const QModelIndex& modeIndex)
{
	QModelIndex miParent = modeIndex.parent();
	if(!miParent.isValid()){ // 在数据源上双击什么都不做
		return;
	}

	QAbstractItemModel* m=(QAbstractItemModel*)modeIndex.model();  

	int iRowCount = m->rowCount();
	int iColCount = m->columnCount();

	//QModelIndexList listModelIndex = selectedIndexes();

	for(int irow = 0; irow < iRowCount; irow++)  
	{  
		int iCurRow = miParent.row();
		QModelIndex x=m->index(irow, 0);  

		QString s= x.data().toString();  
		QMessageBox::about(this,s,s);  
	}


}

void RDataSourceCtrl::OnTreeViewRightBntDown(const QModelIndex& modelIndex)
{
	QModelIndex miParent = modelIndex.parent();
	if(!miParent.isValid()){ // 在数据源上双击什么都不做
		return;
	}


}




//////////////////////////////////////////////////////////////////////////

RLayerControl::RLayerControl(QWidget *parent /*= 0*/)
{
	m_pWnd=NULL;
	m_pParent=NULL;
	
	QVBoxLayout * mainLayout = new QVBoxLayout(this);
	mainLayout->setAlignment(Qt::AlignBottom);
	setLayout(mainLayout);
	
	QHBoxLayout* layAdd = new QHBoxLayout; // 添加、删除图层控制
	m_buttonAdd = new QPushButton("&Add...");
	QPushButton *buttonRemove = new QPushButton("&Remove");
	connect(buttonRemove, SIGNAL(clicked()), this, SLOT(RemoveLayer())); // 移除图层
	layAdd->addWidget(m_buttonAdd);
	layAdd->addWidget(buttonRemove);
	layAdd->setAlignment(Qt::AlignBottom); // 按钮保持在底部
	
	QGroupBox * grpAdd = new QGroupBox("Layers");
	grpAdd->setLayout(layAdd);
	grpAdd->setAlignment(Qt::AlignBottom);

	QHBoxLayout * layOrder = new QHBoxLayout; // 调整顺序
	QPushButton * buttonUp = new QPushButton("&Up");
	QPushButton * buttonDown = new QPushButton("&Down");
	connect(buttonUp, SIGNAL(clicked()), this, SLOT(UpLayer())); // 上移图层
	connect(buttonDown, SIGNAL(clicked()), this, SLOT(DownLayer())); // 下移图层
	layOrder->addWidget(buttonUp);
	layOrder->addWidget(buttonDown);
	layOrder->setAlignment(Qt::AlignBottom);

	QGroupBox * grpOrder = new QGroupBox("Reorder");
	grpOrder->setLayout(layOrder);
	grpOrder->setAlignment(Qt::AlignBottom);


	m_lstLayers = new QListWidget;
	m_lstLayers->setAutoScroll(true);
	connect(m_lstLayers, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(StyleSetting(QListWidgetItem *)));

	QHBoxLayout * layCtl = new QHBoxLayout;
	layCtl->setAlignment(Qt::AlignBottom);
	layCtl->addWidget(grpAdd);
	layCtl->addWidget(grpOrder);

	QPushButton * butttonOK = new QPushButton("&OK");
	QPushButton * butttonCancel = new QPushButton("&Cancel");
	connect(butttonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(butttonCancel, SIGNAL(clicked()), this, SLOT(close()));

	QVBoxLayout* layOK = new QVBoxLayout;
	layOK->setAlignment(Qt::AlignTop);
	layOK->addWidget(butttonOK);
	layOK->addWidget(butttonCancel);

	QVBoxLayout* layLstCtrl = new QVBoxLayout;
	layLstCtrl->addWidget(m_lstLayers);
	layLstCtrl->addLayout(layCtl);

	QHBoxLayout * layAll = new QHBoxLayout;
	layAll->addLayout(layLstCtrl);
	layAll->addLayout(layOK);

	mainLayout->addLayout(layAll);

	setWindowTitle("Layer Control");
}

RLayerControl::~RLayerControl()
{

}

void RLayerControl::SetParent(QObject* pParent)
 {
	 m_pParent=pParent;
	 bool bconn = connect(m_buttonAdd, SIGNAL(clicked()), m_pParent, SLOT(open())); // 添加图层
 }

void RLayerControl::RefreshLayers()
{
	if(m_pWnd==NULL)
		return;

	m_lstLayers->clear();
	RLayers* arrlayers = m_pWnd->GetLayers();
	for(int i=0; i<arrlayers->size(); i++){
		RLayer* layer = (*arrlayers)[i];
		if(layer != NULL){
			QListWidgetItem *newItem = new QListWidgetItem;
			QString strName = QString::fromLocal8Bit(layer->GetName().c_str());
			newItem->setText(strName);
			m_lstLayers->addItem(newItem);
		}
	}
}

void RLayerControl::StyleSetting(QListWidgetItem *item)
{
	QColorDialog colorDlg;
	colorDlg.exec();

	int iResult(0);
	iResult = colorDlg.result();
	if(iResult){
		QColor color = colorDlg.selectedColor();		
		QString strName = item->text();
		string strLayName = strName.toLocal8Bit().data();// .toStdString();

		if(GetMap() != NULL){
			RLayers* arrLayers = GetMap()->GetLayers();
			RLayer* layer = arrLayers->GetLayerByName(strLayName);

			if(layer==NULL)
				return;

			char chcolor[6];
			sprintf(chcolor, "%d", color.rgb());
			string strColor=chcolor;		
			layer->AddStyle("color", strColor);		
			GetMap()->SetRedrawFlag();
		}
	}
}

void RLayerControl::RemoveLayer()
{
	if(m_pParent==NULL)
		return;

	QListWidgetItem *selectItem = m_lstLayers->currentItem();
	if(selectItem==NULL)
		return;

	// 得到图层名称
	QString strName = selectItem->text();
	string strLayName = strName.toLocal8Bit().data();
	RLayers* arrLayers = GetMap()->GetLayers();
	vector<RLayer*>::iterator iterLay;
	for(iterLay=arrLayers->begin(); iterLay!=arrLayers->end(); ++iterLay){
		if((*iterLay)->GetName() == strLayName){
			delete *iterLay; // 一定要释放内存，当心内存泄漏哦
			arrLayers->erase(iterLay); // 内存释放完，才能删除元素
			m_lstLayers->removeItemWidget(selectItem);
			RefreshLayers();
			GetMap()->SetRedrawFlag();
			break;
		}
	}

}

void RLayerControl::UpLayer() // 上移一层
{
	if(m_pParent==NULL)
		return;

	QListWidgetItem *selectItem = m_lstLayers->currentItem();
	if(selectItem==NULL)
		return;

	// 得到图层名称
	QString strName = selectItem->text();
	string strLayName = strName.toLocal8Bit().data();

	RLayers* arrLayers = GetMap()->GetLayers();
	for(int i=0; i<arrLayers->size(); i++){
		RLayer* laySelect = (*arrLayers)[i];
		if(laySelect->GetName() == strLayName){
			if(i==0)
				continue;

			RLayer* tmp = (*arrLayers)[i-1];
			(*arrLayers)[i-1] = laySelect;
			(*arrLayers)[i] = tmp;

			RefreshLayers();
			GetMap()->SetRedrawFlag();
			break;
		}
	}
}


void RLayerControl::DownLayer() // 下移一层
{
	if(m_pParent==NULL)
		return;

	QListWidgetItem *selectItem = m_lstLayers->currentItem();
	if(selectItem==NULL)
		return;

	// 得到图层名称
	QString strName = selectItem->text();
	string strLayName = strName.toLocal8Bit().data();

	RLayers* arrLayers = GetMap()->GetLayers();
	for(int i=0; i<arrLayers->size(); i++){
		RLayer* laySelect = (*arrLayers)[i];
		if(laySelect->GetName() == strLayName){
			if(i+1==arrLayers->size())
				continue;

			RLayer* tmp = (*arrLayers)[i+1];
			(*arrLayers)[i+1] = laySelect;
			(*arrLayers)[i] = tmp;

			RefreshLayers();
			GetMap()->SetRedrawFlag();
			break;
		}
	}
}

