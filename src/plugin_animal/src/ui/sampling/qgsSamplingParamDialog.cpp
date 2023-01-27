#include "qgsSamplingParamDialog.h"
#include <QCloseEvent>

#include <QDir>
#include <QFileDialog>
#include <QPushButton>
#include "utils/qgsGridUtil.h"

qgsSamplingParamDialog::qgsSamplingParamDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	
	setWindowFlags(windowFlags() 
		& ~Qt::WindowCloseButtonHint 
		& ~Qt::WindowContextHelpButtonHint
		& ~Qt::WindowMinimizeButtonHint
		& ~Qt::WindowMaximizeButtonHint);
	setWindowFlags(windowFlags() |Qt::WindowStaysOnTopHint);
	this->setFixedSize(850, 380);

}

qgsSamplingParamDialog::~qgsSamplingParamDialog()
{}

void qgsSamplingParamDialog::closeEvent(QCloseEvent* e)
{
	e->ignore();
	this->hide();
}
void qgsSamplingParamDialog::on_pbOK_clicked()
{
	this->setResult(1);
	this->hide();
}
void qgsSamplingParamDialog::on_pbCancel_clicked()
{
	this->setResult(0);
	this->hide();
}
//!获取参数
bool qgsSamplingParamDialog::getParams(qgsSampleParams& params)
{
	params.isParamsInited = false;
	params.distance = this->spb_distance->value();
	if (rBGrid->isChecked())
		params.gridType = GridType::RECTANGLE;
	if(rbHexg->isChecked())
		params.gridType = GridType::HEXAGON;
	if(rbHexagon2->isChecked())
		params.gridType = GridType::HEXAGON2;
	params.isParamsInited = true;
	return true;
}
