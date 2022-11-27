#pragma once

#include <QObject>
#include <qgisinterface.h>

#include "base/qgsribbonpluginuibase.h"
//!编辑工具管理类
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
