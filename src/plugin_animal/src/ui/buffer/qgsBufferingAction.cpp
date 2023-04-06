#include "qgsBufferingAction.h"

#include <QVector>

#include <qgisinterface.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <QgsVectorLayerEditBuffer.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsproject.h>
#include "utils/qgsGeoUtils.h"
#include "qgsBufferingDialog.h"

qgsBufferingAction::qgsBufferingAction(QgisInterface* qgsInterface)
	: QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mSamplingEvalAction",
			QString::fromLocal8Bit("缓冲区生成"),
			QString::fromLocal8Bit("根据要素图形生成缓冲区"),
			":/easy_query/images/construction-24px.svg"))
{
	mQgsInterface = qgsInterface;
	mParamDialog = nullptr;

	if (mQgsInterface)
	{
		connect(mQgsInterface->mapCanvas(),
			&QgsMapCanvas::currentLayerChanged,
			this, &qgsBufferingAction::on_currentlayer_changed);

		on_currentlayer_changed(mQgsInterface->mapCanvas()->currentLayer());
	}
}
qgsBufferingAction::~qgsBufferingAction()
{
	if (mParamDialog)
		delete mParamDialog;
	mParamDialog = nullptr;
}
//!计算
int qgsBufferingAction::compute()
{
	QgsMapLayer* lyr = mQgsInterface->mapCanvas()->currentLayer();
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(lyr);
	if (!vectorLayer)return  -1;
	QString epsgId = vectorLayer->crs().authid();
	QString lyrName =QString("%1_buffer").arg(lyr->name());
	QgsVectorLayer* memLayer = qgsGeoUtils::createMemLayer(Qgis::WkbType::MultiPolygon, epsgId, lyrName, "name");
	if (!memLayer)return -1;
	memLayer->startEditing();//启动图层编辑
	int sCount = vectorLayer->selectedFeatures().count();
	if (mParams.bSelected && sCount <= 0) return -1;// 如果使用选中的要素
	bool useField = true;
	if (QString("--").compare(mParams.fieldName) == 0)
		useField = false;

	if (!mParams.bSelected)
		vectorLayer->selectAll();
	int step = 0;
	Qgis::EndCapStyle endCap = Qgis::EndCapStyle(mParams.endCapStyle);
	Qgis::JoinStyle  joinStyle = Qgis::JoinStyle(mParams.joinStyle);
	QgsFeatureList buffFeatures;
	//！处理数据 ，这个方法会不会导致内存超容量？
	{
		QgsFeatureList features = vectorLayer->selectedFeatures();
		QVector<QgsGeometry> geos;
		Q_FOREACH(QgsFeature f , features)
		{
			double dist = mParams.dist;
			if (useField)
				dist = f.attribute(mParams.fieldName).toDouble();
			QgsGeometry geo = f.geometry();
			QgsGeometry outGeo = geo.buffer(dist, mParams.segeMents,
									endCap, joinStyle, mParams.miterLimit);

			if (outGeo.isEmpty() || outGeo.isNull())continue;
			if (mParams.bDissolve)
				geos.append(outGeo);
			else
			{
				QgsFeature feature;
				feature.setGeometry(outGeo);
				feature.initAttributes(memLayer->fields().count());
				feature.setValid(true);
				step++;
				feature.setId(QgsFeatureId(step));
				feature.setAttribute("id", step);
				buffFeatures.append(feature);
			}
		}
		if (geos.size() >= 2)
		{
			QgsGeometry finalGeometry = QgsGeometry::unaryUnion(geos);
			if (!finalGeometry.isEmpty() && !finalGeometry.isNull())
			{
				step++;
				QgsFeature feature;
				feature.initAttributes(memLayer->fields().count());
				feature.setId(QgsFeatureId(step));
				feature.setAttribute("id", step);
				feature.setGeometry(finalGeometry);
				buffFeatures.append(feature);
			}
		}
	}
	if (buffFeatures.count() >= 1)
	{
		memLayer->editBuffer()->addFeatures(buffFeatures);
		memLayer->commitChanges();
	}
	memLayer->endEditCommand();
	memLayer->updateExtents();
	QgsProject::instance()->addMapLayer(memLayer);
	return 1;
}
	//inherited from BaseFilter
int qgsBufferingAction::openInputDialog()
{
	QgsMapLayer* lyr = mQgsInterface->mapCanvas()->currentLayer();
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(lyr);
	if (!vectorLayer)return  -1;

	QStringList fieldLists;
	QgsFields fields = vectorLayer->fields();
	Q_FOREACH(QgsField fld, fields)
		fieldLists<< fld.name();

	if (!mParamDialog)
		mParamDialog = new qgsBufferingDialog(0, fieldLists);
	if (mParamDialog->exec()) //如果
	{
		mParamDialog->hide();
		return 1;
	}
	return 0;
}
void qgsBufferingAction::getParametersFromDialog()
{
	if (!mParamDialog)return;
	mParamDialog->getParams(mParams);
}
/// <summary>
/// 
/// </summary>
/// <param name="lyr"></param>
void qgsBufferingAction::on_currentlayer_changed(QgsMapLayer* lyr)
{
	if (!lyr)this->setEnabled(false);
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(lyr);
	this->setEnabled(vectorLayer);
}