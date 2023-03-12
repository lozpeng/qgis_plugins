#pragma once
#include <qgisinterface.h>

#include "base/QgsBaseActionFilter.h"

#include "qgsTiledLayerBaseAction.h"
/**
*/
class qgsAddTDTLayerAction :public QgsBaseActionFilter, qgsTiledLayerBaseAction
{
	Q_OBJECT

public:
	qgsAddTDTLayerAction(QgisInterface* qgsInterface);
	virtual ~qgsAddTDTLayerAction();
	//!º∆À„
	int compute();
protected:
	//inherited from BaseFilter
	
};