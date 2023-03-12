#include "qgsLayerContextMenu.h"
#include <QMenu>
#include <QAction>

#include <qgslayertreeview.h>
#include <qgslayertree.h>
#include <qgsvectorlayer.h>
#include <qgslayertreenode.h>
#include <qgsproject.h>
#include <qgsdataprovider.h>

//!初始化传入interface参数即可，该类实现自管理
qgsLayerContextMenu::qgsLayerContextMenu(QgisInterface* qgsInterface)
	:mQgsInterface(qgsInterface)
{
	mIsMenuAdded = false;
	connect(QgsProject::instance(), &QgsProject::layerWasAdded, this, &qgsLayerContextMenu::on_LayerAdded);

	mLayerRepaireAction = new QAction(QString::fromLocal8Bit("图层修复..."), this);

	connect(mLayerRepaireAction, SIGNAL(triggered), this, SLOT(on_repair_triggered));

	addLayerMenuAction(mLayerRepaireAction, Qgis::LayerType::Vector);

	QAction* lyrRepair = new QAction(QString::fromLocal8Bit("矢量图层检查"), this);

	connect(lyrRepair, SIGNAL(triggered), this, SLOT(on_update_extent));

	addLayerMenuAction(lyrRepair, Qgis::LayerType::Vector);
}

qgsLayerContextMenu::~qgsLayerContextMenu()
{
	Q_FOREACH(QAction * act, mLayerActions.keys())
	{
		if (act->parent() == this)
		{
			mLayerActions.remove(act);
			delete act;
		}
	}
	mLayerRepaireAction = 0;

}
void qgsLayerContextMenu::removeAllActions()
{
	Q_FOREACH(QAction * act, mLayerActions.keys())
	{
		mQgsInterface->removeCustomActionForLayerType(act);
	}
	mLayerActions.clear();

	disconnect(QgsProject::instance(), &QgsProject::layerWasAdded,
		this, &qgsLayerContextMenu::on_LayerAdded);
}

void qgsLayerContextMenu::addLayerMenuAction(QAction* action, Qgis::LayerType lyrType)
{
	//!创建图层修复工具按钮
	if (mQgsInterface)
		mQgsInterface->addCustomActionForLayerType(action, "", lyrType, false);
	mLayerActions.insert(action, lyrType);
	QMap<QString, QgsMapLayer*> layers = QgsProject::instance()->mapLayers();
	if (layers.count() >= 1)
	{
		Q_FOREACH(QgsMapLayer * l, layers.values())
		{
			if (l->type() == lyrType)
			{
				auto p = l->dataProvider();
				QString pname = p->name();
				if (QString("postgres,spatialite,geopackage").contains(pname.toLower()))
					mQgsInterface->addCustomActionForLayer(action, l);
			}
		}
	}
}

void qgsLayerContextMenu::on_layer_repair_clicked(bool checked)
{

}

void qgsLayerContextMenu::on_update_extent(bool checked)
{

}

void qgsLayerContextMenu::on_LayerAdded(QgsMapLayer* l)
{
	Q_FOREACH(QAction * act, mLayerActions.keys())
	{
		Qgis::LayerType actLyrType = mLayerActions.value(act);
		Qgis::LayerType ltype = l->type();
		if (actLyrType != ltype)continue;
		auto p = l->dataProvider();
		QString pname = p->name();
		if (QString("postgres,spatialite,geopackage").contains(pname.toLower()))
			mQgsInterface->addCustomActionForLayer(act, l);
	}
}