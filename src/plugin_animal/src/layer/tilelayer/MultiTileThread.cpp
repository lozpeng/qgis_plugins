#include <algorithm>
#include <cmath>
#include <QDir>
#include <QBuffer>
#include <QByteArray>
#include <QFile>

#include <qgsmaplayer.h>
#include <qgsmaprenderercache.h>
#include <qgsmapcanvasmap.h>
#include <qgsvectorlayer.h>
#include <qgsgeometry.h>

#include <QStyleOptionGraphicsItem>

#include "MercatorDefine.h"
#include "MultiTileThread.h"
#include "SilenTileDb.h"
#include "qgslogger.h"
#include <time.h>
#include "HttpFun.h"
#include <qgsmaprendererjob.h>
#include <qgsmaprenderercustompainterjob.h>
#include "common.h"
#include <Shlwapi.h> 
#include <qgslayertreegroup.h>
#include <QgsProject.h>
#include <QgsLayerTreeLayer.h>


#define	GRID_SIZE  20
#define MAX_LISTCOUNT	100  //队列里待处理数据的最大个数

CBlockThread::CBlockThread(CCutEDOMThread *pCutEdom, QObject* parent) : QThread(parent)
{
	mStopThread = false;
	m_pCutEdom = pCutEdom;
	mDataValid = false;
}

CBlockThread::~CBlockThread()
{

}

void CBlockThread::Init(CGeoPolygon *DataBoundPolygon, CEDomUtils *pUtils, EV_DB_Header *header, CWriteTileDb	*TileStore)
{
	m_pDataBoundPolygon = DataBoundPolygon;
	m_pUtils = pUtils;
	m_header = header;
	mTileStore = TileStore;
}

void CBlockThread::Fini()
{ 
	mStopThread = true; 
}

bool CBlockThread::AddData(LONGLONG tid)
{
	if (mDataValid)
		return false;

	m_tid = tid;
	mDataValid = true;
	return true;
}

void CBlockThread::run()
{
	while (!mStopThread) {
		if (mDataValid)
		{
			//标明正在处理
			ProcessBlockData(m_pDataBoundPolygon, m_tid, m_pUtils, m_header);
			mDataValid = false;
		}
		msleep(300);
	}
}

//判断pDataBoundPolygon与的关系
int CBlockThread::Block_Polygon(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId, CEDomUtils *pTileUtils)
{
	//判断关系，如果相交返回0，
	//如果pDataBoundPolygon包含tileId返回1，
	//如果tileId包含pDataBoundPolygon多边形返回2
	//如果pDataBoundPolygon和tileId没有任何关系，返回3
	if (!pDataBoundPolygon || pDataBoundPolygon->m_polyon.size() < 1)
		return 3;

	ENVELOPE tileRect;//输出块的范围
	pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);
	CGeoPolygon img_polygon;
	CGeoPoint points;
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.miny));
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.miny));
	img_polygon.m_polyon.append(points);
	img_polygon.UpdateBound();
	//判断多边形是否相交
	return pDataBoundPolygon->PolygonInPolygon(&img_polygon);
}

void CBlockThread::ProcessBlockData(CGeoPolygon *pDataBoundPolygon,
	LONGLONG tileId, CEDomUtils *pTileUtils, EV_DB_Header *header)
{
	int level = BLOCK_ID_LEVEL(tileId);
	if (level > header->maxlevel)
		return;

	//判断关系，如果相交返回0，
	//如果pDataBoundPolygon包含tileId返回1，
	//如果tileId包含pDataBoundPolygon多边形返回2
	//如果pDataBoundPolygon和tileId没有任何关系，返回3
	int result = Block_Polygon(m_pDataBoundPolygon, tileId, pTileUtils);
	switch (result)
	{
	case 0://相交返回0，
	case 2:
	{
		QImage image;
		CutImage(m_pDataBoundPolygon, tileId, pTileUtils, &image);
		m_pCutEdom->UnitBlock(mTileStore,tileId, image);
		//开始进入子块
		LONGLONG child_ids[4];
		SilenTileUtils::GetLowIds(tileId, child_ids);//取得子块的ID编号
		for (int index = 0; index < 4; index++)
		{
			//递归处理
			ProcessBlockData(pDataBoundPolygon,
				child_ids[index], pTileUtils,
				header);
		}
	}
	break;
	case 3://不相关
		DelBlockData(tileId, header);//删除
		break;
	case 1://包含则不做任何处理返回
		break;
	}


}

void CBlockThread::CutImage(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId, CEDomUtils *pTileUtils, QImage *outmap)
{
	////图像裁剪，这个裁剪是对超过数据范围的设置为透明
	ENVELOPE tileRect;//输出块的范围
	pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);
	QPolygon poly;
	int lx, ly, point_count = pDataBoundPolygon->m_polyon[0].m_points.size();
	for (int n = 0; n < point_count; n++)
	{
		double dx = pDataBoundPolygon->m_polyon[0].m_points[n].x();
		double dy = pDataBoundPolygon->m_polyon[0].m_points[n].y();

		lx = (dx - tileRect.minx) * 256 / (tileRect.maxx - tileRect.minx);
		ly = (tileRect.maxy - dy) * 256 / (tileRect.maxy - tileRect.miny);
		poly.append(QPoint(lx, ly));
	}

	QImage bk_image(256,256,QImage::Format::Format_ARGB32);//创建一个256*256的图片
	bk_image.fill(QColor(0, 0, 0, 0));
	QPainter thePainter(&bk_image);
	thePainter.setRenderHint(QPainter::Antialiasing);
	thePainter.setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
	thePainter.setBrush(QBrush(QColor(255, 255, 255, 255), Qt::SolidPattern));
	thePainter.drawPolygon(poly, Qt::WindingFill);
	thePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	*outmap = bk_image.copy();
}

//删除数据块以及子块
void CBlockThread::DelBlockData(LONGLONG tileId, EV_DB_Header *header)
{
	int level = BLOCK_ID_LEVEL(tileId);
	if (level > header->maxlevel)
		return;

	if (!m_pCutEdom->ExitTile(mTileStore,tileId))
		return;

	m_pCutEdom->DelBlock(mTileStore,tileId);
	DelChildBlockData(tileId, header);
}

//删除子块
void CBlockThread::DelChildBlockData(LONGLONG tileId, EV_DB_Header *header)
{
	int level = BLOCK_ID_LEVEL(tileId);
	if (level == header->maxlevel)
		return;

	LONGLONG child_ids[4];
	SilenTileUtils::GetLowIds(tileId, child_ids);//取得子块的ID编号
	for (int index = 0; index < 4; index++)
	{
		if (m_pCutEdom->ExitTile(mTileStore,child_ids[index]))
			DelChildBlockData(child_ids[index], header);

		m_pCutEdom->DelBlock(mTileStore,child_ids[index]);
	}
}

////////////////////////////////////////////////////////////////////
CCutEDOMThread::CCutEDOMThread(QString edompath, CGeoPolygon *DataBoundPolygon, int ProcessThreadCount, QObject* parent)
	: QThread(parent)
	, m_EdomPath(edompath)
	, m_ProcessThreadCount(ProcessThreadCount)
	, m_pDataBoundPolygon(DataBoundPolygon)
{
	mStop = false;
	for (int i = 0; i < m_ProcessThreadCount; i++)
	{
		CBlockThread *p = new CBlockThread(this,parent);
		m_BlockThreads.push_back(p);
	}
}

CCutEDOMThread::~CCutEDOMThread()
{
	for (int i = 0; i < m_ProcessThreadCount; i++)
		SAFE_DELETE(m_BlockThreads[i]);

	m_BlockThreads.clear();
}

bool CCutEDOMThread::ExitTile(CWriteTileDb *pTileDb, LONGLONG tid)
{
	mBlockDBMutex.lock();
	bool bExit = pTileDb->ExitTile(tid);
	mBlockDBMutex.unlock();
	return bExit;
}
void CCutEDOMThread::UnitBlock(CWriteTileDb *pTileDb, LONGLONG tid, QImage bk_image)
{//加入需要合并的数据块
	mBlockDBMutex.lock();//锁定
	UnitImage(tid, pTileDb, &bk_image);
	mBlockDBMutex.unlock();
}

void CCutEDOMThread::DelBlock(CWriteTileDb *pTileDb, LONGLONG tid)
{//添加需要删除的数据块
	mBlockDBMutex.lock();//锁定
	m_DelTIDs.push_back(tid);
	mBlockDBMutex.unlock();
	DelBlockData(pTileDb);
}

void CCutEDOMThread::DelBlockData(CWriteTileDb *pTileDb, bool all)
{
	mBlockDBMutex.lock();//锁定
	int count = m_DelTIDs.size();
	if (count > 0) {
		if (!all)
			if (count > 50)	count = 50;

		bool re = pTileDb->delDatas(&m_DelTIDs[0], count);
		for (int i = count - 1; i >= 0; i--)
			m_DelTIDs.removeAt(i);
	}
	mBlockDBMutex.unlock();
}


void CCutEDOMThread::run()
{
	if (!m_pDataBoundPolygon)
	{
		signalMessage(QString::fromWCharArray(L"没有裁剪多边形数据"));
		return;
	}

	EV_DB_Header	mTileInfo;
	CWriteTileDb		mTileStore;
	clock_t start, finish;
	start = clock();
	if(!mTileStore.open(m_EdomPath))
	{
		signalMessage(QString::fromWCharArray(L"打开EDOM数据失败"));
		return;
	}
	mTileStore.ReadHeader(mTileInfo);
	int minLevel = mTileInfo.minlevel;
	int maxLevel = mTileInfo.maxlevel;
	/*
	处理方法:从最小的级别开始，
	1、如果块完全在裁剪区域范围内，则本块和子块不再递归处理
	2、如果块和裁剪区域相交，则处理，并进入子块判断
	3、如果块在裁剪区域范围外，本块以及子块全部删除
	*/
	int process_index;
	CEDomUtils utils;
	int level = minLevel;//从最小的级别开始，后面的递归
	for (process_index = 0; process_index < m_ProcessThreadCount; process_index++)
	{
		m_BlockThreads[process_index]->Init(m_pDataBoundPolygon,
			&utils, &mTileInfo, &mTileStore);
		m_BlockThreads[process_index]->start();
	}
	bool addover = false;
	signalMessage(QString::fromWCharArray(L"正在处理的数据级别范围(%1-%2)").arg(minLevel).arg(maxLevel));
	int min_lx = utils.GetIndex_X(mTileInfo.minx, level);
	int max_lx = utils.GetIndex_X(mTileInfo.maxx, level);
	int min_ly = utils.GetIndex_X(mTileInfo.miny, level);
	int max_ly = utils.GetIndex_X(mTileInfo.maxy, level);

	int per = 0;
	int value_count = (max_lx - min_lx + 1)* (max_ly - min_ly + 1);
	for (int ly = min_ly; ly <= max_ly; ly++)
	{
		for (int lx = min_lx; lx <= max_lx; lx++)
		{
			if (mStop)	break;
			per++;
			QgsRectangle block_rect;
			utils.GetBlockRect(lx, ly, level, block_rect);
			LONGLONG tid = BLOCK_ID(lx, ly, level);
			addover = false;
			while (!addover) {
				for (process_index = 0; process_index < m_ProcessThreadCount; process_index++)
				{
					if (m_BlockThreads[process_index]->AddData(tid))
					{
						addover = true;
						break;
					}
				}
				msleep(300);
			}
				
			//显示进度条
			signalProcess(per*100 / value_count);
		}
	}

	signalMessage(QString::fromWCharArray(L"正在后台进入最后的处理，请等待...."));
	while (true)
	{
		int process_count = 0;//还在处理的线程个数
		for (process_index = 0; process_index < m_ProcessThreadCount; process_index++)
		{
			if (!m_BlockThreads[process_index]->IsProcessging())
				process_count++;//表示已经处理完成的线程
		}
		msleep(100);
		DelBlockData(&mTileStore,true);
		if (process_count == m_ProcessThreadCount)
			break;
	}
	for (process_index = 0; process_index < m_ProcessThreadCount; process_index++)
		m_BlockThreads[process_index]->Fini();

	DelBlockData(&mTileStore,true);
	QgsRectangle *bound = m_pDataBoundPolygon->GetBound();
	mTileInfo.minx = bound->xMinimum();
	mTileInfo.maxx = bound->xMaximum();
	mTileInfo.miny = bound->yMinimum();
	mTileInfo.maxy = bound->yMaximum();
	mTileStore.DelSysTile();
	mTileStore.SaveSysTile(mTileInfo);
	mTileStore.close();

	finish = clock();//完成时间
	long seconds = (finish - start) / CLOCKS_PER_SEC;//秒
	QString strmsg;
	if (seconds <= 60) {
		strmsg = QString("%1").arg(seconds) + QString::fromWCharArray(L"秒");
	}
	else {
		long d = 0;
		long h = 0;
		long m = seconds / 60;
		seconds = seconds % 60;//得到秒
		if (m>60)
		{
			h = m / 60;
			m = m % 60;
		}
		if (h > 24)
		{//求出天数
			d = h / 24;
			h = h % 24;
		}

		wchar_t msg[256];
		if(d>0)
			swprintf(msg, L"%d天%d时%d分%d秒", d, h, m, seconds);
		else
			swprintf(msg, L"%d时%d分%d秒", h, m, seconds);
		strmsg = QString::fromWCharArray(msg);
	}
	QString tr_timeDiff = QString(QString::fromWCharArray(L"数据处理完成!共消耗时间：")+"%1").arg(strmsg); //float->QString
	signalMessage(tr_timeDiff);
	signalFinish();
}

void CCutEDOMThread::UnitImage(LONGLONG tileId, CWriteTileDb *pTileDb, QImage *bk_map)
{
	QByteArray data = pTileDb->read(tileId);
	data.remove(0, 8);//去掉前面的8个字节
	QImage block_image;
	if (!block_image.loadFromData(data))
		return;

	QPainter thePainter(bk_map);
	thePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	thePainter.drawImage(0, 0, block_image);
	//将图片转换为字节
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);

	std::string saveformat = "JPG";
	//接下来判断数据是否没有透明的，则采用JPG存储，否则采用PNG
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			QColor clr = bk_map->pixelColor(x, y);
			if (clr.alpha() == 0)//表示全透明
			{
				saveformat = "PNG";
				y = 300;//跳出循环
				break;
			}
		}
	}

	bk_map->save(&buffer, saveformat.c_str());

	int size = bytes.size();;
	char type[4];
	strcpy(type, saveformat.c_str());
	type[3] = '\0';
	QByteArray image_buffer;
	image_buffer.append(type, 4);
	image_buffer.append((const char*)&size, sizeof(int));
	image_buffer.append((const char*)bytes, size);
	pTileDb->UpdateData(tileId, image_buffer);
}

//////////////////////////////////////////////////////////
CCutTileThread::CCutTileThread(CTileUtils *utils, QObject* parent)
	 : QThread(parent)
	 , mPixmap(256*GRID_SIZE, 256*GRID_SIZE,QImage::Format_ARGB32)
	 , mAlphaPixmap(256 * GRID_SIZE, 256 * GRID_SIZE, QImage::Format_ARGB32)
{
	m_pTileUtils = utils;
}

  void CCutTileThread::Init(CMultiTileThread *parent, QgsMapCanvas* ifaceCanvas)
  {
	m_Parent = parent;
	mSettings = ifaceCanvas->mapSettings();
	QSize sz = mPixmap.size();
	//int dpi = mPixmap.logicalDpiX();
	//mSettings.setOutputDpi(dpi);
	mSettings.setOutputSize(sz);
	mSettings.setBackgroundColor(Qt::transparent);
//	mSettings.setFlag(QgsMapSettings::Antialiasing, true);//这行代码要去掉
	
	//mSettings.setCrsTransformEnabled(true);
	mAlphaSettings = mSettings;
	//设置绘制器的属性
	mpainter.setRenderHint(QPainter::Antialiasing, true);
	mpainter.setRenderHint(QPainter::TextAntialiasing, true);
	mpainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
	mpainter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	//apply DPI parameter if present. This is an extension of Qgis Mapserver compared to WMS 1.3.
	//Because of backwards compatibility, this parameter is optional
	//	double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
	//int dpm = 5669;//1 / OGC_PX_M;
	//mPixmap.setDotsPerMeterX(dpm);
	//mAlphaPixmap.setDotsPerMeterY(dpm);

	//构建Alpha图层对象
	InitAlphaSettings();
	////////////////////////////////////

	m_bRun = true;
	m_working = false;
	this->start();
}

void CCutTileThread::Fini()
{
	KillThread();
}

CCutTileThread::~CCutTileThread()
{

}

void CCutTileThread::InitAlphaSettings()
{//构建Alpha图层对象
	QList<QgsMapLayer*> Layers;
	InitAlphaData(Layers);
	mAlphaSettings.setLayers(Layers);
}

void CCutTileThread::InitAlphaData(QList<QgsMapLayer*> &Layers)
{
	if (!m_Parent)
		return;

	//有可能是树项，有可能是单个图层
	QString AlphaLayerName = m_Parent->m_AlphaTreeGroupName;
	if (AlphaLayerName.length() < 1)
		return;

	if (AlphaLayerName.left(7).compare("Group::") == 0)
	{
		AddTreeGroup(AlphaLayerName.right(AlphaLayerName.length() - 7), Layers);
	}
	else {
		if (AlphaLayerName.left(7).compare("Layer::") == 0)
		{
			AddMapLayer(AlphaLayerName.right(AlphaLayerName.length() - 7), Layers);
		}
	}
}

void CCutTileThread::AddTreeGroup(QString GroupName, QList<QgsMapLayer*> &Layers)
{
	QgsLayerTreeGroup *group = (QgsLayerTreeGroup*)QgsProject::instance()->layerTreeRoot();
	if (!group)
		return;

	QgsLayerTreeGroup *g = group->findGroup(GroupName);
	if (!g)
		return;

	QList<QgsLayerTreeLayer*> treelayer = g->findLayers();
	int i, count = treelayer.count();
	bool mapInit = false;
	for (i = 0; i<count; i++)
	{
		QgsMapLayer *layer = treelayer[i]->layer();
		// Add the Layer to the Layer Set
		Layers.append(layer);
	}
}

void CCutTileThread::AddMapLayer(QString LayerName, QList<QgsMapLayer*> &Layers)
{
	QList<QgsMapLayer*> mapLayers = QgsProject::instance()->mapLayersByName(LayerName);
	for (int i = 0; i<mapLayers.count(); i++)
	{
		QgsMapLayer *mypLayer = mapLayers[i];
		if (!mypLayer->isValid())
			continue;

		// Add the Layer to the Layer Set
		Layers.append(mypLayer);
	}
}

bool CCutTileThread::AddData(QgsRectangle outRect,int minx,int miny,int maxx,int maxy,
	int level,int minIndX,int minIndY,int maxIndX,int maxIndY)
{
	if(mOutRects.size() >= MAX_LISTCOUNT)
		return false;

	CutTileStru data;
	data.outRect = outRect;
	data.minx = minx;
	data.miny = miny;
	data.maxx = maxx;
	data.maxy = maxy;

	data.level = level;
	data.minIndX = minIndX;
	data.minIndY = minIndY;
	data.maxIndX = maxIndX;
	data.maxIndY = maxIndY;

	mOutRectMutex.lock();
	mOutRects.append(data);
	mOutRectMutex.unlock();
	return true;
}

void CCutTileThread::run()
{
	while(m_bRun)
	{//线程一直循环
		if(mOutRects.size()<1)
		{
			msleep(100);//毫秒
			continue;
		}

		BeginWork();//标记成本线程正在处理
		mOutRectMutex.lock();
		CutTileStru info = mOutRects[0];
		mOutRects.removeFirst();
		mOutRectMutex.unlock();

		//渲染地图至图片
		mSettings.setExtent(info.outRect);
		mPixmap.fill(Qt::transparent);//这行代码可以使背景透明

		mpainter.begin(&mPixmap);//
		QgsMapRendererCustomPainterJob job(mSettings, &mpainter);
		job.renderSynchronously();
		mpainter.end();

		//Alpha操作
		if (mAlphaSettings.layers().count() > 0) {
			mAlphaSettings.setExtent(info.outRect);
			mAlphaPixmap.fill(Qt::transparent);//这行代码可以使背景透明
			mpainter.begin(&mAlphaPixmap);//
			QgsMapRendererCustomPainterJob job(mAlphaSettings, &mpainter);
			job.renderSynchronously();
			mpainter.end();

			//开始处理Alpha图片
			for (int y = 0; y < mPixmap.height(); y++)
			{
				for (int x = 0; x < mPixmap.width(); x++)
				{
					QColor org_clr = mPixmap.pixelColor(x, y);
					if (org_clr.alpha() == 0)
						continue;
					QColor clr = mAlphaPixmap.pixelColor(x, y);
					if (clr.alpha() == 0)
						continue;//透明的部分不进行下去
					//不透明的部分需要把底图数据透明
					org_clr.setAlpha(255-clr.alpha());
					mPixmap.setPixelColor(x,y, org_clr);
				}
			}
		}
		/////////////////////////////

		if(mPixmap.height()>1 && mPixmap.width()>1)
		{
			//开始切割图片数据
			for(int ny = info.miny;ny<info.maxy;ny++)
			{
				if(ny>info.maxIndY)
					break;
				for(int nx = info.minx;nx<info.maxx;nx++)
				{
					if(nx>info.maxIndX)
						break;
					//将渲染的图片数据存储至瓦片数据库中
					LONGLONG id = BLOCK_ID(nx,ny,info.level);
					saveTileData(id,info.outRect,mPixmap);
				}
			}
		}

		EndWork();//标明本线程闲置
	}
}
//#define	COLOR_ERROR	2 //颜色的误差范围
//将瓦片集分割为256*256的小瓦片
void CCutTileThread::saveTileData(LONGLONG tileId,QgsRectangle MapRect,QImage &pixMap)
{
	ENVELOPE tileRect;//输出块的范围
	m_pTileUtils->GetBlockRect(BLOCK_ID_X(tileId),BLOCK_ID_Y(tileId),BLOCK_ID_LEVEL(tileId),tileRect);
	if (!m_Parent->IDInValidateBound(tileRect))
		return;

	if (!IsValid(m_Parent->GetBoundPolygon(), tileId))
		return;//不在区域范围内

	int height = pixMap.height();
	int width = pixMap.width();
	int start_lx = (tileRect.minx - MapRect.xMinimum()) * width / MapRect.width();//计算开始X号
	int start_ly = (MapRect.yMaximum() - tileRect.maxy) * height/ MapRect.height();

	QImage outmap = pixMap.copy(start_lx,start_ly,256,256);
	ShapeImage(m_Parent->GetBoundPolygon(), tileId,&outmap);//按多边形裁剪

	int x, y;
	//////////////////////////////
	std::string saveformat = "PNG";
	int quality = -1;
	if (m_Parent->m_bCompressTile)
	{
		//需要压缩
		quality = 75;//75%的压缩率
		saveformat = "JPG";
		//接下来判断数据是否没有透明的，则采用JPG存储，否则采用PNG
		for (y = 0; y < 256; y++)
		{
			for (x = 0; x < 256; x++)
			{
				QColor clr = outmap.pixelColor(x, y);
				if (clr.alpha() == 0)//表示全透明
				{
					saveformat = "PNG";
					quality = -1;//如果带有透明的通道则不压缩
					y = 300;//跳出循环
					break;
				}
			}
		}
	}

	//将图片转换为字节
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	if(m_Parent->m_bCompressTile)
		outmap = outmap.convertToFormat(QImage::Format_ARGB6666_Premultiplied);//The image is stored using 24-bit indexes into a colormap.
	
//	outmap = outmap.convertToFormat(QImage::Format::Format_Indexed8);
	outmap.save(&buffer, saveformat.c_str(), quality);
	m_Parent->SaveData(tileId,bytes, saveformat);
}

bool CCutTileThread::IsValid(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId)
{
	//图像裁剪，这个裁剪是对超过数据范围的设置为透明
	if (!pDataBoundPolygon || pDataBoundPolygon->m_polyon.size() < 1)
		return true;

	ENVELOPE tileRect;//输出块的范围
	m_pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);
	CGeoPolygon img_polygon;
	CGeoPoint points;
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.miny));
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.miny));
	img_polygon.m_polyon.append(points);
	img_polygon.UpdateBound();
	//判断多边形是否相交
	int result = pDataBoundPolygon->PolygonInPolygon(&img_polygon);
	if (result == 3)
	{
		return false;//区域块如果完全在多边形m_pDataBoundPolygon范围内，则不需要裁剪了
	}
	return true;
}

void CCutTileThread::ShapeImage(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId, QImage *outmap)
{
	//图像裁剪，这个裁剪是对超过数据范围的设置为透明
	if (!pDataBoundPolygon || pDataBoundPolygon->m_polyon.size() < 1)
		return;

	ENVELOPE tileRect;//输出块的范围
	m_pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);

	CGeoPolygon img_polygon;
	CGeoPoint points;
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.miny));
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.miny));
	img_polygon.m_polyon.append(points);
	img_polygon.UpdateBound();
	//判断多边形是否相交
	int result = pDataBoundPolygon->PolygonInPolygon(&img_polygon);
	if (result == 1)
	{
		return;//区域块如果完全在多边形m_pDataBoundPolygon范围内，则不需要裁剪了
	}
	QPolygon poly;
	int lx, ly, point_count = pDataBoundPolygon->m_polyon[0].m_points.size();
	for (int n = 0; n < point_count; n++)
	{
		double dx = pDataBoundPolygon->m_polyon[0].m_points[n].x();
		double dy = pDataBoundPolygon->m_polyon[0].m_points[n].y();

		lx = (dx - tileRect.minx) * 256 / (tileRect.maxx - tileRect.minx);
		ly = (tileRect.maxy - dy) * 256 / (tileRect.maxy - tileRect.miny);
		poly.append(QPoint(lx, ly));
	}

	QImage bk_image = outmap->copy(0, 0, 256, 256);//创建一个256*256的图片
	bk_image.fill(QColor(0, 0, 0, 0));
	QPainter thePainter(&bk_image);
	thePainter.setRenderHint(QPainter::Antialiasing);
	thePainter.setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
	thePainter.setBrush(QBrush(QColor(255, 255, 255, 255), Qt::SolidPattern));
	thePainter.drawPolygon(poly, Qt::WindingFill);
	thePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	thePainter.drawImage(0, 0, *outmap);
	*outmap = bk_image.copy();


	/*
	QRgb rgba = QColor(0, 0, 0, 0).rgba();//替换的颜色可以是透明的，比如QColor(0,0,0，0)。
	for (int ly = 0; ly < 256; ly++)
	{
		double dy = tileRect.maxy - (tileRect.maxy - tileRect.miny) * ly / 256.0;
		for (int lx = 0; lx < 256; lx++)
		{
			double dx = tileRect.minx + (tileRect.maxx - tileRect.minx) * lx / 256.0;
			if (pDataBoundPolygon->InPolygon(dx, dy))
				continue;

			outmap->setPixel(lx, ly, rgba);//设置该像素为透明
		}
	}*/
	///////////////////////////////////////////////

}
////////////////////////////////////////////////////////////////////////
CMultiTileThread::CMultiTileThread ( QObject* parent
  , QgsMapCanvas* ifaceCanvas
  , const EV_DB_Header& tileInfo
  , const  QString tileDbPath
  , bool CreateNewFile
  , int	ProcessNum
  , QString PolygoFilePath
  , QString AlphaName)
   : QThread(parent)
	  , mTileInfo(tileInfo) 
	  , mtileDbPath(tileDbPath)
	  , mStop(false)
	  , m_AlphaTreeGroupName(AlphaName)
{
	mIsCreateNewFile = CreateNewFile;
	m_pObject = parent;
	mTileStore = new SilenTileDb(tileDbPath,tileInfo,mIsCreateNewFile);

	m_bCompressTile = false;
	m_ProcessCount = ProcessNum;
	if (tileInfo.dbtype == 3)
		m_pTileUtils = new CMercatorUtils();
	else
		m_pTileUtils = new CEDomUtils();
	//建立多线程
	CreateCutTileThread(parent,ifaceCanvas);
	ReadBound(PolygoFilePath);
}
  
CMultiTileThread::~CMultiTileThread(void)
{
	FreeCutTileThread();
	SAFE_DELETE(mTileStore);
	SAFE_DELETE(m_pDataBoundPolygon);
	SAFE_DELETE(m_pTileUtils);
}

//判断某个ID是否在区域有效范围内
bool CMultiTileThread::IDInValidateBound(ENVELOPE tileRect)
{
	if (m_ValidateBound.size() < 1)
		return true;//没有数据直接返回TRUE

	QgsRectangle rect(tileRect.minx, tileRect.miny, tileRect.maxx, tileRect.maxy);
	int count = m_ValidateBound.size();
	for (int i = 0; i < count; i++)
	{
		if (m_ValidateBound[i].contains(rect)|| m_ValidateBound[i].intersects(rect))
			return true;//包含或者相交
	}

	return false;
}

void CMultiTileThread::ReadBound(QString shp_file)
{//从shp文件里读取数据范围
	SAFE_DELETE(m_pDataBoundPolygon);
	m_pDataBoundPolygon = NULL;

	//先判断文件是否存在
	if (!QFile::exists(shp_file))return;

	//qgs open shp文件后读取第一个要素的范围
	QgsVectorLayer vlayer = QgsVectorLayer(shp_file, "templ_layer"+qrand(), "ogr");
	if (!vlayer.isValid()|| vlayer.featureCount()<=0)return;

	QgsGeometry qgsGeometry = vlayer.getFeature(QgsFeatureId(0)).geometry();
	m_pDataBoundPolygon = new CGeoPolygon();
	if(!m_pDataBoundPolygon->fromQgsGeometry(qgsGeometry))
	{
		SAFE_DELETE(m_pDataBoundPolygon);
		m_pDataBoundPolygon = nullptr;
		return;
	}
	m_pDataBoundPolygon->UpdateBound();
}

bool CMultiTileThread::ConnectDBSucceed()
{//返回是否连接数据库成功
	if(!mTileStore)
		return false;

	return mTileStore->IsConnection();
}

void CMultiTileThread::SaveData(LONGLONG id,QByteArray databytes, std::string format)
{
	//加入数据到排队队列里
	SaveBlockStru data;
	data.id = id;
	data.bytes = databytes;
	int size = data.bytes.size();
	char type[4];
	strcpy(type,format.c_str());
	type[3] = '\0';
	data.bytes.insert(0, (const char*)&size, sizeof(int));
	data.bytes.insert(0, type, 4);
	mSaveListMutex.lock();//锁定
	m_SaveList.append(data);
	mSaveListMutex.unlock();//解锁
	//image.FreeMemory(buffer);
}

bool CMultiTileThread::AddThread(QgsRectangle outRect,int minx,int miny,int maxx,int maxy,
	int level,int minIndX,int minIndY,int maxIndX,int maxIndY)
{
	//将数据添加到队列个数最少的线程中
	CCutTileThread *pCutThread = NULL;
	int count = 0;
	for(int i=0;i<GetProcessCount();i++)
	{
		if(i==0)
		{
			count = m_CutTileThreads[i]->GetListCount();
			pCutThread = m_CutTileThreads[i];
		}
		else{
			if(count>m_CutTileThreads[i]->GetListCount())
			{
				count = m_CutTileThreads[i]->GetListCount();
				pCutThread = m_CutTileThreads[i];
			}
		}
	}
	if(pCutThread && pCutThread->AddData(outRect,minx,miny,maxx,maxy,level,minIndX,minIndY,maxIndX,maxIndY))
		return true;

	return false;
}
void CMultiTileThread::RenderBlockByLevel(int level)
{//一次采用9*9的块切图，然后分别按照ID块存储，这样可以加快数据切图的速度
	long minIndX = m_pTileUtils->GetIndex_X(mTileInfo.minx,level);
	long minIndY = m_pTileUtils->GetIndex_Y(mTileInfo.miny,level);
	long maxIndX = m_pTileUtils->GetIndex_X(mTileInfo.maxx,level);
	long maxIndY = m_pTileUtils->GetIndex_Y(mTileInfo.maxy,level);
	double lWidth = m_pTileUtils->GetLevelWidth(level);
	wchar_t ss[256];
	swprintf(ss,L"正在处理的级别：%d",level);
	m_msg = QString::fromWCharArray(ss);
	emit signalMessage(m_msg);//发送消息
	emit signalProcess(0);//发送消息

	if (minIndY > maxIndY)
		std::swap(maxIndY, minIndY);

	m_TileCount = (maxIndX - minIndX + 1) * (maxIndY - minIndY + 1);
	m_CurTileStep = 0;//开始计数
	bool exitblock;//判断整个块是否都存在，如果有一个不存在就要处理
	for(int ly = minIndY;ly <= maxIndY;ly+=(GRID_SIZE-2))
	{
		for(int lx=minIndX;lx<=maxIndX;lx+=(GRID_SIZE-2))
		{
			if(mStop)break;//如果用户选择放弃则退出
			
			if(!mIsCreateNewFile)
			{
				//首先判断数据库中是否存在这个ID了，如果已经存在则不需要处理
				exitblock = true;
				for(int ny = ly;ny<ly+GRID_SIZE-2;ny++)
				{
					if(ny>maxIndY)
						break;
					for(int nx = lx;nx<lx+GRID_SIZE-2;nx++)
					{
						if(nx>maxIndX)
							break;
						//将渲染的图片数据存储至瓦片数据库中
						LONGLONG id = BLOCK_ID(nx,ny,level);
						if(!mTileStore->ExitTile(id))
						{//如果有一个不存在，就要对这个数据切图
							exitblock = false;
							break;
						}
					}
					if(!exitblock)
						break;
				}
				if(exitblock)
				{//如果所有的数据都已经存在，则加快进度
					m_CurTileStep+= (GRID_SIZE-2)*(GRID_SIZE-2);
					emit signalProcess(m_CurTileStep * 100 / m_TileCount);
					continue;
				}
			}
			////////////////////////////////////////////////////////
			QgsRectangle MinRect,MaxRect,outRect;
			m_pTileUtils->GetBlockRect(lx-1,ly-1,level,MinRect);//计算出范围
			m_pTileUtils->GetBlockRect(lx+GRID_SIZE-2,ly+GRID_SIZE-2,level,MaxRect);//计算出范围
			outRect.set(MinRect.xMinimum(),qMin(MinRect.yMinimum(), MaxRect.yMinimum()),
					    MaxRect.xMaximum(),qMax(MinRect.yMaximum(), MaxRect.yMaximum()));//计算出范围
		
			while(true)
			{//循环，加入处理数据到线程中
				if(mStop)break;//如果用户选择放弃则退出
				if(AddThread(outRect,lx,ly,lx+GRID_SIZE-2,ly+GRID_SIZE-2,level,minIndX,minIndY,maxIndX,maxIndY))
				{	
					break;
				}

				//本处要暂停时间长点，后台线程队列都满了，需要暂停时间久些
				msleep(500);//本线程休息一下，让给后台切图线程
			}
			//本线程等待，根据处理级别来设置等待时间
			msleep(100);//本线程休息一下，让给后台切图线程

			//调试信息
			ShowThreadInfo();
			SaveTileToDB(false);

			//本线程等待，根据处理级别来设置等待时间
		//	msleep(100);//本线程休息一下，让给后台切图线程
		}
	}
	//最后看看是否还有后台数据在处理，如何还在处理就要等待
	while(true)
	{
		int count = m_CutTileThreads.size();
		int total = 0;
		for(int i=0;i<count;i++)
		{
			if(!m_CutTileThreads[i]->working() && m_CutTileThreads[i]->GetListCount()<1)
				total++;//线程处于闲置状态，表明数据全部切图完成了
		}

		if(total>=count)
			break;//处理完成

		//调试信息
		ShowThreadInfo();
		//毫秒
		msleep(300);//本线程休息一下，让给后台切图线程
		SaveTileToDB(false);//最后将数据保存，全部提交保存
	}

	SaveTileToDB(true);//最后将数据保存，全部提交保存
}

//参数AllCommit为是否全部提交
void CMultiTileThread::SaveTileToDB(bool AllCommit)
{//保存数据块，一次全部处理完成
	int count = m_SaveList.size();
	int thread_count = m_CutTileThreads.size()*10;//根据线程的数量来计算一次批量保存插入的记录个数
	if(!AllCommit && count < thread_count)
	{
		return;
	}
	mSaveListMutex.lock();
	//判断是否在数据库里已经存在了数据的
	for (int i = count - 1; i >= 0; i--)
	{
		LONGLONG id = m_SaveList[i].id;
		if (!mTileStore->ExitTile(id))
		{//如果数据不存在，则不做后面的操作
			continue;
		}
		m_SaveList.removeAt(i);
	}
	mTileStore->save(&m_SaveList);
	//调试信息
	ShowThreadInfo();
	m_CurTileStep+=count;
	////////////////////////////////////
	//test!!!!!!!!!!!!!!!
	//QString strmsg = QString(",test 2:Step=%1, Count=%2,Per=%3").arg(m_CurTileStep).arg(m_TileCount).arg(m_CurTileStep * 100 / m_TileCount);
	//strmsg = m_msg + strmsg;
	//emit signalMessage(strmsg);
	//////////////////////////////////
	emit signalProcess(m_CurTileStep * 100 / m_TileCount);
	m_SaveList.clear();
	mSaveListMutex.unlock();//切记要解锁
}

void CMultiTileThread::ShowThreadInfo()
{
	int ProcessCount = GetProcessCount();
	int total = 0;
	for(int k=0;k<ProcessCount;k++)
	{
		total+=m_CutTileThreads[k]->GetListCount();
	}

	emit signalThreadPercent(total*100/(ProcessCount*MAX_LISTCOUNT));
}

void CMultiTileThread::CreateCutTileThread(QObject* parent,QgsMapCanvas* ifaceCanvas)
{
	for(int i=0;i<GetProcessCount();i++)
	{
		CCutTileThread *thread = new CCutTileThread(m_pTileUtils,parent);
		thread->Init(this,ifaceCanvas);
		m_CutTileThreads.append(thread);
	}
}

void CMultiTileThread::FreeCutTileThread()
{
	int i,count = m_CutTileThreads.size();
	for(i=0;i<count;i++)
	{
		m_CutTileThreads[i]->Fini();
		msleep(500);
	}
	for(i=0;i<count;i++)
	{
		delete m_CutTileThreads[i];
	}
	m_CutTileThreads.clear();
}


void CMultiTileThread::run()
{
	if(mtileDbPath.isEmpty())return ;


	clock_t start,finish;
	start = clock();
	this->mTileStore->startTransaction();
	/////////////////////////////
	 
	for(int l = mTileInfo.minlevel;l <= mTileInfo.maxlevel;l++)
		this->RenderBlockByLevel(l);

	/////////////////////////////////////////
	this->mTileStore->SaveDataInfo(myear,mgdcode,mname,malias,mzt);
	this->mTileStore->endTransaction();
	this->mTileStore->close();
	finish = clock();//完成时间

	//计算完成的时间
	long seconds = (finish-start)/CLOCKS_PER_SEC;//秒
	QString strmsg;
	if(seconds<=60){
		strmsg = QString("%1").arg(seconds)+QString::fromWCharArray(L"秒");
	}
	else{
		long h = 0;
		long m = seconds / 60;
		seconds = seconds % 60;//得到秒
		if(m>60)
		{
			h = m / 60;
			m = m % 60;
		}

		wchar_t msg[256];
		swprintf(msg, L"%d时%d分%d秒",h,m,seconds);
		strmsg = QString::fromWCharArray(msg);
	}

	emit signalMessage(QString::fromWCharArray(L"处理完成"));
	emit signalProcess(100);
	emit signalFinish();
	emit DataFinish(strmsg);//显示消息框表明处理完成
}

///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
CDownloadThread::CDownloadThread( QObject* parent)
	: QThread(parent)
{
	mStop = false;
	mTileStore = NULL;
}

CDownloadThread::~CDownloadThread()
{
	if (mTileStore)
		delete mTileStore;

	mTileStore = NULL;
	mStop = false;
}

void CDownloadThread::run()
{//开始要下载
	//开始要初始化
	//打开原始待纠正图像
	mStop = false;
	mTileInfo.minlevel = m_minlevel;
	mTileInfo.maxlevel = m_maxlevel;

	const QgsRectangle *bound = mDownloadGeometry.GetBound();
	mTileInfo.minx = bound->xMinimum();
	mTileInfo.maxx = bound->xMaximum();
	mTileInfo.miny = bound->yMinimum();
	mTileInfo.maxy = bound->yMaximum();
	
	mTileStore = new SilenTileDb(mtileDbPath, mTileInfo, true);
	if (!mTileStore->IsConnection())
	{
		emit signalMessage(QString::fromWCharArray(L"创建文件路径失败:") + mtileDbPath);
		return;
	}
	emit signalMessage(QString::fromWCharArray(L"创建文件路径成功:") + mtileDbPath);
	emit signalMessage(QString::fromWCharArray(L"开始下载数据，下载级别范围:") + QString("[%1,%2]").arg(m_minlevel).arg(m_maxlevel));

	for (int level = m_minlevel; level <= m_maxlevel; level++)
	{
		int min_lx = SilenTileUtils::GetIndex(mTileInfo.minx, level);
		int max_lx = SilenTileUtils::GetIndex(mTileInfo.maxx, level);
		int min_ly = SilenTileUtils::GetIndex(mTileInfo.miny, level);
		int max_ly = SilenTileUtils::GetIndex(mTileInfo.maxy, level);

		DownloadData(level,min_lx, max_lx, min_ly, max_ly);
	}

	if (mTileStore)
		delete mTileStore;

	mTileStore = NULL;

	//结束
	emit signalMessage(QString::fromWCharArray(L"下载完成"));
	emit signalFinish();
}

void CDownloadThread::DownloadData(int level,int min_lx, int max_lx, int min_ly, int max_ly)
{
	int pos = 0;
	int count = (max_ly - min_ly) * (max_lx - min_lx);
	emit signalThreadPercent(level, pos);
	for (int ly = min_ly; ly <= max_ly; ly++)
	{
		if (mStop)
			return;

		TileRequests requestsFinal;//网络情况下加载
		for (int lx = min_lx; lx <= max_lx; lx++)
		{
			int s = (ly - min_ly) * (lx - min_lx) * 100 / (count == 0 ? 1 : count);
			if (s > pos)
			{
				pos = s;
				emit signalThreadPercent(level, pos);
			}
			LONGLONG tid = BLOCK_ID(lx, ly, level);
			//判断ID是否在区域范围内，如果不再则不需要下周
			GEO_RECT tileRect = SilenTileUtils::GetGeoRectById(tid);
			CGeoPolygon img_polygon;
			CGeoPoint points;
			points.m_points.append(QgsPointXY(tileRect.minx, tileRect.miny));
			points.m_points.append(QgsPointXY(tileRect.minx, tileRect.maxy));
			points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.maxy));
			points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.miny));
			img_polygon.m_polyon.append(points);
			img_polygon.UpdateBound();
			if (this->mDownloadGeometry.PolygonInPolygon(&img_polygon) == 3)
				continue;//不相交

			//下载数据
			//请求的是数据块信息
			QString url = GetURL(tid);
			TileRequest rqst(QUrl(url), m_type+1, QRect(0, 0, 0, 0), tid);
			requestsFinal.append(rqst);//加载到需要下载的队列里

			if (mStop)
				return;

			if (requestsFinal.size() > 10)
			{
				EdomDownloadHandler handler(&mDownloadGeometry, requestsFinal, mTileStore);
				handler.downloadBlocking();

				requestsFinal.clear();
			}
			if (mStop)
				return;

		}

		if (requestsFinal.size() > 0)
		{
			EdomDownloadHandler handler(&mDownloadGeometry,requestsFinal, mTileStore);
			handler.downloadBlocking();

			requestsFinal.clear();
		}

	}
}

QString CDownloadThread::GetURL(LONGLONG id)
{
	switch (m_type)
	{
	case 1:
		return (QString("%1&tid=%2").arg(m_url).arg(id, 4, 16, QLatin1Char('0')));
	case 2:
		{
			int level = BLOCK_ID_LEVEL(id);
			int lx = BLOCK_ID_X(id);
			int ly = BLOCK_ID_Y(id);
			//将易景的瓦片规则转换成天地图的规则
			return GetTDBaseUrl(level + 2, lx + (1 << (level + 1)), (1 << level) - ly - 1);
		}
	case 4://测绘遥感地图
		{
			//将易景的瓦片规则转换成测绘地图的规则
			int level = BLOCK_ID_LEVEL(id);
			int lx = BLOCK_ID_X(id);
			int ly = BLOCK_ID_Y(id);
			/////////////////
			return GetCHBaseUrl(level + 1, lx + (1 << (level + 1)), (1 << level) - ly - 1);
		}
		break;
	default:
		return "";
	}
	return "";
}

//根据不同地图类型，获取下载的url 地址
QString CDownloadThread::GetCHBaseUrl(int downloadlevel, int i, int j)
{
	//http://60.205.227.207:7090/onemap17/rest/wmts?ACCOUNT=ghyforestry&PASSWD=ghyforestry&layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileMatrix=10&TileCol=1664&TileRow=358
	QString url = m_url + QString("/rest/wmts?layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileCol=%1&TileRow=%2&TileMatrix=%3").arg(i).arg(j).arg(downloadlevel);
	if (!mUserID.isEmpty())
		url += QString("&ACCOUNT=%1").arg(mUserID);
	if (!mPwd.isEmpty())
		url += QString("&PASSWD=%1").arg(mPwd);
	return url;
}


//根据不同地图类型，获取下载的url 地址
QString CDownloadThread::GetTDBaseUrl(int downloadlevel, int i, int j)
{
	//http://t5.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=17&TileRow=18240&TileCol=107913&style=default&format=tiles
	//http://t6.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=11&TileRow=284&TileCol=1687&style=default&format=tiles
	// requesturl = "http://t7.tianditu.cn/img_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=img&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + maplevel;
	//requesturl = "http://t5.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	return  m_url + QString("&x=%1&y=%2&l=%3&tk=d5b24677cd5d66023be61f408eeaf45a").arg(i).arg(j).arg(downloadlevel);
	//	requesturl = "http://t2.tianditu.cn/ter_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=ter&tileMatrixSet=c&TileMatrix=" + +downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t7.tianditu.cn/wat_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=wat&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t5.tianditu.cn/raw_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=raw&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t6.tianditu.cn/cva_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=cva&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t5.tianditu.cn/eva_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=eva&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t6.tianditu.com/cia_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=cia&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + downloadlevel;
	//requesturl = " http://t0.tianditu.com/cta_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=cta&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + downloadlevel;
}

//请求结束
void CDownloadThread::slot_requestFinished(bool bSuccess, char type, LONGLONG tid, const QByteArray &data)
{
	if (!bSuccess)
		return;

	switch (type)
	{
	case 1:
		break;
	case 2:
		{
			if (m_type != 1)
			{
				int size = data.size();
				char type[4] = { "png" };
				type[3] = '\0';
				QByteArray buff;
				buff.append((const char*)&size, sizeof(int));
				buff.append(type, 4);
				buff.append(data, size);
			}

			mTileStore->save(tid, data);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////
//CAddTopTileThread
CAddTopTileThread::CAddTopTileThread(QObject* parent
	,const  QString tileDbPath): QThread(parent)
	,mtileDbPath(tileDbPath)
{

}

CAddTopTileThread::~CAddTopTileThread(void)
{

}

QImage* CAddTopTileThread::createImage(int grid) const
{//创建一个256*3大小的位图
	int width = grid;//默认的大小
	int height = grid;
	//直接输出带透明的PNG格式
	//is format jpeg?
	bool jpeg = false;
	QImage* theImage = new QImage(width, height, QImage::Format_ARGB32);
	if (!theImage)
	{
		return nullptr;
	}
	theImage->fill(0);
	//apply DPI parameter if present. This is an extension of Qgis Mapserver compared to WMS 1.3.
	//Because of backwards compatibility, this parameter is optional
	//	double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
	int dpm = 5669;//1 / OGC_PX_M;
	theImage->setDotsPerMeterX(dpm);
	theImage->setDotsPerMeterY(dpm);
	return theImage;
}

void CAddTopTileThread::run()
{
	QString msg;
	//线程独立运行
	mTileStore = new CWriteTileDb();
	//打开数据
	if (mTileStore->open(mtileDbPath))
	{
		msg = QString::fromWCharArray(L"打开edom文件成功");
		emit signalMessage(msg);//发送消息
	}
	else {
		msg = QString::fromWCharArray(L"打开edom文件失败");
		emit signalMessage(msg);//发送消息
	}

	mTileStore->ReadHeader(mTileInfo);
	//开始处理
	wchar_t ss[256];
	int minlevel = mTileInfo.minlevel;
	for (int level = minlevel - 1; level >= 0; level--)
	{
		int minlx = SilenTileUtils::GetIndex(mTileInfo.minx, level);
		int maxlx = SilenTileUtils::GetIndex(mTileInfo.maxx, level);
		int minly = SilenTileUtils::GetIndex(mTileInfo.miny, level);
		int maxly = SilenTileUtils::GetIndex(mTileInfo.maxy, level);

		swprintf(ss, L"开始修补级别：%d", level);
		msg = QString::fromWCharArray(ss);
		emit signalMessage(msg);//发送消息
		int count = (maxly - minly + 1) * (maxlx - minlx + 1);
		int percent = 0;
		for (int ly = minly; ly <= maxly; ly++)
		{
			for (int lx = minlx; lx <= maxlx; lx++)
			{
				LONGLONG id = BLOCK_ID(lx, ly, level);
				signalThreadPercent(level, (++percent * 100 / count));//发送当前进度
				if (mTileStore->ExitTile(id))
					mTileStore->del(id);//存在要删除掉，重新生成

				QImage* theImage = createImage(256);
				if (!theImage)
					continue;

				QPainter *painter = new QPainter(theImage);
				ENVELOPE bound;
				SilenTileUtils::GetBlockRect(lx, ly, level, bound);
				//合并
				bool save_flag = false;
				LONGLONG child_ids[4];
				SilenTileUtils::GetLowIds(id, child_ids);//取得子块的ID编号
				for (int index = 0; index < 4; index++)
				{
					LONGLONG child_id = child_ids[index];
					if (!mTileStore->ExitTile(child_id))
						continue;

					QByteArray data = mTileStore->read(child_id);
					data.remove(0, 8);
					QImage child_img;
					if (!child_img.loadFromData(data))
						continue;

					QImage newImg = child_img.scaled(128, 128);
					ENVELOPE child_bound;
					SilenTileUtils::GetBlockRect(BLOCK_ID_X(child_id), BLOCK_ID_Y(child_id), BLOCK_ID_LEVEL(child_id), child_bound);
					int i_w = (child_bound.minx - bound.minx) * 256 / (bound.maxx - bound.minx);
					int i_h = (bound.maxy - child_bound.maxy) * 256 / (bound.maxy - bound.miny);
					//for (int py = 0; py < 128; py++)
					//{
					//	for (int px = 0; px< 128; px++)
					//	{
					//		theImage->setPixel(px + i_w, py + i_h, newImg.pixel(px, py));
					//	}
					//}
					painter->drawImage(i_w, i_h, newImg);
					save_flag = true;
				}

				//保存数据
				if (save_flag) {
					QByteArray bytes;
					QBuffer buffer(&bytes);
					buffer.open(QIODevice::WriteOnly);
					QImage read = theImage->convertToFormat(QImage::Format_ARGB6666_Premultiplied);//The image is stored using 8-bit indexes into a colormap.
					read.save(&buffer, "PNG");
					buffer.close();
					int size = bytes.size();
					char type[4] = { "png" };
					type[3] = '\0';
					bytes.insert(0, (const char*)&size, sizeof(int));
					bytes.insert(0, type, 4);
					mTileStore->save(id, bytes);
				}

				SAFE_DELETE(theImage);
			}
		}
	}

	mTileStore->DelSysTile();
	//保存头文件信息
	mTileInfo.minlevel = 0;
	mTileStore->SaveSysTile(mTileInfo);

	mTileStore->close();
	SAFE_DELETE(mTileStore);
	
	emit  signalFinish();
}

//////////////////////////////////////////////////////
//CTileUtils
CTileUtils::CTileUtils()
{

}

CTileUtils::~CTileUtils()
{

}
//////////////////////////////////////////////////////////////////
//CEDomUtils
CEDomUtils::CEDomUtils() :CTileUtils()
{

}

CEDomUtils::~CEDomUtils()
{

}

long CEDomUtils::GetIndex_X(double pos, int level)
{
	return SilenTileUtils::GetIndex(pos, level);
}

long CEDomUtils::GetIndex_Y(double pos, int level)
{
	return SilenTileUtils::GetIndex(pos, level);
}

//根据级别计算坐标值
double CEDomUtils::GetCoord_X(long index, int level)
{
	return SilenTileUtils::GetCoord(index, level);
}

//根据级别计算坐标值
double CEDomUtils::GetCoord_Y(long index, int level)
{
	return SilenTileUtils::GetCoord(index, level);
}
//根据行列号计算地理坐标
void CEDomUtils::GetBlockRect(long x, long y, int level, ENVELOPE& extent)
{
	SilenTileUtils::GetBlockRect(x, y, level, extent);
}

//根据行列号计算瓦片范围
void CEDomUtils::GetBlockRect(long x, long y, int level, QgsRectangle& rect)
{
	SilenTileUtils::GetBlockRect(x, y, level, rect);
}

//计算一行的宽度
double CEDomUtils::GetLevelWidth(int level)
{
	return SilenTileUtils::GetLevelWidth(level);
}

//////////////////////////////////////////////////////////////////
//CMercatorUtils
CMercatorUtils::CMercatorUtils():CTileUtils()
{

}

CMercatorUtils::~CMercatorUtils()
{

}

long CMercatorUtils::GetIndex_X(double pos, int level)
{
	return GetMercatorIndex_X(pos, level);
}

long CMercatorUtils::GetIndex_Y(double pos, int level)
{
	return GetMercatorIndex_Y(pos, level);
}

//根据级别计算坐标值
double CMercatorUtils::GetCoord_X(long index, int level)
{
	return GetMercatorCoord_X(index, level);
}

double CMercatorUtils::GetCoord_Y(long index, int level)
{
	return GetMercatorCoord_Y(index, level);
}

//根据行列号计算地理坐标
void CMercatorUtils::GetBlockRect(long x, long y, int level, ENVELOPE& extent)
{
	GetMercatorBlockRect(x, y, level, extent);
}

//根据行列号计算瓦片范围
void CMercatorUtils::GetBlockRect(long x, long y, int level, QgsRectangle& rect)
{
	GetMercatorBlockRect(x, y, level, rect);
}

//计算一行的宽度
double CMercatorUtils::GetLevelWidth(int level)
{
	return GetMercatorLevelWidth(level);
}

double CMercatorUtils::GetRefValue()
{
	return M_MAX_VALUE;
}

//////////////////////////////////////
CUnitTileThread::CUnitTileThread(QObject* parent, const QString tileDbPath, const QStringList edomPaths)
	:QThread(parent)
{
	mStop = false;
	m_SaveDBPath = tileDbPath;
	m_DataList = edomPaths;
}

CUnitTileThread::~CUnitTileThread(void)
{

}

void CUnitTileThread::run()
{
	EV_DB_Header TileInfo;
	TileInfo.maxlevel = 0;
	TileInfo.minlevel = 30;
	TileInfo.minx = 180;
	TileInfo.maxx = -180;
	TileInfo.miny = 90;
	TileInfo.maxy = -90;

	SilenTileDb mdb(m_SaveDBPath, TileInfo,true);
	if (!mdb.IsConnection())
	{
		emit signalMessage(QString::fromWCharArray(L"创建保存文件失败"));//发送消息
		emit signalFinish();
		return;
	}

	clock_t start, finish;
	start = clock();
	int count = m_DataList.size();
	for (int i = 0; i < count; i++)
	{
		UnitData(&mdb, &TileInfo, m_DataList[i]);
	}

	//保存头文件信息 
	mdb.DelSysTile();
	mdb.SaveSysTile(TileInfo);
	mdb.close();

	finish = clock();//完成时间
	long seconds = (finish - start) / CLOCKS_PER_SEC;//秒
	QString strmsg;
	if (seconds <= 60) {
		strmsg = QString("%1").arg(seconds) + QString::fromWCharArray(L"秒");
	}
	else {
		long d = 0;
		long h = 0;
		long m = seconds / 60;
		seconds = seconds % 60;//得到秒
		if (m > 60)
		{
			h = m / 60;
			m = m % 60;
		}
		if (h > 24)
		{//求出天数
			d = h / 24;
			h = h % 24;
		}

		wchar_t msg[256];
		if (d > 0)
			swprintf(msg, L"%d天%d时%d分%d秒", d, h, m, seconds);
		else
			swprintf(msg, L"%d时%d分%d秒", h, m, seconds);
		strmsg = QString::fromWCharArray(msg);
	}
	QString tr_timeDiff = QString(QString::fromWCharArray(L"数据处理完成!共消耗时间：") + "%1").arg(strmsg); //float->QString
	emit signalMessage(tr_timeDiff);
	emit signalFinish();
}

void CUnitTileThread::UnitData(SilenTileDb *SaveDB, EV_DB_Header *SavaHeader, QString DataFile)
{
	CReadTileDb read;
	if (!read.open(DataFile))
		return;

	EV_DB_Header h;
	read.ReadHeader(h);
	SavaHeader->minlevel = __min(SavaHeader->minlevel , h.minlevel);
	SavaHeader->maxlevel = __max(SavaHeader->maxlevel , h.maxlevel);
	SavaHeader->minx = __min(SavaHeader->minx, h.minx);
	SavaHeader->maxx = __max(SavaHeader->maxx, h.maxx);
	SavaHeader->miny = __min(SavaHeader->miny, h.miny);
	SavaHeader->maxy = __max(SavaHeader->maxy, h.maxy);

	QString pathname = DataFile;
	QFileInfo fi = QFileInfo(pathname);
	QString file_name = fi.fileName();
	for (int level = h.minlevel; level <= h.maxlevel; level++)
	{
		emit signalMessage(QString::fromWCharArray(L"正在处理文件：%1， 级别：%2，级别范围：(%3-%4) ").arg(file_name).arg(level).arg(h.minlevel).arg(h.maxlevel));//发送消息

		int minlx = SilenTileUtils::GetIndex(h.minx, level);
		int maxlx = SilenTileUtils::GetIndex(h.maxx, level);
		int minly = SilenTileUtils::GetIndex(h.miny, level);
		int maxly = SilenTileUtils::GetIndex(h.maxy, level);

		int count = (maxly - minly + 1) * (maxlx - minlx + 1);
		int percent = 0;
		for (int ly = minly; ly <= maxly; ly++)
		{
			for (int lx = minlx; lx <= maxlx; lx++)
			{
				LONGLONG id = BLOCK_ID(lx, ly, level);
				QByteArray read_bytes = read.read(id);
				QByteArray save_bytes = SaveDB->read(id);
				QByteArray out_bytes;
				if (MergeData(&read_bytes, &save_bytes, &out_bytes))
				{
					SaveDB->del(id);
					SaveDB->save(id, out_bytes);
				}
				emit signalProcess((++percent * 100 / count));//发送当前进度
			}
		}
	}
}

bool CUnitTileThread::MergeData(QByteArray *read_bytes, QByteArray *save_bytes, QByteArray *out_bytes)
{
	if (read_bytes->size() < 1)
		return false;

	if (save_bytes->size() < 1)
	{
		*out_bytes = *read_bytes;
		return true;
	}

	read_bytes->remove(0, 8);
	save_bytes->remove(0, 8);
	QImage read, save;
	read.loadFromData(*read_bytes);
	save.loadFromData(*save_bytes);

	QPainter thePainter(&read);
//	thePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	thePainter.drawImage(0, 0, save);
	//将图片转换为字节
	QBuffer buffer(out_bytes);
	buffer.open(QIODevice::WriteOnly);
	std::string saveformat = "JPG";
	//接下来判断数据是否没有透明的，则采用JPG存储，否则采用PNG
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			QColor clr = read.pixelColor(x, y);
			if (clr.alpha() == 0)//表示全透明
			{
				saveformat = "PNG";
				y = 300;//跳出循环
				break;
			}
		}
	}

	read.save(&buffer, saveformat.c_str());

	int size = out_bytes->size();;
	char type[4] = { "\0" };
	strcpy(type, saveformat.c_str());
	type[3] = '\0';
	out_bytes->insert(0, (const char*)&size, sizeof(int));
	out_bytes->insert(0, type, 4);

	return true;
}

//#define INT_FAST16_T
//#include "cximage7/ximage.h"
//#pragma comment(lib, "cximage7_x64.lib")
///////////////////////////////////////////////////////////////
//CTransparentThread
CTransparentThread::CTransparentThread(QObject* parent, const QString ImgPath, const QVector<QColor> colors)
{
	m_ImgPath = ImgPath;
	m_colors = colors;
}

CTransparentThread::~CTransparentThread(void)
{

}

void CTransparentThread::run()
{
	FindFile(m_ImgPath);
	emit signalFinish();
}

bool CTransparentThread::FindFile(const QString &path)
{
	QDir dir(path);
	if (!dir.exists())
		return false;

	dir.setFilter(QDir::Dirs | QDir::Files);
	dir.setSorting(QDir::DirsFirst);

	QFileInfoList list = dir.entryInfoList();
	int i = 0;
	bool bIsDir;
	do
	{
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.fileName() == "." | fileInfo.fileName() == "..")
		{
			i++;
			continue;
		}

		bIsDir = fileInfo.isDir();
		if (bIsDir)
		{
			//对文件属性进行处理
		 //fileInfo.size(),fileInfo.fileName(),fileInfo.path();
			FindFile(fileInfo.filePath());
		}
		else
		{
			//为文件
			QString exts = fileInfo.suffix();
			exts = exts.toLower();
			if(exts.compare("tif") == 0)
				TransparentImage(fileInfo.filePath());
		}
		i++;
	} while (i<list.size());

	return true;
}

void CTransparentThread::TransparentImage(QString imgPath)
{
	//wchar_t wpath[256] = L"\0";
	//imgPath.toWCharArray(wpath);

	emit signalMessage(QString::fromWCharArray(L"正在打开文件：%1").arg(imgPath));//发送消息
	QImage image;
	if (!image.load(imgPath))
		return;

	
	emit signalProcess(0);//发送当前进度
	emit signalMessage(QString::fromWCharArray(L"处理文件：%1").arg(imgPath));//发送消息
	bool bSave = false;
	//某些情况下需要读取特定波段，或者需要重组波段顺序。
	//例如VC中显示图像往往需要将buf按照BGR传递给BITMAP，再显示BITMAP。
	//这时只需要修改第11个参数就行了：
	int bufWidth = image.width();   //图像宽度
	int bufHeight = image.height();  //图像高度
	QImage newImage(bufWidth,bufHeight,QImage::Format_ARGB32);
	newImage.fill(Qt::transparent);
	int clr_count = m_colors.size();
	int c;
//	QRgb rgba = QColor(0, 0, 0, 0).rgba();//替换的颜色可以是透明的，比如QColor(0,0,0，0)。
	for (int ly = 0; ly < bufHeight; ly++)
	{
		for (int lx = 0; lx < bufWidth; lx++)
		{
			QColor clr = image.pixelColor(lx, ly);
			for (c = 0; c < clr_count; c++)
			{
				if (m_colors[c].red() == clr.red() &&
					m_colors[c].green() == clr.green() &&
					m_colors[c].blue() == clr.blue())
					break;
			}

			if (c >= clr_count) {
				newImage.setPixelColor(lx, ly, clr);//设置为透明
				bSave = true;
			}
		}
		emit signalProcess((ly+1) * 100 / bufHeight);//发送当前进度
	}

	if (bSave) {
		QString pngpath = imgPath.left(imgPath.length() - 3) + "png";
		emit signalMessage(QString::fromWCharArray(L"正在保存文件：%1").arg(pngpath));//发送消息
		newImage.save(pngpath, "PNG");
		QString pgwpath = imgPath.left(imgPath.length() - 3) + "pgw";
		QString tfwpath = imgPath.left(imgPath.length() - 3) + "tfw";
		copyFileToPath(tfwpath, pgwpath, true);
	}
}

//拷贝文件：
bool CTransparentThread::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
{
	toDir.replace("\\", "/");
	if (sourceDir == toDir) {
		return true;
	}
	if (!QFile::exists(sourceDir)) {
		return false;
	}
	QDir *createfile = new QDir;
	bool exist = createfile->exists(toDir);
	if (exist) {
		if (coverFileIfExist) {
			createfile->remove(toDir);
		}
	}//end if

	if (!QFile::copy(sourceDir, toDir))
	{
		return false;
	}
	return true;
}

//////////////////////////////////
CDemNoiseThread::CDemNoiseThread(QObject* parent, const QStringList Paths,double MinValue,double MaxValue)
{
	mPaths = Paths;
	mStop = false;
	m_MinAlt = MinValue;
	m_MaxAlt = MaxValue;
}

CDemNoiseThread::~CDemNoiseThread(void)
{
}

void CDemNoiseThread::run()
{
	int file_count = mPaths.size();
	for (int f = 0; f < file_count; f++)
	{
		QString filepath = mPaths[f];
		emit signalMessage(QString::fromWCharArray(L"正在读取数据文件：%1....").arg(filepath));
		QFile file(filepath);
		if (!file.open(QFile::ReadOnly | QFile::Text))
		{
			emit signalMessage(QString::fromWCharArray(L"正在读取数据文件失败：%1....").arg(filepath));
			continue;
		}
		QTextStream textIn(&file);
		QString strLine;
		int ncols, nrows;
		double xllcorner, yllcorner, cellsize;
		int	nodata_value;
		strLine = textIn.readLine();//ncols
		if (strLine.lastIndexOf("ncols") < 0)
		{
			emit signalMessage(QString::fromWCharArray(L"正在读取数据错误：%1....").arg(filepath));
			continue;
		}
		strLine = strLine.remove("ncols");
		strLine = strLine.remove(" ");
		ncols = strLine.toInt();

		strLine = textIn.readLine();//nrows
		if (strLine.lastIndexOf("nrows") < 0)
		{
			emit signalMessage(QString::fromWCharArray(L"正在读取数据错误：%1....").arg(filepath));
			continue;
		}
		strLine = strLine.remove("nrows");
		strLine = strLine.remove(" ");
		nrows = strLine.toInt();

		strLine = textIn.readLine();//xllcorner
		strLine = strLine.remove("xllcorner");
		strLine = strLine.remove(" ");
		xllcorner = strLine.toDouble();

		strLine = textIn.readLine();//yllcorner
		strLine = strLine.remove("yllcorner");
		strLine = strLine.remove(" ");
		yllcorner = strLine.toDouble();

		strLine = textIn.readLine();//cellsize
		strLine = strLine.remove("cellsize");
		strLine = strLine.remove(" ");
		cellsize = strLine.toDouble();

		strLine = textIn.readLine();//nodata_value
		strLine = strLine.remove("nodata_value");
		strLine = strLine.remove(" ");
		nodata_value = strLine.toInt();
		double **Values;//二维数组
		Values = new double *[nrows];
		int nx, ny;
		bool failed = false;
		for (ny = 0; ny < nrows; ny++)
		{
			Values[ny] = new double[ncols];
			memset(Values[ny], 0, sizeof(double)*ncols);
			//读取一行
			strLine = textIn.readLine();
			QStringList strValues = strLine.split(" ");
			if (strValues.size() < ncols)
			{
				failed = true;
				break;
			}

			for (nx = 0; nx < ncols; nx++)
			{
				Values[ny][nx] = strValues[nx].toDouble();
			}

			emit signalProcess((ny + 1) * 100 / nrows);
		}

		if (failed)
		{
			emit signalMessage(QString::fromWCharArray(L"正在读取数据错误：%1....").arg(filepath));
			for (ny = 0; ny < nrows; ny++)
			{
				SAFE_DELETE_ARRAY(Values[ny]);
			}
			SAFE_DELETE_ARRAY(Values);
			continue;
		}

		file.close();
		emit signalMessage(QString::fromWCharArray(L"正在处理去噪音：%1....").arg(filepath));

		bool isSave = false;
		for (ny = 0; ny < nrows; ny++)
		{
			for (nx = 0; nx < ncols; nx++)
			{
				if (int(Values[ny][nx]) == nodata_value)
					continue;//无效值不处理

				if (int(Values[ny][nx]) < m_MaxAlt && int(Values[ny][nx]) > m_MinAlt)
					continue;//正确值

				isSave = true;//标名数据被修改
				//有噪点值，方法取周边4个值的平均值
				Values[ny][nx] = nodata_value;
				//int x, y;
				//x = nx - 1;
				//if (x > 0)//左
				//	Values[ny][nx] = Values[ny][x];

				//y = ny + 1;//上
				//if (y < ncols)
				//	Values[ny][nx] = (Values[ny][nx] + Values[y][nx]) / 2;

				//x = nx + 1;//右
				//if (x < nrows)
				//	Values[ny][nx] = (Values[ny][nx] + Values[ny][x]) / 2;

				//y = ny - 1;//下
				//if(y>0)
				//	Values[ny][nx] = (Values[ny][nx] + Values[y][nx]) / 2;
			}
			emit signalProcess((ny + 1) * 100 / nrows);
		}

		if (!isSave)
			continue;

		emit signalMessage(QString::fromWCharArray(L"正在写入文件：%1....").arg(filepath));
		QFile writefile(filepath);
		if (!writefile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			emit signalMessage(QString::fromWCharArray(L"写入数据文件失败：%1....").arg(filepath));
			continue;
		}
		QTextStream textOut(&writefile);
		textOut << QString("ncols         %1").arg(ncols) << endl;
		textOut << QString("nrows         %1").arg(nrows) << endl;
		textOut << QString("xllcorner     %1").arg(xllcorner) << endl;
		textOut << QString("yllcorner     %1").arg(yllcorner) << endl;
		textOut << QString("cellsize      %1").arg(cellsize) << endl;
		textOut << QString("nodata_value  %1").arg(nodata_value) << endl;
		for (ny = 0; ny < nrows; ny++)
		{
			for (nx = 0; nx < ncols; nx++)
			{
				textOut << QString("%1").arg(Values[ny][nx]);
				if (nx < ncols - 1)
					textOut << " ";
				else
					textOut << endl;
			}
			emit signalProcess((ny + 1) * 100 / nrows);
		}

		writefile.close();
		emit signalMessage(QString::fromWCharArray(L"写入成功：%1....").arg(filepath));
		//释放空间
		for (ny = 0; ny < nrows; ny++)
		{
			SAFE_DELETE_ARRAY(Values[ny]);
		}
		SAFE_DELETE_ARRAY(Values);
		////////////////////////////////////
	}
	emit signalFinish();
}
