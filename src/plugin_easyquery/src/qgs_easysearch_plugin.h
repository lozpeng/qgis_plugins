#ifndef QGS_ML_PLUGIN_H_
#define QGS_ML_PLUGIN_H_

#include <QObject>

//QGIS includes
#include "qgisplugin.h"

//
#include "base/QgsBaseActionFilter.h"

#include "action/QgsLayerQueryAction.h"
#include "editor/split/qgsVectorSplitAction.h"
class QAction;
class QToolBar;
class QgsMapLayer;

class QgisInterface;
class qgsExplodeMultiPartsAction;
class qgs_es_plugin : public QObject, public QgisPlugin
{
	Q_OBJECT
public:
	/**
	* Constructor for a plugin. The QgisInterface pointer is passed by
	* QGIS when it attempts to instantiate the plugin.
	* @param theInterface Pointer to the QgisInterface object.
	*/
	qgs_es_plugin(QgisInterface * theInterface);
	//! Destructor
	virtual ~qgs_es_plugin();

	//! init the gui
	virtual void initGui();
	//! unload the plugin
	void unload();
	//! show the help document
	void help();
public slots:
	//当当前图层发生变化时
	void on_currentLayerChanged(QgsMapLayer* layer);

private:
	//! Pointer to the QGIS interface object
	QgisInterface *mQGisIface;
	std::vector<QgsBaseActionFilter*> m_ActionsFilters;
	int addActionFilter(QgsBaseActionFilter* actionFilter);

	qgsLayerQueryAction* mQuerySettingAction;
	qgsExplodeMultiPartsAction* mExplodeAction;
	qgsVectorSplitAction* mSplitAction;
};
#endif
