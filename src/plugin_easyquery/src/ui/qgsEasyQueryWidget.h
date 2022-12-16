#pragma once

#include <QWidget>
#include "ui_qgsEasyQueryWidget.h"

#include <qgisinterface.h>
#include <qgsvectorlayer.h>

//！数据查询控件
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
	//!字段选按钮点击事件
	void on_tbFieldsOption_click();
	//！文本改变后
	void on_leQueryText_textChanged(const QString&);
	//! Enter键点击之后
	void on_leQueryText_returnPressed();
};
