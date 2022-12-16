#include "qgslayerfieldsbasedialog.h"


#include <qgsvectorlayer.h>
#include <qgsfield.h>

qgsLayerFieldsBaseDialog::qgsLayerFieldsBaseDialog(QWidget *parent)
	: QDialog(parent),mCurrentLayer(nullptr)
{
	setupUi(this);
	this->setFixedWidth(400);
	this->setFixedHeight(500);
}

qgsLayerFieldsBaseDialog::~qgsLayerFieldsBaseDialog()
{}


//!�ⲿ���ú��������ڻ�ȡ�öԻ������û�ѡ���˶��ٸ��ֶΣ���������
int qgsLayerFieldsBaseDialog::getSelectedFields(QStringList& strList)
{
	strList.clear();
	int listCount = fieldListWidget->count();
	if (listCount > 0)
	{
		for (int i = 0; i < listCount; ++i)
		{
			QListWidgetItem* aItem = this->fieldListWidget->item(i);    //��ȡһ����
			if (aItem->checkState() == Qt::Checked)
			{
				strList.append(aItem->text());
			}
		}
	}
	return strList.size();
}
//!��ʼ���ֶ�
void qgsLayerFieldsBaseDialog::initFields(bool onlyTxtFields)
{
	this->fieldListWidget->clear();// ���ԭ���ֶ��б�
	if (!mCurrentLayer)return;
	
	QgsFields fields = mCurrentLayer->fields();
	if (fields.size() <= 0)return;
	
	//��ȡ�����ֶΣ������ListViewWidget��ȥ
	Q_FOREACH(QgsField field, fields)
	{
		if (onlyTxtFields)
		{
			QListWidgetItem* fieldItem = new QListWidgetItem(field.name(), fieldListWidget);
			fieldItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			fieldItem->setCheckState(Qt::CheckState::Unchecked);
			this->fieldListWidget->addItem(fieldItem);
		}
		else
		{
			if (field.type() == QVariant::Type::String || field.type() == QVariant::Type::StringList)
			{
				QListWidgetItem* fieldItem = new QListWidgetItem(field.name(), fieldListWidget);
				fieldItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
				fieldItem->setCheckState(Qt::CheckState::Unchecked);
				this->fieldListWidget->addItem(fieldItem);
			}
		}
	}
	fieldListWidget->update();
}


void qgsLayerFieldsBaseDialog::on_chkAllFields_stateChanged(int status)
{
	initFields(this->chkAllFields->isChecked());
}

//! ȷ����ť�����¼�
void qgsLayerFieldsBaseDialog::on_pbOk_click()
{
	this->accept();
}
/**
* ���µ�ǰͼ��
*/
void qgsLayerFieldsBaseDialog::updateCurrentLayer(QgsVectorLayer* currentLayer) {
	this->mCurrentLayer = currentLayer;
	//����ʼ���ֶ��б�
	initFields(this->chkAllFields->isChecked());
}