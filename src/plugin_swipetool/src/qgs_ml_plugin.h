#ifndef QGS_ML_PLUGIN_H_
#define QGS_ML_PLUGIN_H_

#include <QObject>

//QGIS includes
#include "qgisplugin.h"

//
#include "base/QgsBaseActionFilter.h"
#include "extools\vqgsmapswiper.h"

class QAction;
class QToolBar;

class QgisInterface;
class qgs_ml_plugin : public QObject, public QgisPlugin
{
	Q_OBJECT
public:
	/**
	* Constructor for a plugin. The QgisInterface pointer is passed by
	* QGIS when it attempts to instantiate the plugin.
	* @param theInterface Pointer to the QgisInterface object.
	*/
	qgs_ml_plugin(QgisInterface * theInterface);
	//! Destructor
	virtual ~qgs_ml_plugin();

	//! init the gui
	virtual void initGui();
	//! unload the plugin
	void unload();
	//! show the help document
	void help();
public slots:


private:
	//! Pointer to the QGIS interface object
	QgisInterface *mQGisIface;
	vQgsMapSwiper* mMapSwiper;
	std::vector<QgsBaseActionFilter*> m_ActionsFilters;

	int addActionFilter(QgsBaseActionFilter* actionFilter);

};
#endif
