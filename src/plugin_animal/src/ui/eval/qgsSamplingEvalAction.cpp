#include "qgsSamplingEvalAction.h"

#include <QMessageBox>

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgis.h>
#include <qgslinestring.h>
#include <qgsfeature.h>
#include <qgspolygon.h>
#include <qgsvectorlayereditbuffer.h>

qgsSamplingEvalAction::qgsSamplingEvalAction(QgisInterface* qgsInterface)
	: QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mSamplingEvalAction",
			QString::fromLocal8Bit("抽样强度评估"),
			QString::fromLocal8Bit("根据抽样设置结果进行抽样强度计算，计算是否满足要求"),
			":/easy_query/images/build-24px.svg"))
{
	//mParamDialog = nullptr;
	mQgsInterface = qgsInterface;

	if (mQgsInterface)
	{
		connect(mQgsInterface->mapCanvas(),
			&QgsMapCanvas::currentLayerChanged,
			this, &qgsSamplingEvalAction::on_currentlayer_changed);

		on_currentlayer_changed(mQgsInterface->mapCanvas()->currentLayer());
	}
}
qgsSamplingEvalAction::~qgsSamplingEvalAction()
{
	//if (mParamDialog)
	//	delete mParamDialog;
	//mParamDialog = nullptr;

}
void qgsSamplingEvalAction::on_currentlayer_changed(QgsMapLayer* lyr)
{
	if (!lyr)this->setEnabled(false);
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(lyr);
	this->setEnabled(vectorLayer);
}
//!计算
int qgsSamplingEvalAction::compute()
{

	return -1;
}

//inherited from BaseFilter
int qgsSamplingEvalAction::openInputDialog()
{


	return -1;
}
void qgsSamplingEvalAction::getParametersFromDialog()
{



}
