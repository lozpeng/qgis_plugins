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
	//�����¼�
	void _Connect(bool isConnect = true);

	private slots:
	void setMap();
	void disable();
	//����ǰͼ�㷢���仯ʱ
	void on_currentLayerChanged(QgsMapLayer *layer);
};
