#include "qgsDataInqueryAction.h"
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

#include "ui/qgsDataInqueryDialog.h"

qgsDataInqueryAction::qgsDataInqueryAction()
	: QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mQgsDataInqueryAction",
			QString::fromLocal8Bit("���ݻ�ȡ"),
			QString::fromLocal8Bit("���ݿռ�λ�û�ȡ��������"),
			":/easy_query/images/image_search-24px.svg"))
{
	this->m_show_progress = false;
	this->mParamDialog = nullptr;
}
qgsDataInqueryAction::~qgsDataInqueryAction()
{
	this->m_show_progress = false;
	if (this->mParamDialog)
		delete this->mParamDialog;
	this->mParamDialog = nullptr;
}

//!����ط��������ô���
int qgsDataInqueryAction::openInputDialog()
{
	if (!this->m_qgsInterface)return 0;
	if (!mParamDialog)
		mParamDialog = new qgsDataInqueryDialog(this->m_qgsInterface);
	int result = mParamDialog->exec();
	mParamDialog->setVisible(false);
	return 1;
}

///��ȡ�û�����֮���ѡ�е���``��
void qgsDataInqueryAction::getParametersFromDialog()
{
	if (!mParamDialog)return;
	QStringList strList;
	//int result = mParamDialog->getSelectedFields(strList);
}
//!
int qgsLayerQueryAction::compute()
{
	return -1;
}