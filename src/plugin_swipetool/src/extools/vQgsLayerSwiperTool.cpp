#include "vQgsLayerSwiperTool.h"

#include <QgsMapMouseEvent.h>
#include <qgsproject.h>
#include <qgslayertree.h>
#include <qgsmaplayer.h>
#include <qgsmapsettings.h>
#include <qgsmaprendererjob.h>
#include <qgsmapcanvas.h>

#include <QImage>
#include <qgsrectangle.h>
//
vQgsLayerSwiperTool::vQgsLayerSwiperTool( QgsMapCanvas* mapCanvas, int height)
	: QgsMapTool(mapCanvas),
	mMapSwiper(new vSwipMapCanvasItem(mapCanvas)),
	mMapRenderJob(0),
	mMapCanvas(mapCanvas)
{
	mHasSwipe = false;
	mIsCtrlKeyDown = false;
	//鼠标
	mCursorSV = QCursor(QPixmap(":/qgsml/images/split_v.png").scaledToHeight(height));
	mCursorSH = QCursor(QPixmap(":/qgsml/images/split_h.png").scaledToHeight(height));
	mCursorUP = QCursor(QPixmap(":/qgsml/images/up.png").scaledToHeight(height));
	mCursorDown = QCursor(QPixmap(":/qgsml/images/down.png").scaledToHeight(height));
	mCursorLeft = QCursor(QPixmap(":/qgsml/images/left.png").scaledToHeight(height));
	mCursorRight = QCursor(QPixmap(":/qgsml/images/right.png").scaledToHeight(height));
	//
	mCursorBox = QCursor(QPixmap(":/qgsml/images/right.png").scaledToHeight(height));
}

//
vQgsLayerSwiperTool::~vQgsLayerSwiperTool()
{
	/*
	if (mMapSwiper)
		delete mMapSwiper;
	mMapSwiper = nullptr;
	*/
}
//
void vQgsLayerSwiperTool::activate()
{
	this->canvas()->setCursor(QCursor(Qt::CrossCursor));
	_Connect();
	this->mHasSwipe = false;
	this->on_currentLayerChanged(0);
}
void vQgsLayerSwiperTool::canvasPressEvent(QgsMapMouseEvent *e)
{
	this->mHasSwipe = true;
	int w = mMapCanvas->width();
	int h = mMapCanvas->height();
	int dir = -99;
	if (!mIsCtrlKeyDown)
	{
		if (0.25 * w < e->x() && e->x() < 0.75 * w && e->y() < 0.5 * h)
		{
			dir = 0;  // '⬇'
			mMapSwiper->mIsVertical = false;
		}
		if (0.25 * w < e->x() && e->x() < 0.75 * w && e->y() > 0.5 * h) {
			dir = 1;  // '⬆'
			mMapSwiper->mIsVertical = false;
		}
		if (e->x() < 0.25 * w) {
			dir = 2;  //# '➡'
			mMapSwiper->mIsVertical = true;
		}
		if (e->x() > 0.75 * w) {
			dir = 3; // # '⬅'
			mMapSwiper->mIsVertical = true;
		}
	}
	else
	{
		dir = -1;

	}
	if (dir == -99)return;
	
	mFirstPoint.setX(e->x());
	mFirstPoint.setY(e->y());

	mMapSwiper->setDirection(dir);
	mMapCanvas->setCursor(mMapSwiper->mIsVertical ? mCursorSV : mCursorSH);
	mMapSwiper->setImgExtent(e->x(), e->y());
}

void vQgsLayerSwiperTool::canvasReleaseEvent(QgsMapMouseEvent *e)
{
	this->mHasSwipe = false;
	canvasMoveEvent(e);
	//鼠标释放后，移除绘制的线
	this->mMapSwiper->setImgExtent(-99999, -99999);
}

//!按下键盘键时
void vQgsLayerSwiperTool::keyPressEvent(QKeyEvent* e)
{
	if (e->modifiers() == Qt::ControlModifier)
	{
		this->mMapCanvas->setCursor(mCursorBox);
		this->mIsCtrlKeyDown = true;
	}
}
//!释放键盘键时
void vQgsLayerSwiperTool::keyReleaseEvent(QKeyEvent* e)
{
	if (!e->isAutoRepeat())
	{
		this->mIsCtrlKeyDown = false;
		QPoint pnt = this->mCursorBox.pos();
		this->mCursorBox.setPos(pnt.x() + 1, pnt.y() + 1);
	}
}


void vQgsLayerSwiperTool::canvasMoveEvent(QgsMapMouseEvent *e)
{
	if (mHasSwipe)
		this->mMapSwiper->setImgExtent(e->x(), e->y());
	else
	{
		int w = mMapCanvas->width();
		int h = mMapCanvas->height();
		//
		if (e->x() < 0.25 *w)
			mMapCanvas->setCursor(mCursorRight);
		if (e->x() > 0.75 *w)
			mMapCanvas->setCursor(mCursorLeft);
		if (0.25 * w < e->x() && e->x() < 0.75 * w && e->y() < 0.5 * h)
			mMapCanvas->setCursor(mCursorDown);
		if (0.25 * w < e->x() && e->x() < 0.75 * w && e->y() > 0.5 * h)
			mMapCanvas->setCursor(mCursorUP);
	}
}
//连接事件
void vQgsLayerSwiperTool::_Connect(bool isConnect)
{
	if (isConnect)
	{
		connect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &vQgsLayerSwiperTool::setMap);
		connect(mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &vQgsLayerSwiperTool::on_currentLayerChanged);
		connect(QgsProject::instance(), SIGNAL(removeAll()), this, SLOT(disable()));
	}
	else
	{
		disconnect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &vQgsLayerSwiperTool::setMap);
		disconnect(mMapCanvas, &QgsMapCanvas::currentLayerChanged, this, &vQgsLayerSwiperTool::on_currentLayerChanged);
		disconnect(QgsProject::instance(), SIGNAL(removeAll()), this, SLOT(disable()));
	}
}
void vQgsLayerSwiperTool::deactivate()
{
	emit  deactivated();
	this->mMapSwiper->clear();
	_Connect(false);
}
void vQgsLayerSwiperTool::on_currentLayerChanged(QgsMapLayer *layer)
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
	if (curLyrIds.isEmpty() || curLyrIds.size()<=0)
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
	this->mMapSwiper->clear();
	this->mMapSwiper->setMapLayers(lyr_list);
	this->setMap();
}
void vQgsLayerSwiperTool::disable()
{
	emit deactivated();
	this->mMapSwiper->clear();
	this->mHasSwipe = false;
}


void vQgsLayerSwiperTool::setMap()
{
	if (!mMapSwiper || mMapSwiper->mapLayers().size() <= 0)return;

	QgsMapSettings mapSettings = mMapCanvas->mapSettings();
	mapSettings.setLayers(mMapSwiper->mapLayers());

	if(!mMapRenderJob)
	{
		mMapRenderJob = new QgsMapRendererParallelJob(mapSettings);
		connect(mMapRenderJob, &QgsMapRendererParallelJob::finished, [=]() {
			if (mMapRenderJob && mMapSwiper)
			{
				QImage img = mMapRenderJob->renderedImage();
				QgsRectangle qgsRect = mMapCanvas->extent();
				mMapSwiper->setContent(img, qgsRect);
			}
				
			delete mMapRenderJob;
			mMapRenderJob = nullptr;
		});
	}
	mMapRenderJob->start();
	mMapRenderJob->waitForFinished();
}