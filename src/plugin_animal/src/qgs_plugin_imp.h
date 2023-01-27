#pragma once
#include <QObject>

//QGIS includes
#include "qgisplugin.h"
//
#include "base/QgsBaseActionFilter.h"
#include "base/qgsribbonpluginuibase.h"
class QgisInterface;

/**
	动物数据处理插件
**/
class qgs_plugin_imp : public QObject, public QgisPlugin
{
	Q_OBJECT
public:
	/**
	* Constructor for a plugin. The QgisInterface pointer is passed by
	* QGIS when it attempts to instantiate the plugin.
	* @param theInterface Pointer to the QgisInterface object.
	*/
	qgs_plugin_imp(QgisInterface* theInterface);
	//! Destructor
	virtual ~qgs_plugin_imp();

	//! init the gui
	virtual void initGui();
	//! unload the plugin
	void unload();
	//! show the help document
	void help();
private:
	QgisInterface* mQGisIface;
	qgsRibbonPluginUIBase* mUIManager;
};

