#ifndef _RMAPWIDGET_H__
#define _RMAPWIDGET_H__

#pragma once

#include <QWidget>
#include "Base/RMapWnd.h"

//////////////////////////////////////////////////////////////////////////
// 地图窗口控件
//////////////////////////////////////////////////////////////////////////
class RQTCORE_API RMapWidget : public QWidget
{

	Q_OBJECT

public:
	enum actionType{
		Select = 0,
		Pan=1,
		ZoomIn=2,
		ZoomOut=3,

		EditRect=4, // 绘制矩形区域
		EditMove=5, // 移动对象
	};


	RMapWidget(QWidget *parent = 0, Qt::WFlags flags = 0);
	~RMapWidget();

	void AddLayer(const QString& strDsName, const QString& strDtName);

	RMap* GetMap()  { return &m_Map;}

	void SetDrawCache(bool bCache=true) {m_bDrawCacheL2=bCache;}
	bool GetDrawCache() {return m_bDrawCacheL2;}

	// 事件
	void ActionSelect();
	void ActionPan();
	void ActionZoomIn();
	void ActionZoomOut();
	void ActionEditRect();

protected:

	virtual void paintEvent(QPaintEvent * event);


	/************************************************************************/
	/* 鼠标按下事件                                                                     */
	/************************************************************************/
	virtual void mousePressEvent(QMouseEvent *event);


	/************************************************************************/
	/* 鼠标释放事件                                                                     */
	/************************************************************************/
	virtual void mouseReleaseEvent(QMouseEvent *event);

	virtual void wheelEvent(QWheelEvent * event);

	/************************************************************************/
	/* 鼠标移动事件                                                                     */
	/************************************************************************/
	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void mouseDoubleClickEvent(QMouseEvent * event);

	/************************************************************************/
	/* 右键菜单                                                                     */
	/************************************************************************/
	virtual void	contextMenuEvent ( QContextMenuEvent * event );

	void ChangeCursor(actionType actType);

	RMap m_Map;

	actionType m_actType; // 当前鼠标类型
	actionType m_actTypeLast; // 上次鼠标类型

	int m_iLastMousePosX;
	int m_iLastMousePoxY;


	/// 缩放过程是否需要绘制二级缓存
	bool m_bDrawCacheL2;


};


#endif //_RMAPWIDGET_H__
