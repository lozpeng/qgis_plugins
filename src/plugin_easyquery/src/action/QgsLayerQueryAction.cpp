#include "QgsLayerQueryAction.h"

#include <QDialog>
#include <QMessagebox>
#include <qstringlist.h>

#include <qgisinterface.h>
#include <qgsmaplayer.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsfield.h>
#include <qgsfeature.h>


#include "ui/qgslayerfieldsbasedialog.h"

qgsLayerQueryAction::qgsLayerQueryAction()
	: QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mLayerQueryAction",
			QString::fromLocal8Bit("图层查询"),
			QString::fromLocal8Bit("选择某个字段进行图层查询"),
			":/easy_query/images/image_search-24px.svg"))
{
	this->m_show_progress = false;
	this->mVectorLayer = nullptr;
	this->mSearchDialog = nullptr;
	mFieldsSelectorDlg = nullptr;
}
qgsLayerQueryAction::~qgsLayerQueryAction()
{
	this->m_show_progress = false;
	this->mVectorLayer = nullptr;
	if (this->mSearchDialog)
		delete this->mSearchDialog;
	this->mSearchDialog = nullptr;

	if (mFieldsSelectorDlg)
		delete mFieldsSelectorDlg;
	mFieldsSelectorDlg = 0;
}

//!设置当前查询图层，清空字段列表，重新添加
void qgsLayerQueryAction::setCurrentLayer(QgsVectorLayer* pLayer)
{
	//如果为空则不处理，
	if (pLayer == nullptr)return;
	this->mVectorLayer = pLayer;

	if (mFieldsSelectorDlg)
		mFieldsSelectorDlg->updateCurrentLayer(pLayer);
}
//!这个地方弹出设置窗口
int qgsLayerQueryAction::openInputDialog()
{
	if (!this->m_qgsInterface)return 0;
	if (!this->mVectorLayer)
	{
		QMessageBox::information(nullptr,
						QString::fromLocal8Bit("提示"), 
						QString::fromLocal8Bit("请选择一个矢量图层！"));
		return -1;
	}
	if (!mFieldsSelectorDlg)
		mFieldsSelectorDlg = new qgsLayerFieldsBaseDialog(this->m_qgsInterface->mainWindow());
	mFieldsSelectorDlg->updateCurrentLayer(this->mVectorLayer);//更新当前图层
	int result = mFieldsSelectorDlg->exec();
	mFieldsSelectorDlg->setVisible(false);
	return 1;
}

///获取用户设置之后的选中的字段
void qgsLayerQueryAction::getParametersFromDialog()
{
	if (!mFieldsSelectorDlg)return;
	QStringList strList;
	int result = mFieldsSelectorDlg->getSelectedFields(strList);
}