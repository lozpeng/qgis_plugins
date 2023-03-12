#pragma once

#include <QDialog>
#include <QCombobox>

#include <qgisinterface.h>

#include "ui_qgsDataInqueryDialogBase.h"

//!���ݿռ��ѯ��ȡ����
class qgsDataInqueryDialog : public QDialog, public Ui::qgsDataInqueryDialogBase
{
	Q_OBJECT
public:
	qgsDataInqueryDialog(QgisInterface* qgsInterface);

	~qgsDataInqueryDialog();

private:
	QgisInterface* mQgsInterface;  //qgis���

private://functions
	/// <summary>
	/// 
	/// </summary>
	/// <param name="exLyrId">��Ҫ�ų���ͼ������</param>
	void loadLayerNames(QComboBox*,QString exLyrId);
protected slots:
	void on_mComboTargetLyr_currentIndexChanged(int index);

	void on_mComboSrcLyr_currentIndexChanged(int index);
};

