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
//!�ڱ༭ͼ����ѡ��һ������ͼ�κ󣬵������ť����һ��ͼ��ѡ���
//! �û�ѡ��һ���߻�����ͼ����ѡ�е�ͼ������и���ɶ��Ҫ��
//! ����������ʹ��ԭ����
class qgsVectorSplitAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsVectorSplitAction();
	virtual ~qgsVectorSplitAction();
	//!����
	int compute();
protected:
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
	//!�Ƿ��ö���ν����и
	bool isSplitByPolygon();
	//!�ָ�����
	int splitByPolygon(QgsFeature toClipFeature, QgsFeatureList& results);
	//!�ָ���
	int splitByPolyline(QgsFeature toClipFeature, QgsFeatureList& results);
	//! ����¼�봰��
	qgsVectorLayerSelectBaseDialog* mDialog;
	//���и�ͼ��
	QgsVectorLayer* mClipLayer;
private slots:
	void on_layer_editing();
	void on_layer_editingStopped();
};

#endif
