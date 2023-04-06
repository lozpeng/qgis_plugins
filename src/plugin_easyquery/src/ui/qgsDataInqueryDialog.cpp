#include "qgsDataInqueryDialog.h"

#include <QCombobox>

#include <qgisinterface.h>
#include <qgsproject.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>

#include "qgsLayerUtils.h"

qgsDataInqueryDialog::qgsDataInqueryDialog(QgisInterface* qgsInterface):
	mQgsInterface(qgsInterface)
{
	setupUi(this);
	setWindowFlags(windowFlags()
		& ~Qt::WindowCloseButtonHint
		& ~Qt::WindowContextHelpButtonHint
		& ~Qt::WindowMinimizeButtonHint
		& ~Qt::WindowMaximizeButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	//
	qgsLayerUtils::loadVectorLayerNames(this->mComboTargetLyr, QString());
}
qgsDataInqueryDialog::~qgsDataInqueryDialog()
{
	mQgsInterface = nullptr;
}

void qgsDataInqueryDialog::on_mComboTargetLyr_currentIndexChanged(int index)
{
	int idx = mComboTargetLyr->currentIndex();
	QVariant lyrId = mComboTargetLyr->itemData(idx);
	QString str = lyrId.toString();

	qgsLayerUtils::loadVectorLayerNames(this->mComboSrcLyr, str);
	qgsLayerUtils::loadLayerFieldsNames(this->mComboTargetField, str);
}


void qgsDataInqueryDialog::on_mComboSrcLyr_currentIndexChanged(int index)
{
	int idx = mComboSrcLyr->currentIndex();
	QVariant lyrId = mComboSrcLyr->itemData(idx);
	QString str = lyrId.toString();
	qgsLayerUtils::loadLayerFieldsNames(this->mComboSrcField, str);
}
/// <summary>
/// 获取参数
/// </summary>
/// <param name="params"></param>
/// <returns></returns>
bool qgsDataInqueryDialog::getParams(sDataInqueryParams& params)
{
	int idx = mComboSrcLyr->currentIndex();
	QVariant lyrId = mComboSrcLyr->itemData(idx);
	params.srcLyrIdx = lyrId.toString();
	params.srcFldName = mComboSrcField->currentText();

	idx = mComboTargetLyr->currentIndex();
	lyrId = mComboTargetLyr->itemData(idx);
	params.tarLyrIdx = lyrId.toString();
	params.tarFldName = mComboTargetField->currentText();
	return true;
}

void qgsDataInqueryDialog::on_pbOk_clicked(bool checked)
{
	this->accept();
}

void qgsDataInqueryDialog::on_pbCancel_clicked(bool checked)
{
	this->reject();
}