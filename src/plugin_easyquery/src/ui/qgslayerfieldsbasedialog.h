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
	int getSelectedFields(QStringList& strList);//��ȡѡ����ֶ��б�

	void updateCurrentLayer(QgsVectorLayer* currentLayer);
private:
	QgsVectorLayer* mCurrentLayer;
	//!��ʼ���ֶ��б�onlyTxtFields ����ֻ��ʾ�ַ����ֶ�
	void initFields(bool onlyTxtFields);
private slots:
	//!���û���Ҫ��ʾ�����ֶ�ʱ
	void on_chkAllFields_stateChanged(int);
	//! ȷ����ť�����¼�
	void on_pbOk_click();
};
