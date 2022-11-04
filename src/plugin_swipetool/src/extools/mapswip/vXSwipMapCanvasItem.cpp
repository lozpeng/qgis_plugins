#include "vXSwipMapCanvasItem.h"
#include <QPen>

const int vRadius = 200;
vXSwipMapCanvasItem::vXSwipMapCanvasItem(QgsMapCanvas *mapCanvas)
	:QgsMapCanvasItem(mapCanvas)
{
	mIsVertical = false;
	is_paint = false;
	this->mLength = 0;
	this->mLayers.clear();
	mMapCanvas = mapCanvas;
}

vXSwipMapCanvasItem::~vXSwipMapCanvasItem()
{
}


void vXSwipMapCanvasItem::setDirection(int dir)
{
	
}
void vXSwipMapCanvasItem::setContent(QImage& img, QgsRectangle& rect)
{
	this->mImg = img;
	this->setRect(rect);
}
void vXSwipMapCanvasItem::setMapLayers(QList<QgsMapLayer*> layers)
{
	this->mLayers.clear();
	this->mLayers.append(layers);
}
void vXSwipMapCanvasItem::setImgExtent(int x, int y)
{
	if (!is_paint)
	{
		this->setX(0);
		this->setY(0);
	}
	mX = x;
	mY = y;
	this->is_paint = true;

	this->update();
}
void vXSwipMapCanvasItem::clear() {
	this->is_paint = false;
	mLayers.clear();
}
void vXSwipMapCanvasItem::paint(QPainter *painter)
{
	if (mLayers.length() <= 0 || !is_paint)
		return;
	qreal halfRadius = vRadius / 2;
	qreal tX = mX  - halfRadius; //计算出圆心的位置
	qreal tY = mY  - halfRadius;//计算出圆心的位置

	QImage img = this->mImg.copy(tX, tY, vRadius, vRadius);
	//绘制之前根据⚪将图像进行过滤
	img = circleImage(img, halfRadius);
	painter->drawImage(QRectF(tX, tY, vRadius, vRadius), img);

	//绘制一个⚪ ，
	QPen pen = QPen(Qt::PenStyle::DashDotDotLine);
	pen.setColor(QColor(18, 150, 219));
	pen.setWidth(4);
	painter->setPen(pen);

	painter->drawEllipse(tX, tY, vRadius, vRadius);
}

QImage vXSwipMapCanvasItem::circleImage(QImage& img, int radius)
{
	QPixmap pixmapa = QPixmap::fromImage(img);
	qreal d = radius * 2;
	//新建一张图片
	QPixmap pixmap(d, d);

	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	QPainterPath path;
	path.addEllipse(0, 0, d, d);
	painter.setClipPath(path);
	painter.drawPixmap(0, 0, d, d, pixmapa);
	return pixmap.toImage();
}
