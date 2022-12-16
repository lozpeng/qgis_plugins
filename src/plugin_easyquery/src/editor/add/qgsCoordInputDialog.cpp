#include "qgsCoordInputDialog.h"

#include <QEvent>
#include <QDialog>
#include <Qdialog>
#include <qevent.h>
#include <qgspointxy.h>

qgsCoordInputDialog::qgsCoordInputDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	mCoordInputed = false;
}

qgsCoordInputDialog::~qgsCoordInputDialog()
{}
//处理关闭事件
void qgsCoordInputDialog::closeEvent(QCloseEvent* e)
{
	e->ignore();
	this->setVisible(false);

}
//! 按键事件
void qgsCoordInputDialog::keyPressEvent(QKeyEvent* e) {
	//判断按键事件为
	if (e->key() == Qt::Key_Enter) //如果是按的Enter键
	{
		//this->ui.coordY->setFocused();
		if (e->modifiers() == Qt::ControlModifier)
		{
			this->accept();
			mCoordInputed = true;
			return;
		}
		this->ui.coordY->setFocus();
	}
	if (e->key() == Qt::Key_Escape)
		this->reject();
	mCoordInputed = false;
}
bool qgsCoordInputDialog::getInputCoords(QgsPointXY& pnt)
{
	if (!mCoordInputed)return false;
	pnt.setX(ui.coordX->value());
	pnt.setY(ui.coordY->value());
	return true;
}