#ifndef _RDATASOURCE_H_
#define _RDATASOURCE_H_

#pragma once
#include <string>
#include <vector>
using namespace std;


#include "Base/RDefine.h"

// ����ʾ�����Ѿ�����׼���ã��Ļص�����
typedef void (__stdcall * RRenderDataPreparedFun) (unsigned int pWnd);

// ��Ƭ������
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

	unsigned int m_pWndHandler; // ����ָ��
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
	
	RTileDownload m_TileDownLoader; // ������
	double m_dLeft,
		m_dTop,
		m_dRight,
		m_dBottom;
};

#define TDT_EMAP "Emap" // ��ͼ
#define TDT_ANNO "Anno" // ���ĵ�����ע
#define TDT_ANNOE "AnnoE" // Ӣ�ĵ�����ע

// ����Դ������, ����������������
// ʸ���ļ�
// Ӱ���ļ�
// �����ͼ
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

// ���ͼ
class BASE_API RDataSourceMapWorld : public RDataSource
{
public:
	RDataSourceMapWorld();
	~RDataSourceMapWorld();
};


// �����ռ�: ��������Դ����
class BASE_API RWorkspace
{
public:
	RWorkspace();
	~RWorkspace();

	// �����ռ�����������Դ
	RDataSource* AddDataSource(const string& strFileName);
	void RemoveDataSource(const string& strDsName);

	void AddDataSource(RDataSource* pDataSource);

	// ͨ�����Ʋ�������Դ
	RDataSource* GetDataSource(const string& strDsName);

	// �ر���������Դ
	void Close();

private:

	// ����Դ����
	vector<RDataSource* > m_pDataSources;
};


#endif //_RDATASOURCE_H_