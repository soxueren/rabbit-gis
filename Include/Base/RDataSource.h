#ifndef _RDATASOURCE_H_
#define _RDATASOURCE_H_

#pragma once
#include <string>
#include <vector>
using namespace std;


#include "Base/RDefine.h"

// 待显示数据已就绪（准备好）的回调函数
typedef void (__stdcall * RRenderDataPreparedFun) (unsigned int pWnd);

// 瓦片下载类
class BASE_API RTileDownload
{
public:
	RTileDownload();
	~RTileDownload(){};


	void Start();

	void SetDownLoadURLs(const vector<string>& arrURLs);
	vector<string>* GetDownLoadURLs(){return &m_arrDownLoadURLs;}
	
	void SetDownLoadFiles(const vector<string>& arrFiles);
	vector<string>* GetDownLoadFiles() {return &m_arrDownLoadFiles;}

	int GetCurrDownloadID() { return m_iDownloadCount;}
	void AddDownloadID() {m_iDownloadCount++;}
	
	void SendFinishedMsg();

	void SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd);

protected:


private:


	vector<string> m_arrDownLoadFiles;
	vector<string> m_arrDownLoadURLs;

	int m_iThreadCount;
	int m_iDownloadCount;

	unsigned int m_pWndHandler; // 窗口指针
	RRenderDataPreparedFun m_pRRenderDataPreparedFun;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class BASE_API RDataset
{
public:
	RDataset();
	~RDataset();

	string GetName() const {return m_strName;}
	void SetName(const string& strName){ m_strName=strName;}

	void  RequestTileData(double dLeft, double dTop, double dRight, double dBottom, int iLevel);

	void GetViewIMGData(double dLeft, double dTop, double dRight, double dBottom, int iLevel,
		int iWidth, int iHeight, unsigned char*& pBuf);

	void GetRequestURL(double dLeft, double dTop, double dRight, double dBottom, int iLevel);

	void DownLoadTiles(const vector<string>& arrURLs, const string& strPath);
	int DownLoadOneFile(const string& strURL, const string& strLocalFile);
	
	void SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd);


	void GetRowAndCol(double dLeft, double dTop, double dRight, double dBottom, int iLevel,
		int& iRowStart, int& iRowEnd, int& iColStart, int& iColEnd, double& dTileWidth);

protected:

	string GetFileName(int iLevel, const string& strType, int iRow, int iCol);
	string GetURLName(int iLevel, const string& strType, int iRow, int iCol);
	string m_strName;
	
	RTileDownload m_TileDownLoader; // 下载器
	double m_dLeft,
		m_dTop,
		m_dRight,
		m_dBottom;
};

#define TDT_EMAP "Emap" // 地图
#define TDT_ANNO "Anno" // 中文地名标注
#define TDT_ANNOE "AnnoE" // 英文地名标注

// 数据源抽象类, 涵盖以下三类数据
// 矢量文件
// 影像文件
// 网络地图
class BASE_API RDataSource
{
public:
	RDataSource();
	virtual ~RDataSource();

	typedef enum Type{
		RDsNone=0,
		RDsWeb=1,
		RDsFileVector=2,
		RDsFileRaster=3,
	} DataSourceType;

	string GetName() const {return m_strName;}
	void SetName(const string& strName){ m_strName=strName;}

	virtual bool Open(const string& strName) {return false;}

	DataSourceType GetType() const { return m_Type;}
	void SetType(int iType) {m_Type = DataSourceType(iType); }

	int GetDatasetCount() const;
	RDataset* GetDataset(int iDt) const ;
	RDataset* GetDataset(const string& strName) const ;

	void AddDataset(RDataset* pDataset);

private:

	vector<RDataset*> m_arrDatasets;
	string m_strName;
	DataSourceType m_Type;
};

//////////////////////////////////////////////////////////////////////////
class OGRDataSource;
class BASE_API RDataSourceVector : public RDataSource
{
public:
	RDataSourceVector();
	virtual ~RDataSourceVector();

	virtual bool Open(const string& strName);


	OGRDataSource* GetOGRDataSource() {return m_poDs;}

private:
	OGRDataSource* m_poDs;
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
class RFileRaster;
class BASE_API RDataSourceRaster : public RDataSource
{
public:
	RDataSourceRaster();
	virtual ~RDataSourceRaster();

	virtual bool Open(const string& strName);

	RFileRaster* GetFileRaster() {return m_pFileRaster;}

private:
	RFileRaster* m_pFileRaster;
};
//////////////////////////////////////////////////////////////////////////

// 天地图
class BASE_API RDataSourceMapWorld : public RDataSource
{
public:
	RDataSourceMapWorld();
	~RDataSourceMapWorld();
};


// 工作空间: 各种数据源管理
class BASE_API RWorkspace
{
public:
	RWorkspace();
	~RWorkspace();

	// 工作空间中新增数据源
	RDataSource* AddDataSource(const string& strFileName);
	void RemoveDataSource(const string& strDsName);

	void AddDataSource(RDataSource* pDataSource);

	// 通过名称查找数据源
	RDataSource* GetDataSource(const string& strDsName);

	// 关闭所有数据源
	void Close();

private:

	// 数据源集合
	vector<RDataSource* > m_pDataSources;
};


#endif //_RDATASOURCE_H_