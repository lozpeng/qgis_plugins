#include "qgsCustomizedTileLayer.h"

#include <qgsrectangle.h>
#include <qgsmapcanvas.h>
#include <qgsmapsettings.h>
#include <qpaintdevice.h>
#include <QVariant>
#include "xfileio.h"
#include "thread/qtilemultithread.h"
#include "common.h"
#include <QgsMessageLog.h>

#include "tileprovider/qTileProvider.h"
#include "tileprovider/qtileproviderutils.h"
#include "web/qhttprequest.h"
namespace geotile {
	//=========================================================================
	QString Q_PLUGIN_LAYER_NAME = "qgscustomizedtilelayer";
	qPluginTileLayerType::qPluginTileLayerType(QgisInterface* qgsInterface, qTileLayerDef lyrDef)
		:QgsPluginLayerType(Q_PLUGIN_LAYER_NAME)
		, mQgsInterface(qgsInterface)
		, m_lyrDef(lyrDef)		
	{
	}
	qPluginTileLayerType::~qPluginTileLayerType() {}

	QString qPluginTileLayerType::name() { return Q_PLUGIN_LAYER_NAME; }

	/** Return new layer of this type. Return NULL on error */
	QgsPluginLayer* qPluginTileLayerType::createLayer()
	{
		return new qgsCustomizedTileLayer(mQgsInterface, m_lyrDef);
	}

	/** Return new layer of this type, using layer URI (specific to this plugin layer type). Return NULL on error.
	* @note added in 2.10
	*/
	QgsPluginLayer* qPluginTileLayerType::createLayer(const QString& uri) {
		qTileLayerDef def;
		def.title = Q_PLUGIN_LAYER_NAME;
		return new qgsCustomizedTileLayer(mQgsInterface, def);
	}

	/** Show plugin layer properties dialog. Return false if the dialog cannot be shown. */
	bool qPluginTileLayerType::showLayerProperties(QgsPluginLayer* layer) {
		return true;
	}

	qgsCustomizedTileLayer::qgsCustomizedTileLayer(QgisInterface* qgsInterface, qTileLayerDef lyrDef)
		: QgsPluginLayer(Q_PLUGIN_LAYER_NAME, lyrDef.title)
			, mTileLyrDef(lyrDef)
			, mQgsInterface(qgsInterface)
			, mTileProvider(nullptr)
	{
		mpTileUtils = NULL;
		//项目的坐标
		QgsMapCanvas* pCanvas = mQgsInterface->mapCanvas();
		const QgsCoordinateReferenceSystem& ldPrj = pCanvas->mapSettings().destinationCrs();
		//ldPrj.createFromProj4("+proj=longlat +a=6378140 +b=6356755.288157528 +no_defs"); 
		//http://spatialreference.org/ref/epsg/4610/proj4/
		if (!mTileLyrDef.prjTxt.isEmpty() && mTileLyrDef.prjTxt.length() >= 5)
		{
			QgsCoordinateReferenceSystem crs;
			crs.fromProj4(mTileLyrDef.prjTxt); //
			this->setCrs(crs);
		}
		else 
			this->setCrs(ldPrj);		
		//打开数据
		switch (lyrDef.type)
		{
			case qTileSourceType::tEdomDb://edom
				mTileProvider = new qEdomTileProvider(this);
				//本地文件
				if (!mTileProvider->InitConnection(lyrDef.url))
					return;

				LoadHeader(mTileProvider->ReadHead());
				this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
				this->setValid(true);
				break;
			case qTileSourceType::tEdomServer://evia server
			{
				mTileProvider = new qEviaServerTileProvider(this);
				mTileProvider->InitConnection(lyrDef.url);
				//绑定事件
				connect(mTileProvider, SIGNAL(signal_requestHeaderFinished(bool, const QByteArray&)), //http请求结束信号
					this, SLOT(slot_requestHeaderFinished(bool, const QByteArray&)));
				connect(mTileProvider, SIGNAL(signal_requestBlockFinished(bool, LONGLONG, const QByteArray&)), //http请求结束信号
					this, SLOT(slot_requestBlockFinished(bool, LONGLONG, const QByteArray&)));

				connect(mTileProvider, SIGNAL(signal_requestBlock(LONGLONG)), //http请求
										this, SLOT(slot_requestBlock(LONGLONG)));

				mTileProvider->RequestHead();
				this->setValid(true);
			}
			break;
			case qTileSourceType::tTdtMapServer:
				mpTileUtils = new qEdomTileProviderUtils();//经纬度
				mTileProvider = new qTdtServerTileProvider(this);//天地图的服务
				mTileProvider->InitConnection(lyrDef.url);
				m_header.minlevel = 0;
				m_header.maxlevel = 18;
				m_header.minx = -180;
				m_header.maxx = 180;
				m_header.miny = -90;
				m_header.maxy = 90;
				this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
				this->setValid(true);
				break;
			case qTileSourceType::tCHJMapServer:
				mpTileUtils = new qEdomTileProviderUtils();//经纬度
				mTileProvider = new qCeHuiServerTileProvider(this);//天地图的服务
				mTileProvider->InitConnection(lyrDef.url);
				((qCeHuiServerTileProvider*)mTileProvider)->SetUSerID(lyrDef.userid, lyrDef.pwd);
				//只列中国的范围
				m_header.minlevel = 0;
				m_header.maxlevel = 18;
				m_header.minx = 73.66;
				m_header.maxx = 135.05;
				m_header.miny = 3.86;
				m_header.maxy = 53.55;
				this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
				this->setValid(true);
				break;
			default:
				return;
		}

		bool result = connect(this, SIGNAL(tileDraw(qTileLayerRender * )),
								this, SLOT(on_TileDraw(qTileLayerRender * )));
	}

	qgsCustomizedTileLayer::~qgsCustomizedTileLayer()
	{
		if (mTileProvider)
		{
			mTileProvider->close();
			delete mTileProvider;
		}
		SAFE_DELETE(mpTileUtils);
	}

	QgsRectangle qgsCustomizedTileLayer::extent() const
	{
		QgsRectangle rect;
		rect.set(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy);
		return rect;
	}
	void qgsCustomizedTileLayer::setTransformContext(const QgsCoordinateTransformContext& coordTransContext)
	{

	}
	bool qgsCustomizedTileLayer::LoadHeader(QByteArray header_data)
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
			mpTileUtils = new qWebMecatorProviderUtils();//墨卡托的参数
		else
			mpTileUtils = new qEdomTileProviderUtils();//经纬度

		this->setValid(true);
		return true;
	}

	/**
	绘制瓦片地图
	**/
	bool qgsCustomizedTileLayer::render(QgsRenderContext& rendererContext, qTileLayerRender* pRender)
	{
		if (!mTileProvider || !mTileProvider->IsConnection() || !mpTileUtils)
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

		qTileRequests requestsFinal;//网络情况下加载
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
				QRect rect(int(pt1.x() - 0.5),
							int(pt1.y() - 0.5), 
							int(pt2.x() - pt1.x() + 1.5), 
							int(pt2.y() - pt1.y()) + 1.5);

				QByteArray data = mTileProvider->ReadBlock(id);
				if (data.isNull() || data.length() < 2)
				{
					//定义tWebMecatorDb 为最后一种本地数据类型，因此大于该类型的都需要从网络上获取数据
					if (mTileProvider->GetType() > qTileSourceType::tWebMecatorDb)
					{
						//表明是需要网络下载
						QString url = mTileProvider->GetURL(id);
						qTileRequest rqst(QUrl(url), mTileProvider->GetType(), rect, id);
						requestsFinal.append(rqst);//加载到需要下载的队列里
					}
					continue;
				}
				data.remove(0, 8);//去掉前面的8个字节
				//画图
				qRenderImageInfo renderData;
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
	/*

	*/
	QgsMapLayerRenderer* qgsCustomizedTileLayer::createMapRenderer(QgsRenderContext& rendererContext)
	{
		qTileLayerRender* pRender = new qTileLayerRender(this, rendererContext);
		Q_EMIT this->tileDraw(pRender);
		this->render(rendererContext, pRender);
		return  pRender;
	}

	void  qgsCustomizedTileLayer::on_TileDraw(qTileLayerRender* pRender)
	{
		this->render(pRender->RenderContext(), pRender);
	}
	bool qgsCustomizedTileLayer::readSymbology(const QDomNode& node, QString& errorMessage,
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

	void qgsCustomizedTileLayer::slot_requestHeaderFinished(bool bSuccess, const QByteArray& data)
	{
		if (!bSuccess)
		{
			if (mTileProvider)
				mTileProvider->RequestHead();
			return;
		}
		if (this->LoadHeader(data))
		{
			//头信息
			this->setExtent(QgsRectangle(m_header.minx, m_header.miny, m_header.maxx, m_header.maxy));//设置范围
		}
	}

	void qgsCustomizedTileLayer::slot_requestBlockFinished(bool bSuccess, LONGLONG tid, const QByteArray& data)
	{
		if (!bSuccess)
			return;
		//数据块
		mQgsInterface->mapCanvas()->invalidateScene();//局部更新
	}

	void qgsCustomizedTileLayer::slot_requestBlock(LONGLONG tid)
	{
		if (mTileProvider && mTileProvider->IsConnection())
			mTileProvider->ReadBlock(tid);
	}
	///////////////////////////////////////////
	///@cond PRIVATE
	qTileLayerRenderFeedBack::qTileLayerRenderFeedBack(qTileLayerRender* r)
		: mR(r)
		, mMinimalPreviewInterval(250)
	{
		setRenderPartialOutput(r->mContext.testFlag(Qgis::RenderContextFlag::RenderPartialOutput));
	}

	void qTileLayerRenderFeedBack::onNewData()
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
	qTileLayerRender::qTileLayerRender(qgsCustomizedTileLayer* layer, QgsRenderContext& rendererContext)
		: QgsMapLayerRenderer(layer->id())
		, mContext(rendererContext)
		, mFeedback(new qTileLayerRenderFeedBack(this))
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

	qTileLayerRender::~qTileLayerRender()
	{
		int count = m_RenderData.size();
		for (int i = 0; i < count; i++)
		{
			SAFE_DELETE(m_RenderData[i].image);
		}
		m_RenderData.clear();
		mFeedback = nullptr;
	}

	bool qTileLayerRender::render()
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

	QgsFeedback* qTileLayerRender::feedback() const
	{
		return mFeedback;
	}
}