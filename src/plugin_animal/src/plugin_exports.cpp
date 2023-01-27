
/**
插件元信息导出类
@Date 2023/1/1
@author lozpeng
@dep  wildlife protect and monitoring center of NFGA
*/

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
#include <qgisinterface.h>
#include <qgisplugin.h>
#include "qgs_plugin_imp.h"

static const QString sName = QString::fromLocal8Bit("野生动物监测数据处理");
static const QString sDescription = QString::fromLocal8Bit("野生动物监测数据处理,对栖息地范围数据进行编辑，监测抽样、抽样强度计算、监测结果计算等");
static const QString sCategory = QString::fromLocal8Bit("vector");
static const QString sPluginVersion = QString::fromLocal8Bit("Version 1.1");
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/wildlife/animals/panda.png";

 // Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin* classFactory(QgisInterface* qgisInterfacePointer)
{
	return new qgs_plugin_imp(qgisInterfacePointer);
}
// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN const QString* name()
{
	return &sName;
}

// Return the description
QGISEXTERN const QString* description()
{
	return &sDescription;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
	return sPluginType;
}

// Return the category
QGISEXTERN const QString* category()
{
	return &sCategory;
}

// Return the version number for the plugin
QGISEXTERN const QString* version()
{
	return &sPluginVersion;
}

QGISEXTERN const QString* icon()
{
	return &sPluginIcon;
}

// Delete ourself
QGISEXTERN void unload(QgisPlugin* pluginPointer)
{
	delete pluginPointer;
}