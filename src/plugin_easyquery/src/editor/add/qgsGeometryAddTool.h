#ifndef QGS_GEOMETRY_ADD_TOOL_H_
#define QGS_GEOMETRY_ADD_TOOL_H_


#include <QObject>
#include <qgsmaptoolcapture.h>
#include <qgsmapcanvas.h>

#include "qgsCoordInputDialog.h"

class qgsGeometryAddTool :public QgsMapToolCapture
{
	Q_OBJECT
public:
	qgsGeometryAddTool(QgsMapCanvas* canvas);
	virtual ~qgsGeometryAddTool();
	void cadCanvasReleaseEvent(QgsMapMouseEvent* e) override;
	void activate() override;

	void keyPressEvent(QKeyEvent* e) override;
	void keyReleaseEvent(QKeyEvent* e) override;
private:
	qgsCoordInputDialog* mCoordInpDialog; //��������Ի���
};
//ͨ�����ֻ��Ƶķ�ʽ��Ӽ���Ҫ��:�ο�QgsMapToolSelectFreehand����ʵ������������Ҫ��
class qgsToolAddFeatureFreehand : public QgsMapTool
{
	Q_OBJECT
public:
	qgsToolAddFeatureFreehand(QgsMapCanvas* canvas);

	virtual ~qgsToolAddFeatureFreehand();

	//! Overridden mouse move event
	virtual void canvasMoveEvent(QgsMapMouseEvent* e) override;

	//! Overridden mouse press event
	virtual void canvasPressEvent(QgsMapMouseEvent* e) override;

	//! Overridden mouse release event
	virtual void canvasReleaseEvent(QgsMapMouseEvent* e) override;

	//!���߱�����ʱ��Ҫ��ȡ��ǰ�༭�ͼ��
	virtual void activate() override;
private:

	//! used for storing all of the maps point for the freehand sketch
	QgsRubberBand* mRubberBand;

	bool mDragging;

	QColor mFillColor;
	QColor mBorderColour;
	QgsVectorLayer* mCurrentLayer;//��ǰͼ��
};
//�༭������
class qgsGeometryEditUtils :public QObject
{
	Q_OBJECT
public:
	////������Ƶ�Ҫ��
	//static void processGeometry(QgsVectorLayer *vlayer,const QgsGeometry* drawGeo);
	//����������˱߽����Ƶķָ�����ã�opType���������ͣ�0.������1.�У�2.�ָ�
	static void processGeometryTopo(QgsVectorLayer* vlayer, const QgsGeometry* drawGeo, int opType);

	//�ֱ���ָ�����ã�opType���������ͣ�0.������1.�У�2.�ָ�
	static void processGeometry(QgsVectorLayer* vlayer, const QgsGeometry drawGeo, int opType);
	//�½�Ҫ�ر�������
	static void saveGeometry(QgsVectorLayer* vlayer, const QgsGeometry geos);
	//�޸�Ҫ�صļ���ͼ��
	static bool changeFeatureGeo(QgsVectorLayer* vlayer, int fid, QgsGeometry* geo, bool changed);

	//����ηָ���������طָ��Ķ����
	static QList<QgsGeometry> intersectsGeometry(QgsVectorLayer* vlayer, const QgsGeometry* drawGeo);
	//����QgsAttributeDialog �����������ݱ༭
	static void editFeatureAttr(QgsVectorLayer* vlayer, QgsFeature* feature);
};
#endif
