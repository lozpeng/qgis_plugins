#pragma once
#include <QObject>

#include <qgisinterface.h>

#include "base/QgsBaseActionFilter.h"

#include "layer/tilelayer/qTiledLayerDef.h"
using namespace geotile;

class qgsTiledLayerBaseAction:public QObject
{
	Q_OBJECT

public :
	qgsTiledLayerBaseAction(QgisInterface* interface);
protected:
	void addTiledLayer(qTileLayerDef lyrDef);

protected:
	QgisInterface* mQgsInterface;
};
