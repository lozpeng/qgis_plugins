#include "qgsBufferingDialog.h"

#include <QEvent>
#include <QCloseEvent>
#include <qgis.h>

qgsBufferingDialog::qgsBufferingDialog(QWidget*parent, QStringList fields)
	: QDialog(parent)
{
	setupUi(this);

	setWindowFlags(windowFlags()
		& ~Qt::WindowCloseButtonHint
		& ~Qt::WindowContextHelpButtonHint
		& ~Qt::WindowMinimizeButtonHint
		& ~Qt::WindowMaximizeButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	if (fields.size() >= 1)
		setFieldLists(fields);
}

qgsBufferingDialog::~qgsBufferingDialog()
{}

void qgsBufferingDialog::setFieldLists(QStringList fields)
{
	this->cmbFields->clear();
	Q_FOREACH(QString fld, fields)
		this->cmbFields->addItem(fld);

	this->cmbFields->addItem("--");
	this->cmbFields->setCurrentText("--");
}
/// <summary>
/// 
/// </summary>
/// <param name="params"></param>
/// <returns></returns>
bool qgsBufferingDialog::getParams(qgsBufferParams& params)
{
	params.bSelected = chkSelected->isChecked();//是否使用选中要素
	params.dist = dsbDistance->value();
	params.fieldName = cmbFields->currentText();
	params.segeMents = sbLine->value();
	params.miterLimit = sbMiterLimit->value();
	params.endCapStyle = cmbEndCapStyle->currentIndex()+1;
	params.joinStyle = cmbJoinStyle->currentIndex() + 1;
	params.bDissolve = chbDissolve->isChecked();
	return false;
}

void qgsBufferingDialog::closeEvent(QCloseEvent* e)
{
	e->ignore();
	this->hide();
}
void qgsBufferingDialog::on_pbProcess_clicked() {
	this->setResult(1);
	this->hide();
}
void qgsBufferingDialog::on_pbCancel_clicked() {
	this->setResult(0);
	this->hide();
}