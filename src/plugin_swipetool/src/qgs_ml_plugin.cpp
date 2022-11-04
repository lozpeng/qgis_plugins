#include "qgs_ml_plugin.h"

#include <qgisinterface.h>
#include <qgsgui.h>

#include <QAction>
#include <QToolBar>
#include <QTranslator>
#include <QDir>
#include <QCoreApplication>

#include "extools\vqgsmapswiper.h"

static const QString sName = QString::fromLocal8Bit("图层SwipeTool插件");
static const QString sDescription = QString::fromLocal8Bit("图层SwipeTool插件");
static const QString sCategory = QString::fromLocal8Bit("vector");
static const QString sPluginVersion = QString::fromLocal8Bit("Version 1.1");
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/qgsml/images/image_search-24px.svg";

qgs_ml_plugin::qgs_ml_plugin(QgisInterface * theInterface) :
	mQGisIface(theInterface), mMapSwiper(0)
{
	/**
	QTranslator*  translator = new QTranslator();
	QString my_qm_dir = QCoreApplication::applicationDirPath();
	QDir dir(my_qm_dir);
	if (dir.cdUp())
	my_qm_dir = dir.absolutePath().append("/i18n/");

	translator->load("silen_data_plugin_zh.qm", my_qm_dir);   //cn是翻译好的qm文件
	QCoreApplication::installTranslator(translator);
	**/
}


qgs_ml_plugin::~qgs_ml_plugin()
{
	if (m_ActionsFilters.size() >= 1)
	{
		Q_FOREACH(QgsBaseActionFilter* filter, m_ActionsFilters)
		{
			delete filter;
		}
		m_ActionsFilters.clear();
	}
}

int qgs_ml_plugin::addActionFilter(QgsBaseActionFilter* actionFilter)
{
	assert(actionFilter);
	actionFilter->setQgsInterface(mQGisIface);

	QAction* action = actionFilter->getAction();
	if (!action)
		return 0;

	//filter already inserted?
	if (std::find(m_ActionsFilters.begin(), m_ActionsFilters.end(), actionFilter)
		!= m_ActionsFilters.end())
		return 0;

	m_ActionsFilters.push_back(actionFilter);
	return 1;
}
//! init the gui
void qgs_ml_plugin::initGui() {
	if (m_ActionsFilters.size() >= 1)return;

	//addActionFilter(new qgsRasterClassifierFilter());
	for (std::vector<QgsBaseActionFilter*>::const_iterator it = m_ActionsFilters.begin();
		it != m_ActionsFilters.end(); ++it)
	{
		mQGisIface->addVectorToolBarIcon((*it)->getAction());
	}
	mMapSwiper = new vQgsMapSwiper(mQGisIface);
	mMapSwiper->init();
}
//! unload the plugin
void qgs_ml_plugin::unload() {
	if (m_ActionsFilters.size() >= 1)
	{
		for (std::vector<QgsBaseActionFilter*>::const_iterator it = m_ActionsFilters.begin();
			it != m_ActionsFilters.end(); ++it)
		{
			mQGisIface->removeVectorToolBarIcon((*it)->getAction());
		}
	}
	if (mMapSwiper)
		mMapSwiper->unload();
}

//! show the help document
void qgs_ml_plugin::help() {

}


/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
 // Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin* classFactory(QgisInterface* qgisInterfacePointer)
{
	return new qgs_ml_plugin(qgisInterfacePointer);
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