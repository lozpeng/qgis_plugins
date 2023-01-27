#pragma once
#include "base/qgsribbonpluginuibase.h"

#include <qgisinterface.h>
/**
	野生动物数据处理与分析插件，ui管理
*/
class qgsAnimalPluginManager:public qgsRibbonPluginUIBase
{
public:
	qgsAnimalPluginManager(QgisInterface* qgsInterface);
	~qgsAnimalPluginManager();
	//overrides 
	void load();
	void unload();
private:
	void initUi();
	RibbonPage* mAnimalPage;

	void setDefaultPages();
};

