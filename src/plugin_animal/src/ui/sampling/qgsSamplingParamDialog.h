#pragma once

#include <QDialog>
#include "ui_qgsSamplingParamDialog.h"
#include "utils/qgsGridUtil.h"

typedef struct qgsSampleParams
{
	int distance;  //距离
	GridType gridType; // 抽样格网类型
	bool isParamsInited; //参数是否正常获取
}sampleParams;
class qgsSamplingParamDialog : public QDialog, public Ui::qgsSamplingParamDialogClass
{
	Q_OBJECT

public:
	qgsSamplingParamDialog(QWidget *parent = nullptr);
	~qgsSamplingParamDialog();
	void closeEvent(QCloseEvent* event);
	//!获取用户设置的参数
	bool getParams(qgsSampleParams& params);
private slots:
	void on_pbOK_clicked();
	void on_pbCancel_clicked();
	//void on_chkBxTemp_stateChanged(int state);
};
