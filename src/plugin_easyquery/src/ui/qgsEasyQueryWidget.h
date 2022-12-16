#pragma once

#include <QWidget>
#include "ui_qgsEasyQueryWidget.h"

#include <qgisinterface.h>
#include <qgsvectorlayer.h>

//�����ݲ�ѯ�ؼ�
class qgsEasyQueryWidget : public QWidget, public Ui::qgsEasyQueryWidgetClass
{
	Q_OBJECT

public:
	qgsEasyQueryWidget(QgisInterface* qgisInterface, QWidget* parent = nullptr);
	~qgsEasyQueryWidget();

private:
	QgisInterface* mQgisInterface;
	QgsVectorLayer* mQueryLayer;
	QStringList  mQueryFields;

	void query(QString txt);
private slots:
	//!�ֶ�ѡ��ť����¼�
	void on_tbFieldsOption_click();
	//���ı��ı��
	void on_leQueryText_textChanged(const QString&);
	//! Enter�����֮��
	void on_leQueryText_returnPressed();
};
