#include "qgsVectorLayerSelectBaseDialog.h"


#include <qevent.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsproject.h>

qgsVectorLayerSelectBaseDialog::qgsVectorLayerSelectBaseDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pbOk, &QPushButton::clicked,this,
				&qgsVectorLayerSelectBaseDialog::on_pbOk_clicked);

	connect(ui.pbCancel, &QPushButton::clicked, this,
				&qgsVectorLayerSelectBaseDialog::on_pbCancle_clicked);
}

qgsVectorLayerSelectBaseDialog::~qgsVectorLayerSelectBaseDialog()
{}

void qgsVectorLayerSelectBaseDialog::closeEvent(QCloseEvent* event)
{
	event->ignore();
	this->setVisible(false);
}
//! 获取用户在Combobox中的选中的图层编号
void qgsVectorLayerSelectBaseDialog::getSelectedLayerId(QString& str)
{
	int idx = ui.comboLayers->currentIndex();
	QVariant lyrId = ui.comboLayers->itemData(idx);
	str = lyrId.toString();
}
void qgsVectorLayerSelectBaseDialog::updateLayers()
{
	ui.comboLayers->clear();
	QgsVectorLayer* layer;
	QMap<QString, QgsMapLayer*> mapLayers = QgsProject::instance()->mapLayers();
	QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

	for (; layer_it != mapLayers.end(); ++layer_it)
	{
		if (layer_it.value()->type() == Qgis::LayerType::Vector)
		{
			layer = qobject_cast<QgsVectorLayer*> (layer_it.value());
			Qgis::WkbType lyrType = layer->wkbType();
			switch (layer->wkbType())
			{
				// polygon
			case Qgis::WkbType::Polygon:
			case Qgis::WkbType::Polygon25D:
			case Qgis::WkbType::MultiPolygon:
			case Qgis::WkbType::MultiPolygon25D:
			case Qgis::WkbType::LineString:
			case Qgis::WkbType::LineString25D:
			case Qgis::WkbType::MultiLineString:
			case Qgis::WkbType::MultiLineString25D:
			{
				QString lyrId = layer->id();
				QString lyrTxt = layer->name();
				ui.comboLayers->addItem(lyrTxt, QVariant(lyrId));
			}
			default:
				break;
			} // switch
		}
	}

	ui.comboLayers->addItem("--", QVariant(-1));
}


void qgsVectorLayerSelectBaseDialog::on_pbOk_clicked(bool checked) {
	this->setResult(1);
	this->accept();
}
void qgsVectorLayerSelectBaseDialog::on_pbCancle_clicked(bool checked)
{
	this->setResult(0);
	this->reject();
}
