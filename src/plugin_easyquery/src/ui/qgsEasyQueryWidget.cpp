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

//!�ֶ�ѡ��ť����¼�
void qgsEasyQueryWidget::on_tbFieldsOption_click()
{
	QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>(mQgisInterface->mapCanvas()->currentLayer());
	if (!vectorLayer)return;

	qgsLayerFieldsBaseDialog* fieldsDialog =new qgsLayerFieldsBaseDialog(this);
	fieldsDialog->updateCurrentLayer(vectorLayer); //���µ�ǰͼ��
	if (QDialog::Accepted == fieldsDialog->exec())
	{
		fieldsDialog->getSelectedFields(mQueryFields);
		//��ȡ�ֶε�ͬʱҲ���õ�ǰ��ѯͼ��
		mQueryLayer = vectorLayer;
	}
	else mQueryFields.clear();
}
//���ı��ı��
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