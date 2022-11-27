#include "qgsGeometryAddTool.h"

#include <QMouseEvent>
#include <QWidget>

#include <qmessagebox.h>
#include <qgsgeometry.h>
#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgisapp.h>
#include <qgsvectorlayereditbuffer.h>
#include <qgsrubberband.h>
#include <qgis.h>
#include <QgsAttributeDialog.h>
#include <qgsadvanceddigitizingdockwidget.h>
//#include <SilenAttributeDialog.h>
#include <QgsMapMouseEvent.h>
#include <qgsproject.h>


qgsGeometryAddTool::qgsGeometryAddTool(QgsMapCanvas* canvas)
	: QgsMapToolCapture(canvas, QgisApp::instance()->cadDockWidget(),
		QgsMapToolCapture::CapturePolygon)
	, mCoordInpDialog(nullptr)
{
	mToolName = tr("qgsGeometryAddTool");
}
qgsGeometryAddTool::~qgsGeometryAddTool() {
	if (mCoordInpDialog)
		delete mCoordInpDialog;
	mCoordInpDialog = nullptr;
}
void qgsGeometryAddTool::activate()
{
	QgsMapToolCapture::activate();
}
void qgsGeometryAddTool::keyReleaseEvent(QKeyEvent* e)
{
	QgsVectorLayer* vlayer = currentVectorLayer();
	if (!vlayer)
	{
		notifyNotVectorLayer();
		return;
	}

	if (!vlayer->isEditable())
	{
		notifyNotEditableLayer();
		return;
	}
	if (e->key() == Qt::Key::Key_C)
		this->canvas()->setMapTool(this);
}
//�����ͷ��¼������û��û�����֮�󵯳��Ĳ���
void qgsGeometryAddTool::keyPressEvent(QKeyEvent* e)
{
	//check if we operate on a vector layer
	QgsVectorLayer* vlayer = currentVectorLayer();
	if (!vlayer)
	{
		notifyNotVectorLayer();
		return;
	}
	if (!vlayer->isEditable())
	{
		notifyNotEditableLayer();
		return;
	}
	//�жϵ�ǰ����ļ� �������봰���Ƿ���ʾ�������ʾ�ˡ����ٴ���������ʾ����
	//
	if (e->key() == Qt::Key::Key_F6
		|| (e->modifiers() == Qt::ControlModifier 
				&& e->key() == Qt::Key::Key_G)) //�����F6���򵯳�
	{
		if (!mCoordInpDialog)
			mCoordInpDialog = new qgsCoordInputDialog(this->canvas());
		if (mCoordInpDialog->isVisible())return;

		int dlgResult = mCoordInpDialog->exec();
		//
		if (dlgResult == QDialog::Accepted)
		{
			if (this->captureCurve() == nullptr)
				this->startCapturing();
			QgsPointXY pntXY;
			if (mCoordInpDialog->getInputCoords(pntXY))
				this->addVertex(pntXY);
		}
		return;
	}
	if (e->key() == Qt::Key::Key_C) //! ����ڱ༭״̬ʱ����C��ʱ��������ͼ���ι��ߣ����ε�ͼ
	{
		//this->canvas()->setMapTool(new qgsMapPanTool()); //���ù���


	}
}


//�û���굯����ʱ�����¼�
void qgsGeometryAddTool::cadCanvasReleaseEvent(QgsMapMouseEvent* e) {
	emit messageDiscarded();
	//check if we operate on a vector layer
	QgsVectorLayer* vlayer = currentVectorLayer();
	if (!vlayer)
	{
		notifyNotVectorLayer();
		return;
	}

	if (!vlayer->isEditable())
	{
		notifyNotEditableLayer();
		return;
	}
	QgsWkbTypes::Type lyrWkbType = vlayer->wkbType();
	if (lyrWkbType != QgsWkbTypes::Polygon && lyrWkbType != QgsWkbTypes::MultiPolygon)
	{
		emit messageEmitted(tr("Layer not a polygon layer!"));
		return;
	}

	//add point to list and to rubber band
	if (e->button() == Qt::LeftButton)
	{
		int error = addVertex(e->mapPoint(), e->mapPointMatch());
		if (error == 1)
		{
			//current layer is not a vector layer
			return;
		}
		else if (error == 2)
		{
			//problem with coordinate transformation
			emit messageEmitted(tr("Cannot transform the point to the layers coordinate system"));
			return;
		}
		startCapturing();
	}
	else if (e->button() == Qt::RightButton)
	{
		if (!isCapturing())
			return;
		deleteTempRubberBand();
		closePolygon();

		//does compoundcurve contain circular strings?
		//does provider support circular strings?
		bool hasCurvedSegments = captureCurve()->hasCurvedSegments();
		bool providerSupportsCurvedSegments = vlayer->dataProvider()->capabilities()
			& QgsVectorDataProvider::CircularGeometries;

		QgsCurve* curveToAdd = nullptr;
		if (hasCurvedSegments && providerSupportsCurvedSegments)
			curveToAdd = captureCurve()->clone();
		else
			curveToAdd = captureCurve()->curveToLine();
		QgsLineString* exteriorRing = curveToAdd->curveToLine();
		exteriorRing->close();
		//std::unique_ptr< QgsPolygon > polygon = qgis::make_unique< QgsPolygon >();
		//polygon->setExteriorRing(exteriorRing);

		QgsGeometry* drawGeo = new QgsGeometry(exteriorRing);
		stopCapturing();

		//��ȡ�û����ƵĶ���Σ�Ȼ��ӵ�ǰͼ���л�ȡ�ཻ��Ҫ��
		QgsRectangle bBox = drawGeo->boundingBox();
		QgsFeatureRequest fr;
		fr = fr.setFilterRect(bBox);
		fr.setFlags(QgsFeatureRequest::ExactIntersect);

		QgsFeatureIterator fit = vlayer->getFeatures(fr);
		int count(0);
		QgsFeature f;
		//�ȸ��ݾ��ο��ȡҪ����Ŀ��һ�㲻���鳬��5������Σ��������5��������û����ƵĶ����̫��
		while (fit.nextFeature(f))
		{
			QgsGeometry g = f.geometry();
			if (!g.isEmpty() && g.intersects(*drawGeo))
				count++;
		}
		if (count <= 0)//������ǰ�༭ͼ����û����֮�ཻ����ֱ���½�,���ﵽʱ��Ҫ��ӹ�����߲���Ҫ����ֱ�ӷ���
		{
			QgsFeature nFeature;
			nFeature.initAttributes(vlayer->fields().count());
			nFeature.setGeometry(*drawGeo); //�����û����Ƶ�ͼ��
			//SilenAttributeDialog sileAttrDlg(vlayer, &nFeature, true);
			//int ret = sileAttrDlg.exec();
			//qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
			vlayer->editBuffer()->addFeature(nFeature);
			//this->mCanvas->refresh();
			vlayer->select(nFeature.id());
			qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
			return;
		}
		int opType = 0;//�������ͣ�0.������1.�У�2.�ָ�
		QString msg;
		if (count >= 1)
		{
			msg = tr("There are %1 polygons intersect with your draw,Do you want to:").arg(count);
			QMessageBox msgBox(QMessageBox::Question, tr("Add Polygon"), msg);
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Yes);
			msgBox.setButtonText(QMessageBox::Yes, tr("Clip"));//ָʹ�û��ƶ���ν�����ͼ�ν����и
			msgBox.setButtonText(QMessageBox::YesAll, tr("Split"));//ָ���ڱ���ͼ�ζԻ���ͼ�ν��зָ�
			int ret = msgBox.exec();
			switch (ret)
			{
			case QMessageBox::Cancel:
			{
				this->mCanvas->refresh();
				delete drawGeo;
				drawGeo = nullptr;
				return;
			}
			case QMessageBox::Yes:
				opType = 1;
				break;
			case QMessageBox::YesAll:
				opType = 2;
				break;
			default:opType = 0;
			}
		}

		qgsGeometryEditUtils::processGeometryTopo(vlayer, drawGeo, opType);
		this->mCanvas->refresh();
		delete drawGeo;
		drawGeo = nullptr;
	}
}
//======================================���Ҫ��ͨ�������ֻ淽ʽ���Ҫ��========================================================
//Ӧ��Ҫ����ָ����Ŀ��ͼ�����������Ʋ�ͬ��Ҫ������
qgsToolAddFeatureFreehand::qgsToolAddFeatureFreehand(QgsMapCanvas* canvas)
	: QgsMapTool(canvas)
	, mDragging(false)
	, mCurrentLayer(nullptr)	
{
	mRubberBand = nullptr;
	mCursor = Qt::ArrowCursor;
	mFillColor = QColor(254, 178, 76, 63);
	mBorderColour = QColor(254, 58, 29, 100);
	mToolName = tr("Geometry Freehand....");
}

qgsToolAddFeatureFreehand::~qgsToolAddFeatureFreehand()
{
	if (mRubberBand)
		delete mRubberBand;
	mRubberBand = nullptr;
}

void qgsToolAddFeatureFreehand::canvasPressEvent(QgsMapMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
		return;
	if (!mCurrentLayer)
	{
		emit messageEmitted(tr("Current Layer is not set or not a vector layer!"));
		return;
	}

	if (!mCurrentLayer->isEditable())
	{
		emit messageEmitted(tr("Layer not in a editing mode!"));
		return;
	}
	QgsWkbTypes::Type lyrGeoType = mCurrentLayer->wkbType();
	if (!mRubberBand)
	{
		//�жϲ�ͬ���͹���rubberBand
		if (lyrGeoType == QgsWkbTypes::Polygon || lyrGeoType == QgsWkbTypes::MultiPolygon
			|| lyrGeoType == QgsWkbTypes::MultiPolygon25D || lyrGeoType == QgsWkbTypes::Polygon25D)
		{
			mRubberBand = new QgsRubberBand(mCanvas, QgsWkbTypes::GeometryType::PolygonGeometry);
			mRubberBand->setFillColor(mFillColor);
			//	mRubberBand->setBorderColor( mBorderColour );
		}
		else if (lyrGeoType == QgsWkbTypes::LineString || lyrGeoType == QgsWkbTypes::MultiLineString)
		{
			mRubberBand = new QgsRubberBand(mCanvas);
			//	mRubberBand->setBorderColor( mBorderColour );
		}
	}
	if (!mRubberBand)
	{
		emit messageEmitted(tr("Current vector layer not supported!"));
		return;
	}
	mRubberBand->addPoint(toMapCoordinates(e->pos()));
	mDragging = true;
}

//!���߱�����ʱ��Ҫ��ȡ��ǰ�༭�ͼ��
void qgsToolAddFeatureFreehand::activate() {
	mCurrentLayer = qobject_cast<QgsVectorLayer*>(mCanvas->currentLayer());
	mCanvas->setCursor(Qt::ArrowCursor);
}
void qgsToolAddFeatureFreehand::canvasMoveEvent(QgsMapMouseEvent* e)
{
	if (!mDragging || !mRubberBand)
		return;
	if (!mCurrentLayer)
	{
		emit messageEmitted(tr("Current Layer is not set or not a vector layer!"));
		return;
	}

	if (!mCurrentLayer->isEditable())
	{
		emit messageEmitted(tr("Layer not in a editing mode!"));
		return;
	}
	mRubberBand->addPoint(toMapCoordinates(e->pos()));
}

void qgsToolAddFeatureFreehand::canvasReleaseEvent(QgsMapMouseEvent* e)
{
	if (!mRubberBand)
		return;
	if (!mCurrentLayer)
	{
		emit messageEmitted(tr("Current Layer is not set or not a vector layer!"));
		return;
	}
	if (!mCurrentLayer->isEditable())
	{
		emit messageEmitted(tr("Layer not in a editing mode!"));
		return;
	}
	bool singleSelect = false;
	if (mRubberBand->numberOfVertices() > 0 && mRubberBand->numberOfVertices() <= 2)
	{
		emit messageDiscarded();
		if (mCurrentLayer)
		{
			QRect selectRect;
			singleSelect = true;
		}
	}
	if (mRubberBand->numberOfVertices() > 2)
	{
		QgsVectorLayer* vlayer = mCurrentLayer;
		QgsGeometry drawGeo = mRubberBand->asGeometry();
		if (!drawGeo.isEmpty())
		{
			//��ȡ�û����ƵĶ���Σ�Ȼ��ӵ�ǰͼ���л�ȡ�ཻ��Ҫ��
			QgsRectangle bBox = drawGeo.boundingBox();
			QgsFeatureRequest fr;
			fr = fr.setFilterRect(bBox);
			fr.setFlags(QgsFeatureRequest::ExactIntersect);

			QgsFeatureIterator fit = vlayer->getFeatures(fr);
			int count(0);
			QgsFeature f;
			//�ȸ��ݾ��ο��ȡҪ����Ŀ��һ�㲻���鳬��5������Σ��������5��������û����ƵĶ����̫��
			while (fit.nextFeature(f))
			{
				QgsGeometry g = f.geometry();
				if (!g.isEmpty() && g.intersects(drawGeo))
					count++;
			}
			if (count <= 0)//������ǰ�༭ͼ����û����֮�ཻ����ֱ���½�,���ﵽʱ��Ҫ��ӹ�����߲���Ҫ����ֱ�ӷ���
			{
				QgsFeature nFeature;
				nFeature.initAttributes(vlayer->fields().count());
				nFeature.setGeometry(drawGeo); //�����û����Ƶ�ͼ��
				//SilenAttributeDialog sileAttrDlg(vlayer, &nFeature, true);
				//qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
				//int ret = sileAttrDlg.exec();
				vlayer->editBuffer()->addFeature(nFeature);
				vlayer->select(nFeature.id());
				qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
				return;
			}
			int opType = 0;//�������ͣ�0.������1.�У�2.�ָ�
			QString msg;
			if (count >= 1)
			{
				msg = QString::fromLocal8Bit("���� %1 ����ƵĶ�����ص�����ѡ��:").arg(count);
				QMessageBox msgBox(QMessageBox::Question, QString::fromLocal8Bit("��Ӷ����"), msg);
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::Cancel);
				msgBox.setDefaultButton(QMessageBox::Yes);
				msgBox.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("��  ��"));//ָʹ�û��ƶ���ν�����ͼ�ν����и
				msgBox.setButtonText(QMessageBox::YesAll, QString::fromLocal8Bit("��  ��"));//ָ���ڱ���ͼ�ζԻ���ͼ�ν��зָ�
				int ret = msgBox.exec();
				switch (ret)
				{
				case QMessageBox::Cancel:
				{
					this->mCanvas->refresh();

					mRubberBand->reset(QgsWkbTypes::GeometryType::PolygonGeometry);
					delete mRubberBand;
					mRubberBand = nullptr;
					mDragging = false;
					return;
				}
				case QMessageBox::Yes:
					opType = 1;
					break;
				case QMessageBox::YesAll:
					opType = 2;
					break;
				default:opType = 0;
				}
			}
			//���������ݣ��������������ͼ����
			qgsGeometryEditUtils::processGeometryTopo(vlayer, &drawGeo, opType);

			this->mCanvas->refresh();
		}
	}
	mRubberBand->reset(QgsWkbTypes::GeometryType::PolygonGeometry);
	delete mRubberBand;
	mRubberBand = nullptr;
	mDragging = false;
}

///=============================Utils��
//������Ƶ�Ҫ��
//void qgsGeometryEditUtils::processGeometry(QgsVectorLayer *vlayer,const QgsGeometry* drawGeo)
//{
//	if(!vlayer || !drawGeo)return ;
//	//��ȡ�û����ƵĶ���Σ�Ȼ��ӵ�ǰͼ���л�ȡ�ཻ��Ҫ��
//	QgsRectangle bBox = drawGeo->boundingBox();
//	QgsFeatureRequest fr ;
//	fr= fr.setFilterRect( bBox );
//	fr.setFlags( QgsFeatureRequest::ExactIntersect );

//	QgsFeatureIterator fit =  vlayer->getFeatures(fr );
//	int count(0);
//	QgsFeature f;
//	//�ȸ��ݾ��ο��ȡҪ����Ŀ��һ�㲻���鳬��5������Σ��������5��������û����ƵĶ����̫��
//	while ( fit.nextFeature( f ) )
//	{
//		QgsGeometry* g = f.geometry();
//		if(!g->isEmpty() && g->intersects(drawGeo))
//			count++;
//	}
//	if(count <= 0)//������ǰ�༭ͼ����û����֮�ཻ����ֱ���½�,���ﵽʱ��Ҫ��ӹ�����߲���Ҫ����ֱ�ӷ���
//	{
//		QgsFeature nFeature;
//		nFeature.initAttributes(vlayer->fields().count());
//		nFeature.setGeometry(*drawGeo); //�����û����Ƶ�ͼ��
//		vlayer->editBuffer()->addFeature(nFeature);
//		//this->mCanvas->refresh();
//		return ;
//	}
//	int opType =0;//�������ͣ�0.������1.�У�2.�ָ�
//	QString msg;
//	if(count>=1)
//	{
//		msg = tr("There are %1 polygons intersect with your draw,Do you want to:").arg(count);
//		QMessageBox msgBox(QMessageBox::Question,tr("Add Polygon"),msg);
//		msgBox.setStandardButtons( QMessageBox::Yes|QMessageBox::YesAll|QMessageBox::Cancel);
//		msgBox.setDefaultButton(QMessageBox::Yes);
//		msgBox.setButtonText( QMessageBox::Yes,tr("Clip"));//ָʹ�û��ƶ���ν�����ͼ�ν����и
//		msgBox.setButtonText(QMessageBox::YesAll,tr("Split"));//ָ���ڱ���ͼ�ζԻ���ͼ�ν��зָ�
//		int ret=msgBox.exec();
//		switch(ret)
//		{
//		case QMessageBox::Cancel:
//			return ;
//		case QMessageBox::Yes:
//			opType = 1;
//			break;
//		case QMessageBox::YesAll:
//			opType=2;
//			break;
//		default:opType=0;
//		}
//	}
//	vlayer->beginEditCommand( tr( "Feature added" ) );		
//	QgsGeometry newGeo;
//	QgsFeatureIterator fit2 =  vlayer->getFeatures(fr);
//	QgsGeometry * diffGeo(nullptr);
//	while(fit2.nextFeature(f))
//	{
//		if ( !f.constGeometry() )
//			continue;
//		QgsGeometry* g = f.geometry();
//		if(g->isEmpty() || !g->intersects(drawGeo))continue;
//		//1.����Ҫ�صļ���ͼ�����û����Ƶļ���ͼ�β���ͬ�Ĳ������ڸ��µ�ǰҪ�صļ��β���
//		//��������Զ���Ķ�ĳЩ�ֶν��а���������������
//		QgsGeometry* fGeo = g->difference(drawGeo); //����Ҫ��ͼ�������ͼ�β��ص�����
//		//g->(drawGeo);
//		QgsGeometry* iterGeos = drawGeo->intersection(g);
//		//2.�Լ�������Ĳ���ͬ����ͼ�ν��������ж��봦����������Ŀ��ͼ��һ��ʱ�������ݱ���
//		if(fGeo && !fGeo->isEmpty())//������첿��ͼ�δ�������Ҫ����ԭ��Ҫ��ͼ��
//		{
//			bool changed = qgsGeometryEditUtils::changeFeatureGeo(vlayer, f.id(), fGeo,false);
//		}
//		if(!iterGeos || iterGeos->isEmpty())continue;
//		//�����Clip�򣬶Ի���ͼ����ȫ����Ҫ�ؽ���ɾ������
//		if(opType ==1){
//			if(drawGeo->contains(g))
//				vlayer->editBuffer()->deleteFeature(f.id());
//			continue;
//		}
//		if(opType==2)//�½��ཻ���ֵļ���ͼ��Ϊһ��Ҫ��
//			qgsGeometryEditUtils::saveGeometry(vlayer,iterGeos);
//		//�����û����ƵĶ��ಿ��ͼ��,�������κ�ͼ���ཻ�Ĳ���ͼ�Σ�ֱ�ӱ���Ϊ�µ�Ҫ��
//		if(diffGeo&&!diffGeo->isEmpty())
//		{
//			QgsGeometry* tmp =  diffGeo->difference(g);
//			delete diffGeo;
//			diffGeo = tmp;
//		}
//		else diffGeo = drawGeo->difference(g);
//	}
//	if(opType==1)
//	{
//		qgsGeometryEditUtils::saveGeometry(vlayer,drawGeo);
//		//QgsFeature nFeature;
//		//nFeature.initAttributes(vlayer->fields().count());
//		//nFeature.setGeometry(*drawGeo); //�����û����Ƶ�ͼ��
//		//vlayer->editBuffer()->addFeature(nFeature);
//	}
//	if(diffGeo && !diffGeo->isEmpty())//���ʣ�µ�ͼ�β�Ϊ����ֱ�Ӵ���Ҫ��
//	{
//		QgsFeature nFeature;
//		nFeature.initAttributes(vlayer->fields().count());
//		nFeature.setGeometry(*diffGeo); //�����û����Ƶ�ͼ��
//		vlayer->editBuffer()->addFeature(nFeature);
//	}
//	vlayer->endEditCommand();
//}

//�ֱ���ָ������
void qgsGeometryEditUtils::processGeometryTopo(QgsVectorLayer* vlayer, const QgsGeometry* drawGeo, int opType)
{
	if (!vlayer->isEditable())
	{
		QMessageBox::about(nullptr, QString::fromLocal8Bit("����"),QString::fromLocal8Bit("ͼ����봦�ڿɱ༭״̬��"));
		return;
	}
	//int opType =0;//�������ͣ�0.������1.�У�2.�ָ�
	if (!vlayer || !drawGeo)
		return;
	QList<QgsGeometry> intesectlist;
	if (opType == 1)//���ã��ж��Ƿ���߽�Լ��������ཻ
	{
		intesectlist = intersectsGeometry(vlayer, drawGeo);
		int count = intesectlist.count();
		if (count > 0)//��߽����ཻ
		{
			for (int i = 0; i < count; i++)
			{
				QgsGeometry pgeometry = intesectlist[i];
				processGeometry(vlayer, pgeometry, opType);
			}
			return;
		}
	}


	//��ȡ�û����ƵĶ���Σ�Ȼ��ӵ�ǰͼ���л�ȡ�ཻ��Ҫ��
	processGeometry(vlayer, *drawGeo, opType);
}

//�ֱ���ָ������
void qgsGeometryEditUtils::processGeometry(QgsVectorLayer* vlayer, const QgsGeometry drawGeo, int opType)
{
	if (!vlayer->isEditable())
	{
		QMessageBox::about(nullptr, QString::fromLocal8Bit("����"), 
										QString::fromLocal8Bit("ͼ����봦�ڱ༭״̬��"));
		return;
	}
	//int opType =0;//�������ͣ�0.������1.�У�2.�ָ�
	if (!vlayer || drawGeo.isEmpty())
		return;
	//QList<QgsGeometry*> intesectlist;
	//if(opType == 1)//���ã��ж��Ƿ���߽�Լ��������ཻ
	//{	
	//	intesectlist = intersectsGeometry(vlayer,drawGeo);
	//	int count = intesectlist.count();
	//	if(count>0)//��߽����ཻ
	//	{
	//		QgsGeometry* pgeometry = nullptr;
	//		for(int i=0; i<count; i++)
	//		{
	//			pgeometry = intesectlist[i];
	//		}
	//		return;
	//	}
	//}


	//��ȡ�û����ƵĶ���Σ�Ȼ��ӵ�ǰͼ���л�ȡ�ཻ��Ҫ��
	QgsRectangle bBox = drawGeo.boundingBox();
	QgsFeatureRequest fr;
	fr = fr.setFilterRect(bBox);
	fr.setFlags(QgsFeatureRequest::ExactIntersect);

	QgsFeatureIterator fit = vlayer->getFeatures(fr);
	int count(0);
	QgsFeature f;
	//�ȸ��ݾ��ο��ȡҪ����Ŀ��һ�㲻���鳬��5������Σ��������5��������û����ƵĶ����̫��
	while (fit.nextFeature(f))
	{
		QgsGeometry g = f.geometry();
		if (!g.isEmpty() && g.intersects(drawGeo))
			count++;
	}
	if (count <= 0)//������ǰ�༭ͼ����û����֮�ཻ����ֱ���½�,���ﵽʱ��Ҫ��ӹ�����߲���Ҫ����ֱ�ӷ���
	{
		//������õ������Ա༭��������Ա༭
		qgsGeometryEditUtils::saveGeometry(vlayer, drawGeo);
		//QgsFeature nFeature;
		//nFeature.initAttributes(vlayer->fields().count());
		//nFeature.setGeometry(*drawGeo); //�����û����Ƶ�ͼ��
		//vlayer->editBuffer()->addFeature(nFeature);
		//this->mCanvas->refresh();
		return;
	}
	vlayer->beginEditCommand(tr("Feature added"));
	QgsGeometry newGeo;
	QgsFeatureIterator fit2 = vlayer->getFeatures(fr);
	QgsGeometry  diffGeo;
	while (fit2.nextFeature(f))
	{
		if (!f.hasGeometry())
			continue;
		QgsGeometry g = f.geometry();
		if (g.isEmpty() || !g.intersects(drawGeo))continue;
		//1.����Ҫ�صļ���ͼ�����û����Ƶļ���ͼ�β���ͬ�Ĳ������ڸ��µ�ǰҪ�صļ��β���
		//��������Զ���Ķ�ĳЩ�ֶν��а���������������
		QgsGeometry fGeo = g.difference(drawGeo); //����Ҫ��ͼ�������ͼ�β��ص�����
		//g->(drawGeo);
		QgsGeometry iterGeos = drawGeo.intersection(g);
		//2.�Լ�������Ĳ���ͬ����ͼ�ν��������ж��봦����������Ŀ��ͼ��һ��ʱ�������ݱ���
		if (!fGeo.isEmpty())//������첿��ͼ�δ�������Ҫ����ԭ��Ҫ��ͼ��
		{
			bool changed = qgsGeometryEditUtils::changeFeatureGeo(vlayer, f.id(), &fGeo, false);
		}
		if (iterGeos.isEmpty())continue;
		//�����Clip�򣬶Ի���ͼ����ȫ����Ҫ�ؽ���ɾ������
		if (opType == 1) {
			if (drawGeo.contains(g))
				vlayer->editBuffer()->deleteFeature(f.id());
			continue;
		}
		if (opType == 2)//�½��ཻ���ֵļ���ͼ��Ϊһ��Ҫ��
			qgsGeometryEditUtils::saveGeometry(vlayer, iterGeos);
		//�����û����ƵĶ��ಿ��ͼ��,�������κ�ͼ���ཻ�Ĳ���ͼ�Σ�ֱ�ӱ���Ϊ�µ�Ҫ��
		if (diffGeo.isEmpty())
		{
			diffGeo = diffGeo.difference(g);
		}
		else diffGeo = drawGeo.difference(g);
	}
	if (opType == 1)
	{
		qgsGeometryEditUtils::saveGeometry(vlayer, drawGeo);
		//QgsFeature nFeature;
		//nFeature.initAttributes(vlayer->fields().count());
		//nFeature.setGeometry(*drawGeo); //�����û����Ƶ�ͼ��
		//vlayer->editBuffer()->addFeature(nFeature);
	}
	if (!diffGeo.isEmpty())//���ʣ�µ�ͼ�β�Ϊ����ֱ�Ӵ���Ҫ��
	{
		QgsFeature nFeature;
		nFeature.initAttributes(vlayer->fields().count());
		nFeature.setGeometry(diffGeo); //�����û����Ƶ�ͼ��
		vlayer->editBuffer()->addFeature(nFeature);
		vlayer->select(nFeature.id());
		editFeatureAttr(vlayer, &nFeature);
	}
	vlayer->endEditCommand();
}

//�½�Ҫ�ر�������
void qgsGeometryEditUtils::saveGeometry(QgsVectorLayer* vlayer, const QgsGeometry geos)
{
	QgsWkbTypes::Type lyrGeoType = vlayer->wkbType();
	QgsWkbTypes::Type geoType = geos.wkbType();
	//����ֻ��WkbPolygon��WkbMultiPolygon���з�����������ݱ���Ĺ���
	if (geoType == lyrGeoType)//�������ͼ�ε�������ͼ����ͬ��ֱ�ӱ���
	{
		QgsFeature nFeature;
		nFeature.initAttributes(vlayer->fields().count());
		nFeature.setGeometry(geos); //�����û����Ƶ�ͼ��
		//����������Ա༭
		//SilenAttributeDialog sileAttrDlg(vlayer,&nFeature,true);
		//int ret = sileAttrDlg.exec();
		//editFeatureAttr(vlayer, &nFeature);
		vlayer->editBuffer()->addFeature(nFeature);
		vlayer->select(nFeature.id());
		editFeatureAttr(vlayer, &nFeature);
		//sileAttrDlg->init(layer,&nFeature,QgsAttributeEditorContext(),true);
		//sileAttrDlg.init()
		//vlayer->editBuffer()->addFeature(nFeature);
		return;
	}
	else if (geoType == QgsWkbTypes::Polygon && lyrGeoType == QgsWkbTypes::MultiPolygon)
	{
		QgsFeature nFeature;
		nFeature.initAttributes(vlayer->fields().count());
		nFeature.setGeometry(geos); //�����û����Ƶ�ͼ��
		//editFeatureAttr(vlayer, &nFeature);
		//SilenAttributeDialog sileAttrDlg(vlayer, &nFeature, true);
		//int ret = sileAttrDlg.exec();
		vlayer->editBuffer()->addFeature(nFeature);
		vlayer->select(nFeature.id());
		editFeatureAttr(vlayer, &nFeature);
		return;
	}
	else if (geoType == QgsWkbTypes::MultiPolygon && lyrGeoType == QgsWkbTypes::Polygon)
	{
		QVector<QgsGeometry> inGeos = geos.asGeometryCollection();
		if (inGeos.count() <= 0)return;
		Q_FOREACH(QgsGeometry geo, inGeos)//ѭ�����еļ���ͼ�λ�ȡ��Ҫ��ͼ������
		{
			if (!geo.isEmpty()) {
				QgsWkbTypes::Type geoType2 = geo.wkbType();
				if (geoType2 == lyrGeoType)//���������������ͬ��ֱ�ӱ��棬������Ҫ�ֽ�Ϊ�Ӳ��ֽ��д���
				{
					QgsFeature nFeature;
					nFeature.initAttributes(vlayer->fields().count());
					nFeature.setGeometry(geos); //�����û����Ƶ�ͼ��						
					//SilenAttributeDialog sileAttrDlg(vlayer, &nFeature, true);
					//int ret = sileAttrDlg.exec();
					vlayer->editBuffer()->addFeature(nFeature);
					vlayer->select(nFeature.id());
					editFeatureAttr(vlayer, &nFeature);
					return;
				}
				else
					if (geoType2 == QgsWkbTypes::MultiPolygon)//�ಿ�������ֽ�ʹ���Feature����
						saveGeometry(vlayer, geo);
			}
		}
	}
}
//�޸�Ҫ�صļ���ͼ��
bool qgsGeometryEditUtils::changeFeatureGeo(QgsVectorLayer* vlayer, int fid, QgsGeometry* inGeo, bool changed)
{
	if (!inGeo || inGeo->isEmpty())return false;
	QgsWkbTypes::Type lyrGeoType = vlayer->wkbType();
	QgsWkbTypes::Type geoType = inGeo->wkbType();

	if (geoType == lyrGeoType)//��������ͼ����ͼ���ͼ��һ����ֱ���滻Ҫ�صļ���ͼ��
	{
		vlayer->editBuffer()->changeGeometry(fid, *inGeo);
		return true;
	}
	//���Ҫ��Ϊ
	else if (geoType == QgsWkbTypes::Polygon && lyrGeoType == QgsWkbTypes::MultiPolygon)
	{
		QgsGeometry mgeo = QgsGeometry::fromMultiPolygonXY(inGeo->asMultiPolygon());
		if (!mgeo.isEmpty())
			vlayer->editBuffer()->changeGeometry(fid, mgeo);
		return true;
	}
	else if (geoType == QgsWkbTypes::MultiPolygon && lyrGeoType == QgsWkbTypes::Polygon)
	{
		QVector<QgsGeometry> inGeos = inGeo->asGeometryCollection();
		if (inGeos.count() <= 0)return false;
		bool changeOld = changed;//Ҫ�صļ��������Ƿ��Ѿ�����
		Q_FOREACH(QgsGeometry geo, inGeos)//ѭ�����еļ���ͼ�λ�ȡ��Ҫ��ͼ������
		{
			if (!geo.isEmpty()) {
				QgsWkbTypes::Type geoType2 = geo.wkbType();
				if (geoType2 == lyrGeoType)//���������������ͬ��ֱ�ӱ��棬������Ҫ�ֽ�Ϊ�Ӳ��ֽ��д���
				{
					if (!changeOld)
					{
						vlayer->editBuffer()->changeGeometry(fid, geo);
						changeOld = true;
					}
					else
					{
						QgsFeature nFeature;
						nFeature.initAttributes(vlayer->fields().count());
						nFeature.setGeometry(geo); //�����û����Ƶ�ͼ��
						vlayer->editBuffer()->addFeature(nFeature);
					}
				}
				else
					if (geoType2 == QgsWkbTypes::MultiPolygon)//�ಿ�������ֽ�ʹ���Feature����
						changeOld = changeFeatureGeo(vlayer, fid, &geo, changeOld);//  saveGeometry(vlayer,geo);
			}
		}
		return changeOld;
	}
	return false;
}

//ͼ��Ҫ�ضԶ���ηָ���������طָ��Ķ����
QList<QgsGeometry> qgsGeometryEditUtils::intersectsGeometry(QgsVectorLayer* vlayer, const QgsGeometry* drawGeo)
{
	QList<QgsGeometry> geometryList;
	if (!vlayer || !drawGeo)
		return geometryList;
	QgsGeometry pgeometry;
	bool boverlay = false;

	//read avoid intersections list from project properties
	bool listReadOk;
	QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry("Digitizing", "/AvoidIntersectionsList", QStringList(), &listReadOk);
	if (listReadOk)
	{
		//go through list, convert each layer to vector layer and call QgsVectorLayer::removePolygonIntersections for each
		QgsVectorLayer* currentLayer = nullptr;
		QStringList::const_iterator aIt = avoidIntersectionsList.constBegin();
		for (; aIt != avoidIntersectionsList.constEnd(); ++aIt)
		{
			currentLayer = dynamic_cast<QgsVectorLayer*>(QgsProject::instance()->mapLayer(*aIt));
			//currentLayer = dynamic_cast<QgsVectorLayer*>(QgsMapLayerRegistry::instance()->mapLayer(*aIt));
			if (currentLayer)
			{
				QgsFeatureIterator fi = currentLayer->getFeatures(QgsFeatureRequest(drawGeo->boundingBox())
					.setFlags(QgsFeatureRequest::ExactIntersect));
				QgsFeature f;
				while (fi.nextFeature(f))
				{
					if (!f.hasGeometry())
						continue;
					QgsGeometry g = f.geometry();

					if (boverlay)
					{
						if (g.isEmpty() || !g.intersects(pgeometry))
							continue;
						if (g.contains(pgeometry))//���������ж�����ڣ����ָ�
						{
							geometryList.append(pgeometry);
							break;
						}

						QgsGeometry intesect = pgeometry.intersection(g);//����Ҫ��ͼ�������ͼ���ص�����
						if (intesect.isEmpty() == false)
							geometryList.append(intesect);
						pgeometry = pgeometry.difference(g);
					}
					else
					{
						if (g.isEmpty() || !g.intersects(*drawGeo))
							continue;
						if (g.contains(*drawGeo))//���������ж�����ڣ����ָ�
						{
							break;
						}

						QgsGeometry intesect = drawGeo->intersection(g);//����Ҫ��ͼ�������ͼ���ص�����
						if (intesect.isEmpty() == false)
						{
							geometryList.append(intesect);
							boverlay = true;
						}

						pgeometry = drawGeo->difference(g);
					}
				}
			}
		}
	}
	if (!pgeometry.isEmpty())
		geometryList.append(pgeometry);

	return geometryList;
}
//�༭���֮��ϵͳ�������Դ����ڱ༭��������
void qgsGeometryEditUtils::editFeatureAttr(QgsVectorLayer* vlayer, QgsFeature* feature)
{
	/*������ʾûЧ��
	QgsAttributeEditorContext context;

	QgsDistanceArea myDa;

	myDa.setSourceCrs(vlayer->crs());
	myDa.setEllipsoidalMode(QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled());
	myDa.setEllipsoid(QgsProject::instance()->readEntry("Measure", "/Ellipsoid", GEO_NONE));
	;
	context.setDistanceArea(myDa);
	context.setVectorLayerTools(QgisApp::instance()->vectorLayerTools());
	context.setFormMode(QgsAttributeEditorContext::StandaloneDialog);

	QgsAttributeDialog *dialog = new QgsAttributeDialog(vlayer, feature,
			false, QgisApp::instance()->mapCanvas()->parentWidget(), true, context);
	dialog->setMode(QgsAttributeForm::MultiEditMode);
	dialog->setWindowFlags(dialog->windowFlags() | Qt::Tool);
	dialog->setObjectName(QString("featureactiondlg:%1:%2").arg(vlayer->id()).arg(feature->id()));
	//dialog->setHighlight(h);
	// delete the dialog when it is closed
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
	*/

	//���´������ʵ�ֱ༭��ʵ��Ҫ�ص����Ա༭��
	//�����Ƚ�Ҫ�������ͼ���к��ٱ༭
	//SilenAttributeDialog sileAttrDlg(vlayer, feature, true);
	//sileAttrDlg.attributeForm()->setMode(QgsAttributeEditorContext::MultiEditMode);
	//int ret = sileAttrDlg.exec();
	//vlayer->deselect(feature->id());
}