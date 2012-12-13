
#ifndef __LAYERCONTROL_H__
#define __LAYERCONTROL_H__

#pragma once

//#include <QListWidget>
#include <QDialog>
#include <QTreeView>
#include <map>
#include <string>
#include "Base/RDefine.h"

using namespace std;

class RMap;
//class Rabbit;
class QPushButton;
class QListWidgetItem;
class QListWidget;

class OGRDataSource;
//class QTreeView;
class QModelIndex;
class GDALDataset;
class QStandardItemModel;

/*! \class RDataSourceCtrl
 * \brief 数据源管理控件 
 */
class RQTCORE_API RDataSourceCtrl : public QTreeView //QWidget{
{
	Q_OBJECT

public:
	explicit RDataSourceCtrl(QWidget *parent = 0);
	~RDataSourceCtrl();

	void AddDataSource(OGRDataSource* pDataSource);

	void AddItem(const string& strDsName, const vector<string>& arrDtNames);
	void RemoveItem(const string& strDsName);

	public slots:
		void OnTreeViewDoubleClicked(const QModelIndex& modeIndex);
		void OnTreeViewRightBntDown(const QModelIndex& modelIndex);

private:
	void AddItem(OGRDataSource* pDataSource);
	//QTreeView *m_tree;
	QStandardItemModel *m_pModel;
	vector<OGRDataSource* > m_arrDataSources;
	vector<GDALDataset*> m_arrDatasets;

	map<string, vector<string>> m_dictItems;// 数据源及数据集信息

};

//////////////////////////////////////////////////////////////////////////

class RQTCORE_API RLayerControl : public QDialog { //QWidget { //  QListWidget{
	Q_OBJECT

public:
	explicit RLayerControl(QWidget *parent = 0);
	//RLayerControl();
	~RLayerControl();

	void SetMap(RMap* pWnd)  {m_pWnd=pWnd;}
	RMap* GetMap() const { return m_pWnd;}

	void SetParent(QObject* pParent);
	QObject* GetParent() const { return m_pParent;}
	
	void RefreshLayers();

	public slots:
		void StyleSetting(QListWidgetItem *item);
		void RemoveLayer();
		void UpLayer(); // 上移一层
		void DownLayer(); // 下移一层

private:

	RMap *m_pWnd;
	QObject* m_pParent;

	QListWidget* m_lstLayers;
	QPushButton *m_buttonAdd;

	map<string, string> m_kvStyles; // 对图层的风格设置；

};


#endif