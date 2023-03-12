#pragma once
#include "base/qgsribbonpluginuibase.h"

#include <qgisinterface.h>

#include "layer/tilelayer/qTiledLayerDef.h"
using namespace geotile;
/**
	Ұ���������ݴ�������������ui����
*/
class qgsPluginUIManager :public qgsRibbonPluginUIBase
{
public:
	qgsPluginUIManager(QgisInterface* qgsInterface);
	~qgsPluginUIManager();
	//overrides 
	void load();
	void unload();
private:
	void initUi();
	RibbonPage* mRibbonPage; //��Ҫ������RibbonPage
};

