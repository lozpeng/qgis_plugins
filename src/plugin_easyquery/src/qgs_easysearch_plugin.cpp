#include "qgs_easysearch_plugin.h"

#include <qgisinterface.h>
#include <qgsgui.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>

#include <QAction>
#include <QToolBar>
#include <QTranslator>
#include <QDir>
#include <QCoreApplication>

#include "action/QgsLayerQueryAction.h"
#include "editor/qgsExplodeMultiPartsAction.h"
#include "editor/split/qgsVectorSplitAction.h"

static const QString sName = QString::fromLocal8Bit("图层快速查询插件");
static const QString sDescription = QString::fromLocal8Bit("图层快速查询插件");
static const QString sCategory = QString::fromLocal8Bit("vector");
static const QString sPluginVersion = QString::fromLocal8Bit("Version 1.1");
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/easy_query/images/image_search-24px.svg";

qgs_es_plugin::qgs_es_plugin(QgisInterface * theInterface) :
	mQGisIface(theInterface)
	, mQuerySettingAction(nullptr)
	, mExplodeAction(nullptr)
	, mSplitAction(nullptr)
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

qgs_es_plugin::~qgs_es_plugin()
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

int qgs_es_plugin::addActionFilter(QgsBaseActionFilter* actionFilter)
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
void qgs_es_plugin::initGui() {
	if (m_ActionsFilters.size() >= 1)return;

	mQuerySettingAction = new qgsLayerQueryAction();
	mQuerySettingAction->setOptData(1);
	mQuerySettingAction->setEnabled(false);
	this->addActionFilter(mQuerySettingAction);

	mExplodeAction = new qgsExplodeMultiPartsAction();
	mExplodeAction->setEnabled(false);
	mExplodeAction->setOptData(1);
	this->addActionFilter(mExplodeAction);
	mSplitAction = new qgsVectorSplitAction();
	mSplitAction->setOptData(1);
	mSplitAction->setEnabled(false);
	this->addActionFilter(mSplitAction);

	for (std::vector<QgsBaseActionFilter*>::const_iterator it = m_ActionsFilters.begin();
		it != m_ActionsFilters.end(); ++it)
	{
		mQGisIface->addVectorToolBarIcon((*it)->getAction());
	}
	//
	//监听地图图层切换事件。
	connect(mQGisIface->mapCanvas(), &QgsMapCanvas::currentLayerChanged, this, &qgs_es_plugin::on_currentLayerChanged);
}
//! unload the plugin
void qgs_es_plugin::unload() {
	if (m_ActionsFilters.size() >= 1)
	{
		for (std::vector<QgsBaseActionFilter*>::const_iterator it = m_ActionsFilters.begin();
			it != m_ActionsFilters.end(); ++it)
		{
			mQGisIface->removeVectorToolBarIcon((*it)->getAction());
		}
	}
}
//! 当前图层改变时，更新当前查询图层及其字段信息
void qgs_es_plugin::on_currentLayerChanged(QgsMapLayer* layer)
{
	//如果图层不是矢量图层则 不查询
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(layer);
	if (!vectorLayer)
	{
		if (mExplodeAction)
			mExplodeAction->setEnabled(false);
		if (mSplitAction)
			mSplitAction->setEnabled(false);
		return;
		//不是矢量图层则不处理，仍然使用上一个图层
	}
	if (mQuerySettingAction)
		mQuerySettingAction->setCurrentLayer(vectorLayer);
	if (mExplodeAction)
		mExplodeAction->setEnabled(vectorLayer->isEditable());
	if (mSplitAction)
		mSplitAction->setEnabled(vectorLayer->isEditable());
}
//! show the help document
void qgs_es_plugin::help() {

}


/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
 // Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin* classFactory(QgisInterface* qgisInterfacePointer)
{
	return new qgs_es_plugin(qgisInterfacePointer);
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