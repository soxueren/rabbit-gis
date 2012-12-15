/** \file RMap.h
 * A brief file description.
 * A more elaborated file description.
 */

#ifndef RMAP_WND_H
#define RMAP_WND_H 

#pragma once

#include <string>
#include <vector>
#include <map>
#include "Base/RFileRaster.h"
#include "Base/RDrawFactroy.h"

using namespace std;

class RFileVector;
class OGRLayer;
class OGRDataSource;
class RDataset;
class RDataSource;

typedef void (__stdcall * RRenderDataPreparedFun) (unsigned int pWnd);

// 矢量图层后台绘制类
class ROGRLayerOnDrawBackground
{
public:

	ROGRLayerOnDrawBackground();
	~ROGRLayerOnDrawBackground();

	void SetOGRLayer(OGRLayer* pLayer);
	OGRLayer* GetOGRLayer(){return m_pLayer;}

	void SetDrawParams(RDrawParameters* pDrawParams);
	RDrawParameters* GetDrawParams(){return m_pDrawParams;}

	void SetDeviceContext(int idcWidth, int idcHeight, unsigned char* pBuf);
	void GetDeviceContext(int& iWidth, int& iHeight, unsigned char*& pBuf);

	RDrawFactory* GetDrawFactory(){ return &m_Drawer;}

	void StartDrawBackground();

	void SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd);

	void SendFinishedMsg();

	unsigned char* GetDrawData(){ return m_pBuf;}


private:
	OGRLayer* m_pLayer;
	RDrawParameters* m_pDrawParams;
	int m_iWinWidth, m_iWinHeight;
	unsigned char* m_pBuf;

	RDrawFactory m_Drawer;
	RRenderDataPreparedFun m_pRenderDataPreparedFun;
	unsigned int m_pWndHander;
	double m_dLeftPer, m_dTopPer, m_dRightPer, m_dBottomPer;
};

//////////////////////////////////////////////////////////////////////////

/*! \brief 抽象图层类
图层是地图的抽象组成部分
*/
class RMap;
class BASE_API RLayer 
{
public:

	RLayer();
	RLayer(RDataSource* pDataSource);
	virtual ~RLayer();

	string GetName() const {return m_strName;}
	void SetName(const string& strName) {m_strName=strName;}

	void SetDataSource(RDataSource * pDataSource){m_pDataSource=pDataSource;}
	RDataSource* GetDataSource() const {return m_pDataSource;}

	void SetMap(RMap* pMap) {m_pMap=pMap;}
	RMap* GetMap() {return m_pMap;}

	void SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd)
	{
		m_layerOnDrawBackground.SetRRenderDataPreparedFun(pRRenderDataPreparedFun, pWnd);
	}

	void SetDrawParams(RDrawParameters* pDrawParams) {m_pDrawParams=pDrawParams;}
	RDrawParameters* GetDrawParams() const {return m_pDrawParams;}

	void AddStyle(const string& strKey, const string& strValue);

	void Draw(int iwidth, int iheight, unsigned char *& buf);
	void Draw(OGRLayer* pLayer, int iWidth, int iHeight, unsigned char *& pBuf);
	void Draw(RFileRaster* pFileRaster, int iWidth, int iHeight, unsigned char*& pBuf);
	void Draw(RDataset* pDataset, int iWidth, int iHeight, unsigned char*& pBuf);

protected:

	int DrawRaster(int iw, int ih, unsigned char*& buf);

	int DrawRGB24(int iw, int ih, unsigned char*& buf);

private: 

	ROGRLayerOnDrawBackground m_layerOnDrawBackground;
	string m_strName; // 图层名称

	RDrawParameters* m_pDrawParams;
	RDataSource * m_pDataSource; // 图层所在的数据源
	RMap* m_pMap; // 地图

	map<string, string> m_layStyles; // 渲染风格
};

class BASE_API RLayers : public vector<RLayer*>
{
public:
	RLayers(){};
	~RLayers();

	void GetBound(double* dleft, double* dtop, double* dright, double* dbottom) const;
	RLayer* GetLayerByName(const string& strName);

};

//////////////////////////////////////////////////////////////////////////
// 屏幕跟踪层
class BASE_API RTrackingLayer
{
public:
	RTrackingLayer();
	~RTrackingLayer();

	void AddGeometry(OGRGeometry* pGeometry, const RStyle& geomStyle);
	void RemoveAll();

	int GetGeometryCount() const {return m_arrGeometrys.size();}

	vector<OGRGeometry*> m_arrGeometrys; // 跟踪层对象数组
	vector<RStyle> m_arrStyles; // 跟踪层对象的风格数组，与对象一一对应

};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////


// 状态回调函数， 包括
// 1.鼠标点坐标位置
// 2.进度信息
typedef void (__stdcall * MapStatusCallBackFun) (unsigned long pHandle, const string& strMsg);


/*! \brief 抽象地图显示类
地图有若干图层组成
*/
class BASE_API RMap
{
public:
	RMap();
	~RMap();

	void* m_pMainWnd; // 用于获取数据源

	void Resize(int iw, int ih);//{m_iwidth=iw; m_iheight=ih;};

	/*! \brief 渲染地图、重绘窗口
	*  \param [in] iw 窗口宽度(设备坐标).
	*  \param [in] ih 窗口高度（设备坐标）.
	*  \param [out] buf 屏幕位图.
	*  \return void.
	*/
	void Draw(int iw, int ih, unsigned char*& buf);

	/*! \brief 缩放过程，先渲染一遍内存位图，提升用户体验
	*  \param [in] iw 窗口宽度(设备坐标).
	*  \param [in] ih 窗口高度（设备坐标）.
	*  \param [in] iPosx 缩放的定位点x（设备坐标）.
	*  \param [in] iPosy 缩放的定位点y（设备坐标）.
	*  \param [in] bZoomIn 放大还是缩小（设备坐标）.
	*  \return void.
	*/
	void DrawWhenZoom(int iw, int ih, int iPosx, int iPosy, bool bZoomIn=true);

	
	/*! \brief 拉框放大，先渲染一遍内存位图，提升用户体验
	*  \param [in] iw 窗口宽度(设备坐标).
	*  \param [in] ih 窗口高度（设备坐标）.
	*  \param [in] ileft 拉框范围left（设备坐标）.
	*  \param [in] itop 拉框范围top（设备坐标）.
	*  \param [in] iright 拉框范围right（设备坐标）.
	*  \param [in] ibottom 拉框范围bottom（设备坐标）.
	*  \return void.
	*/
	void DrawWhenZoomRect(int iw, int ih, int ileft, int itop, int iright, int ibottom);

	void DrawTrackingLayer(int iWinWidth, int iWinHeight);
	
	unsigned char* GetDrawBuf() const {return m_pBuf;}
	unsigned char* GetDrawBufCached() const {return m_pBufCache2;};

	//{{ 下面方法的参数坐标单位均为屏幕（视口）坐标
	void Pan(int idx, int idy);
	void ZoomIn(int x, int y);
	void ZoomOut(int x, int y);
	void ZoomEntire();
	void ZoomRect(int ileft, int itop, int iright, int ibottom, bool bin=true);
	void MouseMove(int idx, int idy); // 参数是偏移量
	//}}

	RDrawParameters* GetDrawParms() { return &m_drawPrams;}

	RLayers* GetLayers() { return &m_arrLayers; }

	void SetDataSource(RDataSource* pDataSource){ m_pDataSource=pDataSource;}
	RDataSource* GetDataSource(){ return m_pDataSource;}

	void AddLayer(RLayer* pLayer);

	void SetRedrawFlag(bool redraw=true){ m_bRedraw=redraw;}
	bool GetRedrawFlag() {return m_bRedraw;}

	void SetRRenderDataPreparedFun(RRenderDataPreparedFun pRRenderDataPreparedFun, unsigned int pWnd);

	RTrackingLayer* GetTrackingLayer()  {return &m_layerTracking;}

	void SetMapStatusCallBackFun(MapStatusCallBackFun pFunc) {m_pMapStatusCallBackFun=pFunc;}
	MapStatusCallBackFun GetMapStatusCallBackFun() {return m_pMapStatusCallBackFun;}

	void SetMapStatusHandle(unsigned long pHandle) {m_pMapStatusHandle=pHandle;}
	unsigned long GetMapStatusHandle() { return m_pMapStatusHandle;}

protected:

	void DrawOnFirstTime(int idcWidth, int idcHeight);

	int draw_rgb24(int iw, int ih, unsigned char*& buf);
	int draw_rgb24_byblock(int iw, int ih, unsigned char*& buf);

	int draw_gray8(int iw, int ih, unsigned char*& buf);
	int draw_gray8_byblock(int iw, int ih, unsigned char*& buf);

	int draw_int16(int iw, int ih, unsigned char*& buf);

	int draw_layer(OGRLayer* player, int iw, int ih, unsigned char*& buf);

private:
	OGRLayer* GetLayerVector(const string& strDsName, const string& strLayerName);
	RDataset* GetLayerWeb(const string& strDsName, const string& strLayerName);

private:
	int m_iwidth;
	int m_iheight;

	bool m_bRedraw;
	unsigned char* m_pBuf; // 需要渲染的内存，屏幕位图

	// 二级缓存，屏幕位图m_pBuf的缓存备份
	// 屏幕移动/缩放操作时使用
	unsigned char* m_pBufCache2;
	
	bool m_bFirstOnDraw; // 地图第一次打开 
	RTrackingLayer m_layerTracking; // 屏幕跟踪层几何对象,用于实时显示鼠标轨迹

	RDrawParameters m_drawPrams;
	RLayers m_arrLayers; // 用于显示的图层数组
	unsigned int m_pWndHandler; // 窗口指针
	RRenderDataPreparedFun m_pRRenderDataPreparedFun;

	RDataSource* m_pDataSource; // 指向地图数据

	MapStatusCallBackFun m_pMapStatusCallBackFun;
	unsigned long m_pMapStatusHandle;
};

#endif