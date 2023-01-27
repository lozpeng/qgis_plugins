#pragma once

#include <QDialog>
#include "ui_qgsSamplingParamDialog.h"
#include "utils/qgsGridUtil.h"

typedef struct qgsSampleParams
{
	int distance;  //����
	GridType gridType; // ������������
	bool isParamsInited; //�����Ƿ�������ȡ
}sampleParams;
class qgsSamplingParamDialog : public QDialog, public Ui::qgsSamplingParamDialogClass
{
	Q_OBJECT

public:
	qgsSamplingParamDialog(QWidget *parent = nullptr);
	~qgsSamplingParamDialog();
	void closeEvent(QCloseEvent* event);
	//!��ȡ�û����õĲ���
	bool getParams(qgsSampleParams& params);
private slots:
	void on_pbOK_clicked();
	void on_pbCancel_clicked();
	//void on_chkBxTemp_stateChanged(int state);
};
