#pragma once

#include <QObject>
#include <qgisinterface.h>

#include "base/qgsribbonpluginuibase.h"

#include "add/qgsGeometryAddTool.h"
//!�༭���߹�����
class qgsEasyEditorManager : public qgsRibbonPluginUIBase
{
	Q_OBJECT

public:
	qgsEasyEditorManager(QgisInterface* qgsInterface,QObject *parent);
	~qgsEasyEditorManager();
	void loadUI();
	void unLoad();
private:
	qgsGeometryAddTool* mGeometryAddTool;
};
