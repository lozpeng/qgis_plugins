#include "qgsAddTDTLayerAction.h"

#include <QMessageBox>

#include <qgsmapcanvas.h>
#include <qgis.h>
#include <qgisinterface.h>


#include "layer/tilelayer/qTiledLayerDef.h"

qgsAddTDTLayerAction::qgsAddTDTLayerAction(QgisInterface* qgsInterface)
	: QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mTiandituSatelliate",
			QString::fromLocal8Bit("���ͼӰ��"),
			QString::fromLocal8Bit("������ͼӰ���ͼ����"),
			":/easy_query/images/build-24px.svg")),
	qgsTiledLayerBaseAction(qgsInterface)
{
}
qgsAddTDTLayerAction::~qgsAddTDTLayerAction()
{

}

//!����
int qgsAddTDTLayerAction::compute()
{
	//QString url = QString("https://t%2.tianditu.gov.cn/img_c/wmts?tk=%1&SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=img&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL={x}&TILEROW={y}&TILEMATRIX={z}")
	//	.arg("1a8cfe275d0b08802de0acca96f5a34c")
	//	.arg("%1");
	QString url = QString("http://t%2.tianditu.com/DataServer?T=img_c&tk=%1")
		.arg("1a8cfe275d0b08802de0acca96f5a34c")
		.arg("%1");
	
	//!������ͼͼ��
	qTileLayerDef def;
	def.title = QString::fromLocal8Bit("���ͼӰ��-��γ��");
	def.type = qTileSourceType::tTdtMapServer;
	def.servers = 8;
	//def.prjTxt = "+proj=longlat +ellps=GRS80 +no_defs ";
	def.url = url;
	//!
	this->addTiledLayer(def);
	return  0;
}
