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
			QString::fromLocal8Bit("数据获取"),
			QString::fromLocal8Bit("根据空间位置获取属性数据"),
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

//!这个地方弹出设置窗口
int qgsDataInqueryAction::openInputDialog()
{
	if (!this->m_qgsInterface)return 0;
	if (!mParamDialog)
		mParamDialog = new qgsDataInqueryDialog(this->m_qgsInterface);
	int result = mParamDialog->exec();
	mParamDialog->setVisible(false);
	return 1;
}

///获取用户设置之后的选中的字``段
void qgsDataInqueryAction::getParametersFromDialog()
{
	if (!mParamDialog)return;
	mParamDialog->getParams(mParams);
}
//!分析计算
int qgsDataInqueryAction::compute()
{
	this->setShowProgressBar(true);
	QgsVectorLayer* vtLyr = qgsLayerUtils::getVectorLayerById(mParams.tarLyrIdx);
	QgsVectorLayer* vsLyr = qgsLayerUtils::getVectorLayerById(mParams.srcLyrIdx);

	if (!vtLyr || !vsLyr)return -1;
	if (!vtLyr->isEditable())
	{
		this->showMessage(QString::fromLocal8Bit("请打开图层的编辑状态..."));
		this->throwError(-33);
		return -1;
	}

	//循环处理目标图层的每一个要素，这里需要判断两个图形是否使用相同的投影
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
	QgsFeature f;   //要素
	while (featureIt.nextFeature(f))
	{
		if (!f.isValid())continue;
		QgsGeometry geo = f.geometry();
		if (geo.isEmpty())continue;
		if (b)
			geo.transform(coordTrans);

		QgsGeometry cent = geo.centroid();  //获取要素的中心点
		QgsRectangle bbox = geo.boundingBox();//获取要素的
		QgsFeatureRequest fr; //进行空间查询
		fr.setFilterRect(bbox);
		QgsFeatureIterator srcfi = vsLyr->getFeatures(fr);
		QgsFeature srcf;
		while (srcfi.nextFeature(srcf))
		{
			if (!srcf.isValid())continue;
			QgsGeometry tGeo = srcf.geometry();
			if (tGeo.isEmpty())continue;
			if (tGeo.contains(cent))//如果图形包括目标要素的中心点，则获取属性
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
	this->showMessage(QString("数据处理完成...!"));
	this->setShowProgressBar(false);
	return 1;
}