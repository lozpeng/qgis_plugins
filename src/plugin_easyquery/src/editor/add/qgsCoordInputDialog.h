#pragma once

#include <QDialog>
#include <QWidget>
#include <QEvent>

#include <qgspoint.h>
#include <qgspointxy.h>
#include "ui_qgsCoordInputDialogBase.h"

class qgsCoordInputDialog : public QDialog
{
	Q_OBJECT

public:
	qgsCoordInputDialog(QWidget *parent = nullptr);
	~qgsCoordInputDialog();
	//! ��ȡ�û����������
	bool getInputCoords(QgsPointXY& pnt);
protected:
	//����ر��¼�
	void closeEvent(QCloseEvent* e) override; 
	//! �����¼�
	void keyPressEvent(QKeyEvent* e) override;
private:
	Ui::qgsCoordInputDialogClass ui;
	bool mCoordInputed;
private slots:

};
