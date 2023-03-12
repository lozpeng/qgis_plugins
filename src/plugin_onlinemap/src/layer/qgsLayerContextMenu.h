#pragma once

#include <QObject>
#include <QMenu>
#include <QAction>

#include <qgslayertreeview.h>
#include <qgisinterface.h>

/**
	Í¼²ãÓÒ¼ü²Ëµ¥
*/
class qgsLayerContextMenu :public QObject
{
public:
	qgsLayerContextMenu(QgisInterface* qgsInterface);
	~qgsLayerContextMenu();
	void addLayerMenuAction(QAction* action, Qgis::LayerType lyrType);
	void removeAllActions();
private:
	bool mIsMenuAdded;
	QgisInterface* mQgsInterface;
	QAction* mLayerRepaireAction;
	QMap<QAction*, Qgis::LayerType> mLayerActions;
private slots:
	void on_LayerAdded(QgsMapLayer* l);
	void on_layer_repair_clicked(bool checked);
	void on_update_extent(bool checked);

};

