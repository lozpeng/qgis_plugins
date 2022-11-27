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
	qgsCoordInputDialog* mCoordInpDialog; //坐标输入对话框
};
//通过随手绘制的方式添加集合要素:参考QgsMapToolSelectFreehand代码实现来构建几何要素
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

	//!工具被激活时需要获取当前编辑活动图层
	virtual void activate() override;
private:

	//! used for storing all of the maps point for the freehand sketch
	QgsRubberBand* mRubberBand;

	bool mDragging;

	QColor mFillColor;
	QColor mBorderColour;
	QgsVectorLayer* mCurrentLayer;//当前图层
};
//编辑帮助类
class qgsGeometryEditUtils :public QObject
{
	Q_OBJECT
public:
	////处理绘制的要素
	//static void processGeometry(QgsVectorLayer *vlayer,const QgsGeometry* drawGeo);
	//处理带有拓扑边界限制的分割与剪裁，opType，操作类型，0.新增，1.切，2.分割
	static void processGeometryTopo(QgsVectorLayer* vlayer, const QgsGeometry* drawGeo, int opType);

	//分别处理分割与剪裁，opType，操作类型，0.新增，1.切，2.分割
	static void processGeometry(QgsVectorLayer* vlayer, const QgsGeometry drawGeo, int opType);
	//新建要素保存数据
	static void saveGeometry(QgsVectorLayer* vlayer, const QgsGeometry geos);
	//修改要素的几何图形
	static bool changeFeatureGeo(QgsVectorLayer* vlayer, int fid, QgsGeometry* geo, bool changed);

	//多边形分割操作，返回分割后的多边形
	static QList<QgsGeometry> intersectsGeometry(QgsVectorLayer* vlayer, const QgsGeometry* drawGeo);
	//弹出QgsAttributeDialog 进行属性数据编辑
	static void editFeatureAttr(QgsVectorLayer* vlayer, QgsFeature* feature);
};
#endif
