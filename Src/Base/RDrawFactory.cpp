
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

// �������Ϊ�豸����
/************************************************************************/
/*  ���ڷ����ƶ�                                                                     */
/************************************************************************/
void RDrawParameters::Pan(int offsetx, int offsety)
{
	double dx=(double)offsetx *m_dDcResolution; // ; // �߼����� 
	double dy = (double)offsety *m_dDcResolution; /// ;

#ifdef use_affine_trans
	//m_mtx *= agg::trans_affine_translation(dx, -dy);
	m_mtx *= agg::trans_affine_translation(offsetx, -offsety); // �ӿ���y�������ƶ����պõ��ڴ�����y�Ḻ���ƶ�
#else
	m_dViewLeft -= dx; // ��Ļ�����ƶ����൱�ڴ��ڣ���ͼ���ӷ�Χ������
	m_dViewRight -= dx;
	m_dViewTop += dy; // ��Ļ�����ƶ����൱�ڴ��ڣ���ͼ���ӷ�Χ������
	m_dViewBottom += dy;
#endif
	}

void RDrawParameters::Zoom(int ix, int iy, double dscale)
{

#ifdef use_affine_trans

	agg::trans_affine mtx=m_mtx;

	double dx=ix, dy=iy;
	dptomp(ix, iy, &dx, &dy); // ��Ļ����㣬ת��Ϊ�߼�(��ͼ)�����

	double dmapw = fabs(m_dRight-m_dLeft); //  ��ͼ��
	double dmaph = fabs(m_dBottom-m_dTop); //  ��ͼ��
	double dmapcenterx = m_dLeft+dmapw*0.5;
	double dmapcentery = m_dBottom+dmaph*0.5;

	dx=dmapcenterx; dy = dmapcentery;

	// ���յ�ͼ����㣬��������
	m_mtx *= agg::trans_affine_translation(-dx, -dy);
	m_mtx *= agg::trans_affine_scaling(dscale);
	m_mtx *= agg::trans_affine_translation(dx, dy);
#else

	// �豸�㣬ת��Ϊ�߼������(���������)
	double dx=ix, dy=iy;
	DPtoLP(ix, iy, &dx, &dy);

	double dwinw = abs(m_iRight-m_iLeft); // ��Ļ��
	double dwinh = abs(m_iBottom-m_iTop); // ��Ļ�� 
	
	if(m_dRatioCustom!=0) // �̶������ߣ�Ҫ�ϸ��ռ�������
	{
		if(dscale>1.0) // ����Χ����
		{
			// ���2��N�η��е�Nֵ
			double d2 = log(double(2));
			double dValue = log(dscale);
			int n=int(dValue/d2);

			dscale = 2<<n;;// ȷ���Ŵ����2��N�η�
		}
		else  // ����Χ��С
		{
			// ���2��N�η��е�Nֵ
			double d2 = log(double(2));
			double dValue = log(1.0/dscale);
			int n=int(dValue/d2);

			dscale = 1.0/(2<<n);;// ȷ����С���� 1/(2��N�η�)
		}
	}

	double dRatioBack = m_dDcResolution;// ��������ת������
	m_dDcResolution /= dscale; // �����Ǹ��±�����

	//if(m_dRatioCustom!=0 && m_dRatio<m_dRatioCustom)
	{
		//m_dRatio = m_dRatioCustom;
		//dscale = m_dRatio/dRatioBack; // ��С���˼��𣬲�������
		//return;
	}

	double dviewwidth = dwinw*m_dDcResolution; // �߼����귶Χ��ȣ���ͼ����ʾ��Χ��ȣ�
	double dviewheight = dwinh*m_dDcResolution;
	double dnew_view_w = (m_dViewRight-m_dViewLeft) / (dscale); // ���ź��߼����
	double dnew_view_h = (m_dViewTop-m_dViewBottom) / (dscale);//

	// ���յ�ǰ��λ��Ϊ�ο�������һ�¿��ӷ�Χ
	m_dViewLeft = dx - (dx-m_dViewLeft)/dscale;
	m_dViewRight =m_dViewLeft+dnew_view_w;
	m_dViewBottom = dy-(dy - m_dViewBottom)/dscale;// dnew_view_h*0.5;
	m_dViewTop = m_dViewBottom+dnew_view_h;

#endif
}


/************************************************************************/
/* 1. ����ͼ���ĵ��ƶ�����Ļ���ĵ㣬
	2. �ٰ��յ�ͼ���ĵ���������Ļ��С
*/
/************************************************************************/
void RDrawParameters::ZoomEntire()
{
	double dmapw = fabs(m_dRight-m_dLeft); //  ��ͼ��
	double dmaph = fabs(m_dBottom-m_dTop); //  ��ͼ��
	double dwinw = abs(m_iRight-m_iLeft); // ��Ļ��
	double dwinh = abs(m_iBottom-m_iTop); // ��Ļ�� 

	double dmapcenterx = m_dLeft+dmapw*0.5;
	double dmapcentery = m_dBottom+dmaph*0.5;

#ifdef use_affine_trans
	
	double offsetx = dwinw*0.5 - dmapcenterx; // ��ͼ���ĵ���Ļ���ĵ�ƫ����x
	double offsety = dwinh*0.5 - dmapcentery; // ��ͼ���ĵ���Ļ���ĵ�ƫ����y

	// �ƶ�����Ļ���ĵ�	
	agg::trans_affine mtx;
	mtx *= agg::trans_affine_translation(offsetx, offsety); // ��ͼ���ĵ㣬�ƶ�����Ļ����
	
	// ���յ�ͼ���ĵ����꣬��������Ļ��С
	mtx *= agg::trans_affine_translation(-dwinw*0.5, -dwinh*0.5);
	m_dRatio*=0.5;
	mtx *= agg::trans_affine_scaling(m_dRatio); // ��������Ļ��С��Χ
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
			m_dDcResolution=dResolutionY;// Ӧ����������Ļ�߶�
		else if(dmapw/dResolutionY > dwinw)
			m_dDcResolution=dResolutionX;// Ӧ����������Ļ���
	}

	double dviewwidth = dwinw*m_dDcResolution;//; // �߼����귶Χ��ȣ���ͼ����ʾ��Χ��ȣ�
	double dviewheight = dwinh*m_dDcResolution;///;

	// �����߼����귶Χ����ͼ����ʾ��Χ��
	m_dViewLeft = dmapcenterx - dviewwidth*0.5;
	m_dViewRight = dmapcenterx + dviewwidth*0.5;
	m_dViewBottom = dmapcentery - dviewheight*0.5;
	m_dViewTop = dmapcentery + dviewheight*0.5;
#endif

}

//; // ���ŵ�ԭʼ�ֱ���, �Ǿ���˵����Ļһ�����ض�Ӧͼ��һ������
void RDrawParameters::ZoomResolution(int imgw, int imgh)
{
	double dmapw = fabs(m_dRight-m_dLeft); //  ��ͼ��
	double dmaph = fabs(m_dBottom-m_dTop); //  ��ͼ��

	double dwinw = abs(m_iRight-m_iLeft); // ��Ļ��
	double dwinh = abs(m_iBottom-m_iTop); // ��Ļ�� 

	double dmapcenterx = m_dLeft+dmapw*0.5;
	double dmapcentery = m_dBottom+dmaph*0.5;
	
	double dx = dmapw / imgw; // Ӱ��x�ֱ���
	double dy = dmaph / imgh;

	double dresolution = dx;
	if(dx>dy)
		dresolution=dy;
	
	double dviewwidth = dwinw * dresolution; // ��Ļ��ȶ�Ӧ���߼�������
	double dviewheight = dwinh * dresolution; // 

	m_dDcResolution = dviewheight / dwinh; // ��Ļ����/�߼�����

	// �����߼����귶Χ����ͼ����ʾ��Χ��
	m_dViewLeft = dmapcenterx - dviewwidth*0.5;
	m_dViewRight = dmapcenterx + dviewwidth*0.5;
	m_dViewBottom = dmapcentery - dviewheight*0.5;
	m_dViewTop = dmapcentery + dviewheight*0.5;
}


void RDrawParameters::ZoomRect(int ileft, int itop, int iright, int ibottom, bool bZoomIn/*=true*/)//; // ��ѡ�Ŵ�
{
	// 1. �����ѡ��Χ���ӿڣ��豸�����ĵ�
	int icentx=ileft + (iright-ileft)*0.5;
	int icenty=itop+(ibottom-itop)*0.5;

	//��Ļ����
	int iWinCentX = (m_iRight-m_iLeft)*0.5;
	int iWinCentY = (m_iBottom-m_iTop)*0.5;

	// ��Ļ���
	int iWinWidth = m_iRight-m_iLeft;
	int iWinHeight = m_iBottom-m_iTop;

	// ���ŷ�Χ���(�豸����)
	int iRectWidth = iright-ileft;
	int iRectHeight = ibottom-itop;

	// 2. ��һ�����ű���
	double dratiox = fabs( iRectWidth*1.0 / iWinWidth );
	double dratioy = fabs( iRectHeight*1.0 / iWinHeight );

	// ���ŷ�Χ���ĵ��Ƶ���Ļ����
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

	ZoomEntire(); // ���δ򿪣�Ӧ��ȫ����ʾ
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
	mtx.flip_y(); // ������x�᾵��
	mtx *= agg::trans_affine_translation(0, m_iBottom); // �������ƾ��룺 �߼�����߶�
	mtx.invert();

	mtx.transform(&dsrcx, &dsrcy);
	*dx=dsrcx; 
	*dy=dsrcy;	
	return;

	*dx = (ix-m_iLeft) / m_dRatio + m_dLeft;
	*dy = (iy-m_iBottom) / m_dRatio + m_dBottom;

	m_mtx.transform(dx, dy);
#else

	// �豸����㣬ת��Ϊ�߼������
	// x = (x' - VOrgx) * WExtx / VExtx + WOrgx;
	// y = (y' - Vorgy) * WExty / VExty + WOrgy;

	*dx = (ix-m_iLeft) * m_dDcResolution + m_dViewLeft;
	*dy = (m_iBottom-iy) * m_dDcResolution + m_dViewBottom;
	//*dy = (iy-m_iTop) / m_dRatio + m_dViewBottom;

#endif
}

// ���ڵ�(x, y)�任Ϊ�ӿڵ�(x', y')������
//	x' = (x - WOrgx) * VExtx / WExtx + VOrgx;
// y' = (y - WOrgy) * VExty / WExty + VOrgy;

void RDrawParameters::LPtoDP(double dx, double dy, double* ix, double* iy) // ��ͼ���ꡪ�����豸����
{
#ifdef use_affine_trans
	agg::trans_affine mtx=m_mtx;
// 	double dleft=m_dLeft;
// 	double dbottom = m_dBottom;
// 	m_mtx.transform(&dleft, &dbottom); // ��ͼ���½��߼�����
// 	
// 	mtx *= agg::trans_affine_translation(-dleft, -dbottom); // ��ͼ���½��ƶ����߼�����ԭ�� 
	mtx.flip_y(); // ������x�᾵��
	mtx *= agg::trans_affine_translation(0, m_iBottom); // �������ƾ��룺 �߼�����߶�

	double dsrcx=dx, dsrcy=dy;
	mtx.transform(&dsrcx, &dsrcy);
	*ix=dsrcx; 
	*iy=dsrcy;	
#else

	*ix = (dx-m_dViewLeft) / m_dDcResolution + m_iLeft;
	*iy = m_iBottom - (dy-m_dViewBottom) / m_dDcResolution; //+ m_iTop; ע�⣡��������Ļ����ԭ������ϽǱ任�����½�
#endif
}


void RDrawParameters::ChangeDP(int iWidth, int iHeight)
{
	m_iRight = m_iLeft+iWidth;
	m_iBottom = m_iTop+iHeight;

	double dviewwidth = iWidth*m_dDcResolution;//m_dRatio; // �߼����귶Χ��ȣ���ͼ����ʾ��Χ��ȣ�
	double dviewheight = iHeight*m_dDcResolution;///m_dRatio;

	// �����߼����귶Χ����ͼ����ʾ��Χ��
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
	if(m_pOGRSpatialReference!=NULL && m_pOGRSpatialReference->IsGeographic()) // ��������
	{			
		double dSemiMajor = m_pOGRSpatialReference->GetSemiMajor(); // ���򳤰��� ��λǧ��
		char* pchUnits=NULL;
		m_pOGRSpatialReference->GetLinearUnits(&pchUnits);

		double dLength = 2*PI*dSemiMajor; // �����س�������ܳ� ��λǧ��
		double dDcResolution = 25.4/m_iDPI; // �豸�ֱ���: һ�����ص���߼����� ��λ mm
		dDcResolution = dDcResolution/dScale; // һ�����ص�ĵ�ͼ���� ��λmm
		m_dDcResolution = dDcResolution/1000000.0/dLength*360.0; // �豸�ֱ���: һ�����ص���߼����� ��λ ����

		// ���տ��ӷ�Χ���ĵ�, ��������ʾ��Χ , ��������굥λ���Ƕ�
		double dViewCenterX = m_dViewLeft+(m_dViewRight-m_dViewLeft)*0.5; 
		double dViewCenterY = m_dViewBottom+(m_dViewTop-m_dViewBottom)*0.5;

		double dViewWidth = (m_iRight-m_iLeft) * m_dDcResolution; // ��ʾ�豸���߼����// ��λ����
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
	������=(ͼ�Ͼ���/ʵ�ʾ���)=(�豸ƽ�泤��/��ͼƽ�泤��)

	�豸����=(�豸���ص���/DPI)��25.4 (����)
	DPI = ÿӢ�����ص���(DotPerInch)
	1 inch = 25.4 mm
	*/
	
	if(m_pOGRSpatialReference!=NULL && m_pOGRSpatialReference->IsGeographic()) // ��������
	{			
		double dSemiMajor = m_pOGRSpatialReference->GetSemiMajor(); // ���򳤰��� ��λ:��
		char* pchUnits=NULL;
		m_pOGRSpatialReference->GetAngularUnits(&pchUnits);

		double dLength = 2*PI*dSemiMajor; // �����س�������ܳ� ��λ:��
		double dDcResolution = 25.4/m_iDPI; // �豸�ֱ���: һ�����ص���߼����� ��λ mm
		double dScale = m_dDcResolution*(dLength*1000.0/360.0); // һ�����ص��Ӧ�߼��ĳ��� ��λmm
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

		double dSemiMajor = pSpatialReference->GetSemiMajor(); // ���򳤰��� ��λǧ��
		double dLength = 2*PI*dSemiMajor; // �����س�������ܳ� ��λǧ��
		double dDcResolution = 25.4/m_iDPI; // �豸�ֱ���: һ�����ص���߼����� ��λ mm
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


// �õ�Ӱ��ı任����
const agg::trans_affine& RDrawParameters::get_mtx(int iimgw, int iimgh)
{
	agg::trans_affine mtx; // ����ͼ��ı任����
	double dratio = m_dDcResolution;
	
	// ���ű����Ӱ��ķֱ��ʿ��ǽ�ȥ
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


