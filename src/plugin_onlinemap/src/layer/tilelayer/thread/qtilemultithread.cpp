#include "qtilemultithread.h"

#include <algorithm>
#include <cmath>
#include <QDir>
#include <QBuffer>
#include <QByteArray>

#include <qgsmaplayer.h>
#include <qgsmaprenderercache.h>
#include <qgsmapcanvasmap.h>
#include <QStyleOptionGraphicsItem>

#include "qgslogger.h"
#include <time.h>

#include "qgsmaprenderercustompainterjob.h"

#include <qgslayertreegroup.h>
#include <QgsProject.h>
#include <QgsLayerTreeLayer.h>
#include <QgsVectorLayer.h>

#include "../common.h"

#include "../schema/MercatorDefine.h"
#include "../tiledb/qtiledb.h"
#include "../web/qhttprequest.h"
#include "../qtileutils.h"
#include "../qtileinfo.h"
#include "../tileprovider/qtileproviderutils.h"
#include "../tileprovider/qTileProvider.h"

namespace geotile {

#define	GRID_SIZE  20
#define MAX_LISTCOUNT	100  //��������������ݵ�������
	qTileBlockThread::qTileBlockThread(qCutEdomThread* pCutEdom, QObject* parent) : QThread(parent)
	{
		mStopThread = false;
		m_pCutEdom = pCutEdom;
		mDataValid = false;
	}

	qTileBlockThread::~qTileBlockThread()
	{

	}

	void qTileBlockThread::Init(QgsPolygonXY* DataBoundPolygon, qEdomTileProviderUtils* pUtils, 
										EV_DB_Header* header, qTileDbPrivateWriter* TileStore)
	{
		m_pDataBoundPolygon = DataBoundPolygon;
		m_pUtils = pUtils;
		m_header = header;
		mTileStore = TileStore;
	}

	void qTileBlockThread::Fini()
	{
		mStopThread = true;
	}

	bool qTileBlockThread::AddData(LONGLONG tid)
	{
		if (mDataValid)
			return false;

		m_tid = tid;
		mDataValid = true;
		return true;
	}

	void qTileBlockThread::run()
	{
		while (!mStopThread) {
			if (mDataValid)
			{
				//�������ڴ���
				ProcessBlockData(m_pDataBoundPolygon, m_tid, m_pUtils, m_header);
				mDataValid = false;
			}
			msleep(300);
		}
	}

	//�ж�pDataBoundPolygon��Ĺ�ϵ
	int qTileBlockThread::Block_Polygon(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId, qEdomTileProviderUtils* pTileUtils)
	{
		//�жϹ�ϵ������ཻ����0��
		//���pDataBoundPolygon����tileId����1��
		//���tileId����pDataBoundPolygon����η���2
		//���pDataBoundPolygon��tileIdû���κι�ϵ������3
		if (!pDataBoundPolygon || pDataBoundPolygon->size() < 1)
			return 3;

		ENVELOPE tileRect;//�����ķ�Χ
		pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);
		QgsPolygonXY img_polygon;
		QgsPolylineXY res;
		res.resize(5);
		res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));
		res.push_back(QgsPointXY(tileRect.minx, tileRect.maxy));
		res.push_back(QgsPointXY(tileRect.maxx, tileRect.maxy));
		res.push_back(QgsPointXY(tileRect.maxx, tileRect.miny));
		res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));

		img_polygon.push_back(res);
		QgsGeometry geo = QgsGeometry::fromPolygonXY(img_polygon);
		QgsGeometry bndGeo = QgsGeometry::fromPolygonXY(*pDataBoundPolygon);
		if (bndGeo.isEmpty())return false;
		//�ж϶�����Ƿ��ཻ
		return bndGeo.contains(geo);
	}

	void qTileBlockThread::ProcessBlockData(QgsPolygonXY* pDataBoundPolygon,
		LONGLONG tileId, qEdomTileProviderUtils* pTileUtils, EV_DB_Header* header)
	{
		int level = BLOCK_ID_LEVEL(tileId);
		if (level > header->maxlevel)
			return;

		//�жϹ�ϵ������ཻ����0��
		//���pDataBoundPolygon����tileId����1��
		//���tileId����pDataBoundPolygon����η���2
		//���pDataBoundPolygon��tileIdû���κι�ϵ������3
		int result = Block_Polygon(m_pDataBoundPolygon, tileId, pTileUtils);
		switch (result)
		{
		case 0://�ཻ����0��
		case 2:
		{
			QImage image;
			CutImage(m_pDataBoundPolygon, tileId, pTileUtils, &image);
			m_pCutEdom->UnitBlock(mTileStore, tileId, image);
			//��ʼ�����ӿ�
			LONGLONG child_ids[4];
			qTileUtils::GetLowIds(tileId, child_ids);//ȡ���ӿ��ID���
			for (int index = 0; index < 4; index++)
			{
				//�ݹ鴦��
				ProcessBlockData(pDataBoundPolygon,
					child_ids[index], pTileUtils,
					header);
			}
		}
		break;
		case 3://�����
			DelBlockData(tileId, header);//ɾ��
			break;
		case 1://���������κδ�����
			break;
		}
	}

	void qTileBlockThread::CutImage(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId, qEdomTileProviderUtils* pTileUtils, QImage* outmap)
	{
		if (!pDataBoundPolygon || pDataBoundPolygon->isEmpty())return;
		////ͼ��ü�������ü��ǶԳ������ݷ�Χ������Ϊ͸��
		ENVELOPE tileRect;//�����ķ�Χ
		pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);
		QPolygon poly;
		QgsPolylineXY line = pDataBoundPolygon->at(0); //ֻҪ�������
		int lx, ly, pnt_count = line.size();
		for (int n = 0; n < pnt_count; n++)
		{
			QgsPointXY pnt = line.at(n);
			double dx = pnt.x();
			double dy = pnt.y();

			lx = (dx - tileRect.minx) * 256 / (tileRect.maxx - tileRect.minx);
			ly = (tileRect.maxy - dy) * 256 / (tileRect.maxy - tileRect.miny);
			poly.append(QPoint(lx, ly));
		}

		QImage bk_image(256, 256, QImage::Format::Format_ARGB32);//����һ��256*256��ͼƬ
		bk_image.fill(QColor(0, 0, 0, 0));
		QPainter thePainter(&bk_image);
		thePainter.setRenderHint(QPainter::Antialiasing);
		thePainter.setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
		thePainter.setBrush(QBrush(QColor(255, 255, 255, 255), Qt::SolidPattern));
		thePainter.drawPolygon(poly, Qt::WindingFill);
		thePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		*outmap = bk_image.copy();
	}

	//ɾ�����ݿ��Լ��ӿ�
	void qTileBlockThread::DelBlockData(LONGLONG tileId, EV_DB_Header* header)
	{
		int level = BLOCK_ID_LEVEL(tileId);
		if (level > header->maxlevel)
			return;

		if (!m_pCutEdom->ExitTile(mTileStore, tileId))
			return;

		m_pCutEdom->DelBlock(mTileStore, tileId);
		DelChildBlockData(tileId, header);
	}

	//ɾ���ӿ�
	void qTileBlockThread::DelChildBlockData(LONGLONG tileId, EV_DB_Header* header)
	{
		int level = BLOCK_ID_LEVEL(tileId);
		if (level == header->maxlevel)
			return;

		LONGLONG child_ids[4];
		qTileUtils::GetLowIds(tileId, child_ids);//ȡ���ӿ��ID���
		for (int index = 0; index < 4; index++)
		{
			if (m_pCutEdom->ExitTile(mTileStore, child_ids[index]))
				DelChildBlockData(child_ids[index], header);

			m_pCutEdom->DelBlock(mTileStore, child_ids[index]);
		}
	}

	////////////////////////////////////////////////////////////////////
	qCutEdomThread::qCutEdomThread(QString edompath, QgsPolygonXY* DataBoundPolygon, int ProcessThreadCount, QObject* parent)
		: QThread(parent)
		, m_EdomPath(edompath)
		, m_ProcessThreadCount(ProcessThreadCount)
		, m_pDataBoundPolygon(DataBoundPolygon)
	{
		mStop = false;
		for (int i = 0; i < m_ProcessThreadCount; i++)
		{
			qTileBlockThread* p = new qTileBlockThread(this, parent);
			m_BlockThreads.push_back(p);
		}
	}

	qCutEdomThread::~qCutEdomThread()
	{
		for (int i = 0; i < m_ProcessThreadCount; i++)
			SAFE_DELETE(m_BlockThreads[i]);

		m_BlockThreads.clear();
	}

	bool qCutEdomThread::ExitTile(qTileDbPrivateWriter* pTileDb, LONGLONG tid)
	{
		mBlockDBMutex.lock();
		bool bExit = pTileDb->ExitTile(tid);
		mBlockDBMutex.unlock();
		return bExit;
	}
	void qCutEdomThread::UnitBlock(qTileDbPrivateWriter* pTileDb, LONGLONG tid, QImage bk_image)
	{//������Ҫ�ϲ������ݿ�
		mBlockDBMutex.lock();//����
		UnitImage(tid, pTileDb, &bk_image);
		mBlockDBMutex.unlock();
	}

	void qCutEdomThread::DelBlock(qTileDbPrivateWriter* pTileDb, LONGLONG tid)
	{//�����Ҫɾ�������ݿ�
		mBlockDBMutex.lock();//����
		m_DelTIDs.push_back(tid);
		mBlockDBMutex.unlock();
		DelBlockData(pTileDb);
	}

	void qCutEdomThread::DelBlockData(qTileDbPrivateWriter* pTileDb, bool all)
	{
		mBlockDBMutex.lock();//����
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


	void qCutEdomThread::run()
	{
		if (!m_pDataBoundPolygon)
		{
			signalMessage(QString::fromWCharArray(L"û�вü����������"));
			return;
		}

		EV_DB_Header	mTileInfo;
		qTileDbPrivateWriter		mTileStore;
		clock_t start, finish;
		start = clock();
		if (!mTileStore.open(m_EdomPath))
		{
			signalMessage(QString::fromWCharArray(L"��EDOM����ʧ��"));
			return;
		}
		mTileStore.ReadHeader(mTileInfo);
		int minLevel = mTileInfo.minlevel;
		int maxLevel = mTileInfo.maxlevel;
		/*
		������:����С�ļ���ʼ��
		1���������ȫ�ڲü�����Χ�ڣ��򱾿���ӿ鲻�ٵݹ鴦��
		2�������Ͳü������ཻ�������������ӿ��ж�
		3��������ڲü�����Χ�⣬�����Լ��ӿ�ȫ��ɾ��
		*/
		int process_index;
		qEdomTileProviderUtils utils;
		int level = minLevel;//����С�ļ���ʼ������ĵݹ�
		for (process_index = 0; process_index < m_ProcessThreadCount; process_index++)
		{
			m_BlockThreads[process_index]->Init(m_pDataBoundPolygon,
				&utils, &mTileInfo, &mTileStore);
			m_BlockThreads[process_index]->start();
		}
		bool addover = false;
		signalMessage(QString::fromWCharArray(L"���ڴ�������ݼ���Χ(%1-%2)").arg(minLevel).arg(maxLevel));
		int min_lx = utils.GetIndex_X(mTileInfo.minx, level);
		int max_lx = utils.GetIndex_X(mTileInfo.maxx, level);
		int min_ly = utils.GetIndex_X(mTileInfo.miny, level);
		int max_ly = utils.GetIndex_X(mTileInfo.maxy, level);

		int per = 0;
		int value_count = (max_lx - min_lx + 1) * (max_ly - min_ly + 1);
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

				//��ʾ������
				signalProcess(per * 100 / value_count);
			}
		}

		signalMessage(QString::fromWCharArray(L"���ں�̨�������Ĵ�����ȴ�...."));
		while (true)
		{
			int process_count = 0;//���ڴ�����̸߳���
			for (process_index = 0; process_index < m_ProcessThreadCount; process_index++)
			{
				if (!m_BlockThreads[process_index]->IsProcessging())
					process_count++;//��ʾ�Ѿ�������ɵ��߳�
			}
			msleep(100);
			DelBlockData(&mTileStore, true);
			if (process_count == m_ProcessThreadCount)
				break;
		}
		for (process_index = 0; process_index < m_ProcessThreadCount; process_index++)
			m_BlockThreads[process_index]->Fini();

		DelBlockData(&mTileStore, true);
		QgsGeometry geo = QgsGeometry::fromPolygonXY(*m_pDataBoundPolygon);
		QgsRectangle bound = geo.boundingBox();
		mTileInfo.minx = bound.xMinimum();
		mTileInfo.maxx = bound.xMaximum();
		mTileInfo.miny = bound.yMinimum();
		mTileInfo.maxy = bound.yMaximum();
		mTileStore.DelSysTile();
		mTileStore.SaveSysTile(mTileInfo);
		mTileStore.close();

		finish = clock();//���ʱ��
		long seconds = (finish - start) / CLOCKS_PER_SEC;//��
		QString strmsg;
		if (seconds <= 60) {
			strmsg = QString("%1").arg(seconds) + QString::fromWCharArray(L"��");
		}
		else {
			long d = 0;
			long h = 0;
			long m = seconds / 60;
			seconds = seconds % 60;//�õ���
			if (m > 60)
			{
				h = m / 60;
				m = m % 60;
			}
			if (h > 24)
			{//�������
				d = h / 24;
				h = h % 24;
			}

			wchar_t msg[256];
			if (d > 0)
				swprintf(msg, L"%d��%dʱ%d��%d��", d, h, m, seconds);
			else
				swprintf(msg, L"%dʱ%d��%d��", h, m, seconds);
			strmsg = QString::fromWCharArray(msg);
		}
		QString tr_timeDiff = QString(QString::fromWCharArray(L"���ݴ������!������ʱ�䣺") + "%1").arg(strmsg); //float->QString
		signalMessage(tr_timeDiff);
		signalFinish();
	}

	void qCutEdomThread::UnitImage(LONGLONG tileId, qTileDbPrivateWriter* pTileDb, QImage* bk_map)
	{
		QByteArray data = pTileDb->read(tileId);
		data.remove(0, 8);//ȥ��ǰ���8���ֽ�
		QImage block_image;
		if (!block_image.loadFromData(data))
			return;

		QPainter thePainter(bk_map);
		thePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		thePainter.drawImage(0, 0, block_image);
		//��ͼƬת��Ϊ�ֽ�
		QByteArray bytes;
		QBuffer buffer(&bytes);
		buffer.open(QIODevice::WriteOnly);

		string saveformat = "JPG";
		//�������ж������Ƿ�û��͸���ģ������JPG�洢���������PNG
		for (int y = 0; y < 256; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				QColor clr = bk_map->pixelColor(x, y);
				if (clr.alpha() == 0)//��ʾȫ͸��
				{
					saveformat = "PNG";
					y = 300;//����ѭ��
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
	qCutTileThread::qCutTileThread(qTileProviderUtils* utils, QObject* parent)
		: QThread(parent)
		, mPixmap(256 * GRID_SIZE, 256 * GRID_SIZE, QImage::Format_ARGB32)
		, mAlphaPixmap(256 * GRID_SIZE, 256 * GRID_SIZE, QImage::Format_ARGB32)
	{
		m_pTileUtils = utils;
	}

	void qCutTileThread::Init(qTileMultiThread* parent, QgsMapCanvas* ifaceCanvas)
	{
		m_Parent = parent;
		mSettings = ifaceCanvas->mapSettings();
		QSize sz = mPixmap.size();
		//int dpi = mPixmap.logicalDpiX();
		//mSettings.setOutputDpi(dpi);
		mSettings.setOutputSize(sz);
		mSettings.setBackgroundColor(Qt::transparent);
		//	mSettings.setFlag(QgsMapSettings::Antialiasing, true);//���д���Ҫȥ��

			//mSettings.setCrsTransformEnabled(true);
		mAlphaSettings = mSettings;
		//���û�����������
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

		//����Alphaͼ�����
		InitAlphaSettings();
		////////////////////////////////////

		m_bRun = true;
		m_working = false;
		this->start();
	}

	void qCutTileThread::Fini()
	{
		KillThread();
	}

	qCutTileThread::~qCutTileThread()
	{

	}

	void qCutTileThread::InitAlphaSettings()
	{//����Alphaͼ�����
		QList<QgsMapLayer*> Layers;
		InitAlphaData(Layers);
		mAlphaSettings.setLayers(Layers);
	}

	void qCutTileThread::InitAlphaData(QList<QgsMapLayer*>& Layers)
	{
		if (!m_Parent)
			return;

		//�п���������п����ǵ���ͼ��
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

	void qCutTileThread::AddTreeGroup(QString GroupName, QList<QgsMapLayer*>& Layers)
	{
		QgsLayerTreeGroup* group = (QgsLayerTreeGroup*)QgsProject::instance()->layerTreeRoot();
		if (!group)
			return;

		QgsLayerTreeGroup* g = group->findGroup(GroupName);
		if (!g)
			return;

		QList<QgsLayerTreeLayer*> treelayer = g->findLayers();
		int i, count = treelayer.count();
		bool mapInit = false;
		for (i = 0; i < count; i++)
		{
			QgsMapLayer* layer = treelayer[i]->layer();
			// Add the Layer to the Layer Set
			Layers.append(layer);
		}
	}

	void qCutTileThread::AddMapLayer(QString LayerName, QList<QgsMapLayer*>& Layers)
	{
		QList<QgsMapLayer*> mapLayers = QgsProject::instance()->mapLayersByName(LayerName);
		for (int i = 0; i < mapLayers.count(); i++)
		{
			QgsMapLayer* mypLayer = mapLayers[i];
			if (!mypLayer->isValid())
				continue;

			// Add the Layer to the Layer Set
			Layers.append(mypLayer);
		}
	}

	bool qCutTileThread::AddData(QgsRectangle outRect, int minx, int miny, int maxx, int maxy,
		int level, int minIndX, int minIndY, int maxIndX, int maxIndY)
	{
		if (mOutRects.size() >= MAX_LISTCOUNT)
			return false;

		qCutTileStru data;
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

	void qCutTileThread::run()
	{
		while (m_bRun)
		{//�߳�һֱѭ��
			if (mOutRects.size() < 1)
			{
				msleep(100);//����
				continue;
			}

			BeginWork();//��ǳɱ��߳����ڴ���
			mOutRectMutex.lock();
			qCutTileStru info = mOutRects[0];
			mOutRects.removeFirst();
			mOutRectMutex.unlock();

			//��Ⱦ��ͼ��ͼƬ
			mSettings.setExtent(info.outRect);
			mPixmap.fill(Qt::transparent);//���д������ʹ����͸��

			mpainter.begin(&mPixmap);//
			QgsMapRendererCustomPainterJob job(mSettings, &mpainter);
			job.renderSynchronously();
			mpainter.end();

			//Alpha����
			if (mAlphaSettings.layers().count() > 0) {
				mAlphaSettings.setExtent(info.outRect);
				mAlphaPixmap.fill(Qt::transparent);//���д������ʹ����͸��
				mpainter.begin(&mAlphaPixmap);//
				QgsMapRendererCustomPainterJob job(mAlphaSettings, &mpainter);
				job.renderSynchronously();
				mpainter.end();

				//��ʼ����AlphaͼƬ
				for (int y = 0; y < mPixmap.height(); y++)
				{
					for (int x = 0; x < mPixmap.width(); x++)
					{
						QColor org_clr = mPixmap.pixelColor(x, y);
						if (org_clr.alpha() == 0)
							continue;
						QColor clr = mAlphaPixmap.pixelColor(x, y);
						if (clr.alpha() == 0)
							continue;//͸���Ĳ��ֲ�������ȥ
						//��͸���Ĳ�����Ҫ�ѵ�ͼ����͸��
						org_clr.setAlpha(255 - clr.alpha());
						mPixmap.setPixelColor(x, y, org_clr);
					}
				}
			}
			/////////////////////////////

			if (mPixmap.height() > 1 && mPixmap.width() > 1)
			{
				//��ʼ�и�ͼƬ����
				for (int ny = info.miny; ny < info.maxy; ny++)
				{
					if (ny > info.maxIndY)
						break;
					for (int nx = info.minx; nx < info.maxx; nx++)
					{
						if (nx > info.maxIndX)
							break;
						//����Ⱦ��ͼƬ���ݴ洢����Ƭ���ݿ���
						LONGLONG id = BLOCK_ID(nx, ny, info.level);
						saveTileData(id, info.outRect, mPixmap);
					}
				}
			}

			EndWork();//�������߳�����
		}
	}
	//#define	COLOR_ERROR	2 //��ɫ����Χ
	//����Ƭ���ָ�Ϊ256*256��С��Ƭ
	void qCutTileThread::saveTileData(LONGLONG tileId, QgsRectangle MapRect, QImage& pixMap)
	{
		ENVELOPE tileRect;//�����ķ�Χ
		m_pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);
		if (!m_Parent->IDInValidateBound(tileRect))
			return;

		if (!IsValid(m_Parent->GetBoundPolygon(), tileId))
			return;//��������Χ��

		int height = pixMap.height();
		int width = pixMap.width();
		int start_lx = (tileRect.minx - MapRect.xMinimum()) * width / MapRect.width();//���㿪ʼX��
		int start_ly = (MapRect.yMaximum() - tileRect.maxy) * height / MapRect.height();

		QImage outmap = pixMap.copy(start_lx, start_ly, 256, 256);
		ShapeImage(m_Parent->GetBoundPolygon(), tileId, &outmap);//������βü�

		int x, y;
		//////////////////////////////
		string saveformat = "PNG";
		int quality = -1;
		if (m_Parent->m_bCompressTile)
		{
			//��Ҫѹ��
			quality = 75;//75%��ѹ����
			saveformat = "JPG";
			//�������ж������Ƿ�û��͸���ģ������JPG�洢���������PNG
			for (y = 0; y < 256; y++)
			{
				for (x = 0; x < 256; x++)
				{
					QColor clr = outmap.pixelColor(x, y);
					if (clr.alpha() == 0)//��ʾȫ͸��
					{
						saveformat = "PNG";
						quality = -1;//�������͸����ͨ����ѹ��
						y = 300;//����ѭ��
						break;
					}
				}
			}
		}

		//��ͼƬת��Ϊ�ֽ�
		QByteArray bytes;
		QBuffer buffer(&bytes);
		buffer.open(QIODevice::WriteOnly);
		if (m_Parent->m_bCompressTile)
			outmap = outmap.convertToFormat(QImage::Format_ARGB6666_Premultiplied);//The image is stored using 24-bit indexes into a colormap.

	//	outmap = outmap.convertToFormat(QImage::Format::Format_Indexed8);
		outmap.save(&buffer, saveformat.c_str(), quality);
		m_Parent->SaveData(tileId, bytes, saveformat);
	}

	bool qCutTileThread::IsValid(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId)
	{
		//ͼ��ü�������ü��ǶԳ������ݷ�Χ������Ϊ͸��
		if (!pDataBoundPolygon || pDataBoundPolygon->size() < 1)
			return true;

		ENVELOPE tileRect;//�����ķ�Χ
		m_pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);
		QgsPolygonXY img_polygon;
		QgsPolylineXY res;
		res.resize(5);
		res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));
		res.push_back(QgsPointXY(tileRect.minx, tileRect.maxy));
		res.push_back(QgsPointXY(tileRect.maxx, tileRect.maxy));
		res.push_back(QgsPointXY(tileRect.maxx, tileRect.miny));
		res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));

		img_polygon.push_back(res);
		QgsGeometry geo = QgsGeometry::fromPolygonXY(img_polygon);
		QgsGeometry bndGeo = QgsGeometry::fromPolygonXY(*pDataBoundPolygon);
		if (bndGeo.isEmpty())return false;
		//�ж϶�����Ƿ��ཻ
		return !bndGeo.contains(geo); //����������ȫ�ڶ����m_pDataBoundPolygon��Χ�ڣ�����Ҫ�ü���
	}

	void qCutTileThread::ShapeImage(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId, QImage* outmap)
	{
		//ͼ��ü�������ü��ǶԳ������ݷ�Χ������Ϊ͸��
		if (!pDataBoundPolygon || pDataBoundPolygon->size() < 1)
			return;

		ENVELOPE tileRect;//�����ķ�Χ
		m_pTileUtils->GetBlockRect(BLOCK_ID_X(tileId), BLOCK_ID_Y(tileId), BLOCK_ID_LEVEL(tileId), tileRect);

		QgsPolygonXY img_polygon;
		QgsPolylineXY res;
		res.resize(5);
		res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));
		res.push_back(QgsPointXY(tileRect.minx, tileRect.maxy));
		res.push_back(QgsPointXY(tileRect.maxx, tileRect.maxy));
		res.push_back(QgsPointXY(tileRect.maxx, tileRect.miny));
		res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));

		img_polygon.push_back(res);
		QgsGeometry geo = QgsGeometry::fromPolygonXY(img_polygon);
		QgsGeometry bndGeo = QgsGeometry::fromPolygonXY(*pDataBoundPolygon);
		if (bndGeo.isEmpty() || geo.isEmpty())return;
		if (bndGeo.contains(geo))
			return;//����������ȫ�ڶ����m_pDataBoundPolygon��Χ�ڣ�����Ҫ�ü���

		QPolygon poly;
		QgsPolylineXY line = pDataBoundPolygon->at(0); //ֻҪ�������
		int lx, ly, pnt_count = line.size();
		for (int n = 0; n < pnt_count; n++)
		{
			QgsPointXY pnt = line.at(n);
			double dx = pnt.x();
			double dy = pnt.y();

			lx = (dx - tileRect.minx) * 256 / (tileRect.maxx - tileRect.minx);
			ly = (tileRect.maxy - dy) * 256 / (tileRect.maxy - tileRect.miny);
			poly.append(QPoint(lx, ly));
		}

		QImage bk_image = outmap->copy(0, 0, 256, 256);//����һ��256*256��ͼƬ
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
		QRgb rgba = QColor(0, 0, 0, 0).rgba();//�滻����ɫ������͸���ģ�����QColor(0,0,0��0)��
		for (int ly = 0; ly < 256; ly++)
		{
			double dy = tileRect.maxy - (tileRect.maxy - tileRect.miny) * ly / 256.0;
			for (int lx = 0; lx < 256; lx++)
			{
				double dx = tileRect.minx + (tileRect.maxx - tileRect.minx) * lx / 256.0;
				if (pDataBoundPolygon->InPolygon(dx, dy))
					continue;

				outmap->setPixel(lx, ly, rgba);//���ø�����Ϊ͸��
			}
		}*/
		///////////////////////////////////////////////

	}
	////////////////////////////////////////////////////////////////////////
	qTileMultiThread::qTileMultiThread(QObject* parent
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
		mTileStore = new qTileDb(tileDbPath, tileInfo, mIsCreateNewFile);

		m_bCompressTile = false;
		m_ProcessCount = ProcessNum;
		if (tileInfo.dbtype == 3)
			m_pTileUtils = new qWebMecatorProviderUtils();
		else
			m_pTileUtils = new qEdomTileProviderUtils();
		//�������߳�
		CreateCutTileThread(parent, ifaceCanvas);
		ReadBound(PolygoFilePath);
	}

	qTileMultiThread::~qTileMultiThread(void)
	{
		FreeCutTileThread();
		SAFE_DELETE(mTileStore);
		SAFE_DELETE(m_pDataBoundPolygon);
		SAFE_DELETE(m_pTileUtils);
	}

	//�ж�ĳ��ID�Ƿ���������Ч��Χ��
	bool qTileMultiThread::IDInValidateBound(ENVELOPE tileRect)
	{
		if (m_ValidateBound.size() < 1)
			return true;//û������ֱ�ӷ���TRUE

		QgsRectangle rect(tileRect.minx, tileRect.miny, tileRect.maxx, tileRect.maxy);
		int count = m_ValidateBound.size();
		for (int i = 0; i < count; i++)
		{
			if (m_ValidateBound[i].contains(rect) || m_ValidateBound[i].intersects(rect))
				return true;//���������ཻ
		}

		return false;
	}

	void qTileMultiThread::ReadBound(QString shp_file)
	{//��shp�ļ����ȡ���ݷ�Χ
		m_pDataBoundPolygon = NULL;
		//���ж��ļ��Ƿ����
		QFileInfo fileInfo(shp_file);
		if (!fileInfo.exists() || !fileInfo.isFile())
			return;

		//qgs open shp�ļ����ȡ��һ��Ҫ�صķ�Χ
		QgsVectorLayer vlayer = QgsVectorLayer(shp_file, "templ_layer" + qrand(), "ogr");
		if (!vlayer.isValid() || vlayer.featureCount() <= 0)return;

		QgsGeometry qgsGeometry = vlayer.getFeature(QgsFeatureId(0)).geometry();
		if (!qgsGeometry.isEmpty() && qgsGeometry.type() == Qgis::GeometryType::Polygon)
		{
			QgsRectangle rect = qgsGeometry.boundingBox();
			m_pDataBoundPolygon = new  QgsPolygonXY();
			QgsPolylineXY qgsLine;
			qgsLine.append(QgsPointXY(rect.xMinimum(), rect.yMinimum()));
			qgsLine.append(QgsPointXY(rect.xMinimum(), rect.yMaximum()));
			qgsLine.append(QgsPointXY(rect.xMaximum(), rect.yMaximum()));
			qgsLine.append(QgsPointXY(rect.xMaximum(), rect.yMinimum()));
			qgsLine.append(QgsPointXY(rect.xMinimum(), rect.yMinimum()));
			m_pDataBoundPolygon->append(qgsLine);
		}
		else m_pDataBoundPolygon = nullptr;
	}

	bool qTileMultiThread::ConnectDBSucceed()
	{//�����Ƿ��������ݿ�ɹ�
		if (!mTileStore)
			return false;

		return mTileStore->IsConnection();
	}

	void qTileMultiThread::SaveData(LONGLONG id, QByteArray databytes, string format)
	{
		//�������ݵ��ŶӶ�����
		qSaveBlockStru data;
		data.id = id;
		data.bytes = databytes;
		int size = data.bytes.size();
		char type[4];
		strcpy(type, format.c_str());
		type[3] = '\0';
		data.bytes.insert(0, (const char*)&size, sizeof(int));
		data.bytes.insert(0, type, 4);
		mSaveListMutex.lock();//����
		m_SaveList.append(data);
		mSaveListMutex.unlock();//����
		//image.FreeMemory(buffer);
	}

	bool qTileMultiThread::AddThread(QgsRectangle outRect, int minx, int miny, int maxx, int maxy,
		int level, int minIndX, int minIndY, int maxIndX, int maxIndY)
	{
		//��������ӵ����и������ٵ��߳���
		qCutTileThread* pCutThread = NULL;
		int count = 0;
		for (int i = 0; i < GetProcessCount(); i++)
		{
			if (i == 0)
			{
				count = m_CutTileThreads[i]->GetListCount();
				pCutThread = m_CutTileThreads[i];
			}
			else {
				if (count > m_CutTileThreads[i]->GetListCount())
				{
					count = m_CutTileThreads[i]->GetListCount();
					pCutThread = m_CutTileThreads[i];
				}
			}
		}
		if (pCutThread && pCutThread->AddData(outRect, minx, miny, maxx, maxy, level, minIndX, minIndY, maxIndX, maxIndY))
			return true;

		return false;
	}
	void qTileMultiThread::RenderBlockByLevel(int level)
	{//һ�β���9*9�Ŀ���ͼ��Ȼ��ֱ���ID��洢���������Լӿ�������ͼ���ٶ�
		long minIndX = m_pTileUtils->GetIndex_X(mTileInfo.minx, level);
		long minIndY = m_pTileUtils->GetIndex_Y(mTileInfo.miny, level);
		long maxIndX = m_pTileUtils->GetIndex_X(mTileInfo.maxx, level);
		long maxIndY = m_pTileUtils->GetIndex_Y(mTileInfo.maxy, level);
		double lWidth = m_pTileUtils->GetLevelWidth(level);
		wchar_t ss[256];
		swprintf(ss, L"���ڴ���ļ���%d", level);
		m_msg = QString::fromWCharArray(ss);
		emit signalMessage(m_msg);//������Ϣ
		emit signalProcess(0);//������Ϣ

		if (minIndY > maxIndY)
			swap(maxIndY, minIndY);

		m_TileCount = (maxIndX - minIndX + 1) * (maxIndY - minIndY + 1);
		m_CurTileStep = 0;//��ʼ����
		bool exitblock;//�ж��������Ƿ񶼴��ڣ������һ�������ھ�Ҫ����
		for (int ly = minIndY; ly <= maxIndY; ly += (GRID_SIZE - 2))
		{
			for (int lx = minIndX; lx <= maxIndX; lx += (GRID_SIZE - 2))
			{
				if (mStop)break;//����û�ѡ��������˳�

				if (!mIsCreateNewFile)
				{
					//�����ж����ݿ����Ƿ�������ID�ˣ�����Ѿ���������Ҫ����
					exitblock = true;
					for (int ny = ly; ny < ly + GRID_SIZE - 2; ny++)
					{
						if (ny > maxIndY)
							break;
						for (int nx = lx; nx < lx + GRID_SIZE - 2; nx++)
						{
							if (nx > maxIndX)
								break;
							//����Ⱦ��ͼƬ���ݴ洢����Ƭ���ݿ���
							LONGLONG id = BLOCK_ID(nx, ny, level);
							if (!mTileStore->ExitTile(id))
							{//�����һ�������ڣ���Ҫ�����������ͼ
								exitblock = false;
								break;
							}
						}
						if (!exitblock)
							break;
					}
					if (exitblock)
					{//������е����ݶ��Ѿ����ڣ���ӿ����
						m_CurTileStep += (GRID_SIZE - 2) * (GRID_SIZE - 2);
						emit signalProcess(m_CurTileStep * 100 / m_TileCount);
						continue;
					}
				}
				////////////////////////////////////////////////////////
				QgsRectangle MinRect, MaxRect, outRect;
				m_pTileUtils->GetBlockRect(lx - 1, ly - 1, level, MinRect);//�������Χ
				m_pTileUtils->GetBlockRect(lx + GRID_SIZE - 2, ly + GRID_SIZE - 2, level, MaxRect);//�������Χ
				outRect.set(MinRect.xMinimum(), min(MinRect.yMinimum(), MaxRect.yMinimum()),
					MaxRect.xMaximum(), max(MinRect.yMaximum(), MaxRect.yMaximum()));//�������Χ

				while (true)
				{//ѭ�������봦�����ݵ��߳���
					if (mStop)break;//����û�ѡ��������˳�
					if (AddThread(outRect, lx, ly, lx + GRID_SIZE - 2, ly + GRID_SIZE - 2, level, minIndX, minIndY, maxIndX, maxIndY))
					{
						break;
					}

					//����Ҫ��ͣʱ�䳤�㣬��̨�̶߳��ж����ˣ���Ҫ��ͣʱ���Щ
					msleep(500);//���߳���Ϣһ�£��ø���̨��ͼ�߳�
				}
				//���̵߳ȴ������ݴ����������õȴ�ʱ��
				msleep(100);//���߳���Ϣһ�£��ø���̨��ͼ�߳�

				//������Ϣ
				ShowThreadInfo();
				SaveTileToDB(false);

				//���̵߳ȴ������ݴ����������õȴ�ʱ��
			//	msleep(100);//���߳���Ϣһ�£��ø���̨��ͼ�߳�
			}
		}
		//��󿴿��Ƿ��к�̨�����ڴ�����λ��ڴ����Ҫ�ȴ�
		while (true)
		{
			int count = m_CutTileThreads.size();
			int total = 0;
			for (int i = 0; i < count; i++)
			{
				if (!m_CutTileThreads[i]->working() && m_CutTileThreads[i]->GetListCount() < 1)
					total++;//�̴߳�������״̬����������ȫ����ͼ�����
			}

			if (total >= count)
				break;//�������

			//������Ϣ
			ShowThreadInfo();
			//����
			msleep(300);//���߳���Ϣһ�£��ø���̨��ͼ�߳�
			SaveTileToDB(false);//������ݱ��棬ȫ���ύ����
		}

		SaveTileToDB(true);//������ݱ��棬ȫ���ύ����
	}

	//����AllCommitΪ�Ƿ�ȫ���ύ
	void qTileMultiThread::SaveTileToDB(bool AllCommit)
	{//�������ݿ飬һ��ȫ���������
		int count = m_SaveList.size();
		int thread_count = m_CutTileThreads.size() * 10;//�����̵߳�����������һ�������������ļ�¼����
		if (!AllCommit && count < thread_count)
		{
			return;
		}
		mSaveListMutex.lock();
		//�ж��Ƿ������ݿ����Ѿ����������ݵ�
		for (int i = count - 1; i >= 0; i--)
		{
			LONGLONG id = m_SaveList[i].id;
			if (!mTileStore->ExitTile(id))
			{//������ݲ����ڣ���������Ĳ���
				continue;
			}
			m_SaveList.removeAt(i);
		}
		mTileStore->save(&m_SaveList);
		//������Ϣ
		ShowThreadInfo();
		m_CurTileStep += count;
		////////////////////////////////////
		//test!!!!!!!!!!!!!!!
		//QString strmsg = QString(",test 2:Step=%1, Count=%2,Per=%3").arg(m_CurTileStep).arg(m_TileCount).arg(m_CurTileStep * 100 / m_TileCount);
		//strmsg = m_msg + strmsg;
		//emit signalMessage(strmsg);
		//////////////////////////////////
		emit signalProcess(m_CurTileStep * 100 / m_TileCount);
		m_SaveList.clear();
		mSaveListMutex.unlock();//�м�Ҫ����
	}

	void qTileMultiThread::ShowThreadInfo()
	{
		int ProcessCount = GetProcessCount();
		int total = 0;
		for (int k = 0; k < ProcessCount; k++)
		{
			total += m_CutTileThreads[k]->GetListCount();
		}

		emit signalThreadPercent(total * 100 / (ProcessCount * MAX_LISTCOUNT));
	}

	void qTileMultiThread::CreateCutTileThread(QObject* parent, QgsMapCanvas* ifaceCanvas)
	{
		for (int i = 0; i < GetProcessCount(); i++)
		{
			qCutTileThread* thread = new qCutTileThread(m_pTileUtils, parent);
			thread->Init(this, ifaceCanvas);
			m_CutTileThreads.append(thread);
		}
	}

	void qTileMultiThread::FreeCutTileThread()
	{
		int i, count = m_CutTileThreads.size();
		for (i = 0; i < count; i++)
		{
			m_CutTileThreads[i]->Fini();
			msleep(500);
		}
		for (i = 0; i < count; i++)
		{
			delete m_CutTileThreads[i];
		}
		m_CutTileThreads.clear();
	}


	void qTileMultiThread::run()
	{
		if (mtileDbPath.isEmpty())return;


		clock_t start, finish;
		start = clock();
		this->mTileStore->startTransaction();
		/////////////////////////////

		for (int l = mTileInfo.minlevel; l <= mTileInfo.maxlevel; l++)
			this->RenderBlockByLevel(l);

		/////////////////////////////////////////
		this->mTileStore->SaveDataInfo(myear, mgdcode, mname, malias, mzt);
		this->mTileStore->endTransaction();
		this->mTileStore->close();
		finish = clock();//���ʱ��

		//������ɵ�ʱ��
		long seconds = (finish - start) / CLOCKS_PER_SEC;//��
		QString strmsg;
		if (seconds <= 60) {
			strmsg = QString("%1").arg(seconds) + QString::fromWCharArray(L"��");
		}
		else {
			long h = 0;
			long m = seconds / 60;
			seconds = seconds % 60;//�õ���
			if (m > 60)
			{
				h = m / 60;
				m = m % 60;
			}

			wchar_t msg[256];
			swprintf(msg, L"%dʱ%d��%d��", h, m, seconds);
			strmsg = QString::fromWCharArray(msg);
		}

		emit signalMessage(QString::fromWCharArray(L"�������"));
		emit signalProcess(100);
		emit signalFinish();
		emit DataFinish(strmsg);//��ʾ��Ϣ������������
	}

	///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////
	qTileDownloadThread::qTileDownloadThread(QObject* parent)
		: QThread(parent)
	{
		mStop = false;
		mTileStore = NULL;
	}

	qTileDownloadThread::~qTileDownloadThread()
	{
		if (mTileStore)
			delete mTileStore;

		mTileStore = NULL;
		mStop = false;
	}

	void qTileDownloadThread::run()
	{//��ʼҪ����
		//��ʼҪ��ʼ��
		//��ԭʼ������ͼ��
		mStop = false;
		mTileInfo.minlevel = m_minlevel;
		mTileInfo.maxlevel = m_maxlevel;
		QgsGeometry geo = QgsGeometry::fromPolygonXY(mDownloadGeometry);
		QgsRectangle bound = geo.boundingBox();
		mTileInfo.minx = bound.xMinimum();
		mTileInfo.maxx = bound.xMaximum();
		mTileInfo.miny = bound.yMinimum();
		mTileInfo.maxy = bound.yMaximum();

		mTileStore = new qTileDb(mtileDbPath, mTileInfo, true);
		if (!mTileStore->IsConnection())
		{
			emit signalMessage(QString::fromWCharArray(L"�����ļ�·��ʧ��:") + mtileDbPath);
			return;
		}
		emit signalMessage(QString::fromWCharArray(L"�����ļ�·���ɹ�:") + mtileDbPath);
		emit signalMessage(QString::fromWCharArray(L"��ʼ�������ݣ����ؼ���Χ:") + QString("[%1,%2]").arg(m_minlevel).arg(m_maxlevel));

		for (int level = m_minlevel; level <= m_maxlevel; level++)
		{
			int min_lx = qTileUtils::GetIndex(mTileInfo.minx, level);
			int max_lx = qTileUtils::GetIndex(mTileInfo.maxx, level);
			int min_ly = qTileUtils::GetIndex(mTileInfo.miny, level);
			int max_ly = qTileUtils::GetIndex(mTileInfo.maxy, level);

			DownloadData(level, min_lx, max_lx, min_ly, max_ly);
		}

		if (mTileStore)
			delete mTileStore;

		mTileStore = NULL;

		//����
		emit signalMessage(QString::fromWCharArray(L"�������"));
		emit signalFinish();
	}

	void qTileDownloadThread::DownloadData(int level, int min_lx, int max_lx, int min_ly, int max_ly)
	{
		int pos = 0;
		int count = (max_ly - min_ly) * (max_lx - min_lx);
		emit signalThreadPercent(level, pos);
		for (int ly = min_ly; ly <= max_ly; ly++)
		{
			if (mStop)
				return;

			qTileRequests requestsFinal;//��������¼���
			for (int lx = min_lx; lx <= max_lx; lx++)
			{
				int s = (ly - min_ly) * (lx - min_lx) * 100 / (count == 0 ? 1 : count);
				if (s > pos)
				{
					pos = s;
					emit signalThreadPercent(level, pos);
				}
				LONGLONG tid = BLOCK_ID(lx, ly, level);
				//�ж�ID�Ƿ�������Χ�ڣ������������Ҫ����
				GEO_RECT tileRect = qTileUtils::GetGeoRectById(tid);
				QgsPolygonXY img_polygon;
				QgsPolylineXY res;
				res.resize(5);
				res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));
				res.push_back(QgsPointXY(tileRect.minx, tileRect.maxy));
				res.push_back(QgsPointXY(tileRect.maxx, tileRect.maxy));
				res.push_back(QgsPointXY(tileRect.maxx, tileRect.miny));
				res.push_back(QgsPointXY(tileRect.minx, tileRect.miny));

				img_polygon.push_back(res);
				QgsGeometry geo = QgsGeometry::fromPolygonXY(img_polygon);
				QgsGeometry bndGeo = QgsGeometry::fromPolygonXY(mDownloadGeometry);
				if (bndGeo.isEmpty() || geo.isEmpty())continue;
				if (!bndGeo.contains(geo))
					continue;//���ཻ

				//��������
				//����������ݿ���Ϣ
				QString url = GetURL(tid);
				qTileRequest rqst(QUrl(url), m_type + 1, QRect(0, 0, 0, 0), tid);
				requestsFinal.append(rqst);//���ص���Ҫ���صĶ�����

				if (mStop)
					return;

				if (requestsFinal.size() > 10)
				{
					qEdomDownloadHandler handler(&mDownloadGeometry, requestsFinal, mTileStore);
					handler.downloadBlocking();

					requestsFinal.clear();
				}
				if (mStop)
					return;

			}

			if (requestsFinal.size() > 0)
			{
				qEdomDownloadHandler handler(&mDownloadGeometry, requestsFinal, mTileStore);
				handler.downloadBlocking();

				requestsFinal.clear();
			}

		}
	}

	QString qTileDownloadThread::GetURL(LONGLONG id)
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
			//���׾�����Ƭ����ת�������ͼ�Ĺ���
			return GetTDBaseUrl(level + 2, lx + (1 << (level + 1)), (1 << level) - ly - 1);
		}
		case 4://���ң�е�ͼ
		{
			//���׾�����Ƭ����ת���ɲ���ͼ�Ĺ���
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

	//���ݲ�ͬ��ͼ���ͣ���ȡ���ص�url ��ַ
	QString qTileDownloadThread::GetCHBaseUrl(int downloadlevel, int i, int j)
	{
		//http://60.205.227.207:7090/onemap17/rest/wmts?ACCOUNT=ghyforestry&PASSWD=ghyforestry&layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileMatrix=10&TileCol=1664&TileRow=358
		QString url = m_url + QString("/rest/wmts?layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileCol=%1&TileRow=%2&TileMatrix=%3").arg(i).arg(j).arg(downloadlevel);
		if (!mUserID.isEmpty())
			url += QString("&ACCOUNT=%1").arg(mUserID);
		if (!mPwd.isEmpty())
			url += QString("&PASSWD=%1").arg(mPwd);
		return url;
	}


	//���ݲ�ͬ��ͼ���ͣ���ȡ���ص�url ��ַ
	QString qTileDownloadThread::GetTDBaseUrl(int downloadlevel, int i, int j)
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

	//�������
	void qTileDownloadThread::slot_requestFinished(bool bSuccess, char type, LONGLONG tid, const QByteArray& data)
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
		, const  QString tileDbPath) : QThread(parent)
		, mtileDbPath(tileDbPath)
	{

	}

	CAddTopTileThread::~CAddTopTileThread(void)
	{

	}

	QImage* CAddTopTileThread::createImage(int grid) const
	{//����һ��256*3��С��λͼ
		int width = grid;//Ĭ�ϵĴ�С
		int height = grid;
		//ֱ�������͸����PNG��ʽ
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
		//�̶߳�������
		mTileStore = new qTileDbPrivateWriter();
		//������
		if (mTileStore->open(mtileDbPath))
		{
			msg = QString::fromWCharArray(L"��edom�ļ��ɹ�");
			emit signalMessage(msg);//������Ϣ
		}
		else {
			msg = QString::fromWCharArray(L"��edom�ļ�ʧ��");
			emit signalMessage(msg);//������Ϣ
		}

		mTileStore->ReadHeader(mTileInfo);
		//��ʼ����
		wchar_t ss[256];
		int minlevel = mTileInfo.minlevel;
		for (int level = minlevel - 1; level >= 0; level--)
		{
			int minlx = qTileUtils::GetIndex(mTileInfo.minx, level);
			int maxlx = qTileUtils::GetIndex(mTileInfo.maxx, level);
			int minly = qTileUtils::GetIndex(mTileInfo.miny, level);
			int maxly = qTileUtils::GetIndex(mTileInfo.maxy, level);

			swprintf(ss, L"��ʼ�޲�����%d", level);
			msg = QString::fromWCharArray(ss);
			emit signalMessage(msg);//������Ϣ
			int count = (maxly - minly + 1) * (maxlx - minlx + 1);
			int percent = 0;
			for (int ly = minly; ly <= maxly; ly++)
			{
				for (int lx = minlx; lx <= maxlx; lx++)
				{
					LONGLONG id = BLOCK_ID(lx, ly, level);
					signalThreadPercent(level, (++percent * 100 / count));//���͵�ǰ����
					if (mTileStore->ExitTile(id))
						mTileStore->del(id);//����Ҫɾ��������������

					QImage* theImage = createImage(256);
					if (!theImage)
						continue;

					QPainter* painter = new QPainter(theImage);
					ENVELOPE bound;
					qTileUtils::GetBlockRect(lx, ly, level, bound);
					//�ϲ�
					bool save_flag = false;
					LONGLONG child_ids[4];
					qTileUtils::GetLowIds(id, child_ids);//ȡ���ӿ��ID���
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
						qTileUtils::GetBlockRect(BLOCK_ID_X(child_id), BLOCK_ID_Y(child_id), BLOCK_ID_LEVEL(child_id), child_bound);
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

					//��������
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
		//����ͷ�ļ���Ϣ
		mTileInfo.minlevel = 0;
		mTileStore->SaveSysTile(mTileInfo);

		mTileStore->close();
		SAFE_DELETE(mTileStore);

		emit  signalFinish();
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

		qTileDb mdb(m_SaveDBPath, TileInfo, true);
		if (!mdb.IsConnection())
		{
			emit signalMessage(QString::fromWCharArray(L"���������ļ�ʧ��"));//������Ϣ
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

		//����ͷ�ļ���Ϣ 
		mdb.DelSysTile();
		mdb.SaveSysTile(TileInfo);
		mdb.close();

		finish = clock();//���ʱ��
		long seconds = (finish - start) / CLOCKS_PER_SEC;//��
		QString strmsg;
		if (seconds <= 60) {
			strmsg = QString("%1").arg(seconds) + QString::fromWCharArray(L"��");
		}
		else {
			long d = 0;
			long h = 0;
			long m = seconds / 60;
			seconds = seconds % 60;//�õ���
			if (m > 60)
			{
				h = m / 60;
				m = m % 60;
			}
			if (h > 24)
			{//�������
				d = h / 24;
				h = h % 24;
			}

			wchar_t msg[256];
			if (d > 0)
				swprintf(msg, L"%d��%dʱ%d��%d��", d, h, m, seconds);
			else
				swprintf(msg, L"%dʱ%d��%d��", h, m, seconds);
			strmsg = QString::fromWCharArray(msg);
		}
		QString tr_timeDiff = QString(QString::fromWCharArray(L"���ݴ������!������ʱ�䣺") + "%1").arg(strmsg); //float->QString
		emit signalMessage(tr_timeDiff);
		emit signalFinish();
	}

	void CUnitTileThread::UnitData(qTileDb* SaveDB, EV_DB_Header* SavaHeader, QString DataFile)
	{
		qTileDbPrivateReader read;
		if (!read.open(DataFile))
			return;

		EV_DB_Header h;
		read.ReadHeader(h);
		SavaHeader->minlevel = __min(SavaHeader->minlevel, h.minlevel);
		SavaHeader->maxlevel = __max(SavaHeader->maxlevel, h.maxlevel);
		SavaHeader->minx = __min(SavaHeader->minx, h.minx);
		SavaHeader->maxx = __max(SavaHeader->maxx, h.maxx);
		SavaHeader->miny = __min(SavaHeader->miny, h.miny);
		SavaHeader->maxy = __max(SavaHeader->maxy, h.maxy);

		QString pathname = DataFile;
		QFileInfo fi = QFileInfo(pathname);
		QString file_name = fi.fileName();
		for (int level = h.minlevel; level <= h.maxlevel; level++)
		{
			emit signalMessage(QString::fromWCharArray(L"���ڴ����ļ���%1�� ����%2������Χ��(%3-%4) ").arg(file_name).arg(level).arg(h.minlevel).arg(h.maxlevel));//������Ϣ

			int minlx = qTileUtils::GetIndex(h.minx, level);
			int maxlx = qTileUtils::GetIndex(h.maxx, level);
			int minly = qTileUtils::GetIndex(h.miny, level);
			int maxly = qTileUtils::GetIndex(h.maxy, level);

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
					emit signalProcess((++percent * 100 / count));//���͵�ǰ����
				}
			}
		}
	}

	bool CUnitTileThread::MergeData(QByteArray* read_bytes, QByteArray* save_bytes, QByteArray* out_bytes)
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
		//��ͼƬת��Ϊ�ֽ�
		QBuffer buffer(out_bytes);
		buffer.open(QIODevice::WriteOnly);
		string saveformat = "JPG";
		//�������ж������Ƿ�û��͸���ģ������JPG�洢���������PNG
		for (int y = 0; y < 256; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				QColor clr = read.pixelColor(x, y);
				if (clr.alpha() == 0)//��ʾȫ͸��
				{
					saveformat = "PNG";
					y = 300;//����ѭ��
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

	bool CTransparentThread::FindFile(const QString& path)
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
				//���ļ����Խ��д���
			 //fileInfo.size(),fileInfo.fileName(),fileInfo.path();
				FindFile(fileInfo.filePath());
			}
			else
			{
				//Ϊ�ļ�
				QString exts = fileInfo.suffix();
				exts = exts.toLower();
				if (exts.compare("tif") == 0)
					TransparentImage(fileInfo.filePath());
			}
			i++;
		} while (i < list.size());

		return true;
	}

	void CTransparentThread::TransparentImage(QString imgPath)
	{
		//wchar_t wpath[256] = L"\0";
		//imgPath.toWCharArray(wpath);

		emit signalMessage(QString::fromWCharArray(L"���ڴ��ļ���%1").arg(imgPath));//������Ϣ
		QImage image;
		if (!image.load(imgPath))
			return;


		emit signalProcess(0);//���͵�ǰ����
		emit signalMessage(QString::fromWCharArray(L"�����ļ���%1").arg(imgPath));//������Ϣ
		bool bSave = false;
		//ĳЩ�������Ҫ��ȡ�ض����Σ�������Ҫ���鲨��˳��
		//����VC����ʾͼ��������Ҫ��buf����BGR���ݸ�BITMAP������ʾBITMAP��
		//��ʱֻ��Ҫ�޸ĵ�11�����������ˣ�
		int bufWidth = image.width();   //ͼ����
		int bufHeight = image.height();  //ͼ��߶�
		QImage newImage(bufWidth, bufHeight, QImage::Format_ARGB32);
		newImage.fill(Qt::transparent);
		int clr_count = m_colors.size();
		int c;
		//	QRgb rgba = QColor(0, 0, 0, 0).rgba();//�滻����ɫ������͸���ģ�����QColor(0,0,0��0)��
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
					newImage.setPixelColor(lx, ly, clr);//����Ϊ͸��
					bSave = true;
				}
			}
			emit signalProcess((ly + 1) * 100 / bufHeight);//���͵�ǰ����
		}

		if (bSave) {
			QString pngpath = imgPath.left(imgPath.length() - 3) + "png";
			emit signalMessage(QString::fromWCharArray(L"���ڱ����ļ���%1").arg(pngpath));//������Ϣ
			newImage.save(pngpath, "PNG");
			QString pgwpath = imgPath.left(imgPath.length() - 3) + "pgw";
			QString tfwpath = imgPath.left(imgPath.length() - 3) + "tfw";
			copyFileToPath(tfwpath, pgwpath, true);
		}
	}

	//�����ļ���
	bool CTransparentThread::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
	{
		toDir.replace("\\", "/");
		if (sourceDir == toDir) {
			return true;
		}
		if (!QFile::exists(sourceDir)) {
			return false;
		}
		QDir* createfile = new QDir;
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
	CDemNoiseThread::CDemNoiseThread(QObject* parent, const QStringList Paths, double MinValue, double MaxValue)
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
			emit signalMessage(QString::fromWCharArray(L"���ڶ�ȡ�����ļ���%1....").arg(filepath));
			QFile file(filepath);
			if (!file.open(QFile::ReadOnly | QFile::Text))
			{
				emit signalMessage(QString::fromWCharArray(L"���ڶ�ȡ�����ļ�ʧ�ܣ�%1....").arg(filepath));
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
				emit signalMessage(QString::fromWCharArray(L"���ڶ�ȡ���ݴ���%1....").arg(filepath));
				continue;
			}
			strLine = strLine.remove("ncols");
			strLine = strLine.remove(" ");
			ncols = strLine.toInt();

			strLine = textIn.readLine();//nrows
			if (strLine.lastIndexOf("nrows") < 0)
			{
				emit signalMessage(QString::fromWCharArray(L"���ڶ�ȡ���ݴ���%1....").arg(filepath));
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
			double** Values;//��ά����
			Values = new double* [nrows];
			int nx, ny;
			bool failed = false;
			for (ny = 0; ny < nrows; ny++)
			{
				Values[ny] = new double[ncols];
				memset(Values[ny], 0, sizeof(double) * ncols);
				//��ȡһ��
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
				emit signalMessage(QString::fromWCharArray(L"���ڶ�ȡ���ݴ���%1....").arg(filepath));
				for (ny = 0; ny < nrows; ny++)
				{
					SAFE_DELETE_ARRAY(Values[ny]);
				}
				SAFE_DELETE_ARRAY(Values);
				continue;
			}

			file.close();
			emit signalMessage(QString::fromWCharArray(L"���ڴ���ȥ������%1....").arg(filepath));

			bool isSave = false;
			for (ny = 0; ny < nrows; ny++)
			{
				for (nx = 0; nx < ncols; nx++)
				{
					if (int(Values[ny][nx]) == nodata_value)
						continue;//��Чֵ������

					if (int(Values[ny][nx]) < m_MaxAlt && int(Values[ny][nx]) > m_MinAlt)
						continue;//��ȷֵ

					isSave = true;//�������ݱ��޸�
					//�����ֵ������ȡ�ܱ�4��ֵ��ƽ��ֵ
					Values[ny][nx] = nodata_value;
					//int x, y;
					//x = nx - 1;
					//if (x > 0)//��
					//	Values[ny][nx] = Values[ny][x];

					//y = ny + 1;//��
					//if (y < ncols)
					//	Values[ny][nx] = (Values[ny][nx] + Values[y][nx]) / 2;

					//x = nx + 1;//��
					//if (x < nrows)
					//	Values[ny][nx] = (Values[ny][nx] + Values[ny][x]) / 2;

					//y = ny - 1;//��
					//if(y>0)
					//	Values[ny][nx] = (Values[ny][nx] + Values[y][nx]) / 2;
				}
				emit signalProcess((ny + 1) * 100 / nrows);
			}

			if (!isSave)
				continue;

			emit signalMessage(QString::fromWCharArray(L"����д���ļ���%1....").arg(filepath));
			QFile writefile(filepath);
			if (!writefile.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				emit signalMessage(QString::fromWCharArray(L"д�������ļ�ʧ�ܣ�%1....").arg(filepath));
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
			emit signalMessage(QString::fromWCharArray(L"д��ɹ���%1....").arg(filepath));
			//�ͷſռ�
			for (ny = 0; ny < nrows; ny++)
			{
				SAFE_DELETE_ARRAY(Values[ny]);
			}
			SAFE_DELETE_ARRAY(Values);
			////////////////////////////////////
		}
		emit signalFinish();
	}

}
