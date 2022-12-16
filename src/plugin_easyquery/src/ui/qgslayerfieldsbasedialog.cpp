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


//!外部调用函数，用于获取该对话框中用户选择了多少个字段，返回数量
int qgsLayerFieldsBaseDialog::getSelectedFields(QStringList& strList)
{
	strList.clear();
	int listCount = fieldListWidget->count();
	if (listCount > 0)
	{
		for (int i = 0; i < listCount; ++i)
		{
			QListWidgetItem* aItem = this->fieldListWidget->item(i);    //获取一个项
			if (aItem->checkState() == Qt::Checked)
			{
				strList.append(aItem->text());
			}
		}
	}
	return strList.size();
}
//!初始化字段
void qgsLayerFieldsBaseDialog::initFields(bool onlyTxtFields)
{
	this->fieldListWidget->clear();// 清空原有字段列表
	if (!mCurrentLayer)return;
	
	QgsFields fields = mCurrentLayer->fields();
	if (fields.size() <= 0)return;
	
	//获取所有字段，添加至ListViewWidget中去
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

//! 确定按钮处理事件
void qgsLayerFieldsBaseDialog::on_pbOk_click()
{
	this->accept();
}
/**
* 更新当前图层
*/
void qgsLayerFieldsBaseDialog::updateCurrentLayer(QgsVectorLayer* currentLayer) {
	this->mCurrentLayer = currentLayer;
	//！初始化字段列表
	initFields(this->chkAllFields->isChecked());
}