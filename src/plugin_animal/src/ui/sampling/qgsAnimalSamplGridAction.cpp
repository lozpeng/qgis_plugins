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
			QString::fromLocal8Bit("生成抽样网格"),
			QString::fromLocal8Bit("根据给定的多边形范围和距离生成抽样网格"),
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
//!计算
int qgsAnimalSamplGridAction::compute()
{
	if (mParams.distance <= 0)return -1;
	//计算并生成公里网格
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
		//QMessageBox::information(mQgsInterface->mainWindow(), QString::fromLocal8Bit("提示"), 
		//	QString::fromLocal8Bit("请选择要布设的范围"));
		return -1;
	}
	QString srid = vectorLayer->crs().authid();
	if (srid.startsWith("EPSG:"))
		srid = srid.replace("EPSG:", "");
	int epsgCode = srid.toInt();
	 //!数据都先暂时存放在内存中
	QString fields = "field=minx:double&field=maxx:double&field=miny:double&field=maxy:double";
	QgsVectorLayer* gridLayer = qgsGeoUtils::createMemLayer(QgsWkbTypes::Polygon, srid, QString::fromLocal8Bit("抽样网格"), fields);
	if (!gridLayer)return -1;
	if (gridLayer)
		QgsProject::instance()->addMapLayer(gridLayer);

	QgsFeatureIterator qgsFeatureIt = vectorLayer->getSelectedFeatures();
	QgsFeature qgsFeature;
	QgsGeometry qgsGeom;
	while (qgsFeatureIt.nextFeature(qgsFeature)) //取得几何范围，如果多个则先融合然后计算
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
//！打开参数设置对话框
int qgsAnimalSamplGridAction::openInputDialog() {
	if (!mParamDialog)
		mParamDialog = new qgsSamplingParamDialog();
	if (mParamDialog->exec()) //如果
	{
		mParamDialog->hide();
		return 1;
	}
	return 0;
}
//！从参数设置对话框中获取参数，
void qgsAnimalSamplGridAction::getParametersFromDialog()
{
	this->mParamDialog->getParams(this->mParams);
}
