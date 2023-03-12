#pragma once
#include "base/qgsribbonpluginuibase.h"

#include <qgisinterface.h>

#include "layer/tilelayer/qTiledLayerDef.h"
using namespace geotile;
/**
	野生动物数据处理与分析插件，ui管理
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
	RibbonPage* mRibbonPage; //主要操作的RibbonPage
};

