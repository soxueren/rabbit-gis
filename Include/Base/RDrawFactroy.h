
#ifndef RDRAW_FACTORY_INCLUDE_H
#define RDRAW_FACTORY_INCLUDE_H


//#include "geos.h"

/************************************************************************/
/* ���𴰿ڣ��߼����꣩<=>�ӿڣ��豸���֮꣩���ת��
�߼�������ͨ�� ����ͼ �ĵ���Χ ƫ�Ƶ�����ԭ�㣨0,0���õ������ң�����Ϊ��

���ڣ� ����Ҫ��ʾ��ЩҪ��
�ӿڣ� ����Ҫ����ʾ���豸��ʲôλ��

�����������任���۹�ʽ���£�
1. �豸����㣬ת��Ϊ�߼������
	x = (x' - VOrgx) * WExtx / VExtx + WOrgx;
	y = (y' - VOrgy) * WExty / VExty + WOrgy;

2. ���ڵ�(x, y)�任Ϊ�ӿڵ�(x', y')������
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

class  BASE_API RDrawParameters //RDrawParameters
{
public:
	RDrawParameters();
	~RDrawParameters();

	void Init(int il, int it, int ir, int ib, double dl, double dt, double dr, double db);
	void Pan(int offsetx, int offsety);
	void Zoom(int ix, int iy, double dscale);
	void ZoomEntire();
	void ZoomResolution(int imgw, int imgh); // ���ŵ�ԭʼ�ֱ���, ���Ӱ��������Ч
	void ZoomRect(int ileft, int itop, int iright, int ibottom, bool bZoomIn=true); // ��ѡ�Ŵ�

	void DPtoLP(int ix, int iy, double* dx, double* dy); // �豸���ꡪ�����߼����� (��ͼ����)
	void MPtoDP(double dx, double dy, double* ix, double* iy); // ��ͼ���ꡪ�����豸����
	
	/*! \brief �����豸���,�����豸��������
	*  \param [in] iWidth ���ڿ��(�豸����).
	*  \param [in] iHeight ���ڸ߶ȣ��豸���꣩.
	*  \return void.
	*/
	void ChangeDP(int iWidth, int iHeight);

	//! \brief ���ӷ�Χ
	void GetViewBound(double* dviewleft, double* dviewtop, double* dviewright, double* dviewbottom);

	//!\brief ��ͼ����Χ
	void GetMapBound(double* dleft, double* dtop, double* dright, double* dbottom);
	double GetRatio() {return m_dRatio;}
	void SetRatio(double dRatio){m_dRatio=dRatio;}

	//bool GetCustomRatio(){}
	//void SetCustomRatio(bool bUse){}

	double GetCustomRatio(){return m_dRatioCustom;}
	void SetCustomRatio(double dRatio){m_dRatioCustom=dRatio;}

	const agg::trans_affine& get_mtx(int iimgw, int iimgh);

	bool GetCancel() {return m_bcancel;}
	void SetCancel(bool bcancel=true){ m_bcancel=bcancel; }

protected:
	bool is_changed(int il, int it, int ir, int ib, double dl, double dt, double dr, double db);
	bool m_bcancel; // ȡ����Ⱦ��־

private:

	int m_ileft, m_itop, m_iright, m_ibottom; // �豸���귶Χ m_itop<m_ibottom
	double m_dleft, m_dtop, m_dright, m_dbottom; // ��ͼ�������귶Χ
	double m_dviewleft, m_dviewtop, m_dviewright, m_dviewbottom; // ��ͼ����ʾ��Χ����Ⱦ��Χ��

	double m_dRatio; // ����ת������ ���豸����/�߼����꣩
	double m_dRatioCustom; // ʹ���Զ����ת������
	agg::trans_affine m_mtx; // ��������任����
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

/*! \brief ��ʾ������
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
