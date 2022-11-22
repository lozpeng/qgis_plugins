#pragma once

#include <QDialog>
#include "ui_qgslayerfieldsbasedialog.h"

class QgsVectorLayer;

class qgsLayerFieldsBaseDialog : public QDialog, public Ui::qgsLayerFieldsBaseDialogClass
{
	Q_OBJECT

public:
	qgsLayerFieldsBaseDialog(QWidget* parent = nullptr);
	~qgsLayerFieldsBaseDialog();
	int getSelectedFields(QStringList& strList);//获取选择的字段列表

	void updateCurrentLayer(QgsVectorLayer* currentLayer);
private:
	QgsVectorLayer* mCurrentLayer;
	//!初始化字段列表，onlyTxtFields 表明只显示字符型字段
	void initFields(bool onlyTxtFields);
private slots:
	//!当用户需要显示所有字段时
	void on_chkAllFields_stateChanged(int);
	//! 确定按钮处理事件
	void on_pbOk_click();
};
