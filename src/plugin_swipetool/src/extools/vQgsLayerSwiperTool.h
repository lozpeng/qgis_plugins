#ifndef V_QGS_LAYER_SWIPER_H_
#define V_QGS_LAYER_SWIPER_H_

#include "./mapswip/vSwipMapCanvasItem.h"

#include <QComboBox>

#include <qgsmaptool.h>
#include <qgsmapcanvas.h>
#include <qgsmaprendererparalleljob.h>

//Qgis 图层swipe工具
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

	//!按下键盘键时
	virtual void keyPressEvent(QKeyEvent* e);
	//!释放键盘键时
	virtual void keyReleaseEvent(QKeyEvent* e);
private:
	QPoint mFirstPoint;
	vSwipMapCanvasItem* mMapSwiper = 0;
	bool mHasSwipe;
	bool mIsCtrlKeyDown;  //是否按住control键
	QgsMapCanvas* mMapCanvas = 0;
	QgsMapRendererParallelJob* mMapRenderJob = 0;
	//鼠标样式
	QCursor mCursorSV;
	//!
	QCursor mCursorSH;
	QCursor mCursorUP;
	QCursor mCursorDown;
	QCursor mCursorLeft;
	QCursor mCursorRight;
	QCursor mCursorBox;
	//链接事件
	void _Connect(bool isConnect=true);

private slots:
	void setMap();
	void disable();
	//当当前图层发生变化时
	void on_currentLayerChanged(QgsMapLayer *layer);
};
#endif

