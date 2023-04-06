#include "qgsLayerUtils.h"

#include <qgisinterface.h>
#include <qgsproject.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsfields.h>

/// <summary>
	/// ��ȡʸ��ͼ����ݱ��
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
QgsVectorLayer* qgsLayerUtils::getVectorLayerById(QString id)
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
/// <summary>
/// ����ͼ�����ƻ�ȡʸ��ͼ��
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
QgsVectorLayer* qgsLayerUtils::getVectorLayerByName(QString name) {
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
/// <summary>
/// ��ȡ���е�ʸ��ͼ��������Combobo��
/// </summary>
/// <param name="combox"></param>
/// <param name="exLyrId"></param>
void qgsLayerUtils::loadVectorLayerNames(QComboBox* combox, QString exLyrId) {
	if (!combox)
		return;

	combox->clear();

	QMap<QString, QgsMapLayer*> mapLayers = QgsProject::instance()->mapLayers(true);
	if (mapLayers.size() <= 0)return;
	QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

	//!ͼ�㴦��
	for (; layer_it != mapLayers.end(); ++layer_it)
	{
		if (layer_it.value()->type() == Qgis::LayerType::Vector)
		{
			QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*> (layer_it.value());
			if (vectorLayer) //��ͼ��Ϊʸ��ͼ��ʱ��֧�����ݴ���
			{
				QString lyrId = vectorLayer->id();
				//
				if (!exLyrId.isEmpty() && exLyrId.compare(lyrId, Qt::CaseInsensitive) == 0)
					continue;
				combox->addItem(vectorLayer->name(), QVariant(lyrId));
			}
		}
	}
}
/// <summary>
/// ��ȡͼ����ֶ�����
/// </summary>
/// <param name="combo"></param>
/// <param name="lyrIdx"></param>
void qgsLayerUtils::loadLayerFieldsNames(QComboBox* combo, QString lyrIdx)
{
	if (!combo)return;
	QgsVectorLayer* tLyr = getVectorLayerById(lyrIdx);
	if (!tLyr)return;
	combo->clear();

	QgsFields fields  = tLyr->fields();
	if (fields.count() <= 0)return;
	Q_FOREACH(QgsField f, fields)
	{
		QString fName = f.name();
		combo->addItem(fName);
	}
}