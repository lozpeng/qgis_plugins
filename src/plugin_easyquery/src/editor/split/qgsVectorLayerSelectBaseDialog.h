#pragma once

#include <QDialog>
#include "ui_qgsVectorLayerSelectBaseDialog.h"

class qgsVectorLayerSelectBaseDialog : public QDialog
{
	Q_OBJECT

public:
	qgsVectorLayerSelectBaseDialog(QWidget *parent = nullptr);
	~qgsVectorLayerSelectBaseDialog();
	void closeEvent(QCloseEvent* event);
	//!��ȡ�û�ѡ�е�ͼ����
	void getSelectedLayerId(QString& str);
private:
	Ui::qgsVectorLayerSelectBaseDialogClass ui;
public:
	void updateLayers();
private slots:
	void on_pbOk_clicked(bool checked);
	void on_pbCancle_clicked(bool checked);
};
