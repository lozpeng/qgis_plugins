#include "qgsVectorSplitAction.h"
#include "qgsVectorLayerSelectBaseDialog.h"

#include <QVector>

#include <qgsgeometry.h>
#include <qgsvectorlayer.h>
#include <qgsmapcanvas.h>
#include <qgisinterface.h>
#include <qgsproject.h>
#include <qgsvectorlayereditbuffer.h>
#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgsvectorlayerutils.h>

qgsVectorSplitAction::qgsVectorSplitAction():
	mDialog(0), mClipLayer(0),
	QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mGeometrySplitAction",
			QString::fromLocal8Bit("要素分割"),
			QString::fromLocal8Bit("使用图层分割选中要素"),
			":/easy_query/images/build-24px.svg"))
{


}
qgsVectorSplitAction::~qgsVectorSplitAction()
{
	if (mDialog)
		delete mDialog;
	mDialog = 0;
}
//！当图层开启编辑之后
void qgsVectorSplitAction::on_layer_editing()
{
	this->setEnabled(true);
}
void qgsVectorSplitAction::on_layer_editingStopped()
{
	this->setEnabled(false);
}

int qgsVectorSplitAction::openInputDialog()
{
	if (!this->m_qgsInterface)return -1;

	if (!mDialog)
		mDialog = new qgsVectorLayerSelectBaseDialog(this->m_qgsInterface->mainWindow());
	mDialog->updateLayers();
	int result = mDialog->exec();
	mDialog->close();
	return result;
}
void qgsVectorSplitAction::getParametersFromDialog() {
	QString lyrId("-1");
	mDialog->getSelectedLayerId(lyrId);
	if (QString("-1").compare(lyrId) == 0)
		mClipLayer = 0;
	else
	{
		QgsMapLayer* mpLyr = QgsProject::instance()->mapLayer(lyrId);
		if (!mpLyr)mClipLayer = 0;
		else 
			mClipLayer = qobject_cast<QgsVectorLayer*> (mpLyr);
	}
}
//!计算
int qgsVectorSplitAction::compute()
{
	if (!m_qgsInterface|| !mClipLayer)return -1;
	//1..获取当前选中的图层，并判断是否处于编辑状态
	//QgsVectorLayer* pCurrentLayer = m_qgsInterface->
	QgsMapCanvas* mapCanvas = m_qgsInterface->mapCanvas();
	auto layr = mapCanvas->currentLayer();
	QgsVectorLayer* vctLyr = qobject_cast<QgsVectorLayer*>(layr);
	if (!vctLyr || !vctLyr->isEditable())return -1;
	if (vctLyr->selectedFeatureCount() <= 0)return -1;
	QgsFeatureList selectFeatures = vctLyr->selectedFeatures();
	int count = selectFeatures.count();
	bool clipByPolygon = this->isSplitByPolygon();
	for (int i = 0; i < count; i++) //处理每一个要素
	{
		QgsFeature gFeature = selectFeatures.at(i);
		QgsFeatureList results;
		int n= 0;
		if (clipByPolygon)
			n = splitByPolygon(gFeature, results);
		else 
			n = splitByPolyline(gFeature, results);
		if (n >= 1 && n == results.count())
		{
			vctLyr->deleteFeature(gFeature.id()); //删除原来的要素
			vctLyr->editBuffer()->addFeatures(results);
		}
		else
			this->showMessage(QString::fromLocal8Bit("要素分割失败:"+gFeature.id()));
	}
	return 0;
}
//!分割多边形，分两中情况
int qgsVectorSplitAction::splitByPolygon(QgsFeature toClipFeature, QgsFeatureList& results) {
	if (!mClipLayer)return -1;
	QgsFeatureRequest fr;
	fr.setFilterRect(toClipFeature.geometry().boundingBox());
	QgsFeatureIterator fi = mClipLayer->getFeatures(fr); //根据需要切割的要素查询切割图层，获取相交的几何图形
	
	QgsAttributes attrs = toClipFeature.attributes();
	QgsGeometry toClipGeometry = toClipFeature.geometry();
	int count=0;
	QgsGeometry allGeometry; //记录分割出来的各个小部分，最后用来和原来的要素计算不一致的地方，用于保存剩余部分
	QgsFeature clipFeature; //这是一个切割多边形
	while (fi.nextFeature(clipFeature))
	{
		QgsGeometry clipGeometry = clipFeature.geometry();
		if (!clipGeometry.intersects(toClipGeometry))continue;

		QgsGeometry nGeometry = clipGeometry.intersection(toClipGeometry);//求出两个多边形的相交部分，
		if (nGeometry.isEmpty())
		{
			QString str = nGeometry.lastError();
			this->showMessage(str);
			continue;
		}
		if (allGeometry.isEmpty())
			allGeometry.fromWkb(nGeometry.asWkb());
		else allGeometry = allGeometry.combine(nGeometry);

		QgsFeature nwFeature;
		nwFeature.initAttributes(toClipFeature.attributeCount());
		nwFeature.setGeometry(nGeometry);
		nwFeature.setAttributes(attrs);
		results.append(nwFeature);
		count++;
	}
	if (!allGeometry.isEmpty()) //如果不为空
	{
		QgsGeometry leftGeometry = allGeometry.difference(toClipGeometry);
		if (leftGeometry.isEmpty())
		{
			QString str = leftGeometry.lastError();
			this->showMessage(str);
		}
		else
		{
			QgsFeature nwFeature;
			nwFeature.initAttributes(toClipFeature.attributeCount());
			nwFeature.setGeometry(leftGeometry);
			nwFeature.setAttributes(attrs);
			results.append(nwFeature);
			count++;
		}
	}
	fi.close();
	return count;
}
//!分割线
int qgsVectorSplitAction::splitByPolyline(QgsFeature toClipFeature, QgsFeatureList& results) {

	if (!mClipLayer)return -1;
	QgsFeatureRequest fr;
	fr.setFilterRect(toClipFeature.geometry().boundingBox());
	QgsFeatureIterator fi = mClipLayer->getFeatures(fr); //根据需要切割的要素查询切割图层，获取相交的几何图形

	QgsAttributes attrs = toClipFeature.attributes();
	QgsGeometry toClipGeometry = toClipFeature.geometry();
	int count = 0;
	QgsGeometry allGeometry; //记录分割出来的各个小部分，最后用来和原来的要素计算不一致的地方，用于保存剩余部分
	QgsFeature clipFeature; //这是一个切割线
	//首先将线合并，避免小线段不能与多边形或目标几何多边形相交，不能进行数据切割
	//合并完之后再将线进行打散
	QgsGeometry clipGeometry; // 用于切割的线
	while (fi.nextFeature(clipFeature))
	{
		QgsGeometry g = clipFeature.geometry();
		if (g.isEmpty())continue;

		if (clipGeometry.isEmpty())
			clipGeometry.fromWkb(g.asWkb());
		else clipGeometry.combine(g);
	}
	if (clipGeometry.isEmpty()) {
		this->showMessage(QString::fromLocal8Bit("切割图层中未查询到与目标几何图形重叠的几何图形...."));
		return -1;
	}
	QgsPolylineXY line = clipGeometry.asPolyline();
	QVector< QgsGeometry > newGeometries;
	QVector<QgsPointXY> topologyTestPoints;
	//!分割
	Qgis::GeometryOperationResult result = toClipGeometry.splitGeometry(line, newGeometries,false,topologyTestPoints);
	int n = 0;
	if (result == Qgis::GeometryOperationResult::Success) //分割成功，则处理
	{
		if (newGeometries.count() >= 1)
		{
			Q_FOREACH(QgsGeometry geo, newGeometries)
			{
				QgsFeature nwFeature;
				nwFeature.initAttributes(toClipFeature.attributeCount());
				nwFeature.setGeometry(geo);
				nwFeature.setAttributes(attrs);
				results.append(nwFeature);
				n++;
			}
		}
	}
	return n;
}
bool qgsVectorSplitAction::isSplitByPolygon()
{
	if (!mClipLayer)return false;

	QgsWkbTypes::Type lyrType = mClipLayer->wkbType();
	return lyrType == QgsWkbTypes::Polygon || lyrType == QgsWkbTypes::Polygon25D ||
		lyrType == QgsWkbTypes::MultiPolygon || QgsWkbTypes::MultiPolygon25D == lyrType;
}