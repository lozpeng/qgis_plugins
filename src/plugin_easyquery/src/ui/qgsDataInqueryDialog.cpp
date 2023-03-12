#include "qgsDataInqueryDialog.h"

#include <QCombobox>

#include <qgisinterface.h>
#include <qgsproject.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>

qgsDataInqueryDialog::qgsDataInqueryDialog(QgisInterface* qgsInterface):
	mQgsInterface(qgsInterface)
{
	setupUi(this);

}


void qgsDataInqueryDialog::on_mComboTargetLyr_currentIndexChanged(int index)
{
	QString lyrId = mComboTargetLyr->currentIndex();


}
void qgsDataInqueryDialog::on_mComboSrcLyr_currentIndexChanged(int index)
{

}
QgsVectorLayer* vectorLayerByName(const QString& name)
{
	QMap<QString, QgsMapLayer*> mapLayers = QgsProject::instance()->mapLayers();
	QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

	for (; layer_it != mapLayers.end(); ++layer_it)
	{
		if (layer_it.value()->type() == Qgis::LayerType::Vector && layer_it.value()->name() == name)
		{
			return qobject_cast<QgsVectorLayer*>(layer_it.value());
		}
	}
	return 0;
}
QgsVectorLayer* vectorLayerById(const QString& id)
{
	QMap<QString, QgsMapLayer*> mapLayers = QgsProject::instance()->mapLayers();
	QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

	for (; layer_it != mapLayers.end(); ++layer_it)
	{
		if (layer_it.value()->type() == Qgis::LayerType::Vector && layer_it.value()->id() == id)
		{
			return qobject_cast<QgsVectorLayer*>(layer_it.value());
		}
	}
	return 0;
}
//！加载图层名称
void qgsDataInqueryDialog::loadLayerNames(QComboBox* combox,QString exLyrId)
{
	if (!mQgsInterface)return;
	combox->clear();

	QMap<QString, QgsMapLayer*> mapLayers =  QgsProject::instance()->mapLayers(true);
	if (mapLayers.size() <= 0)return;
	QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

	//!图层处理
	for (; layer_it != mapLayers.end(); ++layer_it)
	{
		if (layer_it.value()->type() == Qgis::LayerType::Vector)
		{
			QgsVectorLayer*  vectorLayer = qobject_cast<QgsVectorLayer*> (layer_it.value());
			if (vectorLayer) //当图层为矢量图层时才支持数据处理
			{
				QString lyrId = vectorLayer->id();
				//
				if (!exLyrId.isEmpty() && exLyrId.compare(lyrId, Qt::CaseInsensitive)==0)
					continue;
				combox->addItem(vectorLayer->name(), QVariant(lyrId));
			}
		}
	}
}