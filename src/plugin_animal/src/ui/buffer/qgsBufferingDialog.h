#pragma once

#include <QObject>
#include <QDialog>


#include "ui_qgsBufferingDialogBase.h"

struct qgsBufferParams {
	double bSelected;  //仅使用选中要素
	double dist;       //距离
	int bufferType;    //缓冲类型
	bool bUseField;    //是否使用字段
	QString fieldName; //字段名
	bool bDissolve;    //是否合并
	int endCapStyle;   //结束类型
	int joinStyle;     //连接类型
	double miterLimit;    //尖角限制
	int segeMents;		//线段
};
class qgsBufferingDialog : public QDialog, public Ui::qgsBufferingDialogBase
{
	Q_OBJECT
public:
	qgsBufferingDialog(QWidget *parent,QStringList fields);
	~qgsBufferingDialog();
	/// <summary>
	/// 获取用户设置的缓冲生成参数
	/// </summary>
	/// <param name="params"></param>
	/// <returns></returns>
	bool getParams(qgsBufferParams& params);
	void closeEvent(QCloseEvent* event);
	//
	void setFieldLists(QStringList fields);
private slots:
	void on_pbProcess_clicked();
	void on_pbCancel_clicked();
};
