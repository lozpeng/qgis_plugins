#pragma once
#include "base/qgsribbonpluginuibase.h"

#include <qgisinterface.h>
/**
	Ұ���������ݴ�������������ui����
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

