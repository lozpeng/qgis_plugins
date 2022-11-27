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
	//! 获取用户输入的坐标
	bool getInputCoords(QgsPointXY& pnt);
protected:
	//处理关闭事件
	void closeEvent(QCloseEvent* e) override; 
	//! 按键事件
	void keyPressEvent(QKeyEvent* e) override;
private:
	Ui::qgsCoordInputDialogClass ui;
	bool mCoordInputed;
private slots:

};
