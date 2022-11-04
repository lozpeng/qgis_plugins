#pragma once

#include <QObject>
#include <QImage>
#include <qlist.h>

#include <qgsmapcanvas.h>
#include <qgsmapcanvasitem.h>
#include <qgsmaplayer.h>

class vXSwipMapCanvasItem : public QgsMapCanvasItem
{
public:
	vXSwipMapCanvasItem(QgsMapCanvas *mapCanvas);
	~vXSwipMapCanvasItem();
	//
	void setContent(QImage& img, QgsRectangle& rect);
	void setImgExtent(int x, int y);
	void clear();
	void setDirection(int dir);
	void setMapLayers(QList<QgsMapLayer*> layers);
	QList<QgsMapLayer*> mapLayers() { return mLayers; }
	void setLength(int len) { mLength = len; }
	bool mIsVertical;
protected:
	virtual void paint(QPainter *painter);
	//½«ÊäÈëµÄÍ¼Æ¬ÇÐ¸îÎªÔ²ÐÎÍ¼Æ¬
	QImage circleImage(QImage& img,int radius);
private:
	//Í¼²ã
	QList<QgsMapLayer*> mLayers;
	bool is_paint; //ÊÇ·ñ»æÖÆ
	QImage mImg;  //Í¼Ïñ
	int mLength;

	qreal mX;
	qreal mY;
};
