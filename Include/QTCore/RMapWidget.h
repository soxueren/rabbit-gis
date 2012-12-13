#ifndef _RMAPWIDGET_H__
#define _RMAPWIDGET_H__

#pragma once

#include <QWidget>
#include "Base/RMapWnd.h"

//////////////////////////////////////////////////////////////////////////
// ��ͼ���ڿؼ�
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

		EditRect=4, // ���ƾ�������
		EditMove=5, // �ƶ�����
	};


	RMapWidget(QWidget *parent = 0, Qt::WFlags flags = 0);
	~RMapWidget();

	void AddLayer(const QString& strDsName, const QString& strDtName);

	RMap* GetMap()  { return &m_Map;}

	void SetDrawCache(bool bCache=true) {m_bDrawCacheL2=bCache;}
	bool GetDrawCache() {return m_bDrawCacheL2;}

	// �¼�
	void ActionSelect();
	void ActionPan();
	void ActionZoomIn();
	void ActionZoomOut();
	void ActionEditRect();

protected:

	virtual void paintEvent(QPaintEvent * event);


	/************************************************************************/
	/* ��갴���¼�                                                                     */
	/************************************************************************/
	virtual void mousePressEvent(QMouseEvent *event);


	/************************************************************************/
	/* ����ͷ��¼�                                                                     */
	/************************************************************************/
	virtual void mouseReleaseEvent(QMouseEvent *event);

	virtual void wheelEvent(QWheelEvent * event);

	/************************************************************************/
	/* ����ƶ��¼�                                                                     */
	/************************************************************************/
	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void mouseDoubleClickEvent(QMouseEvent * event);

	/************************************************************************/
	/* �Ҽ��˵�                                                                     */
	/************************************************************************/
	virtual void	contextMenuEvent ( QContextMenuEvent * event );

	void ChangeCursor(actionType actType);

	RMap m_Map;

	actionType m_actType; // ��ǰ�������
	actionType m_actTypeLast; // �ϴ��������

	int m_iLastMousePosX;
	int m_iLastMousePoxY;


	/// ���Ź����Ƿ���Ҫ���ƶ�������
	bool m_bDrawCacheL2;


};


#endif //_RMAPWIDGET_H__
