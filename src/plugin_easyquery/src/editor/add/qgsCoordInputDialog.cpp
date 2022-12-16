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
//����ر��¼�
void qgsCoordInputDialog::closeEvent(QCloseEvent* e)
{
	e->ignore();
	this->setVisible(false);

}
//! �����¼�
void qgsCoordInputDialog::keyPressEvent(QKeyEvent* e) {
	//�жϰ����¼�Ϊ
	if (e->key() == Qt::Key_Enter) //����ǰ���Enter��
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