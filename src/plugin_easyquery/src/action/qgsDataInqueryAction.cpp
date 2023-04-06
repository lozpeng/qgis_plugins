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
#include <qgscoordinatereferencesystem.h>

#include "qgsLayerUtils.h"
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
	mParamDialog->getParams(mParams);
}
//!��������
int qgsDataInqueryAction::compute()
{
	this->setShowProgressBar(true);
	QgsVectorLayer* vtLyr = qgsLayerUtils::getVectorLayerById(mParams.tarLyrIdx);
	QgsVectorLayer* vsLyr = qgsLayerUtils::getVectorLayerById(mParams.srcLyrIdx);

	if (!vtLyr || !vsLyr)return -1;
	if (!vtLyr->isEditable())
	{
		this->showMessage(QString::fromLocal8Bit("���ͼ��ı༭״̬..."));
		this->throwError(-33);
		return -1;
	}

	//ѭ������Ŀ��ͼ���ÿһ��Ҫ�أ�������Ҫ�ж�����ͼ���Ƿ�ʹ����ͬ��ͶӰ
	QgsCoordinateReferenceSystem tarRefSys = vtLyr->crs();
	QgsCoordinateReferenceSystem srcRefSys = vsLyr->crs();
	QgsCoordinateTransform coordTrans;
	bool b = false;
	if (tarRefSys.srsid() != srcRefSys.srsid())
	{
		coordTrans.setDestinationCrs(srcRefSys);
		coordTrans.setSourceCrs(tarRefSys);
		b = true;
	}
	QgsFeatureIterator featureIt = vtLyr->getFeatures();
	QgsFeature f;   //Ҫ��
	while (featureIt.nextFeature(f))
	{
		if (!f.isValid())continue;
		QgsGeometry geo = f.geometry();
		if (geo.isEmpty())continue;
		if (b)
			geo.transform(coordTrans);

		QgsGeometry cent = geo.centroid();  //��ȡҪ�ص����ĵ�
		QgsRectangle bbox = geo.boundingBox();//��ȡҪ�ص�
		QgsFeatureRequest fr; //���пռ��ѯ
		fr.setFilterRect(bbox);
		QgsFeatureIterator srcfi = vsLyr->getFeatures(fr);
		QgsFeature srcf;
		while (srcfi.nextFeature(srcf))
		{
			if (!srcf.isValid())continue;
			QgsGeometry tGeo = srcf.geometry();
			if (tGeo.isEmpty())continue;
			if (tGeo.contains(cent))//���ͼ�ΰ���Ŀ��Ҫ�ص����ĵ㣬���ȡ����
			{
				QVariant qv = srcf.attribute(mParams.srcFldName);
				if (qv.isNull())
					continue;
				else {
					f.setAttribute(mParams.tarFldName, qv);
					vtLyr->updateFeature(f);
				} 
				break;
			}
		}
	}
	this->showMessage(QString("���ݴ������...!"));
	this->setShowProgressBar(false);
	return 1;
}