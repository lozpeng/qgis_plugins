#include "qgsAnimalSamplGridAction.h"

#include <QMessageBox>

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgis.h>
#include <qgslinestring.h>
#include <qgsfeature.h>
#include <qgspolygon.h>
#include <qgsvectorlayereditbuffer.h>


#include "utils/qgsGeoUtils.h"
#include "utils/qgsGridUtil.h"

qgsAnimalSamplGridAction::qgsAnimalSamplGridAction(QgisInterface* qgsInterface)
	: QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mAnimalGridsGenAction",
			QString::fromLocal8Bit("���ɳ�������"),
			QString::fromLocal8Bit("���ݸ����Ķ���η�Χ�;������ɳ�������"),
			":/easy_query/images/build-24px.svg"))
{
	mParamDialog = nullptr;
	mQgsInterface = qgsInterface;

	if (mQgsInterface)
	{
		connect(mQgsInterface->mapCanvas(), 
				&QgsMapCanvas::currentLayerChanged, 
				this, &qgsAnimalSamplGridAction::on_currentlayer_changed);

		on_currentlayer_changed(mQgsInterface->mapCanvas()->currentLayer());
	}
}
qgsAnimalSamplGridAction::~qgsAnimalSamplGridAction()
{
	if (mParamDialog)
		delete mParamDialog;
	mParamDialog = nullptr;

}
void qgsAnimalSamplGridAction::on_currentlayer_changed(QgsMapLayer* lyr)
{
	if (!lyr)this->setEnabled(false);
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(lyr);
	this->setEnabled(vectorLayer);
}
//!����
int qgsAnimalSamplGridAction::compute()
{
	if (mParams.distance <= 0)return -1;
	//���㲢���ɹ�������
	if (!this->mParams.isParamsInited)
		return -1;


	QgsMapLayer* lyr = mQgsInterface->mapCanvas()->currentLayer();
	if (!lyr)return -1;
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(lyr);
	if (!vectorLayer)
		return -1;
	int count = vectorLayer->selectedFeatureCount();
	if (count <= 0)
	{
		//QMessageBox::information(mQgsInterface->mainWindow(), QString::fromLocal8Bit("��ʾ"), 
		//	QString::fromLocal8Bit("��ѡ��Ҫ����ķ�Χ"));
		return -1;
	}
	QString srid = vectorLayer->crs().authid();
	if (srid.startsWith("EPSG:"))
		srid = srid.replace("EPSG:", "");
	int epsgCode = srid.toInt();
	 //!���ݶ�����ʱ������ڴ���
	QString fields = "field=minx:double&field=maxx:double&field=miny:double&field=maxy:double";
	QgsVectorLayer* gridLayer = qgsGeoUtils::createMemLayer(QgsWkbTypes::Polygon, srid, QString::fromLocal8Bit("��������"), fields);
	if (!gridLayer)return -1;
	if (gridLayer)
		QgsProject::instance()->addMapLayer(gridLayer);

	QgsFeatureIterator qgsFeatureIt = vectorLayer->getSelectedFeatures();
	QgsFeature qgsFeature;
	QgsGeometry qgsGeom;
	while (qgsFeatureIt.nextFeature(qgsFeature)) //ȡ�ü��η�Χ�������������ں�Ȼ�����
	{
		if (qgsFeature.isValid())
		{
			QgsGeometry featureGeom = qgsFeature.geometry();
			if (qgsGeom.isEmpty())
				qgsGeom.fromWkb(featureGeom.asWkb());
			else
			{
				QgsGeometry tmp;
				tmp.fromWkb(featureGeom.asWkb());
				qgsGeom = qgsGeom.combine(tmp);
			}
		}
	}
	if (qgsGeom.isEmpty())return  -1;

	return qgsGridUtil::genGrids(qgsGeom, epsgCode,mParams.distance, gridLayer, mParams.gridType);
}

//inherited from BaseFilter
//���򿪲������öԻ���
int qgsAnimalSamplGridAction::openInputDialog() {
	if (!mParamDialog)
		mParamDialog = new qgsSamplingParamDialog();
	if (mParamDialog->exec()) //���
	{
		mParamDialog->hide();
		return 1;
	}
	return 0;
}
//���Ӳ������öԻ����л�ȡ������
void qgsAnimalSamplGridAction::getParametersFromDialog()
{
	this->mParamDialog->getParams(this->mParams);
}
