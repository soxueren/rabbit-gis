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

// ʸ��ͼ���̨������
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

/*! \brief ����ͼ����
ͼ���ǵ�ͼ�ĳ�����ɲ���
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
	string m_strName; // ͼ������

	RDrawParameters* m_pDrawParams;
	RDataSource * m_pDataSource; // ͼ�����ڵ�����Դ
	RMap* m_pMap; // ��ͼ

	map<string, string> m_layStyles; // ��Ⱦ���
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
// ��Ļ���ٲ�
class BASE_API RTrackingLayer
{
public:
	RTrackingLayer();
	~RTrackingLayer();

	void AddGeometry(OGRGeometry* pGeometry, const RStyle& geomStyle);
	void RemoveAll();

	int GetGeometryCount() const {return m_arrGeometrys.size();}

	vector<OGRGeometry*> m_arrGeometrys; // ���ٲ��������
	vector<RStyle> m_arrStyles; // ���ٲ����ķ�����飬�����һһ��Ӧ

};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////


// ״̬�ص������� ����
// 1.��������λ��
// 2.������Ϣ
typedef void (__stdcall * MapStatusCallBackFun) (unsigned long pHandle, const string& strMsg);


/*! \brief �����ͼ��ʾ��
��ͼ������ͼ�����
*/
class BASE_API RMap
{
public:
	RMap();
	~RMap();

	void* m_pMainWnd; // ���ڻ�ȡ����Դ

	void Resize(int iw, int ih);//{m_iwidth=iw; m_iheight=ih;};

	/*! \brief ��Ⱦ��ͼ���ػ洰��
	*  \param [in] iw ���ڿ��(�豸����).
	*  \param [in] ih ���ڸ߶ȣ��豸���꣩.
	*  \param [out] buf ��Ļλͼ.
	*  \return void.
	*/
	void Draw(int iw, int ih, unsigned char*& buf);

	/*! \brief ���Ź��̣�����Ⱦһ���ڴ�λͼ�������û�����
	*  \param [in] iw ���ڿ��(�豸����).
	*  \param [in] ih ���ڸ߶ȣ��豸���꣩.
	*  \param [in] iPosx ���ŵĶ�λ��x���豸���꣩.
	*  \param [in] iPosy ���ŵĶ�λ��y���豸���꣩.
	*  \param [in] bZoomIn �Ŵ�����С���豸���꣩.
	*  \return void.
	*/
	void DrawWhenZoom(int iw, int ih, int iPosx, int iPosy, bool bZoomIn=true);

	
	/*! \brief ����Ŵ�����Ⱦһ���ڴ�λͼ�������û�����
	*  \param [in] iw ���ڿ��(�豸����).
	*  \param [in] ih ���ڸ߶ȣ��豸���꣩.
	*  \param [in] ileft ����Χleft���豸���꣩.
	*  \param [in] itop ����Χtop���豸���꣩.
	*  \param [in] iright ����Χright���豸���꣩.
	*  \param [in] ibottom ����Χbottom���豸���꣩.
	*  \return void.
	*/
	void DrawWhenZoomRect(int iw, int ih, int ileft, int itop, int iright, int ibottom);

	void DrawTrackingLayer(int iWinWidth, int iWinHeight);
	
	unsigned char* GetDrawBuf() const {return m_pBuf;}
	unsigned char* GetDrawBufCached() const {return m_pBufCache2;};

	//{{ ���淽���Ĳ������굥λ��Ϊ��Ļ���ӿڣ�����
	void Pan(int idx, int idy);
	void ZoomIn(int x, int y);
	void ZoomOut(int x, int y);
	void ZoomEntire();
	void ZoomRect(int ileft, int itop, int iright, int ibottom, bool bin=true);
	void MouseMove(int idx, int idy); // ������ƫ����
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
	unsigned char* m_pBuf; // ��Ҫ��Ⱦ���ڴ棬��Ļλͼ

	// �������棬��Ļλͼm_pBuf�Ļ��汸��
	// ��Ļ�ƶ�/���Ų���ʱʹ��
	unsigned char* m_pBufCache2;
	
	bool m_bFirstOnDraw; // ��ͼ��һ�δ� 
	RTrackingLayer m_layerTracking; // ��Ļ���ٲ㼸�ζ���,����ʵʱ��ʾ���켣

	RDrawParameters m_drawPrams;
	RLayers m_arrLayers; // ������ʾ��ͼ������
	unsigned int m_pWndHandler; // ����ָ��
	RRenderDataPreparedFun m_pRRenderDataPreparedFun;

	RDataSource* m_pDataSource; // ָ���ͼ����

	MapStatusCallBackFun m_pMapStatusCallBackFun;
	unsigned long m_pMapStatusHandle;
};

#endif