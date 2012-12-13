
#include "QTCore/RMapWidget.h"
#include "Desktop/Rabbit.h"
#include "gdal.h"
#include <gdal_priv.h>
#include <QLabel>

#include "ogrsf_frmts.h"
#include <QPainter>
#include <QMenuBar>
#include <QContextMenuEvent>

#include "Base/FileType.h"
#include "Base/RDataSource.h"

//#include <QStatusBar>


//typedef void (__stdcall * TileDownloadFinishedFun) (unsigned int pWnd);
void __stdcall TileDownloadFinished(unsigned int pWnd)
{
	RMapWidget* pWidget = (RMapWidget* )pWnd;
	if(pWidget != NULL)
	{
		pWidget->GetMap()->SetRedrawFlag(true);
		pWidget->update();
	}
}

RMapWidget::RMapWidget(QWidget *parent /*= 0*/, Qt::WFlags flags/* = 0*/)
{
	m_actType = Select;
	m_actTypeLast  = Select;

	m_iLastMousePosX=0;
	m_iLastMousePoxY = 0;

	m_bDrawCacheL2 = false;
	setMouseTracking(true); // 开启鼠标移动跟踪事件


}

RMapWidget::~RMapWidget()
{

}

void RMapWidget::AddLayer(const QString& strDsName, const QString& strDtName)
{
	if(strDsName.isEmpty() || strDtName.isEmpty())
		return ;

	string strDataSource = strDsName.toLocal8Bit().data();
	string strDataset = strDtName.toLocal8Bit().data();

	RDataSource* pDs=GetMap()->GetDataSource();

	RLayer * pLayer = new RLayer(pDs);
	pLayer->SetName(strDataset);
	

	m_Map.AddLayer(pLayer);
}

void RMapWidget::ActionSelect()
{
	m_actTypeLast = m_actType;
	m_actType = Select;
	ChangeCursor(m_actType);
}

void RMapWidget::ActionPan()
{
	m_actTypeLast = m_actType;
	m_actType = Pan;
	ChangeCursor(m_actType);
}

void RMapWidget::ActionZoomIn()
{
	m_actTypeLast = m_actType;
	m_actType = ZoomIn;
	ChangeCursor(m_actType);
}

void RMapWidget::ActionZoomOut()
{
	m_actTypeLast = m_actType;
	m_actType = ZoomOut;
	ChangeCursor(m_actType);
}
void RMapWidget::ActionEditRect()
{
	m_actTypeLast = m_actType;
	m_actType = EditRect;
	ChangeCursor(m_actType);
}

void RMapWidget::ChangeCursor(actionType actType)
{
	if(actType == Select)
	{
		QCursor cur(Qt::ArrowCursor);
		setCursor(cur);
	}
	else if (actType == Pan)
	{
		QCursor cur;// =  cursor();
		cur.setShape(Qt::OpenHandCursor);
		setCursor(cur);
	}
	else if (actType == ZoomIn)
	{
		QString strZoomIn = "images/zoomin.png";
		QPixmap pxm(strZoomIn);
		if(!pxm.isNull()){
			QCursor cur(pxm);
			setCursor(cur);	
		}
	}
	else if (actType == ZoomOut)
	{
		QString strZoom = "images/zoomout.png";
		QPixmap pxm(strZoom);
		if(!pxm.isNull()){
			QCursor cur(pxm);
			setCursor(cur);	
		}
	}
	else if (actType == EditRect)
	{
		QCursor cur(Qt::CrossCursor);
		setCursor(cur);
	}
	else if (actType == EditMove)
	{
		QCursor cur(Qt::SizeAllCursor);
		setCursor(cur);
	}
}


void RMapWidget::paintEvent(QPaintEvent * event)
{	
	int iWndWidth = geometry().width();
	int iWndHeight = geometry().height();

	QSize winSize(iWndWidth, iWndHeight); 
	// 先在内存位图中将要素渲染一遍
	QImage image(winSize, QImage::Format_RGB888);	

	int iBytesPerLine = iWndWidth*3; // RGB一行字节数
	if (iBytesPerLine%4)
		iBytesPerLine += 4-iBytesPerLine%4;

	if(m_bDrawCacheL2)
	{
		unsigned char* pwinbuf = (unsigned char*)image.bits();
		unsigned char* pbuf = m_Map.GetDrawBuf();
		memcpy(pwinbuf, pbuf, iBytesPerLine*iWndHeight); 

		// Qt中贴出结果位图
		QPainter winPainter(this);
		winPainter.drawImage(0, 0, image);
		m_bDrawCacheL2=false;

		update();
		return;
	}

	unsigned char* pwinbuf = (unsigned char*)image.bits();
	memset(pwinbuf, 205, iBytesPerLine*iWndHeight); // 这个颜色决定了屏幕可视范围默认背景色
	GetMap()->SetRRenderDataPreparedFun(&TileDownloadFinished, (unsigned int)this);
	GetMap()->Draw(iWndWidth, iWndHeight, pwinbuf);
	
	// Qt中贴出结果位图
	QPainter winPainter(this);

	int x=geometry().x();
	int y=geometry().y();
	winPainter.drawImage(0, 0, image);
	update(); // 这个很重要, 避免绘制结束后窗口不会主动更新.
}


void RMapWidget::mousePressEvent(QMouseEvent *event)
{
	if(event==NULL)
		return;

	if(m_actType==Pan || m_actType==Select)
	{
		m_iLastMousePosX = event->pos().x();
		m_iLastMousePoxY = event->pos().y();

		QCursor cur(Qt::ClosedHandCursor);
		//m_centralWidget->setCursor(cur); // 设置为“关手”光标
	}
	else if(m_actType==ZoomIn || m_actType==ZoomOut || m_actType==EditRect){
		m_iLastMousePosX = event->pos().x();
		m_iLastMousePoxY = event->pos().y();
	}
}
void RMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event == NULL)
		return;

	// 鼠标右键，返回选择操作状态
	if(event->button() == Qt::RightButton)
	{
		if(m_actType!=Select)
			ActionSelect();
		return;
	}

	//if(m_actType_last==zoom_in || m_actType_last==zoom_out) // 取消上次缩放的绘制处理
	//	m_Map.GetDrawParms()->SetCancel(true);

	int x = event->pos().x();
	int y = event->pos().y(); // 这不是鼠标点的真实坐标，因为它的原点在主窗体左上角（包含了菜单、工具条）

	int iviewOrgx = geometry().x();
	int iviewOrgy = geometry().y();
	int ix=x-iviewOrgx, iy=y-iviewOrgy; // 鼠标逻辑坐标点

	if(m_actType == Pan)
	{
		int idx = x-m_iLastMousePosX;
		int idy = y-m_iLastMousePoxY;

		m_Map.Pan(idx, idy);
		QCursor cur(Qt::OpenHandCursor);
		setCursor(cur); // 设置为“开手”光标
		update();
	}
	else if(m_actType==ZoomIn)
	{
		if(abs(x-m_iLastMousePosX)<=3 && abs(y-m_iLastMousePoxY)<=3)
		{ // 小于3个像素
			m_Map.ZoomIn(ix, iy);
			int iWinWidth = geometry().width();
			int iWinHeight =geometry().height();
			m_Map.DrawWhenZoom(iWinWidth, iWinHeight, ix, iy, true);
			m_bDrawCacheL2=true;
		}
		else
		{
			m_Map.ZoomRect(m_iLastMousePosX, m_iLastMousePoxY, ix, iy);
			int iWinWidth = geometry().width();
			int iWinHeight = geometry().height();
			m_Map.DrawWhenZoomRect(iWinWidth, iWinHeight, m_iLastMousePosX, m_iLastMousePoxY, ix, iy);
			m_bDrawCacheL2=true;
		}
		update();
	}
	else if(m_actType==ZoomOut)
	{
		if(abs(x-m_iLastMousePosX)<=3 && abs(y-m_iLastMousePoxY)<=3){ // 小于3个像素，认为移动
			m_Map.ZoomOut(ix, iy);
			int iWinWidth = geometry().width();
			int iWinHeight = geometry().height();
			m_Map.DrawWhenZoom(iWinWidth, iWinHeight, ix, iy, false);
			m_bDrawCacheL2=true;
		}
		else
		{
			m_Map.ZoomRect(m_iLastMousePosX, m_iLastMousePoxY, ix, iy, false);
			int iWinWidth = geometry().width();
			int iWinHeight = geometry().height();
			m_Map.DrawWhenZoomRect(iWinWidth, iWinHeight, m_iLastMousePosX, m_iLastMousePoxY, ix, iy);
			m_bDrawCacheL2=true;
		}
		update();
	}
	else if(m_actType == Select)
	{
		//m_Map.SetRedrawFlag();
		//m_bDrawCacheL2=true; // 清除跟踪层对象
		//update();
	}
}

// 添加锚点
void _AddPointToTrackingLayer(RTrackingLayer* pTrackingLayer, 
							  double dx1, double dy1, double dx2, double dy2,
							  RStyle& geoStyle)
{
	OGRLinearRing ogrLineString;
	ogrLineString.addPoint(dx1, dy1);
	ogrLineString.addPoint(dx2, dy1);
	ogrLineString.addPoint(dx2, dy2);
	ogrLineString.addPoint(dx1, dy2);
	OGRPolygon ogrPolygon;
	ogrPolygon.addRingDirectly(&ogrLineString);
	pTrackingLayer->AddGeometry(&ogrPolygon, geoStyle);
}

void RMapWidget::mouseMoveEvent(QMouseEvent* event)
{
	if(event == NULL)
		return;

	int x = event->pos().x();
	int y = event->pos().y(); // 这不是显示范围的真正坐标点，因为它的原点在主窗体左上角（包含了菜单、工具条）

	int iviewOrgx = geometry().x();
	int iviewOrgy = geometry().y();
	double dx=x-iviewOrgx, dy=y-iviewOrgy;

	m_Map.GetDrawParms()->DPtoLP(dx, dy, &dx, &dy);
// 	double dRatio = m_Map.GetDrawParms()->GetRatio();
// 	double dResolution = (256*4) / 360.0; // 像素坐标/地理坐标
// 	int iScale  = dRatio / dResolution;

	// 求出2的N次方中的N值
// 	double d2 = log(double(2));
// 	double dValue = log(double(iScale));
// 	int n=int(dValue/d2);
// 	int iLevel = 2+n;

	char chTmp[256];
	chTmp[255]='\0';
	sprintf_s(chTmp, "坐标: %lf, %lf", dx, dy);
	string strMsg=chTmp;

	MapStatusCallBackFun pFunc = GetMap()->GetMapStatusCallBackFun();
	unsigned long pHandle = GetMap()->GetMapStatusHandle();
	if(pFunc!=NULL && pHandle!=NULL)
	{
		pFunc(pHandle, strMsg);
	}

	if(event->buttons() & Qt::LeftButton)
	{ // 按住鼠标左键，移动
		if(m_actType==Pan)
		{
			int idx = x-m_iLastMousePosX;
			int idy = y-m_iLastMousePoxY;

			m_Map.MouseMove(idx, idy); // 移屏过程有闪屏问题，暂时先封起来
			update();
		}	
	}
	else
	{
		
	}
}

void RMapWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	if(event == NULL)
		return;
	OGRGeometry* pTrackingGeom =NULL;// m_Map.GetTrackingGeometry();
	if(pTrackingGeom !=NULL)
	{
		int x = event->pos().x();
		int y = event->pos().y(); // 这不是显示范围的真正坐标点，因为它的原点在主窗体左上角（包含了菜单、工具条）

		int iviewOrgx = geometry().x();
		int iviewOrgy = geometry().y();

		int ix=x-iviewOrgx, iy=y-iviewOrgy;

		double dx,dy;
		m_Map.GetDrawParms()->DPtoLP(ix, iy, &dx, &dy);
		OGRPoint ogrPoint(dx, dy);
		if(pTrackingGeom->Intersect(&ogrPoint))
		{

		}
	}
}





void RMapWidget::wheelEvent(QWheelEvent * event)
{
	if(event == NULL)
		return;

	int x = event->pos().x();
	int y = event->pos().y(); // 这不是显示范围的真正坐标点，因为它的原点在主窗体左上角（包含了菜单、工具条）

	int iviewOrgx = geometry().x();
	int iviewOrgy = geometry().y();

	int ix=x-iviewOrgx, iy=y-iviewOrgy;
	int numDegrees = event->delta() / 8;
	bool bZoomIn=false;
	if(numDegrees > 0){
		m_Map.ZoomIn(ix, iy);
		bZoomIn=true;
	}
	else
		m_Map.ZoomOut(ix, iy);

	int iWinWidth= geometry().width();
	int iWinHeight= geometry().height();
	m_Map.DrawWhenZoom(iWinWidth, iWinHeight, ix, iy, bZoomIn);
	m_bDrawCacheL2=true;

	update();

}

void RMapWidget::contextMenuEvent ( QContextMenuEvent * event )
{	
	QPoint Pos = event->globalPos();
	//m_pMainWnd->GetRightBtnMenu()->exec(Pos);
}




