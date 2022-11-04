#ifndef V_QGS_LAYER_SWIPER_H_
#define V_QGS_LAYER_SWIPER_H_

#include "./mapswip/vSwipMapCanvasItem.h"

#include <QComboBox>

#include <qgsmaptool.h>
#include <qgsmapcanvas.h>
#include <qgsmaprendererparalleljob.h>

//Qgis ͼ��swipe����
class vQgsLayerSwiperTool : public QgsMapTool
{
public:
	vQgsLayerSwiperTool(QgsMapCanvas* mapCanvas,int height);
	~vQgsLayerSwiperTool();

	virtual void activate();
	virtual void deactivate();

	virtual void canvasMoveEvent(QgsMapMouseEvent *e);
	virtual void canvasReleaseEvent(QgsMapMouseEvent *e);
	virtual void canvasPressEvent(QgsMapMouseEvent *e);

	//!���¼��̼�ʱ
	virtual void keyPressEvent(QKeyEvent* e);
	//!�ͷż��̼�ʱ
	virtual void keyReleaseEvent(QKeyEvent* e);
private:
	QPoint mFirstPoint;
	vSwipMapCanvasItem* mMapSwiper = 0;
	bool mHasSwipe;
	bool mIsCtrlKeyDown;  //�Ƿ�סcontrol��
	QgsMapCanvas* mMapCanvas = 0;
	QgsMapRendererParallelJob* mMapRenderJob = 0;
	//�����ʽ
	QCursor mCursorSV;
	//!
	QCursor mCursorSH;
	QCursor mCursorUP;
	QCursor mCursorDown;
	QCursor mCursorLeft;
	QCursor mCursorRight;
	QCursor mCursorBox;
	//�����¼�
	void _Connect(bool isConnect=true);

private slots:
	void setMap();
	void disable();
	//����ǰͼ�㷢���仯ʱ
	void on_currentLayerChanged(QgsMapLayer *layer);
};
#endif

