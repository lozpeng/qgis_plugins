#include "vSwipMapCanvasItem.h"
#include <QPen>
#include <QImage>
#include <qgsrectangle.h>

vSwipMapCanvasItem::vSwipMapCanvasItem(QgsMapCanvas *mapCanvas)
	:QgsMapCanvasItem(mapCanvas)
{
	mIsVertical = false;
	is_paint = false;
	this->mLength = 0;
	this->mLayers.clear();
	mMapCanvas = mapCanvas;
}

vSwipMapCanvasItem::~vSwipMapCanvasItem()
{
}


void vSwipMapCanvasItem::setDirection(int dir)
{
	// 0:'⬇', 1 : '⬆', 2 : '➡', 3 : '⬅'
	this->mSwipeDir = dir;
	this->mStartX = 0;
	this->mStartY = 0;
	this->mEndX = this->boundingRect().width();
	this->mEndY = this->boundingRect().height();
}
void vSwipMapCanvasItem::setContent(QImage& img, QgsRectangle& rect)
{
	this->mImg = img;
	this->setRect(rect);
}
void vSwipMapCanvasItem::setMapLayers(QList<QgsMapLayer*> layers)
{
	this->mLayers.clear();
	this->mLayers.append(layers);
}
void vSwipMapCanvasItem::setImgExtent(int x, int y)
{
	if (!is_paint)
	{
		this->setX(0);
		this->setY(0);
	}
	mX = x;
	mY = y;
	if (mSwipeDir == 0)//'⬇'
		mEndY = y;
	if (mSwipeDir == 1)
		mStartY = y;
	if (mSwipeDir == 2)
		mEndX = x;
	if (mSwipeDir == 3)
		mStartX = x;
	this->is_paint = true;

	this->update();
}
void vSwipMapCanvasItem::clear() {
	this->is_paint = false;
	mLayers.clear();
}
void vSwipMapCanvasItem::paint(QPainter *painter)
{
	if (mLayers.length() <= 0 || !is_paint)
		return;
	int w = this->boundingRect().width();
	int h = this->boundingRect().height();
	QPen pen = QPen(Qt::PenStyle::DashDotDotLine);
	pen.setColor(QColor(18, 150, 219));
	pen.setWidth(4);
	painter->setPen(pen);

	if (this->mIsVertical)
		painter->drawLine(QPointF(mX, 0), QPointF(mX, h));
	else 
		painter->drawLine(QPointF(0, mY), QPointF(w, mY));

	QImage img = this->mImg.copy(mStartX, mStartY, mEndX, mEndY);
	painter->drawImage(QRectF(mStartX, mStartY, mEndX, mEndY), img);
}
