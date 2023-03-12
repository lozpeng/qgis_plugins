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
			QString::fromLocal8Bit("����ǿ������"),
			QString::fromLocal8Bit("���ݳ������ý�����г���ǿ�ȼ��㣬�����Ƿ�����Ҫ��"),
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
//!����
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
