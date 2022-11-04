#pragma once

#include <QObject>
#include <QComboBox>

#include <qgsmaptool.h>
#include <qgsmapcanvas.h>
#include <qgsmaprendererparalleljob.h>

#include "mapswip\vXSwipMapCanvasItem.h"

class vQgsXRayMagnifyTool : public QgsMapTool
{
	Q_OBJECT
public:
	vQgsXRayMagnifyTool(QgsMapCanvas* mapCanvas, int height);
	~vQgsXRayMagnifyTool();

	virtual void activate();
	virtual void deactivate();

	virtual void canvasMoveEvent(QgsMapMouseEvent *e);
	virtual void canvasReleaseEvent(QgsMapMouseEvent *e);
	virtual void canvasPressEvent(QgsMapMouseEvent *e);
private:
	QPoint mFirstPoint;
	bool mHasSwipe;
	vXSwipMapCanvasItem* mXSwiper;
	QgsMapCanvas* mMapCanvas;
	QgsMapRendererParallelJob* mMapRenderJob;
	//链接事件
	void _Connect(bool isConnect = true);

	private slots:
	void setMap();
	void disable();
	//当当前图层发生变化时
	void on_currentLayerChanged(QgsMapLayer *layer);
};
