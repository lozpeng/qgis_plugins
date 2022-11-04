#pragma once

#include <QImage>
#include <qlist.h>

#include <qgsmapcanvas.h>
#include <qgsmapcanvasitem.h>
#include <qgsmaplayer.h>
#include <qgsrectangle.h>

//��ͼ�ָ�
class vSwipMapCanvasItem : public QgsMapCanvasItem
{
public:
	vSwipMapCanvasItem(QgsMapCanvas *mapCanvas);
	~vSwipMapCanvasItem();
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

private :
	//ͼ��
	QList<QgsMapLayer*> mLayers;
	bool is_paint; //�Ƿ����
	int mSwipeDir; //�����ķ���
	QImage mImg;  //ͼ��
	int mLength;

	qreal mX;
	qreal mY;
	//
	qreal mStartX;
	qreal mStartY;
	qreal mEndX;
	qreal mEndY;
};

