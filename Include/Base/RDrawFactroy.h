
#ifndef RDRAW_FACTORY_INCLUDE_H
#define RDRAW_FACTORY_INCLUDE_H


//#include "geos.h"

/************************************************************************/
/* 负责窗口（逻辑坐标）<=>视口（设备坐标）之间的转换
逻辑坐标是通过 将地图 的地理范围 偏移到坐标原点（0,0）得到，向右，向上为正

窗口： 决定要显示那些要素
视口： 决定要素显示到设备的什么位置

基本的两个变换理论公式如下：
1. 设备坐标点，转换为逻辑坐标点
	x = (x' - VOrgx) * WExtx / VExtx + WOrgx;
	y = (y' - VOrgy) * WExty / VExty + WOrgy;

2. 窗口点(x, y)变换为视口点(x', y')方法：
	x' = (x - WOrgx) * VExtx / WExtx + VOrgx;
	y' = (y - WOrgy) * VExty / WExty + VOrgy;

*/
/************************************************************************/

#include "agg_trans_affine.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

#include "Base/RDefine.h"

#include <map>
using namespace std;

class OGRSpatialReference;

// 绘制参数类
class  BASE_API RDrawParameters 
{
public:
	RDrawParameters();
	~RDrawParameters();

	void Init(int il, int it, int ir, int ib, double dl, double dt, double dr, double db);

	// 用户操作
	/*! \brief 平移
	*  \param [in] iOffsetX (设备坐标).
	*  \param [in] iOffsetY 设备坐标）.
	*  \return void.
	*/
	void Pan(int iOffsetX, int iOffsetY);
	
	/*! \brief 缩放
	*  \param [in] iX (设备坐标).
	*  \param [in] iY (设备坐标）.
	*  \param [in] dScale 缩放比率.
	*  \return void.
	*/
	void Zoom(int iX, int iY, double dScale);

	void ZoomEntire();
	void ZoomResolution(int imgw, int imgh); // 缩放到原始分辨率, 针对影像数据有效
	void ZoomRect(int ileft, int itop, int iright, int ibottom, bool bZoomIn=true); // 框选放大

	/*! \brief 设备坐标转逻辑坐标
	*  \param [in] iX (设备坐标).
	*  \param [in] iY (设备坐标）.
	*  \param [out] dX (逻辑坐标).
	*  \param [out] dX (逻辑坐标).
	*  \return void.
	*/
	void DPtoLP(int iX, int iY, double* dX, double* dY); // 设备坐标——》逻辑坐标 (地图坐标)

	/*! \brief 逻辑坐标转设备坐标
	*  \param [in] dX (逻辑坐标).
	*  \param [in] dX (逻辑坐标).
	*  \param [out] iX (设备坐标).
	*  \param [out] iY (设备坐标）.
	*  \return void.
	*/
	void LPtoDP(double dX, double dY, double* iX, double* iY); // 地图坐标——》设备坐标
	
	/*! \brief 调整设备宽度,用于设备窗口缩放
	*  \param [in] iWidth 窗口宽度(设备坐标).
	*  \param [in] iHeight 窗口高度（设备坐标）.
	*  \return void.
	*/
	void ChangeDP(int iWidth, int iHeight);

	//! \brief 可视范围
	void GetViewBound(double* dviewleft, double* dviewtop, double* dviewright, double* dviewbottom);

	//!\brief 地图地理范围
	void GetMapBound(double* dleft, double* dtop, double* dright, double* dbottom);
	//double GetRatio() {return m_dRatio;}
	//void SetRatio(double dRatio){m_dRatio=dRatio;}

	void SetScale(double dScale);
	double GetScale();

	//bool GetCustomRatio(){}
	//void SetCustomRatio(bool bUse){}

	double GetCustomRatio(){return m_dRatioCustom;}
	void SetCustomRatio(double dRatio){m_dRatioCustom=dRatio;}

	const agg::trans_affine& get_mtx(int iimgw, int iimgh);

	bool GetCancel() {return m_bcancel;}
	void SetCancel(bool bcancel=true){ m_bcancel=bcancel; }

	void SetSpatialReference(OGRSpatialReference* pSpatialReference) { m_pOGRSpatialReference=pSpatialReference;}
	OGRSpatialReference* GetSpatialReference() {return m_pOGRSpatialReference;}

protected:
	bool is_changed(int il, int it, int ir, int ib, double dl, double dt, double dr, double db);
	bool m_bcancel; // 取消渲染标志

private:

	int m_iLeft, m_iTop, m_iRight, m_iBottom; // 设备坐标范围 m_itop<m_ibottom
	double m_dLeft, m_dTop, m_dRight, m_dBottom; // 地图地理坐标范围
	double m_dViewLeft, m_dViewTop, m_dViewRight, m_dViewBottom; // 地图的显示范围（渲染范围）

	// 坐标转换比率 (设备坐标/逻辑坐标)
	/*
	

	*/
	double m_dScale;

	//double m_dRatio;
	double m_dDcResolution; // 设备分辨率： 单个像素点对应的地理长度
	int m_iDPI;
	double m_dRatioCustom; // 使用自定义的转换比率
	agg::trans_affine m_mtx; // 地理坐标变换矩阵

	// 用于坐标系统转换
	OGRSpatialReference* m_pOGRSpatialReference;
};

class OGRGeometry;
class OGRPoint;
class OGRLineString;
class OGRPolygon;

class RStyle{
public:
	RStyle():m_iColor(0), m_bVisible(true) {}

	unsigned int m_iColor;
	bool m_bVisible;
};

/*! \brief 显示工厂类
*/
class BASE_API RDrawFactory
{
public:

	RDrawFactory();
	~RDrawFactory();

	void Init(int iw, int ih, unsigned char* pbuf);//{m_iheight=ih; m_iwidth=iw; m_pbuf=pbuf;}

	void DrawGeometry(RDrawParameters* pDrawParms, OGRGeometry ** pGeometry,  int iCount, RStyle* layStyle);

protected:

	void DrawGeoPoint(RDrawParameters* pDrawParms, OGRPoint** pGeometry, int iCount, RStyle* layStyle);
	void DrawGeoLineString(RDrawParameters* pDrawParms, OGRLineString** pGeometry, int iCount, RStyle* layStyle);
	void DrawGeoPolygon(RDrawParameters* pDrawParms, OGRPolygon** pGeometry, int iCount, RStyle* layStyle);

private:

	int m_iwidth;
	int m_iheight;
	int m_ibytesPerLine;
	unsigned char* m_pbuf;
	
	agg::rasterizer_scanline_aa<> m_ras;
	agg::scanline_u8 m_sl;
};


#endif //RDRAW_FACTORY_INCLUDE_H
