#include "qgsEasyQueryWidget.h"

#include "qgslayerfieldsbasedialog.h"
#include <qgsmapcanvas.h>
qgsEasyQueryWidget::qgsEasyQueryWidget(QgisInterface* qgisInterface, QWidget *parent)
	: QWidget(parent)
	, mQgisInterface(qgisInterface)
	, mQueryLayer(nullptr)
{
	setupUi(this);
	mQueryFields.clear();
}

qgsEasyQueryWidget::~qgsEasyQueryWidget()
{}

//!字段选按钮点击事件
void qgsEasyQueryWidget::on_tbFieldsOption_click()
{
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(mQgisInterface->mapCanvas()->currentLayer());
	if (!vectorLayer)return;

	qgsLayerFieldsBaseDialog* fieldsDialog =new qgsLayerFieldsBaseDialog(this);
	fieldsDialog->updateCurrentLayer(vectorLayer); //更新当前图层
	if (QDialog::Accepted == fieldsDialog->exec())
	{
		fieldsDialog->getSelectedFields(mQueryFields);
		//获取字段的同时也设置当前查询图层
		mQueryLayer = vectorLayer;
	}
	else mQueryFields.clear();
}
//！文本改变后
void qgsEasyQueryWidget::on_leQueryText_textChanged(const QString& text)
{
	this->query(text);
}
void qgsEasyQueryWidget::on_leQueryText_returnPressed()
{
	QString text = leQueryText->text();
	query(text);
}
void qgsEasyQueryWidget::query(QString txt)
{
	if (!mQueryLayer || txt.isEmpty() || mQueryFields.isEmpty())return;
	QString wherClause;
	Q_FOREACH(QString field, mQueryFields)
	{
		if (wherClause.isEmpty() || wherClause.isNull())
			wherClause = QString("{0} LIKE '%{1}%'").arg(field, txt);
		else
			wherClause = QString("{0} OR {1} LIKE '%{2}%'").arg(wherClause, field, txt);
	}
	mQueryLayer->selectByExpression(wherClause);
}