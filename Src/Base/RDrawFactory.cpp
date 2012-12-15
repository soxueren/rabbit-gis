
#include "Base/RDrawFactroy.h"
#include "ogrsf_frmts.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgb.h"
#include "agg_renderer_base.h"
#include "agg_ellipse.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"


//////////////////////////////////////////////////////////////////////////


#define MMPERPIXEL 3 // 1mm���ٸ�����, �豸�޹ص���ʾ�������Ժ�Ҫ���ŵ������ļ�

RDrawParameters::RDrawParameters() : m_bcancel(false),
m_ileft(0), m_itop(0), m_iright(0), m_ibottom(0),
m_dleft(0), m_dtop(0), m_dright(0), m_dbottom(0),
m_dviewleft(0), m_dviewtop(0), m_dviewright(0), m_dviewbottom(0),
m_dRatio(1.0),
m_dRatioCustom(0)
{

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
	double dx=(double)offsetx / m_dRatio; // 
	double dy = (double)offsety / m_dRatio;

#ifdef use_affine_trans
	//m_mtx *= agg::trans_affine_translation(dx, -dy);
	m_mtx *= agg::trans_affine_translation(offsetx, -offsety); // �ӿ���y�������ƶ����պõ��ڴ�����y�Ḻ���ƶ�
#else
	m_dviewleft -= dx; // ��Ļ�����ƶ����൱�ڴ��ڣ���ͼ���ӷ�Χ������
	m_dviewright -= dx;
	m_dviewtop += dy; // ��Ļ�����ƶ����൱�ڴ��ڣ���ͼ���ӷ�Χ������
	m_dviewbottom += dy;
#endif
	}

void RDrawParameters::Zoom(int ix, int iy, double dscale)
{

#ifdef use_affine_trans

	agg::trans_affine mtx=m_mtx;

	double dx=ix, dy=iy;
	dptomp(ix, iy, &dx, &dy); // ��Ļ����㣬ת��Ϊ�߼�(��ͼ)�����

	double dmapw = fabs(m_dright-m_dleft); //  ��ͼ��
	double dmaph = fabs(m_dbottom-m_dtop); //  ��ͼ��
	double dmapcenterx = m_dleft+dmapw*0.5;
	double dmapcentery = m_dbottom+dmaph*0.5;

	dx=dmapcenterx; dy = dmapcentery;

	// ���յ�ͼ����㣬��������
	m_mtx *= agg::trans_affine_translation(-dx, -dy);
	m_mtx *= agg::trans_affine_scaling(dscale);
	m_mtx *= agg::trans_affine_translation(dx, dy);
#else

	// �豸�㣬ת��Ϊ�߼������(���������)
	double dx=ix, dy=iy;
	DPtoLP(ix, iy, &dx, &dy);

	double dwinw = abs(m_iright-m_ileft); // ��Ļ��
	double dwinh = abs(m_ibottom-m_itop); // ��Ļ�� 
	
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

	double dRatioBack = m_dRatio;// ��������ת������
	m_dRatio *= dscale; // �����Ǹ��±�����

	if(m_dRatioCustom!=0 && m_dRatio<m_dRatioCustom)
	{
		m_dRatio = m_dRatioCustom;
		dscale = m_dRatio/dRatioBack; // ��С���˼��𣬲�������
		//return;
	}

	double dviewwidth = dwinw/m_dRatio; // �߼����귶Χ��ȣ���ͼ����ʾ��Χ��ȣ�
	double dviewheight = dwinh/m_dRatio;
	double dnew_view_w = (m_dviewright-m_dviewleft) / (dscale); // ���ź��߼����
	double dnew_view_h = (m_dviewtop-m_dviewbottom) / (dscale);//

	// ���յ�ǰ��λ��Ϊ�ο�������һ�¿��ӷ�Χ
	m_dviewleft = dx - (dx-m_dviewleft)/dscale;
	m_dviewright =m_dviewleft+dnew_view_w;
	m_dviewbottom = dy-(dy - m_dviewbottom)/dscale;// dnew_view_h*0.5;
	m_dviewtop = m_dviewbottom+dnew_view_h;

#endif
}


/************************************************************************/
/* 1. ����ͼ���ĵ��ƶ�����Ļ���ĵ㣬
	2. �ٰ��յ�ͼ���ĵ���������Ļ��С
*/
/************************************************************************/
void RDrawParameters::ZoomEntire()
{
	double dmapw = fabs(m_dright-m_dleft); //  ��ͼ��
	double dmaph = fabs(m_dbottom-m_dtop); //  ��ͼ��
	double dwinw = abs(m_iright-m_ileft); // ��Ļ��
	double dwinh = abs(m_ibottom-m_itop); // ��Ļ�� 

	double dmapcenterx = m_dleft+dmapw*0.5;
	double dmapcentery = m_dbottom+dmaph*0.5;

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

	double dx = fabs(double(m_iright-m_ileft) / (m_dright-m_dleft));
	double dy = fabs(double(m_ibottom-m_itop) / (m_dbottom-m_dtop));

	if(m_dRatioCustom!=0)
	{
		m_dRatio = m_dRatioCustom;
	}
	else{

		if(dmaph*dx > dwinh)
			m_dRatio=dy;// Ӧ����������Ļ�߶�
		else if(dmapw*dy > dwinw)
			m_dRatio=dx;// Ӧ����������Ļ���
	}

	double dviewwidth = dwinw/m_dRatio; // �߼����귶Χ��ȣ���ͼ����ʾ��Χ��ȣ�
	double dviewheight = dwinh/m_dRatio;

	// �����߼����귶Χ����ͼ����ʾ��Χ��
	m_dviewleft = dmapcenterx - dviewwidth*0.5;
	m_dviewright = dmapcenterx + dviewwidth*0.5;
	m_dviewbottom = dmapcentery - dviewheight*0.5;
	m_dviewtop = dmapcentery + dviewheight*0.5;
#endif

}

//; // ���ŵ�ԭʼ�ֱ���, �Ǿ���˵����Ļһ�����ض�Ӧͼ��һ������
void RDrawParameters::ZoomResolution(int imgw, int imgh)
{
	double dmapw = fabs(m_dright-m_dleft); //  ��ͼ��
	double dmaph = fabs(m_dbottom-m_dtop); //  ��ͼ��

	double dwinw = abs(m_iright-m_ileft); // ��Ļ��
	double dwinh = abs(m_ibottom-m_itop); // ��Ļ�� 

	double dmapcenterx = m_dleft+dmapw*0.5;
	double dmapcentery = m_dbottom+dmaph*0.5;
	
	double dx = dmapw / imgw; // Ӱ��x�ֱ���
	double dy = dmaph / imgh;

	double dresolution = dx;
	if(dx>dy)
		dresolution=dy;
	
	double dviewwidth = dwinw * dresolution; // ��Ļ��ȶ�Ӧ���߼�������
	double dviewheight = dwinh * dresolution; // 

	m_dRatio = dwinh / dviewheight; // ��Ļ����/�߼�����

	// �����߼����귶Χ����ͼ����ʾ��Χ��
	m_dviewleft = dmapcenterx - dviewwidth*0.5;
	m_dviewright = dmapcenterx + dviewwidth*0.5;
	m_dviewbottom = dmapcentery - dviewheight*0.5;
	m_dviewtop = dmapcentery + dviewheight*0.5;
}


void RDrawParameters::ZoomRect(int ileft, int itop, int iright, int ibottom, bool bZoomIn/*=true*/)//; // ��ѡ�Ŵ�
{
	// 1. �����ѡ��Χ���ӿڣ��豸�����ĵ�
	int icentx=ileft + (iright-ileft)*0.5;
	int icenty=itop+(ibottom-itop)*0.5;

	//��Ļ����
	int iWinCentX = (m_iright-m_ileft)*0.5;
	int iWinCentY = (m_ibottom-m_itop)*0.5;

	// ��Ļ���
	int iWinWidth = m_iright-m_ileft;
	int iWinHeight = m_ibottom-m_itop;

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

	// 1. �Ȱѿ�ѡ�ĵ���Χ��һ��
	double dleft, dtop, dright, dbottom;
	DPtoLP(ileft, ibottom, &dleft, &dbottom);
	DPtoLP(iright, itop, &dright, &dtop);


	// 2. �������ű���
	double dmapw = fabs(dright-dleft); //  ��ͼ��
	double dmaph = fabs(dtop-dbottom); //  ��ͼ��

	double dwinw = abs(m_iright-m_ileft); // ��Ļ��
	double dwinh = abs(m_ibottom-m_itop); // ��Ļ�� 

	double dmapcenterx = dleft+dmapw*0.5;
	double dmapcentery = dbottom+dmaph*0.5;

	double dx = fabs(dmapw/dwinw);
	double dy = fabs(dmaph/dwinh);

	if(dx > dy)
		m_dRatio=dy;// Ӧ����������Ļ�߶�
	//else if(dmapw*dy > dwinw)
	//	m_dRatio=dx;// Ӧ����������Ļ���

	// 3. �����߼����귶Χ����ͼ����ʾ��Χ��
	double dviewwidth = dwinw/m_dRatio; // �߼����귶Χ��ȣ���ͼ����ʾ��Χ��ȣ�
	double dviewheight = dwinh/m_dRatio;

	m_dviewleft = dmapcenterx - dviewwidth*0.5;
	m_dviewright = dmapcenterx + dviewwidth*0.5;
	m_dviewbottom = dmapcentery - dviewheight*0.5;
	m_dviewtop = dmapcentery + dviewheight*0.5;

}


void RDrawParameters::Init(int il, int it, int ir, int ib, double dl, double dt, double dr, double db)
{
	if(!is_changed(il, it, ir, ib, dl, dt, dr, db))
		return;

	m_ileft=il; m_itop=it; m_iright=ir; m_ibottom=ib;
	m_dleft=dl; m_dtop=dt; m_dright=dr; m_dbottom=db;
	m_dviewleft=dl; m_dviewtop=dt; m_dviewright=dr; m_dviewbottom=db;	

	ZoomEntire(); // ���δ򿪣�Ӧ��ȫ����ʾ
	return;
}

bool RDrawParameters::is_changed(int il, int it, int ir, int ib, double dl, double dt, double dr, double db)
{
	if(m_ileft!=il || m_itop!=it || m_iright!=ir || m_ibottom!=ib)
		return true;
	else if(m_dleft!=dl || m_dtop!=dt || m_dright!=dr || m_dbottom!=db)
		return true;

	return false;

}


void RDrawParameters::DPtoLP(int ix, int iy, double* dx, double* dy)
{
	double dsrcx=ix, dsrcy=iy;

#ifdef use_affine_trans
	agg::trans_affine mtx=m_mtx;
	mtx.flip_y(); // ������x�᾵��
	mtx *= agg::trans_affine_translation(0, m_ibottom); // �������ƾ��룺 �߼�����߶�
	mtx.invert();

	mtx.transform(&dsrcx, &dsrcy);
	*dx=dsrcx; 
	*dy=dsrcy;	
	return;

	*dx = (ix-m_ileft) / m_dRatio + m_dleft;
	*dy = (iy-m_ibottom) / m_dRatio + m_dbottom;

	m_mtx.transform(dx, dy);
#else

	// �豸����㣬ת��Ϊ�߼������
	// x = (x' - VOrgx) * WExtx / VExtx + WOrgx;
	// y = (y' - Vorgy) * WExty / VExty + WOrgy;

	*dx = (ix-m_ileft) / m_dRatio + m_dviewleft;
	*dy = (m_ibottom-iy) / m_dRatio + m_dviewbottom;
	//*dy = (iy-m_itop) / m_dRatio + m_dviewbottom;

#endif
}

// ���ڵ�(x, y)�任Ϊ�ӿڵ�(x', y')������
//	x' = (x - WOrgx) * VExtx / WExtx + VOrgx;
// y' = (y - WOrgy) * VExty / WExty + VOrgy;

void RDrawParameters::MPtoDP(double dx, double dy, double* ix, double* iy) // ��ͼ���ꡪ�����豸����
{
#ifdef use_affine_trans
	agg::trans_affine mtx=m_mtx;
// 	double dleft=m_dleft;
// 	double dbottom = m_dbottom;
// 	m_mtx.transform(&dleft, &dbottom); // ��ͼ���½��߼�����
// 	
// 	mtx *= agg::trans_affine_translation(-dleft, -dbottom); // ��ͼ���½��ƶ����߼�����ԭ�� 
	mtx.flip_y(); // ������x�᾵��
	mtx *= agg::trans_affine_translation(0, m_ibottom); // �������ƾ��룺 �߼�����߶�

	double dsrcx=dx, dsrcy=dy;
	mtx.transform(&dsrcx, &dsrcy);
	*ix=dsrcx; 
	*iy=dsrcy;	
#else

	*ix = (dx-m_dviewleft) * m_dRatio + m_ileft;
	*iy = m_ibottom - (dy-m_dviewbottom) * m_dRatio; //+ m_itop; ע�⣡��������Ļ����ԭ������ϽǱ任�����½�
#endif
}


void RDrawParameters::ChangeDP(int iWidth, int iHeight)
{
	m_iright = m_ileft+iWidth;
	m_ibottom = m_itop+iHeight;

	double dviewwidth = iWidth/m_dRatio; // �߼����귶Χ��ȣ���ͼ����ʾ��Χ��ȣ�
	double dviewheight = iHeight/m_dRatio;

	// �����߼����귶Χ����ͼ����ʾ��Χ��
	m_dviewright = m_dviewleft + dviewwidth;
	m_dviewbottom =  m_dviewtop - dviewheight;
}

void RDrawParameters::GetViewBound(double* dviewleft, double* dviewtop, double* dviewright, double* dviewbottom)
{
	*dviewleft=m_dviewleft;
	*dviewtop=m_dviewtop;
	*dviewright=m_dviewright;
	*dviewbottom=m_dviewbottom;
}

void RDrawParameters::GetMapBound(double* dleft, double* dtop, double* dright, double* dbottom)
{
	*dleft=m_dleft;
	*dtop=m_dtop;
	*dright=m_dright;
	*dbottom=m_dbottom;
}


// �õ�Ӱ��ı任����
const agg::trans_affine& RDrawParameters::get_mtx(int iimgw, int iimgh)
{
	agg::trans_affine mtx; // ����ͼ��ı任����
	double dratio = GetRatio();
	
	// ���ű����Ӱ��ķֱ��ʿ��ǽ�ȥ
	double dresoutionx = (m_dright-m_dleft)	/ iimgw;
	double dresoutiony = (m_dtop-m_dbottom) / iimgh;

	mtx *= agg::trans_affine_scaling(dratio*dresoutionx, dratio*dresoutiony);

	double dleft, dtop, dright, dbottom;
	GetMapBound(&dleft, &dtop, &dright, &dbottom);

	double doffsetx(0);
	double doffsety(0);
	MPtoDP(dleft, dtop, &doffsetx, &doffsety);
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
		pDrawParms->MPtoDP(dx, dy, &dx, &dy);

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
		pDrawParms->MPtoDP(dx, dy, &dx, &dy);

		agg::path_storage ps_line;
		ps_line.move_to(dx, dy);

		for(int ip=1; ip<ipntNums; ++ip){
			pLine->getPoint(ip, &pnt);
			dx=pnt.getX(), dy=pnt.getY();
			pDrawParms->MPtoDP(dx, dy, &dx, &dy);
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
		pDrawParms->MPtoDP(dx, dy, &dx, &dy);

		agg::path_storage ps_line;
		ps_line.move_to(dx, dy);

		for(int ip=1; ip<ipntNums; ++ip){
			poLine->getPoint(ip, &pnt);
			dx=pnt.getX(), dy=pnt.getY();
			pDrawParms->MPtoDP(dx, dy, &dx, &dy);
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


