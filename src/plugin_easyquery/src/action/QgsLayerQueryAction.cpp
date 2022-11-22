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
			QString::fromLocal8Bit("ͼ���ѯ"),
			QString::fromLocal8Bit("ѡ��ĳ���ֶν���ͼ���ѯ"),
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

//!���õ�ǰ��ѯͼ�㣬����ֶ��б��������
void qgsLayerQueryAction::setCurrentLayer(QgsVectorLayer* pLayer)
{
	//���Ϊ���򲻴���
	if (pLayer == nullptr)return;
	this->mVectorLayer = pLayer;

	if (mFieldsSelectorDlg)
		mFieldsSelectorDlg->updateCurrentLayer(pLayer);
}
//!����ط��������ô���
int qgsLayerQueryAction::openInputDialog()
{
	if (!this->m_qgsInterface)return 0;
	if (!this->mVectorLayer)
	{
		QMessageBox::information(nullptr,
						QString::fromLocal8Bit("��ʾ"), 
						QString::fromLocal8Bit("��ѡ��һ��ʸ��ͼ�㣡"));
		return -1;
	}
	if (!mFieldsSelectorDlg)
		mFieldsSelectorDlg = new qgsLayerFieldsBaseDialog(this->m_qgsInterface->mainWindow());
	mFieldsSelectorDlg->updateCurrentLayer(this->mVectorLayer);//���µ�ǰͼ��
	int result = mFieldsSelectorDlg->exec();
	mFieldsSelectorDlg->setVisible(false);
	return 1;
}

///��ȡ�û�����֮���ѡ�е��ֶ�
void qgsLayerQueryAction::getParametersFromDialog()
{
	if (!mFieldsSelectorDlg)return;
	QStringList strList;
	int result = mFieldsSelectorDlg->getSelectedFields(strList);
}