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
	//�������ͼƬ�и�ΪԲ��ͼƬ
	QImage circleImage(QImage& img,int radius);
private:
	//ͼ��
	QList<QgsMapLayer*> mLayers;
	bool is_paint; //�Ƿ����
	QImage mImg;  //ͼ��
	int mLength;

	qreal mX;
	qreal mY;
};
