#pragma once

#include <QObject>
#include <QDialog>


#include "ui_qgsBufferingDialogBase.h"

struct qgsBufferParams {
	double bSelected;  //��ʹ��ѡ��Ҫ��
	double dist;       //����
	int bufferType;    //��������
	bool bUseField;    //�Ƿ�ʹ���ֶ�
	QString fieldName; //�ֶ���
	bool bDissolve;    //�Ƿ�ϲ�
	int endCapStyle;   //��������
	int joinStyle;     //��������
	double miterLimit;    //�������
	int segeMents;		//�߶�
};
class qgsBufferingDialog : public QDialog, public Ui::qgsBufferingDialogBase
{
	Q_OBJECT
public:
	qgsBufferingDialog(QWidget *parent,QStringList fields);
	~qgsBufferingDialog();
	/// <summary>
	/// ��ȡ�û����õĻ������ɲ���
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
