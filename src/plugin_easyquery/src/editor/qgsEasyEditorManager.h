#pragma once

#include <QObject>
#include <qgisinterface.h>

#include "base/qgsribbonpluginuibase.h"
//!�༭���߹�����
class qgsEasyEditorManager  : public QObject
#ifdef WITH_QTITAN
	,public qgsRibbonPluginUIBase
#endif
{
	Q_OBJECT

public:
	qgsEasyEditorManager(QgisInterface* qgsInterface,QObject *parent);
	~qgsEasyEditorManager();
	void loadUI();
	void unLoad();
};
