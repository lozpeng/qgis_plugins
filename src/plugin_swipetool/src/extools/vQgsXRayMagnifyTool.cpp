#include "vQgsXRayMagnifyTool.h"


#include <QgsMapMouseEvent.h>
#include <qgsproject.h>
#include <qgslayertree.h>
#include <qgsmaplayer.h>
#include <qgsmapsettings.h>
#include <qgsmaprendererjob.h>
#include <qgsmapcanvas.h>

#include <QImage>
#include <qgsrectangle.h>

#include "vXSwipMapCanvasItem.h"
vQgsXRayMagnifyTool::~vQgsXRayMagnifyTool()
{
}

vQgsXRayMagnifyTool::vQgsXRayMagnifyTool(QgsMapCanvas* mapCanvas, int height)
	: QgsMapTool(mapCanvas),
	mXSwiper(new vXSwipMapCanvasItem(mapCanvas)),
	mMapRenderJob(0),
	mMapCanvas(mapCanvas)
{
	mHasSwipe = false;
	//鼠标
}
void vQgsXRayMagnifyTool::activate()
{
	this->canvas()->setCursor(QCursor(QPixmap(":/qgsml/images/pan.png").scaledToHeight(24)));
	_Connect();
	this->mHasSwipe = false;
	this->on_currentLayerChanged(0);
}
void vQgsXRayMagnifyTool::canvasPressEvent(QgsMapMouseEvent *e)
{
	this->mHasSwipe = true;
	mFirstPoint.setX(e->x());
	mFirstPoint.setY(e->y());
	mXSwiper->setImgExtent(e->x(), e->y());
}

void vQgsXRayMagnifyTool::canvasReleaseEvent(QgsMapMouseEvent *e)
{
	this->mHasSwipe = false;
	canvasMoveEvent(e);
	//鼠标释放后，移除绘制的线
	//
	mXSwiper->setImgExtent(-9999, -9999);
}

void vQgsXRayMagnifyTool::canvasMoveEvent(QgsMapMouseEvent *e)
{
	if (mHasSwipe)
		this->mXSwiper->setImgExtent(e->x(), e->y());
}
//连接事件
void vQgsXRayMagnifyTool::_Connect(bool isConnect)
{
	if (isConnect)
	{
		connect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &vQgsXRayMagnifyTool::setMap);
		connect(mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &vQgsXRayMagnifyTool::on_currentLayerChanged);
		connect(QgsProject::instance(), SIGNAL(removeAll()), this, SLOT(disable()));
	}
	else
	{
		disconnect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &vQgsXRayMagnifyTool::setMap);
		disconnect(mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &vQgsXRayMagnifyTool::on_currentLayerChanged);
		disconnect(QgsProject::instance(), SIGNAL(removeAll()), this, SLOT(disable()));
	}
}
void vQgsXRayMagnifyTool::deactivate()
{
	emit  deactivated();
	_Connect(false);
}
void vQgsXRayMagnifyTool::on_currentLayerChanged(QgsMapLayer *layer)
{
	QStringList curLyrIds;
	if (layer)
		curLyrIds << layer->id();
	else {
		if (mMapCanvas->currentLayer())
			curLyrIds << mMapCanvas->currentLayer()->id();
		else
		{
			//QgsProject::instance()->layerTreeRoot()->s
			//if the current selected layer is not a maplayer ,it's a group layer 
			//how to get it
			//mMapCanvas->
			//QgsProject::instance()->layerTreeRoot()->c
		}
	}
	if (curLyrIds.isEmpty() || curLyrIds.size() <= 0)
		return;

	QList<QgsMapLayer*> lyrs = QgsProject::instance()->layerTreeRoot()->checkedLayers();
	QList<QgsMapLayer*> lyr_list;
	foreach(QgsMapLayer* lyr, lyrs)
	{
		QString lyrId = lyr->id();
		if (curLyrIds.contains(lyrId)) //if no need to draw then except
			continue;
		lyr_list.append(lyr);
	}
	this->mXSwiper->clear();
	this->mXSwiper->setMapLayers(lyr_list);
	this->setMap();
}
void vQgsXRayMagnifyTool::disable()
{
	emit deactivated();
	this->mXSwiper->clear();
	this->mHasSwipe = false;
}


void vQgsXRayMagnifyTool::setMap()
{
	if (!mXSwiper || mXSwiper->mapLayers().size() <= 0)return;

	QgsMapSettings mapSettings = mMapCanvas->mapSettings();
	mapSettings.setLayers(mXSwiper->mapLayers());

	if (!mMapRenderJob)
	{
		mMapRenderJob = new QgsMapRendererParallelJob(mapSettings);
		connect(mMapRenderJob, &QgsMapRendererParallelJob::finished, [=]() {
			if (mMapRenderJob && mXSwiper)
			{
				QImage img = mMapRenderJob->renderedImage();
				QgsRectangle qgsRect = mMapCanvas->extent();
				mXSwiper->setContent(img, qgsRect);
			}
			delete mMapRenderJob;
			mMapRenderJob = nullptr;
		});
	}
	mMapRenderJob->start();
	mMapRenderJob->waitForFinished();
}
