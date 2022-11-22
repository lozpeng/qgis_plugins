#ifndef QGS_VECTOR_SPLIT_ACTION_H_
#define QGS_VECTOR_SPLIT_ACTION_H_

#include "base/QgsBaseActionFilter.h"

#include "qgsVectorLayerSelectBaseDialog.h"

#include <QObject>
#include <QVector>
#include <qgsfeature.h>

class QAction;
class QToolBar;
class QgsMapLayer;

class QgisInterface;
class QgsGeometry;
class QgsVectorLayer;
//!在编辑图层中选择一个或多个图形后，点击本按钮弹出一个图层选择框
//! 用户选择一个线或多边形图层后对选中的图层进行切割，生成多个要素
//! 属性数据则使用原来的
class qgsVectorSplitAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsVectorSplitAction();
	virtual ~qgsVectorSplitAction();
	//!计算
	int compute();
protected:
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
	//!是否用多边形进行切割？
	bool isSplitByPolygon();
	//!分割多边形
	int splitByPolygon(QgsFeature toClipFeature, QgsFeatureList& results);
	//!分割线
	int splitByPolyline(QgsFeature toClipFeature, QgsFeatureList& results);
	//! 参数录入窗口
	qgsVectorLayerSelectBaseDialog* mDialog;
	//！切割图层
	QgsVectorLayer* mClipLayer;
private slots:
	void on_layer_editing();
	void on_layer_editingStopped();
};

#endif
