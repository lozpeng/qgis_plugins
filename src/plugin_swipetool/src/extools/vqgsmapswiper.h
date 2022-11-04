#pragma once

#include <QObject>
#include <QComboBox>

#include <qgisinterface.h>
#include <qgsmaptool.h>

#include "vQgsLayerSwiperTool.h"
#include "vQgsXRayMagnifyTool.h"
//
class vQgsMapSwiper : public QObject
{
	Q_OBJECT

public:
	vQgsMapSwiper(QgisInterface * qgsInterface);
	~vQgsMapSwiper();
	void init();
	void unload();
private :
	QgisInterface * mQgsInterface;
	QgsMapTool* mPrevTool;

	QAction* mAction;
	vQgsLayerSwiperTool* mSwiperTool;


	QAction* mXAction;
	vQgsXRayMagnifyTool* mXSwiperTool;
private slots:
	void on_ation_triggered(bool checked);
	void on_xation_triggered(bool checked);
	void on_mapToolSet(QgsMapTool *newTool, QgsMapTool *oldTool);
};
