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
			QString::fromLocal8Bit("Ҫ�طָ�"),
			QString::fromLocal8Bit("ʹ��ͼ��ָ�ѡ��Ҫ��"),
			":/easy_query/images/build-24px.svg"))
{


}
qgsVectorSplitAction::~qgsVectorSplitAction()
{
	if (mDialog)
		delete mDialog;
	mDialog = 0;
}
//����ͼ�㿪���༭֮��
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
//!����
int qgsVectorSplitAction::compute()
{
	if (!m_qgsInterface|| !mClipLayer)return -1;
	//1..��ȡ��ǰѡ�е�ͼ�㣬���ж��Ƿ��ڱ༭״̬
	//QgsVectorLayer* pCurrentLayer = m_qgsInterface->
	QgsMapCanvas* mapCanvas = m_qgsInterface->mapCanvas();
	auto layr = mapCanvas->currentLayer();
	QgsVectorLayer* vctLyr = qobject_cast<QgsVectorLayer*>(layr);
	if (!vctLyr || !vctLyr->isEditable())return -1;
	if (vctLyr->selectedFeatureCount() <= 0)return -1;
	QgsFeatureList selectFeatures = vctLyr->selectedFeatures();
	int count = selectFeatures.count();
	bool clipByPolygon = this->isSplitByPolygon();
	for (int i = 0; i < count; i++) //����ÿһ��Ҫ��
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
			vctLyr->deleteFeature(gFeature.id()); //ɾ��ԭ����Ҫ��
			vctLyr->editBuffer()->addFeatures(results);
		}
		else
			this->showMessage(QString::fromLocal8Bit("Ҫ�طָ�ʧ��:"+gFeature.id()));
	}
	return 0;
}
//!�ָ����Σ����������
int qgsVectorSplitAction::splitByPolygon(QgsFeature toClipFeature, QgsFeatureList& results) {
	if (!mClipLayer)return -1;
	QgsFeatureRequest fr;
	fr.setFilterRect(toClipFeature.geometry().boundingBox());
	QgsFeatureIterator fi = mClipLayer->getFeatures(fr); //������Ҫ�и��Ҫ�ز�ѯ�и�ͼ�㣬��ȡ�ཻ�ļ���ͼ��
	
	QgsAttributes attrs = toClipFeature.attributes();
	QgsGeometry toClipGeometry = toClipFeature.geometry();
	int count=0;
	QgsGeometry allGeometry; //��¼�ָ�����ĸ���С���֣����������ԭ����Ҫ�ؼ��㲻һ�µĵط������ڱ���ʣ�ಿ��
	QgsFeature clipFeature; //����һ���и�����
	while (fi.nextFeature(clipFeature))
	{
		QgsGeometry clipGeometry = clipFeature.geometry();
		if (!clipGeometry.intersects(toClipGeometry))continue;

		QgsGeometry nGeometry = clipGeometry.intersection(toClipGeometry);//�����������ε��ཻ���֣�
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
	if (!allGeometry.isEmpty()) //�����Ϊ��
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
//!�ָ���
int qgsVectorSplitAction::splitByPolyline(QgsFeature toClipFeature, QgsFeatureList& results) {

	if (!mClipLayer)return -1;
	QgsFeatureRequest fr;
	fr.setFilterRect(toClipFeature.geometry().boundingBox());
	QgsFeatureIterator fi = mClipLayer->getFeatures(fr); //������Ҫ�и��Ҫ�ز�ѯ�и�ͼ�㣬��ȡ�ཻ�ļ���ͼ��

	QgsAttributes attrs = toClipFeature.attributes();
	QgsGeometry toClipGeometry = toClipFeature.geometry();
	int count = 0;
	QgsGeometry allGeometry; //��¼�ָ�����ĸ���С���֣����������ԭ����Ҫ�ؼ��㲻һ�µĵط������ڱ���ʣ�ಿ��
	QgsFeature clipFeature; //����һ���и���
	//���Ƚ��ߺϲ�������С�߶β��������λ�Ŀ�꼸�ζ�����ཻ�����ܽ��������и�
	//�ϲ���֮���ٽ��߽��д�ɢ
	QgsGeometry clipGeometry; // �����и����
	while (fi.nextFeature(clipFeature))
	{
		QgsGeometry g = clipFeature.geometry();
		if (g.isEmpty())continue;

		if (clipGeometry.isEmpty())
			clipGeometry.fromWkb(g.asWkb());
		else clipGeometry.combine(g);
	}
	if (clipGeometry.isEmpty()) {
		this->showMessage(QString::fromLocal8Bit("�и�ͼ����δ��ѯ����Ŀ�꼸��ͼ���ص��ļ���ͼ��...."));
		return -1;
	}
	QgsPolylineXY line = clipGeometry.asPolyline();
	QVector< QgsGeometry > newGeometries;
	QVector<QgsPointXY> topologyTestPoints;
	//!�ָ�
	Qgis::GeometryOperationResult result = toClipGeometry.splitGeometry(line, newGeometries,false,topologyTestPoints);
	int n = 0;
	if (result == Qgis::GeometryOperationResult::Success) //�ָ�ɹ�������
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