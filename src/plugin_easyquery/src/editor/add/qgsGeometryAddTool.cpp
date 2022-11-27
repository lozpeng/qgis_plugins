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
//键盘释放事件，当用户敲击键盘之后弹出的操作
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
	//判断当前输入的键 坐标输入窗口是否显示，如果显示了。则不再处理，否则显示坐标
	//
	if (e->key() == Qt::Key::Key_F6
		|| (e->modifiers() == Qt::ControlModifier 
				&& e->key() == Qt::Key::Key_G)) //如果是F6键则弹出
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
	if (e->key() == Qt::Key::Key_C) //! 如果在编辑状态时按下C键时，开启地图漫游工具，漫游地图
	{
		//this->canvas()->setMapTool(new qgsMapPanTool()); //设置工具


	}
}


//用户鼠标弹起来时处理事件
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

		//获取用户绘制的多边形，然后从当前图层中获取相交的要素
		QgsRectangle bBox = drawGeo->boundingBox();
		QgsFeatureRequest fr;
		fr = fr.setFilterRect(bBox);
		fr.setFlags(QgsFeatureRequest::ExactIntersect);

		QgsFeatureIterator fit = vlayer->getFeatures(fr);
		int count(0);
		QgsFeature f;
		//先根据矩形框获取要素数目，一般不建议超过5个多边形，如果超过5个则表明用户绘制的多边形太大
		while (fit.nextFeature(f))
		{
			QgsGeometry g = f.geometry();
			if (!g.isEmpty() && g.intersects(*drawGeo))
				count++;
		}
		if (count <= 0)//表明当前编辑图层中没有与之相交，则直接新建,这里到时需要添加规则或者不需要保存直接返回
		{
			QgsFeature nFeature;
			nFeature.initAttributes(vlayer->fields().count());
			nFeature.setGeometry(*drawGeo); //保存用户绘制的图形
			//SilenAttributeDialog sileAttrDlg(vlayer, &nFeature, true);
			//int ret = sileAttrDlg.exec();
			//qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
			vlayer->editBuffer()->addFeature(nFeature);
			//this->mCanvas->refresh();
			vlayer->select(nFeature.id());
			qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
			return;
		}
		int opType = 0;//操作类型，0.新增，1.切，2.分割
		QString msg;
		if (count >= 1)
		{
			msg = tr("There are %1 polygons intersect with your draw,Do you want to:").arg(count);
			QMessageBox msgBox(QMessageBox::Question, tr("Add Polygon"), msg);
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Yes);
			msgBox.setButtonText(QMessageBox::Yes, tr("Clip"));//指使用绘制多边形将本底图形进行切割，
			msgBox.setButtonText(QMessageBox::YesAll, tr("Split"));//指基于本底图形对绘制图形进行分割
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
//======================================添加要素通过自由手绘方式添加要素========================================================
//应该要根据指定的目的图层类型来绘制不同的要素类型
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
		//判断不同类型构建rubberBand
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

//!工具被激活时需要获取当前编辑活动图层
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
			//获取用户绘制的多边形，然后从当前图层中获取相交的要素
			QgsRectangle bBox = drawGeo.boundingBox();
			QgsFeatureRequest fr;
			fr = fr.setFilterRect(bBox);
			fr.setFlags(QgsFeatureRequest::ExactIntersect);

			QgsFeatureIterator fit = vlayer->getFeatures(fr);
			int count(0);
			QgsFeature f;
			//先根据矩形框获取要素数目，一般不建议超过5个多边形，如果超过5个则表明用户绘制的多边形太大
			while (fit.nextFeature(f))
			{
				QgsGeometry g = f.geometry();
				if (!g.isEmpty() && g.intersects(drawGeo))
					count++;
			}
			if (count <= 0)//表明当前编辑图层中没有与之相交，则直接新建,这里到时需要添加规则或者不需要保存直接返回
			{
				QgsFeature nFeature;
				nFeature.initAttributes(vlayer->fields().count());
				nFeature.setGeometry(drawGeo); //保存用户绘制的图形
				//SilenAttributeDialog sileAttrDlg(vlayer, &nFeature, true);
				//qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
				//int ret = sileAttrDlg.exec();
				vlayer->editBuffer()->addFeature(nFeature);
				vlayer->select(nFeature.id());
				qgsGeometryEditUtils::editFeatureAttr(vlayer, &nFeature);
				return;
			}
			int opType = 0;//操作类型，0.新增，1.切，2.分割
			QString msg;
			if (count >= 1)
			{
				msg = QString::fromLocal8Bit("共有 %1 与绘制的多边形重叠，请选择:").arg(count);
				QMessageBox msgBox(QMessageBox::Question, QString::fromLocal8Bit("添加多边形"), msg);
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::Cancel);
				msgBox.setDefaultButton(QMessageBox::Yes);
				msgBox.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("切  割"));//指使用绘制多边形将本底图形进行切割，
				msgBox.setButtonText(QMessageBox::YesAll, QString::fromLocal8Bit("分  割"));//指基于本底图形对绘制图形进行分割
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
			//处理几何数据，并将结果保存至图层中
			qgsGeometryEditUtils::processGeometryTopo(vlayer, &drawGeo, opType);

			this->mCanvas->refresh();
		}
	}
	mRubberBand->reset(QgsWkbTypes::GeometryType::PolygonGeometry);
	delete mRubberBand;
	mRubberBand = nullptr;
	mDragging = false;
}

///=============================Utils类
//处理绘制的要素
//void qgsGeometryEditUtils::processGeometry(QgsVectorLayer *vlayer,const QgsGeometry* drawGeo)
//{
//	if(!vlayer || !drawGeo)return ;
//	//获取用户绘制的多边形，然后从当前图层中获取相交的要素
//	QgsRectangle bBox = drawGeo->boundingBox();
//	QgsFeatureRequest fr ;
//	fr= fr.setFilterRect( bBox );
//	fr.setFlags( QgsFeatureRequest::ExactIntersect );

//	QgsFeatureIterator fit =  vlayer->getFeatures(fr );
//	int count(0);
//	QgsFeature f;
//	//先根据矩形框获取要素数目，一般不建议超过5个多边形，如果超过5个则表明用户绘制的多边形太大
//	while ( fit.nextFeature( f ) )
//	{
//		QgsGeometry* g = f.geometry();
//		if(!g->isEmpty() && g->intersects(drawGeo))
//			count++;
//	}
//	if(count <= 0)//表明当前编辑图层中没有与之相交，则直接新建,这里到时需要添加规则或者不需要保存直接返回
//	{
//		QgsFeature nFeature;
//		nFeature.initAttributes(vlayer->fields().count());
//		nFeature.setGeometry(*drawGeo); //保存用户绘制的图形
//		vlayer->editBuffer()->addFeature(nFeature);
//		//this->mCanvas->refresh();
//		return ;
//	}
//	int opType =0;//操作类型，0.新增，1.切，2.分割
//	QString msg;
//	if(count>=1)
//	{
//		msg = tr("There are %1 polygons intersect with your draw,Do you want to:").arg(count);
//		QMessageBox msgBox(QMessageBox::Question,tr("Add Polygon"),msg);
//		msgBox.setStandardButtons( QMessageBox::Yes|QMessageBox::YesAll|QMessageBox::Cancel);
//		msgBox.setDefaultButton(QMessageBox::Yes);
//		msgBox.setButtonText( QMessageBox::Yes,tr("Clip"));//指使用绘制多边形将本底图形进行切割，
//		msgBox.setButtonText(QMessageBox::YesAll,tr("Split"));//指基于本底图形对绘制图形进行分割
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
//		//1.计算要素的几何图形与用户绘制的几何图形不相同的部分用于更新当前要素的几何部分
//		//这里可以自定义的对某些字段进行按面积比例分配计算
//		QgsGeometry* fGeo = g->difference(drawGeo); //计算要素图形与绘制图形不重叠部分
//		//g->(drawGeo);
//		QgsGeometry* iterGeos = drawGeo->intersection(g);
//		//2.对计算出来的不相同部分图形进行类型判断与处理，当类型与目标图层一致时进行数据保存
//		if(fGeo && !fGeo->isEmpty())//如果差异部分图形存在则需要更新原来要素图形
//		{
//			bool changed = qgsGeometryEditUtils::changeFeatureGeo(vlayer, f.id(), fGeo,false);
//		}
//		if(!iterGeos || iterGeos->isEmpty())continue;
//		//如果是Clip则，对绘制图形完全包括要素进行删除处理
//		if(opType ==1){
//			if(drawGeo->contains(g))
//				vlayer->editBuffer()->deleteFeature(f.id());
//			continue;
//		}
//		if(opType==2)//新建相交部分的几何图形为一个要素
//			qgsGeometryEditUtils::saveGeometry(vlayer,iterGeos);
//		//计算用户绘制的多余部分图形,即不与任何图形相交的部分图形，直接保存为新的要素
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
//		//nFeature.setGeometry(*drawGeo); //保存用户绘制的图形
//		//vlayer->editBuffer()->addFeature(nFeature);
//	}
//	if(diffGeo && !diffGeo->isEmpty())//如果剩下的图形不为空则直接创建要素
//	{
//		QgsFeature nFeature;
//		nFeature.initAttributes(vlayer->fields().count());
//		nFeature.setGeometry(*diffGeo); //保存用户绘制的图形
//		vlayer->editBuffer()->addFeature(nFeature);
//	}
//	vlayer->endEditCommand();
//}

//分别处理分割与剪裁
void qgsGeometryEditUtils::processGeometryTopo(QgsVectorLayer* vlayer, const QgsGeometry* drawGeo, int opType)
{
	if (!vlayer->isEditable())
	{
		QMessageBox::about(nullptr, QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("图层必须处于可编辑状态！"));
		return;
	}
	//int opType =0;//操作类型，0.新增，1.切，2.分割
	if (!vlayer || !drawGeo)
		return;
	QList<QgsGeometry> intesectlist;
	if (opType == 1)//剪裁，判断是否与边界约束多边形相交
	{
		intesectlist = intersectsGeometry(vlayer, drawGeo);
		int count = intesectlist.count();
		if (count > 0)//与边界有相交
		{
			for (int i = 0; i < count; i++)
			{
				QgsGeometry pgeometry = intesectlist[i];
				processGeometry(vlayer, pgeometry, opType);
			}
			return;
		}
	}


	//获取用户绘制的多边形，然后从当前图层中获取相交的要素
	processGeometry(vlayer, *drawGeo, opType);
}

//分别处理分割与剪裁
void qgsGeometryEditUtils::processGeometry(QgsVectorLayer* vlayer, const QgsGeometry drawGeo, int opType)
{
	if (!vlayer->isEditable())
	{
		QMessageBox::about(nullptr, QString::fromLocal8Bit("错误"), 
										QString::fromLocal8Bit("图层必须处于编辑状态！"));
		return;
	}
	//int opType =0;//操作类型，0.新增，1.切，2.分割
	if (!vlayer || drawGeo.isEmpty())
		return;
	//QList<QgsGeometry*> intesectlist;
	//if(opType == 1)//剪裁，判断是否与边界约束多边形相交
	//{	
	//	intesectlist = intersectsGeometry(vlayer,drawGeo);
	//	int count = intesectlist.count();
	//	if(count>0)//与边界有相交
	//	{
	//		QgsGeometry* pgeometry = nullptr;
	//		for(int i=0; i<count; i++)
	//		{
	//			pgeometry = intesectlist[i];
	//		}
	//		return;
	//	}
	//}


	//获取用户绘制的多边形，然后从当前图层中获取相交的要素
	QgsRectangle bBox = drawGeo.boundingBox();
	QgsFeatureRequest fr;
	fr = fr.setFilterRect(bBox);
	fr.setFlags(QgsFeatureRequest::ExactIntersect);

	QgsFeatureIterator fit = vlayer->getFeatures(fr);
	int count(0);
	QgsFeature f;
	//先根据矩形框获取要素数目，一般不建议超过5个多边形，如果超过5个则表明用户绘制的多边形太大
	while (fit.nextFeature(f))
	{
		QgsGeometry g = f.geometry();
		if (!g.isEmpty() && g.intersects(drawGeo))
			count++;
	}
	if (count <= 0)//表明当前编辑图层中没有与之相交，则直接新建,这里到时需要添加规则或者不需要保存直接返回
	{
		//这里最好弹出属性编辑框进行属性编辑
		qgsGeometryEditUtils::saveGeometry(vlayer, drawGeo);
		//QgsFeature nFeature;
		//nFeature.initAttributes(vlayer->fields().count());
		//nFeature.setGeometry(*drawGeo); //保存用户绘制的图形
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
		//1.计算要素的几何图形与用户绘制的几何图形不相同的部分用于更新当前要素的几何部分
		//这里可以自定义的对某些字段进行按面积比例分配计算
		QgsGeometry fGeo = g.difference(drawGeo); //计算要素图形与绘制图形不重叠部分
		//g->(drawGeo);
		QgsGeometry iterGeos = drawGeo.intersection(g);
		//2.对计算出来的不相同部分图形进行类型判断与处理，当类型与目标图层一致时进行数据保存
		if (!fGeo.isEmpty())//如果差异部分图形存在则需要更新原来要素图形
		{
			bool changed = qgsGeometryEditUtils::changeFeatureGeo(vlayer, f.id(), &fGeo, false);
		}
		if (iterGeos.isEmpty())continue;
		//如果是Clip则，对绘制图形完全包括要素进行删除处理
		if (opType == 1) {
			if (drawGeo.contains(g))
				vlayer->editBuffer()->deleteFeature(f.id());
			continue;
		}
		if (opType == 2)//新建相交部分的几何图形为一个要素
			qgsGeometryEditUtils::saveGeometry(vlayer, iterGeos);
		//计算用户绘制的多余部分图形,即不与任何图形相交的部分图形，直接保存为新的要素
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
		//nFeature.setGeometry(*drawGeo); //保存用户绘制的图形
		//vlayer->editBuffer()->addFeature(nFeature);
	}
	if (!diffGeo.isEmpty())//如果剩下的图形不为空则直接创建要素
	{
		QgsFeature nFeature;
		nFeature.initAttributes(vlayer->fields().count());
		nFeature.setGeometry(diffGeo); //保存用户绘制的图形
		vlayer->editBuffer()->addFeature(nFeature);
		vlayer->select(nFeature.id());
		editFeatureAttr(vlayer, &nFeature);
	}
	vlayer->endEditCommand();
}

//新建要素保存数据
void qgsGeometryEditUtils::saveGeometry(QgsVectorLayer* vlayer, const QgsGeometry geos)
{
	QgsWkbTypes::Type lyrGeoType = vlayer->wkbType();
	QgsWkbTypes::Type geoType = geos.wkbType();
	//这里只对WkbPolygon和WkbMultiPolygon进行分析处理和数据保存的工作
	if (geoType == lyrGeoType)//如果几何图形的类型与图层相同则直接保存
	{
		QgsFeature nFeature;
		nFeature.initAttributes(vlayer->fields().count());
		nFeature.setGeometry(geos); //保存用户绘制的图形
		//这里进行属性编辑
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
		nFeature.setGeometry(geos); //保存用户绘制的图形
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
		Q_FOREACH(QgsGeometry geo, inGeos)//循环所有的几何图形获取需要的图形数据
		{
			if (!geo.isEmpty()) {
				QgsWkbTypes::Type geoType2 = geo.wkbType();
				if (geoType2 == lyrGeoType)//如果两个的类型相同则直接保存，否则需要分解为子部分进行处理
				{
					QgsFeature nFeature;
					nFeature.initAttributes(vlayer->fields().count());
					nFeature.setGeometry(geos); //保存用户绘制的图形						
					//SilenAttributeDialog sileAttrDlg(vlayer, &nFeature, true);
					//int ret = sileAttrDlg.exec();
					vlayer->editBuffer()->addFeature(nFeature);
					vlayer->select(nFeature.id());
					editFeatureAttr(vlayer, &nFeature);
					return;
				}
				else
					if (geoType2 == QgsWkbTypes::MultiPolygon)//多部多边形则分解和创建Feature保存
						saveGeometry(vlayer, geo);
			}
		}
	}
}
//修改要素的几何图形
bool qgsGeometryEditUtils::changeFeatureGeo(QgsVectorLayer* vlayer, int fid, QgsGeometry* inGeo, bool changed)
{
	if (!inGeo || inGeo->isEmpty())return false;
	QgsWkbTypes::Type lyrGeoType = vlayer->wkbType();
	QgsWkbTypes::Type geoType = inGeo->wkbType();

	if (geoType == lyrGeoType)//如果输入的图形与图层的图形一致则直接替换要素的几何图形
	{
		vlayer->editBuffer()->changeGeometry(fid, *inGeo);
		return true;
	}
	//如果要素为
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
		bool changeOld = changed;//要素的几何数据是否已经修正
		Q_FOREACH(QgsGeometry geo, inGeos)//循环所有的几何图形获取需要的图形数据
		{
			if (!geo.isEmpty()) {
				QgsWkbTypes::Type geoType2 = geo.wkbType();
				if (geoType2 == lyrGeoType)//如果两个的类型相同则直接保存，否则需要分解为子部分进行处理
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
						nFeature.setGeometry(geo); //保存用户绘制的图形
						vlayer->editBuffer()->addFeature(nFeature);
					}
				}
				else
					if (geoType2 == QgsWkbTypes::MultiPolygon)//多部多边形则分解和创建Feature保存
						changeOld = changeFeatureGeo(vlayer, fid, &geo, changeOld);//  saveGeometry(vlayer,geo);
			}
		}
		return changeOld;
	}
	return false;
}

//图层要素对多边形分割操作，返回分割后的多边形
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
						if (g.contains(pgeometry))//包含在已有多边形内，不分割
						{
							geometryList.append(pgeometry);
							break;
						}

						QgsGeometry intesect = pgeometry.intersection(g);//计算要素图形与绘制图形重叠部分
						if (intesect.isEmpty() == false)
							geometryList.append(intesect);
						pgeometry = pgeometry.difference(g);
					}
					else
					{
						if (g.isEmpty() || !g.intersects(*drawGeo))
							continue;
						if (g.contains(*drawGeo))//包含在已有多边形内，不分割
						{
							break;
						}

						QgsGeometry intesect = drawGeo->intersection(g);//计算要素图形与绘制图形重叠部分
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
//编辑完成之后系统弹出属性窗用于编辑属性数据
void qgsGeometryEditUtils::editFeatureAttr(QgsVectorLayer* vlayer, QgsFeature* feature)
{
	/*高亮提示没效果
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

	//以下代码可以实现编辑，实现要素的属性编辑，
	//建议先将要素添加至图层中后再编辑
	//SilenAttributeDialog sileAttrDlg(vlayer, feature, true);
	//sileAttrDlg.attributeForm()->setMode(QgsAttributeEditorContext::MultiEditMode);
	//int ret = sileAttrDlg.exec();
	//vlayer->deselect(feature->id());
}