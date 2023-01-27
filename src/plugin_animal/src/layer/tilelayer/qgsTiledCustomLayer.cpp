#include "qgsTiledCustomLayer.h"

#include <qgsrectangle.h>
#include <qgsmapcanvas.h>
#include <qgsmapsettings.h>
#include <qpaintdevice.h>
#include <QVariant>
#include "xfileio.h"
#include "MultiTileThread.h"
#include "common.h"
#include <QgsMessageLog.h>

qgsTiledCustomLayer::qgsTiledCustomLayer(QgisInterface* qgsInterface, qgsTileLayerDef lyrDef)
	: QgsPluginLayer("qgsCustomTilelayer", lyrDef.title),
	mTileLyrDef(lyrDef)
	, mQgsInterface(qgsInterface)
{
	mpTileUtils = NULL;
	//项目的坐标
	QgsMapCanvas* pCanvas = mQgsInterface->mapCanvas();
	const QgsCoordinateReferenceSystem& ldPrj = pCanvas->mapSettings().destinationCrs();
	//ldPrj.createFromProj4("+proj=longlat +a=6378140 +b=6356755.288157528 +no_defs"); //http://spatialreference.org/ref/epsg/4610/proj4/
	this->setCrs(ldPrj);

	//打开数据
	switch (lyrDef.type)
	{
	case 1://edom
		m_db = new CEDOMTileDB(this);
		//本地文件
		if (!m_db->InitConnection(lyrDef.url))
			return;

		LoadHeader(m_db->ReadHead());
		this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
		break;
	case 2://evia server
	{
		m_db = new CEviaServerTileDB(this);
		m_db->InitConnection(lyrDef.url);
		//绑定事件
		connect(m_db, SIGNAL(signal_requestHeaderFinished(bool, const QByteArray&)), //http请求结束信号
			this, SLOT(slot_requestHeaderFinished(bool, const QByteArray&)));
		connect(m_db, SIGNAL(signal_requestBlockFinished(bool, LONGLONG, const QByteArray&)), //http请求结束信号
			this, SLOT(slot_requestBlockFinished(bool, LONGLONG, const QByteArray&)));

		connect(m_db, SIGNAL(signal_requestBlock(LONGLONG)), //http请求
			this, SLOT(slot_requestBlock(LONGLONG)));

		m_db->RequestHead();
	}
	break;
	case 3:
		mpTileUtils = new CEDomUtils();//经纬度
		m_db = new CTDServerTileDB(this);//天地图的服务
		m_db->InitConnection(lyrDef.url);
		m_header.minlevel = 0;
		m_header.maxlevel = 18;
		m_header.minx = -180;
		m_header.maxx = 180;
		m_header.miny = -90;
		m_header.maxy = 90;
		this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
		break;
	case 4:
		mpTileUtils = new CEDomUtils();//经纬度
		m_db = new CCHServerTileDB(this);//天地图的服务
		m_db->InitConnection(lyrDef.url);
		((CCHServerTileDB*)m_db)->SetUSerID(lyrDef.userid, lyrDef.pwd);
		//只列中国的范围
		m_header.minlevel = 0;
		m_header.maxlevel = 18;
		m_header.minx = 73.66;
		m_header.maxx = 135.05;
		m_header.miny = 3.86;
		m_header.maxy = 53.55;
		this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
		break;
	default:
		return;
	}

	connect(this, SIGNAL(signal_requestDraw(QgsRenderContext&, CTileLayerRender*)), this, SLOT(slot_requestDraw(QgsRenderContext&, CTileLayerRender*)));//请求绘制的消息
}

qgsTiledCustomLayer::~qgsTiledCustomLayer()
{
	if (m_db)
	{
		m_db->close();
		delete m_db;
	}

	SAFE_DELETE(mpTileUtils);
}

QgsRectangle qgsTiledCustomLayer::extent() const
{
	QgsRectangle rect;
	rect.set(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy);
	return rect;
}

bool qgsTiledCustomLayer::LoadHeader(QByteArray header_data)
{
	memset((void*)&m_header, 0, sizeof(EV_DB_Header));
	//读取头信息
	if (header_data.size() < 1)
		return false;

	CXFileIOMem buffer;
	buffer.putData((const void*)header_data.data(), header_data.size());
	buffer.read((char*)&m_header.size, sizeof(m_header.size));
	buffer.read((char*)&m_header.version, sizeof(m_header.version));
	buffer.read((char*)&m_header.dbtype, sizeof(m_header.dbtype));
	buffer.read((char*)&m_header.tileGeoSize, sizeof(m_header.tileGeoSize));
	buffer.read((char*)&m_header.tileGridSize, sizeof(m_header.tileGridSize));
	buffer.read((char*)&m_header.jpgQuality, sizeof(m_header.jpgQuality));
	buffer.read((char*)&m_header.minlevel, sizeof(m_header.minlevel));
	buffer.read((char*)&m_header.maxlevel, sizeof(m_header.maxlevel));
	buffer.seek(sizeof(char) * 2, CXFileIO::XFILEIO_SEEK_CUR);
	//buffer.read((char*)&m_header.reserved,		sizeof(char)*2);
	buffer.read((char*)&m_header.minx, sizeof(double));
	buffer.read((char*)&m_header.miny, sizeof(double));
	buffer.read((char*)&m_header.maxx, sizeof(double));
	buffer.read((char*)&m_header.maxy, sizeof(double));
	buffer.read((char*)&m_header.factor, sizeof(m_header.factor));
	buffer.read((char*)&m_header.lowvalue, sizeof(float));
	buffer.read((char*)&m_header.highvalue, sizeof(float));
	buffer.read((char*)&m_header.dfactor, sizeof(double));

	if (m_header.dbtype == 3)
		mpTileUtils = new CMercatorUtils();//墨卡托的参数
	else
		mpTileUtils = new CEDomUtils();//经纬度

	this->setValid(true);
	return true;
}

/**
绘制瓦片地图
**/
bool qgsTiledCustomLayer::render(QgsRenderContext& rendererContext, CTileLayerRender* pRender)
{
	if (!m_db || !m_db->IsConnection() || !mpTileUtils)
		return true;

	QgsRectangle map_rect = rendererContext.extent();
	if (map_rect.isNull() || map_rect.isEmpty() ||
		m_header.maxx<map_rect.xMinimum() || m_header.minx>map_rect.xMaximum() ||
		m_header.maxy < map_rect.yMinimum() || m_header.miny>map_rect.yMaximum())
		return true;

	//计算范围
	double geo_width = map_rect.width();
	double geo_height = map_rect.height();
	//计算屏幕坐标与地理坐标的关系
	int level = 0;// m_header.minlevel;
	QgsMapToPixel mp = rendererContext.mapToPixel();
	int view_width = rendererContext.mapToPixel().mapWidth();//view_rect.width();
	int view_height = rendererContext.mapToPixel().mapHeight();//view_rect.height();
	long minIndX, minIndY, maxIndX, maxIndY;
	double block_width, mapUnitsPerPixel = mp.mapUnitsPerPixel();
	while (level < m_header.maxlevel)
	{
		block_width = (double)(mpTileUtils->GetRefValue() / (1 << (level + 1)));
		if (block_width <= mapUnitsPerPixel * 256)
			break;

		level++;
	}
	if (level < m_header.minlevel)
		return true;//级别不在范围内
	minIndX = mpTileUtils->GetIndex_X(map_rect.xMinimum(), level);
	minIndY = mpTileUtils->GetIndex_Y(map_rect.yMinimum(), level);
	maxIndX = mpTileUtils->GetIndex_X(map_rect.xMaximum(), level);
	maxIndY = mpTileUtils->GetIndex_Y(map_rect.yMaximum(), level);
	if (minIndY > maxIndY)
		swap(maxIndY, minIndY);

	TileRequests requestsFinal;//网络情况下加载
	//读取数据
	QgsPointXY pt1, pt2;
	for (int ly = minIndY; ly <= maxIndY; ly++)
	{
		for (int lx = minIndX; lx <= maxIndX; lx++)
		{
			LONGLONG id = BLOCK_ID(lx, ly, level);
			QgsRectangle qrect;
			mpTileUtils->GetBlockRect(lx, ly, level, qrect);
			//计算出本块对应的屏幕坐标
			pt1 = mp.transform(qrect.xMinimum(), qrect.yMaximum());
			pt2 = mp.transform(qrect.xMaximum(), qrect.yMinimum());
			QRect rect(int(pt1.x() - 0.5), int(pt1.y() - 0.5), int(pt2.x() - pt1.x() + 1.5), int(pt2.y() - pt1.y()) + 1.5);

			QByteArray data = m_db->ReadBlock(id);
			if (data.isNull() || data.length() < 2)
			{
				if (m_db->GetType() > 1)
				{
					//标明是需要网络下载
					QString url = m_db->GetURL(id);
					TileRequest rqst(QUrl(url), m_db->GetType(), rect, id);
					requestsFinal.append(rqst);//加载到需要下载的队列里
				}
				continue;
			}
			data.remove(0, 8);//去掉前面的8个字节
			//画图
			RenderImageInfo renderData;
			renderData.image = new QImage;
			renderData.image->loadFromData((uchar*)data.data(), data.size());
			renderData.rect = rect;
			pRender->m_RenderData.append(renderData);
		}
	}

	if (requestsFinal.size() > 0)
	{
		QgsWmsTiledImageDownloadHandler handler(requestsFinal, pRender);
		handler.downloadBlocking();
	}

	return true;
}
QgsMapLayerRenderer* qgsTiledCustomLayer::createMapRenderer(QgsRenderContext& rendererContext)
{
	CTileLayerRender* pRender = new CTileLayerRender(this, rendererContext);
	emit signal_requestDraw(rendererContext, pRender);
	return  pRender;
};

bool qgsTiledCustomLayer::readSymbology(const QDomNode& node, QString& errorMessage,
	QgsReadWriteContext& context, StyleCategories categories) {
	//读取
	//if (!domnode.hasChildNodes())
	//	return false;

	//QDomNode typeNode, connNode, nameNode;
	//QDomNodeList NodeList = domnode.childNodes();
	//int count = NodeList.count();
	//for (int i = 0; i < count; i++)
	//{
	//	QDomNode node = NodeList.item(i);
	//	QString name = node.nodeName();
	//	if (name == "type")
	//	{
	//		continue;
	//	}
	//	if (name == "name")
	//	{
	//		continue;
	//	}
	//	if (name == "connection")
	//	{
	//		continue;
	//	}
	//}
	return true;
}

void qgsTiledCustomLayer::slot_requestHeaderFinished(bool bSuccess, const QByteArray& data)
{
	if (!bSuccess)
	{
		if (m_db)
			m_db->RequestHead();
		return;
	}
	if (this->LoadHeader(data))
	{
		//头信息
		this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
	}
}

void qgsTiledCustomLayer::slot_requestBlockFinished(bool bSuccess, LONGLONG tid, const QByteArray& data)
{
	if (!bSuccess)
		return;

	//数据块
	mQgsInterface->mapCanvas()->invalidateScene();//局部更新
}

void qgsTiledCustomLayer::slot_requestBlock(LONGLONG tid)
{
	if (m_db && m_db->IsConnection())
		m_db->ReadBlock(tid);
}

void qgsTiledCustomLayer::slot_requestDraw(QgsRenderContext& renderContext, CTileLayerRender* pRender)
{
	this->render(renderContext, pRender);
}

//=========================================================================
PluginTileLayerType::PluginTileLayerType(QgisInterface* qgsInterface, qgsTileLayerDef lyrDef)
	:QgsPluginLayerType("qgsCustomTilelayer")
	, mQgsInterface(qgsInterface)
	, m_lyrDef(lyrDef)
{
}

QString PluginTileLayerType::name() { return "qgsCustomTilelayer"; }

/** Return new layer of this type. Return NULL on error */
QgsPluginLayer* PluginTileLayerType::createLayer()
{
	return new qgsTiledCustomLayer(mQgsInterface, m_lyrDef);
}

/** Return new layer of this type, using layer URI (specific to this plugin layer type). Return NULL on error.
* @note added in 2.10
*/
QgsPluginLayer* PluginTileLayerType::createLayer(const QString& uri) {
	qgsTileLayerDef def;
	def.title = "qgsCustomTilelayer";
	return new qgsTiledCustomLayer(mQgsInterface, def);
}

/** Show plugin layer properties dialog. Return false if the dialog cannot be shown. */
bool PluginTileLayerType::showLayerProperties(QgsPluginLayer* layer) {
	return true;
}

///////////////////////////////////////////

///@cond PRIVATE
CTileLayerRendererFeedback::CTileLayerRendererFeedback(CTileLayerRender* r)
	: mR(r)
	, mMinimalPreviewInterval(250)
{
	setRenderPartialOutput(r->mContext.testFlag(Qgis::RenderContextFlag::RenderPartialOutput));
}

void CTileLayerRendererFeedback::onNewData()
{
	if (!renderPartialOutput())
		return;  // we were not asked for partial renders and we may not have a temporary image for overwriting...

	// update only once upon a time
	// (preview itself takes some time)
	if (mLastPreview.isValid() && mLastPreview.msecsTo(QTime::currentTime()) < mMinimalPreviewInterval)
		return;

	// TODO: update only the area that got new data

	QgsDebugMsg(QStringLiteral("new raster preview! %1").arg(mLastPreview.msecsTo(QTime::currentTime())));
	QTime t;
	t.start();
	QgsRasterBlockFeedback feedback;
	feedback.setPreviewOnly(true);
	feedback.setRenderPartialOutput(true);
	//QgsRasterIterator iterator(mR->mPipe->last());
	//QgsRasterDrawer drawer(&iterator);
	//drawer.draw(mR->mPainter, mR->mRasterViewPort, mR->mMapToPixel, &feedback);
	QgsDebugMsg(QStringLiteral("total raster preview time: %1 ms").arg(t.elapsed()));
	mLastPreview = QTime::currentTime();
}

///@endcond
///
CTileLayerRender::CTileLayerRender(qgsTiledCustomLayer* layer, QgsRenderContext& rendererContext)
	: QgsMapLayerRenderer(layer->id())
	, mContext(rendererContext)
	, mFeedback(new CTileLayerRendererFeedback(this))
{
	const QgsMapToPixel& qgsMapToPixel = rendererContext.mapToPixel();
	mMapToPixel = &qgsMapToPixel;

	QgsMapToPixel mapToPixel = qgsMapToPixel;
	if (mapToPixel.mapRotation())
	{
		// unset rotation for the sake of local computations.
		// Rotation will be handled by QPainter later
		// TODO: provide a method of QgsMapToPixel to fetch map center
		//       in geographical units
		QgsPointXY center = mapToPixel.toMapCoordinates(
			static_cast<int>(mapToPixel.mapWidth() / 2.0),
			static_cast<int>(mapToPixel.mapHeight() / 2.0)
		);
		mapToPixel.setMapRotation(0, center.x(), center.y());
	}
}

CTileLayerRender::~CTileLayerRender()
{
	int count = m_RenderData.size();
	for (int i = 0; i < count; i++)
	{
		SAFE_DELETE(m_RenderData[i].image);
	}
	m_RenderData.clear();
	delete mFeedback;
}

bool CTileLayerRender::render()
{
	if (!mFeedback)
		return true; // outside of layer extent - nothing to do

	int count = m_RenderData.size();
	QPainter* painter = mContext.painter();
	for (int i = 0; i < count; i++)
	{
		if (m_RenderData[i].image->width() > 0 && m_RenderData[i].image->height() > 0)
		{
			painter->drawImage(m_RenderData[i].rect, *m_RenderData[i].image);
		}
	}

	return true;
}

QgsFeedback* CTileLayerRender::feedback() const
{
	return mFeedback;
}

