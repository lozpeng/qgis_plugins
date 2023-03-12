#include "qgsAddTDTLayerAction.h"

#include <QMessageBox>

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgis.h>
#include <qgslinestring.h>
#include <qgsfeature.h>
#include <qgspolygon.h>
#include <qgsvectorlayereditbuffer.h>
#include  <qgsapplication.h>
#include <qgisinterface.h>

#include "layer/tilelayer/qTiledLayerDef.h"
#include "layer/tilelayer/qgsCustomizedTileLayer.h"
using namespace geotile;

qgsTiledLayerBaseAction::qgsTiledLayerBaseAction(QgisInterface* interface):
	mQgsInterface(interface)
{
}
void qgsTiledLayerBaseAction::addTiledLayer(qTileLayerDef lyrDef)
{
	qPluginTileLayerType* plugin = new qPluginTileLayerType(mQgsInterface, lyrDef);
	QgsApplication::pluginLayerRegistry()->addPluginLayerType(plugin);
	qgsCustomizedTileLayer* lyr = (qgsCustomizedTileLayer*)plugin->createLayer();

	mQgsInterface->mapCanvas()->setExtent(lyr->extent());
	//QList<QgsMapLayer*> layers = mQgsInterface->mapCanvas()->layers();
	//layers.append(lyr);
	//mQgsInterface->mapCanvas()->setLayers(layers);
	//mQgsInterface->mapCanvas()->refresh();
	QgsProject::instance()->addMapLayer(lyr);
}

