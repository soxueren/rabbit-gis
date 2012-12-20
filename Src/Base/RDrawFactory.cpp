
#include "Base/RDrawFactroy.h"
#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgb.h"
#include "agg_renderer_base.h"
#include "agg_ellipse.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"


//////////////////////////////////////////////////////////////////////////

#define PI 3.14159265358979323846264338327950288

RDrawParameters::RDrawParameters() : m_bcancel(false),
m_iLeft(0), m_iTop(0), m_iRight(0), m_iBottom(0),
m_dLeft(0), m_dTop(0), m_dRight(0), m_dBottom(0),
m_dViewLeft(0), m_dViewTop(0), m_dViewRight(0), m_dViewBottom(0),
m_dRatioCustom(0)
{
	m_pOGRSpatialReference=NULL;
	m_iDPI=96;
	m_dDcResolution=1.0;

}
RDrawParameters::~RDrawParameters()
{

}

// 传入参数为设备坐标
/************************************************************************/
/*  窗口反向移动                                                                     */
/************************************************************************/
void RDrawParameters::Pan(int offsetx, int offsety)
{
	double dx=(double)offsetx *m_dDcResolution; // ; // 逻辑坐标 
	double dy = (double)offsety *m_dDcResolution; /// ;

#ifdef use_affine_trans
	//m_mtx *= agg::trans_affine_translation(dx, -dy);
	m_mtx *= agg::trans_affine_translation(offsetx, -offsety); // 视口向y轴正向移动，刚好等于窗口向y轴负向移动
#else
	m_dViewLeft -= dx; // 屏幕向右移动，相当于窗口（地图可视范围）左移
	m_dViewRight -= dx;
	m_dViewTop += dy; // 屏幕向下移动，相当于窗口（地图可视范围）上移
	m_dViewBottom += dy;
#endif
	}

void RDrawParameters::Zoom(int ix, int iy, double dscale)
{

#ifdef use_affine_trans

	agg::trans_affine mtx=m_mtx;

	double dx=ix, dy=iy;
	dptomp(ix, iy, &dx, &dy); // 屏幕坐标点，转换为逻辑(地图)坐标点

	double dmapw = fabs(m_dRight-m_dLeft); //  地图宽
	double dmaph = fabs(m_dBottom-m_dTop); //  地图高
	double dmapcenterx = m_dLeft+dmapw*0.5;
	double dmapcentery = m_dBottom+dmaph*0.5;

	dx=dmapcenterx; dy = dmapcentery;

	// 按照地图坐标点，进行缩放
	m_mtx *= agg::trans_affine_translation(-dx, -dy);
	m_mtx *= agg::trans_affine_scaling(dscale);
	m_mtx *= agg::trans_affine_translation(dx, dy);
#else

	// 设备点，转换为逻辑坐标点(地理坐标点)
	double dx=ix, dy=iy;
	DPtoLP(ix, iy, &dx, &dy);

	double dwinw = abs(m_iRight-m_iLeft); // 屏幕宽
	double dwinh = abs(m_iBottom-m_iTop); // 屏幕高 
	
	if(m_dRatioCustom!=0) // 固定比例尺，要严格按照级别缩放
	{
		if(dscale>1.0) // 按范围扩大
		{
			// 求出2的N次方中的N值
			double d2 = log(double(2));
			double dValue = log(dscale);
			int n=int(dValue/d2);

			dscale = 2<<n;;// 确保放大比是2的N次方
		}
		else  // 按范围缩小
		{
			// 求出2的N次方中的N值
			double d2 = log(double(2));
			double dValue = log(1.0/dscale);
			int n=int(dValue/d2);

			dscale = 1.0/(2<<n);;// 确保缩小比是 1/(2的N次方)
		}
	}

	double dRatioBack = m_dDcResolution;// 备份坐标转换比率
	m_dDcResolution /= dscale; // 别忘记更新比例尺

	//if(m_dRatioCustom!=0 && m_dRatio<m_dRatioCustom)
	{
		//m_dRatio = m_dRatioCustom;
		//dscale = m_dRatio/dRatioBack; // 缩小到此级别，不在缩放
		//return;
	}

	double dviewwidth = dwinw*m_dDcResolution; // 逻辑坐标范围宽度（地图可显示范围宽度）
	double dviewheight = dwinh*m_dDcResolution;
	double dnew_view_w = (m_dViewRight-m_dViewLeft) / (dscale); // 缩放后逻辑宽度
	double dnew_view_h = (m_dViewTop-m_dViewBottom) / (dscale);//

	// 按照当前点位置为参考，调整一下可视范围
	m_dViewLeft = dx - (dx-m_dViewLeft)/dscale;
	m_dViewRight =m_dViewLeft+dnew_view_w;
	m_dViewBottom = dy-(dy - m_dViewBottom)/dscale;// dnew_view_h*0.5;
	m_dViewTop = m_dViewBottom+dnew_view_h;

#endif
}


/************************************************************************/
/* 1. 将地图中心点移动到屏幕中心点，
	2. 再按照地图中心点缩放至屏幕大小
*/
/************************************************************************/
void RDrawParameters::ZoomEntire()
{
	double dmapw = fabs(m_dRight-m_dLeft); //  地图宽
	double dmaph = fabs(m_dBottom-m_dTop); //  地图高
	double dwinw = abs(m_iRight-m_iLeft); // 屏幕宽
	double dwinh = abs(m_iBottom-m_iTop); // 屏幕高 

	double dmapcenterx = m_dLeft+dmapw*0.5;
	double dmapcentery = m_dBottom+dmaph*0.5;

#ifdef use_affine_trans
	
	double offsetx = dwinw*0.5 - dmapcenterx; // 地图中心到屏幕中心的偏移量x
	double offsety = dwinh*0.5 - dmapcentery; // 地图中心到屏幕中心的偏移量y

	// 移动到屏幕中心点	
	agg::trans_affine mtx;
	mtx *= agg::trans_affine_translation(offsetx, offsety); // 地图中心点，移动到屏幕中心
	
	// 按照地图中心点坐标，缩放至屏幕大小
	mtx *= agg::trans_affine_translation(-dwinw*0.5, -dwinh*0.5);
	m_dRatio*=0.5;
	mtx *= agg::trans_affine_scaling(m_dRatio); // 缩放至屏幕大小范围
	mtx *= agg::trans_affine_translation(dwinw*0.5, dwinh*0.5);

	m_mtx = mtx;

#else

	double dResolutionX = fabs((m_dRight-m_dLeft) / double(m_iRight-m_iLeft));
	double dResolutionY = fabs( (m_dBottom-m_dTop) / double(m_iBottom-m_iTop));

	if(m_dRatioCustom!=0)
	{
		//m_dRatio = m_dRatioCustom;
	}
	else{

		if(dmaph/dResolutionX > dwinh)
			m_dDcResolution=dResolutionY;// 应当缩放至屏幕高度
		else if(dmapw/dResolutionY > dwinw)
			m_dDcResolution=dResolutionX;// 应当缩放至屏幕宽度
	}

	double dviewwidth = dwinw*m_dDcResolution;//; // 逻辑坐标范围宽度（地图可显示范围宽度）
	double dviewheight = dwinh*m_dDcResolution;///;

	// 调整逻辑坐标范围（地图可显示范围）
	m_dViewLeft = dmapcenterx - dviewwidth*0.5;
	m_dViewRight = dmapcenterx + dviewwidth*0.5;
	m_dViewBottom = dmapcentery - dviewheight*0.5;
	m_dViewTop = dmapcentery + dviewheight*0.5;
#endif

}

//; // 缩放到原始分辨率, 那就是说，屏幕一个像素对应图像一个像素
void RDrawParameters::ZoomResolution(int imgw, int imgh)
{
	double dmapw = fabs(m_dRight-m_dLeft); //  地图宽
	double dmaph = fabs(m_dBottom-m_dTop); //  地图高

	double dwinw = abs(m_iRight-m_iLeft); // 屏幕宽
	double dwinh = abs(m_iBottom-m_iTop); // 屏幕高 

	double dmapcenterx = m_dLeft+dmapw*0.5;
	double dmapcentery = m_dBottom+dmaph*0.5;
	
	double dx = dmapw / imgw; // 影像x分辨率
	double dy = dmaph / imgh;

	double dresolution = dx;
	if(dx>dy)
		dresolution=dy;
	
	double dviewwidth = dwinw * dresolution; // 屏幕宽度对应的逻辑坐标宽度
	double dviewheight = dwinh * dresolution; // 

	m_dDcResolution = dviewheight / dwinh; // 屏幕坐标/逻辑坐标

	// 调整逻辑坐标范围（地图可显示范围）
	m_dViewLeft = dmapcenterx - dviewwidth*0.5;
	m_dViewRight = dmapcenterx + dviewwidth*0.5;
	m_dViewBottom = dmapcentery - dviewheight*0.5;
	m_dViewTop = dmapcentery + dviewheight*0.5;
}


void RDrawParameters::ZoomRect(int ileft, int itop, int iright, int ibottom, bool bZoomIn/*=true*/)//; // 框选放大
{
	// 1. 求出框选范围的视口（设备）中心点
	int icentx=ileft + (iright-ileft)*0.5;
	int icenty=itop+(ibottom-itop)*0.5;

	//屏幕中心
	int iWinCentX = (m_iRight-m_iLeft)*0.5;
	int iWinCentY = (m_iBottom-m_iTop)*0.5;

	// 屏幕宽高
	int iWinWidth = m_iRight-m_iLeft;
	int iWinHeight = m_iBottom-m_iTop;

	// 缩放范围宽高(设备坐标)
	int iRectWidth = iright-ileft;
	int iRectHeight = ibottom-itop;

	// 2. 算一算缩放比例
	double dratiox = fabs( iRectWidth*1.0 / iWinWidth );
	double dratioy = fabs( iRectHeight*1.0 / iWinHeight );

	// 缩放范围中心点移到屏幕中心
	Pan(-(icentx-iWinCentX), -(icenty-iWinCentY));

	double dScale=dratiox;
	if(iRectWidth<iRectHeight)
		dScale=dratioy;

	if(bZoomIn)
		dScale=1.0/dScale;

	Zoom(iWinCentX, iWinCentY, dScale);
	return;
}


void RDrawParameters::Init(int il, int it, int ir, int ib, double dl, double dt, double dr, double db)
{
	if(!is_changed(il, it, ir, ib, dl, dt, dr, db))
		return;

	m_iLeft=il; m_iTop=it; m_iRight=ir; m_iBottom=ib;
	m_dLeft=dl; m_dTop=dt; m_dRight=dr; m_dBottom=db;
	m_dViewLeft=dl; m_dViewTop=dt; m_dViewRight=dr; m_dViewBottom=db;	

	ZoomEntire(); // 初次打开，应当全副显示
	return;
}

bool RDrawParameters::is_changed(int il, int it, int ir, int ib, double dl, double dt, double dr, double db)
{
	if(m_iLeft!=il || m_iTop!=it || m_iRight!=ir || m_iBottom!=ib)
		return true;
	else if(m_dLeft!=dl || m_dTop!=dt || m_dRight!=dr || m_dBottom!=db)
		return true;

	return false;

}


void RDrawParameters::DPtoLP(int ix, int iy, double* dx, double* dy)
{
	double dsrcx=ix, dsrcy=iy;

#ifdef use_affine_trans
	agg::trans_affine mtx=m_mtx;
	mtx.flip_y(); // 对象沿x轴镜像
	mtx *= agg::trans_affine_translation(0, m_iBottom); // 对象上移距离： 逻辑坐标高度
	mtx.invert();

	mtx.transform(&dsrcx, &dsrcy);
	*dx=dsrcx; 
	*dy=dsrcy;	
	return;

	*dx = (ix-m_iLeft) / m_dRatio + m_dLeft;
	*dy = (iy-m_iBottom) / m_dRatio + m_dBottom;

	m_mtx.transform(dx, dy);
#else

	// 设备坐标点，转换为逻辑坐标点
	// x = (x' - VOrgx) * WExtx / VExtx + WOrgx;
	// y = (y' - Vorgy) * WExty / VExty + WOrgy;

	*dx = (ix-m_iLeft) * m_dDcResolution + m_dViewLeft;
	*dy = (m_iBottom-iy) * m_dDcResolution + m_dViewBottom;
	//*dy = (iy-m_iTop) / m_dRatio + m_dViewBottom;

#endif
}

// 窗口点(x, y)变换为视口点(x', y')方法：
//	x' = (x - WOrgx) * VExtx / WExtx + VOrgx;
// y' = (y - WOrgy) * VExty / WExty + VOrgy;

void RDrawParameters::LPtoDP(double dx, double dy, double* ix, double* iy) // 地图坐标——》设备坐标
{
#ifdef use_affine_trans
	agg::trans_affine mtx=m_mtx;
// 	double dleft=m_dLeft;
// 	double dbottom = m_dBottom;
// 	m_mtx.transform(&dleft, &dbottom); // 地图左下角逻辑坐标
// 	
// 	mtx *= agg::trans_affine_translation(-dleft, -dbottom); // 地图左下角移动至逻辑坐标原点 
	mtx.flip_y(); // 对象沿x轴镜像
	mtx *= agg::trans_affine_translation(0, m_iBottom); // 对象上移距离： 逻辑坐标高度

	double dsrcx=dx, dsrcy=dy;
	mtx.transform(&dsrcx, &dsrcy);
	*ix=dsrcx; 
	*iy=dsrcy;	
#else

	*ix = (dx-m_dViewLeft) / m_dDcResolution + m_iLeft;
	*iy = m_iBottom - (dy-m_dViewBottom) / m_dDcResolution; //+ m_iTop; 注意！！！将屏幕坐标原点从左上角变换到左下角
#endif
}


void RDrawParameters::ChangeDP(int iWidth, int iHeight)
{
	m_iRight = m_iLeft+iWidth;
	m_iBottom = m_iTop+iHeight;

	double dviewwidth = iWidth*m_dDcResolution;//m_dRatio; // 逻辑坐标范围宽度（地图可显示范围宽度）
	double dviewheight = iHeight*m_dDcResolution;///m_dRatio;

	// 调整逻辑坐标范围（地图可显示范围）
	m_dViewRight = m_dViewLeft + dviewwidth;
	m_dViewBottom =  m_dViewTop - dviewheight;
}

void RDrawParameters::GetViewBound(double* dviewleft, double* dviewtop, double* dviewright, double* dviewbottom)
{
	*dviewleft=m_dViewLeft;
	*dviewtop=m_dViewTop;
	*dviewright=m_dViewRight;
	*dviewbottom=m_dViewBottom;
}

void RDrawParameters::GetMapBound(double* dleft, double* dtop, double* dright, double* dbottom)
{
	*dleft=m_dLeft;
	*dtop=m_dTop;
	*dright=m_dRight;
	*dbottom=m_dBottom;
}

void RDrawParameters::SetScale(double dScale)
{
	if(m_pOGRSpatialReference!=NULL && m_pOGRSpatialReference->IsGeographic()) // 地理坐标
	{			
		double dSemiMajor = m_pOGRSpatialReference->GetSemiMajor(); // 椭球长半轴 单位千米
		char* pchUnits=NULL;
		m_pOGRSpatialReference->GetLinearUnits(&pchUnits);

		double dLength = 2*PI*dSemiMajor; // 椭球沿长半轴的周长 单位千米
		double dDcResolution = 25.4/m_iDPI; // 设备分辨率: 一个像素点的逻辑长度 单位 mm
		dDcResolution = dDcResolution/dScale; // 一个像素点的地图长度 单位mm
		m_dDcResolution = dDcResolution/1000000.0/dLength*360.0; // 设备分辨率: 一个像素点的逻辑长度 单位 弧度

		// 按照可视范围中心点, 调整可显示范围 , 下面的坐标单位都是度
		double dViewCenterX = m_dViewLeft+(m_dViewRight-m_dViewLeft)*0.5; 
		double dViewCenterY = m_dViewBottom+(m_dViewTop-m_dViewBottom)*0.5;

		double dViewWidth = (m_iRight-m_iLeft) * m_dDcResolution; // 显示设备的逻辑宽度// 单位弧度
		double dViewHeight =  (m_iBottom-m_iTop) * m_dDcResolution;

		m_dViewLeft = dViewCenterX-dViewWidth*0.5;
		m_dViewRight = m_dViewLeft+dViewWidth;
		m_dViewBottom =dViewCenterY - dViewHeight*0.5;
		m_dViewTop = m_dViewBottom+dViewHeight;

		// for test
					//m_dRatio=1.0/m_dDcResolution;
		return;
	}
}

double RDrawParameters::GetScale()
{
	/*
	比例尺=(图上距离/实际距离)=(设备平面长度/地图平面长度)

	设备长度=(设备像素点数/DPI)×25.4 (毫米)
	DPI = 每英寸像素点数(DotPerInch)
	1 inch = 25.4 mm
	*/
	
	if(m_pOGRSpatialReference!=NULL && m_pOGRSpatialReference->IsGeographic()) // 地理坐标
	{			
		double dSemiMajor = m_pOGRSpatialReference->GetSemiMajor(); // 椭球长半轴 单位:米
		char* pchUnits=NULL;
		m_pOGRSpatialReference->GetAngularUnits(&pchUnits);

		double dLength = 2*PI*dSemiMajor; // 椭球沿长半轴的周长 单位:米
		double dDcResolution = 25.4/m_iDPI; // 设备分辨率: 一个像素点的逻辑长度 单位 mm
		double dScale = m_dDcResolution*(dLength*1000.0/360.0); // 一个像素点对应逻辑的长度 单位mm
		dScale = dDcResolution/dScale;
		return dScale;
	}
	else if(m_pOGRSpatialReference!=NULL&&m_pOGRSpatialReference->IsProjected())
	{
		char* pchUnits=NULL;
		m_pOGRSpatialReference->GetLinearUnits(&pchUnits);
		OGRSpatialReference* pSpatialReference = m_pOGRSpatialReference->CloneGeogCS();
		pSpatialReference->GetLinearUnits(&pchUnits);
		double dUnits = pSpatialReference->GetAngularUnits(&pchUnits);

		double dSemiMajor = pSpatialReference->GetSemiMajor(); // 椭球长半轴 单位千米
		double dLength = 2*PI*dSemiMajor; // 椭球沿长半轴的周长 单位千米
		double dDcResolution = 25.4/m_iDPI; // 设备分辨率: 一个像素点的逻辑长度 单位 mm
		double dScale = dDcResolution / (m_dDcResolution*1000);
		return dScale;

	}
	else if(m_pOGRSpatialReference!=NULL && m_pOGRSpatialReference->IsLocal())
	{
		double dScale = m_dDcResolution;
		return dScale;
	}
	else
	{
		return 25.4/m_iDPI/1000.0/m_dDcResolution;
	}

	return 1.0;
}


// 得到影像的变换矩阵
const agg::trans_affine& RDrawParameters::get_mtx(int iimgw, int iimgh)
{
	agg::trans_affine mtx; // 构建图像的变换矩阵
	double dratio = m_dDcResolution;
	
	// 缩放必须把影像的分辨率考虑进去
	double dresoutionx = (m_dRight-m_dLeft)	/ iimgw;
	double dresoutiony = (m_dTop-m_dBottom) / iimgh;

	mtx *= agg::trans_affine_scaling(dratio*dresoutionx, dratio*dresoutiony);

	double dleft, dtop, dright, dbottom;
	GetMapBound(&dleft, &dtop, &dright, &dbottom);

	double doffsetx(0);
	double doffsety(0);
	LPtoDP(dleft, dtop, &doffsetx, &doffsety);
	mtx *= agg::trans_affine_translation(doffsetx, doffsety);
	return mtx;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


RDrawFactory::RDrawFactory():
m_iwidth(0),
m_iheight(0),
m_ibytesPerLine(0),
m_pbuf(NULL)
{

}

RDrawFactory::~RDrawFactory()
{

}

void RDrawFactory::Init(int iw, int ih, unsigned char* pbuf)
{
	m_iheight=ih; 
	m_iwidth=iw; 
	m_pbuf=pbuf;
	
	int ibytes = m_iwidth * 24 / 8;
	if(ibytes%4)
		ibytes += 4-ibytes%4;

	 m_ibytesPerLine=ibytes;
}


void RDrawFactory::DrawGeometry(RDrawParameters* pDrawParms, OGRGeometry ** pGeometry,  int iCount, RStyle* layStyle)
{
	if(pDrawParms==NULL && pGeometry==NULL && iCount>0)
		return;

	if(!layStyle->m_bVisible)
		return;

	if (wkbFlatten(pGeometry[0]->getGeometryType()) == wkbPoint){
		OGRPoint **poPoint = (OGRPoint **) pGeometry;
		DrawGeoPoint(pDrawParms, poPoint, iCount, layStyle);
	}
	else if(wkbFlatten(pGeometry[0]->getGeometryType()) == wkbLineString){
		OGRLineString **poLineString = (OGRLineString **) pGeometry;
		DrawGeoLineString(pDrawParms, poLineString, iCount, layStyle);
	}

	else if(wkbFlatten(pGeometry[0]->getGeometryType()) == wkbPolygon){
		OGRPolygon **poPolygon = (OGRPolygon **) pGeometry;
		DrawGeoPolygon(pDrawParms, poPolygon,iCount, layStyle);
	}
/*
	else if(wkbFlatten(pGeometry->getGeometryType()) == wkbLineString){

	}

	else if(wkbFlatten(pGeometry->getGeometryType()) == wkbLineString){

	}

	else if(wkbFlatten(pGeometry->getGeometryType()) == wkbLineString){

	}

	else if(wkbFlatten(pGeometry->getGeometryType()) == wkbLineString){

	}
*/

}

void RDrawFactory::DrawGeoPoint(RDrawParameters* pDrawParms, OGRPoint** pGeometry, int iCount, RStyle* layStyle)
{
	agg::rendering_buffer rbuf_win(m_pbuf, m_iwidth, m_iheight, m_ibytesPerLine);

	typedef agg::pixfmt_bgr24 pixfmt;
	pixfmt pixf(rbuf_win);
	agg::renderer_base<pixfmt> renb(pixf);


	for(int i=0; i<iCount; i++){
		OGRPoint *poPoint = (OGRPoint *) pGeometry[i];
		double dx = poPoint->getX();
		double dy = poPoint->getY();
		pDrawParms->LPtoDP(dx, dy, &dx, &dy);

		agg::ellipse e1(dx, dy, 3, 3, 100);
		m_ras.add_path(e1);
	}

	if(layStyle->m_iColor!=0){
		int r = (layStyle->m_iColor>>16)&0xff;
		int g = (layStyle->m_iColor>>8)&0xff;
		int b = layStyle->m_iColor&0xff;

		agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(255,255,255));
		agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(b, g, r));

		return;
	}
	agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(85, 170, 0));
}


void RDrawFactory::DrawGeoLineString(RDrawParameters* pDrawParms, OGRLineString** pGeometry, int iCount, RStyle* layStyle)
{
	agg::rendering_buffer rbuf_win(m_pbuf, m_iwidth, m_iheight, m_ibytesPerLine);

	typedef agg::pixfmt_bgr24 pixfmt;
	pixfmt pixf(rbuf_win);
	agg::renderer_base<pixfmt> renb(pixf);

	for(int i=0; i<iCount; i++){
		OGRLineString* pLine = (OGRLineString* )pGeometry[i];
		int ipntNums = pLine->getNumPoints();
		OGRPoint pnt;
		pLine->getPoint(0, &pnt);
		double dx=pnt.getX(), dy=pnt.getY();
		pDrawParms->LPtoDP(dx, dy, &dx, &dy);

		agg::path_storage ps_line;
		ps_line.move_to(dx, dy);

		for(int ip=1; ip<ipntNums; ++ip){
			pLine->getPoint(ip, &pnt);
			dx=pnt.getX(), dy=pnt.getY();
			pDrawParms->LPtoDP(dx, dy, &dx, &dy);
			ps_line.line_to(dx, dy);//pnt.getY());
		}

		agg::conv_stroke<agg::path_storage> cs(ps_line);
		m_ras.add_path(cs);
	}

	if(layStyle!=NULL){
		int r = (layStyle->m_iColor>>16)&0xff;
		int g = (layStyle->m_iColor>>8)&0xff;
		int b = layStyle->m_iColor&0xff;
		agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(255,255,255));
		agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(b, g, r));
		return;
	}
	agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(85, 170, 255));


}
void RDrawFactory::DrawGeoPolygon(RDrawParameters* pDrawParms, OGRPolygon** pGeometry, int iCount, RStyle* layStyle)
{
	agg::rendering_buffer rbuf_win(m_pbuf, m_iwidth, m_iheight, m_ibytesPerLine);

	typedef agg::pixfmt_bgr24 pixfmt;
	pixfmt pixf(rbuf_win);
	agg::renderer_base<pixfmt> renb(pixf);

	for(int i=0; i<iCount; i++){
		OGRPolygon* poPolygon = (OGRPolygon* )pGeometry[i];
		OGRLineString* poLine = (OGRLineString* )poPolygon->getExteriorRing();

		int ipntNums = poLine->getNumPoints();
		OGRPoint pnt;
		poLine->getPoint(0, &pnt);
		double dx=pnt.getX(), dy=pnt.getY();
		pDrawParms->LPtoDP(dx, dy, &dx, &dy);

		agg::path_storage ps_line;
		ps_line.move_to(dx, dy);

		for(int ip=1; ip<ipntNums; ++ip){
			poLine->getPoint(ip, &pnt);
			dx=pnt.getX(), dy=pnt.getY();
			pDrawParms->LPtoDP(dx, dy, &dx, &dy);
			ps_line.line_to(dx, dy);
		}

		//agg::conv_stroke<agg::path_storage> cs(ps_line);
		m_ras.add_path(ps_line);
	}

	agg::rgba8 acolor = agg::rgba8(255, 170, 255);
	
	if(layStyle!=NULL && layStyle->m_iColor != 0){
		int iColor = layStyle->m_iColor ;
		int r = (iColor>>16)&0xff;
		int g = (iColor>>8)&0xff;
		int b = iColor&0xff;
		agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(255, 255, 255));
		agg::render_scanlines_aa_solid(m_ras, m_sl, renb, agg::rgba8(b, g, r));
		return;
	}
	agg::render_scanlines_aa_solid(m_ras, m_sl, renb, acolor);
}


