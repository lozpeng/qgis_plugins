#include "HttpFun.h"
#include <QTextCodec>
//#include "qgsnetworkaccessmanager.h"
//#include <QNetworkCacheMetaData>
#include <QSettings>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QTimer>
#include <QgsMessageLog.h>
#include <QBuffer>
#include "qgstilecache.h"
#include <qgslogger.h>
#include "qgsTiledCustomLayer.h"
#include "common.h"

const int nHTTP_TIME = 10000; //10秒

HttpFun::HttpFun(char type,LONGLONG id,QObject *parent) :
    QObject(parent)
{
	m_type = type;
	m_tileid = id;
    m_pNetworkManager = new QNetworkAccessManager(this);
    m_pNetworkReply = NULL;
    m_pTimer = new QTimer;
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(slot_requestTimeout()));//超时信号
}

//发起请求
void HttpFun::sendRequest(const QString &strUrl)
{
	m_strUrl = strUrl;

	QNetworkRequest netRequest;
	netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	netRequest.setUrl(QUrl(strUrl)); //地址信息
	if (strUrl.toLower().startsWith("https"))//https请求，需ssl支持(下载openssl拷贝libeay32.dll和ssleay32.dll文件至Qt bin目录或程序运行目录)
	{
		//QSslConfiguration sslConfig;
		//sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
		//sslConfig.setProtocol(QSsl::TlsV1_1);
		//netRequest.setSslConfiguration(sslConfig);
	}

	m_pNetworkReply = m_pNetworkManager->get(netRequest); //发起get请求

	connect(m_pNetworkReply,SIGNAL(finished()),this,SLOT(slot_requestFinished())); //请求完成信号
    m_pTimer->start(nHTTP_TIME);
}

void HttpFun::postRequest(const QString& strUrl, const QString& posUrl)
{
	m_strUrl = strUrl;

	QNetworkRequest netRequest;
	netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	netRequest.setUrl(QUrl(strUrl)); //地址信息
	if (strUrl.toLower().startsWith("https"))//https请求，需ssl支持(下载openssl拷贝libeay32.dll和ssleay32.dll文件至Qt bin目录或程序运行目录)
	{
		//QSslConfiguration sslConfig;
		//sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
		//sslConfig.setProtocol(QSsl::TlsV1_1);
		//netRequest.setSslConfiguration(sslConfig);
	}

	QString strBody(posUrl); //http body部分，可封装参数信息
	QByteArray contentByteArray = strBody.toLatin1();//转成二进制
	m_pNetworkReply = m_pNetworkManager->post(netRequest, contentByteArray);//发起post请求

	connect(m_pNetworkReply, SIGNAL(finished()), this, SLOT(slot_requestFinished())); //请求完成信号
	m_pTimer->start(nHTTP_TIME);

}

//请求结束
void HttpFun::slot_requestFinished()
{
    m_pTimer->stop();//关闭定时器
    QByteArray resultContent = m_pNetworkReply->readAll();
  //  QTextCodec* pCodec = QTextCodec::codecForName("UTF-8");
  //  QString strResult = pCodec->toUnicode(resultContent);
    int nHttpCode = m_pNetworkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();//http返回码
    if(nHttpCode == 200)//成功
    {
        emit signal_requestFinished(true,m_type,m_tileid,resultContent);//请求成功
    }
    else
    {
        emit signal_requestFinished(false,m_type,m_tileid,resultContent);//请求失败
    }
 	disconnect(m_pNetworkReply,0,this,0);
	m_pNetworkReply->deleteLater();
    this->deleteLater(); //释放内存
}

//请求超时
void HttpFun::slot_requestTimeout()
{
    emit signal_requestFinished(false,m_type,m_tileid,"timeout");//请求失败
	disconnect(m_pNetworkReply,0,this,0);
    m_pNetworkReply->deleteLater();
    this->deleteLater();//释放内存
}

enum QgsWmsTileAttribute
{
	TileReqNo = QNetworkRequest::User + 0,
	TileRect = QNetworkRequest::User + 1,
	TileRetry = QNetworkRequest::User + 2,
	TileType = QNetworkRequest::User + 3
};

void parseServiceException(QDomElement const & e, QString& errorTitle, QString& errorText)
{

	QString seCode = e.attribute("code");
	QString seText = e.text();

	errorTitle = ("Service Exception");

	// set up friendly descriptions for the service exception
	if (seCode == "InvalidFormat")
	{
		errorText = ("Request contains a format not offered by the server.");
	}
	else if (seCode == "InvalidCRS")
	{
		errorText = ("Request contains a CRS not offered by the server for one or more of the Layers in the request.");
	}
	else if (seCode == "InvalidSRS")  // legacy WMS < 1.3.0
	{
		errorText = ("Request contains a SRS not offered by the server for one or more of the Layers in the request.");
	}
	else if (seCode == "LayerNotDefined")
	{
		errorText = ("GetMap request is for a Layer not offered by the server, "
			"or GetFeatureInfo request is for a Layer not shown on the map.");
	}
	else if (seCode == "StyleNotDefined")
	{
		errorText = ("Request is for a Layer in a Style not offered by the server.");
	}
	else if (seCode == "LayerNotQueryable")
	{
		errorText = ("GetFeatureInfo request is applied to a Layer which is not declared queryable.");
	}
	else if (seCode == "InvalidPoint")
	{
		errorText = ("GetFeatureInfo request contains invalid X or Y value.");
	}
	else if (seCode == "CurrentUpdateSequence")
	{
		errorText = ("Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to "
			"current value of service metadata update sequence number.");
	}
	else if (seCode == "InvalidUpdateSequence")
	{
		errorText = ("Value of (optional) UpdateSequence parameter in GetCapabilities request is greater "
			"than current value of service metadata update sequence number.");
	}
	else if (seCode == "MissingDimensionValue")
	{
		errorText = ("Request does not include a sample dimension value, and the server did not declare a "
			"default value for that dimension.");
	}
	else if (seCode == "InvalidDimensionValue")
	{
		errorText = ("Request contains an invalid sample dimension value.");
	}
	else if (seCode == "OperationNotSupported")
	{
		errorText = ("Request is for an optional operation that is not supported by the server.");
	}
	else if (seCode.isEmpty())
	{
		errorText = ("(No error code was reported)");
	}
	else
	{
		errorText = seCode + ' ' + ("(Unknown error code)");
	}

	errorText += '\n' + ("The WMS vendor also reported: ");
	errorText += seText;

	// TODO = e.attribute("locator");

	QgsDebugMsg(QString("exiting with composed error message '%1'.").arg(errorText));
}

bool parseServiceExceptionReportDom(QByteArray const & xml, QString& errorTitle, QString& errorText)
{

#ifdef QGISDEBUG
	//test the content of the QByteArray
	QString responsestring(xml);
	QgsDebugMsg("received the following data: " + responsestring);
#endif

	// Convert completed document into a Dom
	QDomDocument doc;
	QString errorMsg;
	int errorLine;
	int errorColumn;
	bool contentSuccess = doc.setContent(xml, false, &errorMsg, &errorLine, &errorColumn);

	if (!contentSuccess)
	{
		errorTitle = ("Dom Exception");
		errorText = QString("Could not get WMS Service Exception: %1 at line %2 column %3\n\nResponse was:\n\n%4")
			.arg(errorMsg)
			.arg(errorLine)
			.arg(errorColumn)
			.arg(QString(xml));

		QgsLogger::debug("Dom Exception: " + errorText);

		return false;
	}

	QDomElement docElem = doc.documentElement();

	// TODO: Assert the docElem.tagName() is "ServiceExceptionReport"

	// serviceExceptionProperty.version = docElem.attribute("version");

	// Start walking through XML.
	QDomNode n = docElem.firstChild();

	while (!n.isNull())
	{
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if (!e.isNull())
		{
			QgsDebugMsg(e.tagName()); // the node really is an element.

			QString tagName = e.tagName();
			if (tagName.startsWith("wms:"))
				tagName = tagName.mid(4);

			if (tagName == "ServiceException")
			{
				QgsDebugMsg("  ServiceException.");
				parseServiceException(e, errorTitle, errorText);
			}

		}
		n = n.nextSibling();
	}

	QgsDebugMsg("exiting.");

	return true;
}

QMap<QString, QgsWmsStatistics::Stat> QgsWmsStatistics::sData;

/////////////////////////////////////////////////////////
// ----------数据下载模块
QgsWmsTiledImageDownloadHandler::QgsWmsTiledImageDownloadHandler(const TileRequests& requests, CTileLayerRender* pRender)
	: mRender(pRender)
	, mEventLoop(new QEventLoop)
{

	Q_FOREACH(const TileRequest& r, requests)
	{
		QNetworkRequest request(r.url);
		mAuth.setAuthorization(request);
		request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
		request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileReqNo), r.tile_id);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRect), r.rect);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileType), r.type);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRetry), 0);

//		QNetworkReply *reply = QgsNetworkAccessManager::instance()->get(request);
//		connect(reply, SIGNAL(finished()), this, SLOT(tileReplyFinished()));

//		mReplies << reply;
	}
}

QgsWmsTiledImageDownloadHandler::~QgsWmsTiledImageDownloadHandler()
{
	delete mEventLoop;
}

void QgsWmsTiledImageDownloadHandler::downloadBlocking()
{

	mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);

	Q_ASSERT(mReplies.isEmpty());
}


void QgsWmsTiledImageDownloadHandler::tileReplyFinished()
{
	QObject *obj = sender();
	if (!obj)
		return;

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(obj);

#if defined(QGISDEBUG)
	bool fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();
	QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri(mProviderUri);
	if (fromCache)
		stat.cacheHits++;
	else
		stat.cacheMisses++;
#endif
#if defined(QGISDEBUG)
	QgsDebugMsgLevel("raw headers:", 3);
	Q_FOREACH(const QNetworkReply::RawHeaderPair &pair, reply->rawHeaderPairs())
	{
		QgsDebugMsgLevel(QString(" %1:%2")
			.arg(QString::fromUtf8(pair.first),
				QString::fromUtf8(pair.second)), 3);
	}
#endif

	//if (QgsNetworkAccessManager::instance()->cache())
	//{
	//	QNetworkCacheMetaData cmd = QgsNetworkAccessManager::instance()->cache()->metaData(reply->request().url());

	//	QNetworkCacheMetaData::RawHeaderList hl;
	//	Q_FOREACH(const QNetworkCacheMetaData::RawHeader &h, cmd.rawHeaders())
	//	{
	//		if (h.first != "Cache-Control")
	//			hl.append(h);
	//	}
	//	cmd.setRawHeaders(hl);

	//	QgsDebugMsg(QString("expirationDate:%1").arg(cmd.expirationDate().toString()));
	//	if (cmd.expirationDate().isNull())
	//	{
	//		QSettings s;
	//		cmd.setExpirationDate(QDateTime::currentDateTime().addSecs(s.value("/qgis/defaultTileExpiry", "24").toInt() * 60 * 60));
	//	}

	//	QgsNetworkAccessManager::instance()->cache()->updateMetaData(cmd);
	//}

	if (reply->error() == QNetworkReply::NoError)
	{
		QNetworkRequest request = reply->request();
		LONGLONG tileReqNo = request.attribute(static_cast<QNetworkRequest::Attribute>(TileReqNo)).toULongLong();
		QRectF r = request.attribute(static_cast<QNetworkRequest::Attribute>(TileRect)).toRectF();
		int datatype = request.attribute(static_cast<QNetworkRequest::Attribute>(TileType)).toInt();
		QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
		if (!redirect.isNull())
		{
			QNetworkRequest request(redirect.toUrl());
			mAuth.setAuthorization(request);
			request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
			request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
			request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileReqNo), tileReqNo);
			request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRect), r);
			request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRetry), 0);

			mReplies.removeOne(reply);
			reply->deleteLater();

	//		QgsDebugMsg(QString("redirected gettile: %1").arg(redirect.toString()));
	//		reply = QgsNetworkAccessManager::instance()->get(request);
	//		mReplies << reply;

	//		connect(reply, SIGNAL(finished()), this, SLOT(tileReplyFinished()));

			return;
		}

		QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		if (!status.isNull() && status.toInt() >= 400)
		{
			QVariant phrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);

		//	QgsWmsProvider::showMessageBox(tr("Tile request error"), tr("Status: %1\nReason phrase: %2").arg(status.toInt()).arg(phrase.toString()));

			mReplies.removeOne(reply);
			reply->deleteLater();

			if (mReplies.isEmpty())
				finish();

			return;
		}

		QByteArray buf = reply->readAll();
		if (datatype == 2)
			buf.remove(0, 8);//去掉前面的8个字节

		RenderImageInfo renderData;
		renderData.image = new QImage();
		renderData.image->loadFromData(buf);
		if (!renderData.image->isNull()&& renderData.image->width() == 256 && renderData.image->height() == 256)
		{
			renderData.rect = r.toRect();
			mRender->m_RenderData.append(renderData);
			QgsTileCache::insertTile(reply->url(), *renderData.image);
		}
		else {
			SAFE_DELETE(renderData.image);
		}

		mReplies.removeOne(reply);
		reply->deleteLater();

		if (mReplies.isEmpty())
			finish();

	}
	else
	{
		mReplies.removeOne(reply);
		reply->deleteLater();

		if (mReplies.isEmpty())
			finish();
	}

#if 0
	const QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri(mProviderUri);
	emit statusChanged(tr("%n tile requests in background", "tile request count", mReplies.count())
		+ tr(", %n cache hits", "tile cache hits", stat.cacheHits)
		+ tr(", %n cache misses.", "tile cache missed", stat.cacheMisses)
		+ tr(", %n errors.", "errors", stat.errors)
	);
#endif
}

void QgsWmsTiledImageDownloadHandler::cancelled()
{
	QgsDebugMsg("Caught cancelled() signal");
	Q_FOREACH(QNetworkReply* reply, mReplies)
	{
		QgsDebugMsg("Aborting tiled network request");
		reply->abort();
	}
}


void QgsWmsTiledImageDownloadHandler::repeatTileRequest(QNetworkRequest const &oldRequest)
{
/*	QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri(mProviderUri);

	if (stat.errors == 100)
	{
		QgsMessageLog::logMessage(tr("Not logging more than 100 request errors."), tr("WMS"));
	}

	QNetworkRequest request(oldRequest);

	QString url = request.url().toString();
	int tileReqNo = request.attribute(static_cast<QNetworkRequest::Attribute>(TileReqNo)).toInt();
	int tileNo = request.attribute(static_cast<QNetworkRequest::Attribute>(TileIndex)).toInt();
	int retry = request.attribute(static_cast<QNetworkRequest::Attribute>(TileRetry)).toInt();
	retry++;

	QSettings s;
	int maxRetry = s.value("/qgis/defaultTileMaxRetry", "3").toInt();
	if (retry > maxRetry)
	{
		if (stat.errors < 100)
		{
			QgsMessageLog::logMessage(tr("Tile request max retry error. Failed %1 requests for tile %2 of tileRequest %3 (url: %4)")
				.arg(maxRetry).arg(tileNo).arg(tileReqNo).arg(url), tr("WMS"));
		}
		return;
	}

	mAuth.setAuthorization(request);
	if (stat.errors < 100)
	{
		QgsMessageLog::logMessage(tr("repeat tileRequest %1 tile %2(retry %3)")
			.arg(tileReqNo).arg(tileNo).arg(retry), tr("WMS"), QgsMessageLog::INFO);
	}
	QgsDebugMsg(QString("repeat tileRequest %1 %2(retry %3) for url: %4").arg(tileReqNo).arg(tileNo).arg(retry).arg(url));
	request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRetry), retry);

	QNetworkReply *reply = QgsNetworkAccessManager::instance()->get(request);
	mReplies << reply;
	connect(reply, SIGNAL(finished()), this, SLOT(tileReplyFinished()));
	*/
}


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// ----------数据下载模块
EdomDownloadHandler::EdomDownloadHandler(CGeoPolygon *geo, const TileRequests& requests, SilenTileDb* TileStore)
	: mEventLoop(new QEventLoop)
{
	mTileStore = TileStore;
	pDataBoundPolygon = geo;
	Q_FOREACH(const TileRequest& r, requests)
	{
		QNetworkRequest request(r.url);
		mAuth.setAuthorization(request);
		request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
		request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileReqNo), r.tile_id);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRect), r.rect);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileType), r.type);
		request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRetry), 0);

//		QNetworkReply *reply = QgsNetworkAccessManager::instance()->get(request);
//		connect(reply, SIGNAL(finished()), this, SLOT(tileReplyFinished()));

//		mReplies << reply;
	}
}

EdomDownloadHandler::~EdomDownloadHandler()
{
	delete mEventLoop;
}

void EdomDownloadHandler::downloadBlocking()
{

	mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);

	Q_ASSERT(mReplies.isEmpty());
}


void EdomDownloadHandler::tileReplyFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

#if defined(QGISDEBUG)
	bool fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();
	QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri(mProviderUri);
	if (fromCache)
		stat.cacheHits++;
	else
		stat.cacheMisses++;
#endif
#if defined(QGISDEBUG)
	QgsDebugMsgLevel("raw headers:", 3);
	Q_FOREACH(const QNetworkReply::RawHeaderPair &pair, reply->rawHeaderPairs())
	{
		QgsDebugMsgLevel(QString(" %1:%2")
			.arg(QString::fromUtf8(pair.first),
				QString::fromUtf8(pair.second)), 3);
	}
#endif

/*	if (QgsNetworkAccessManager::instance()->cache())
	{
		QNetworkCacheMetaData cmd = QgsNetworkAccessManager::instance()->cache()->metaData(reply->request().url());

		QNetworkCacheMetaData::RawHeaderList hl;
		Q_FOREACH(const QNetworkCacheMetaData::RawHeader &h, cmd.rawHeaders())
		{
			if (h.first != "Cache-Control")
				hl.append(h);
		}
		cmd.setRawHeaders(hl);

		QgsDebugMsg(QString("expirationDate:%1").arg(cmd.expirationDate().toString()));
		if (cmd.expirationDate().isNull())
		{
			QSettings s;
			cmd.setExpirationDate(QDateTime::currentDateTime().addSecs(s.value("/qgis/defaultTileExpiry", "24").toInt() * 60 * 60));
		}

		QgsNetworkAccessManager::instance()->cache()->updateMetaData(cmd);
	}

	LONGLONG tileReqNo = reply->request().attribute(static_cast<QNetworkRequest::Attribute>(TileReqNo)).toULongLong();
	QRectF r = reply->request().attribute(static_cast<QNetworkRequest::Attribute>(TileRect)).toRectF();
	int datatype = reply->request().attribute(static_cast<QNetworkRequest::Attribute>(TileType)).toInt();
#ifdef QGISDEBUG
	int retry = reply->request().attribute(static_cast<QNetworkRequest::Attribute>(TileRetry)).toInt();
#endif

	QgsDebugMsg(QString("tile reply %1 (%2) tile:%3(retry %4) rect:%5,%6 %7,%8) fromcache:%9 error:%10 url:%11")
		.arg(tileReqNo).arg(mTileReqNo).arg(tileNo).arg(retry)
		.arg(r.left(), 0, 'f').arg(r.bottom(), 0, 'f').arg(r.right(), 0, 'f').arg(r.top(), 0, 'f')
		.arg(fromCache)
		.arg(reply->errorString(),
			reply->url().toString())
	);

	if (reply->error() == QNetworkReply::NoError)
	{
		QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
		if (!redirect.isNull())
		{
			QNetworkRequest request(redirect.toUrl());
			mAuth.setAuthorization(request);
			request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
			request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
			request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileReqNo), tileReqNo);
			request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRect), r);
			request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRetry), 0);

			mReplies.removeOne(reply);
			reply->deleteLater();

			QgsDebugMsg(QString("redirected gettile: %1").arg(redirect.toString()));
			reply = QgsNetworkAccessManager::instance()->get(request);
			mReplies << reply;

			connect(reply, SIGNAL(finished()), this, SLOT(tileReplyFinished()));

			return;
		}

		QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		if (!status.isNull() && status.toInt() >= 400)
		{
			QVariant phrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);

			mReplies.removeOne(reply);
			reply->deleteLater();

			if (mReplies.isEmpty())
				finish();

			return;
		}

		QByteArray buf = reply->readAll();
		if (datatype == 2)
			buf.remove(0, 8);

		CutImage(tileReqNo, buf);//要做区域的裁剪
		int size = buf.size();
		char type[4] = { "png" };
		type[3] = '\0';
		buf.insert(0, (const char*)&size, sizeof(int));
		buf.insert(0, type, 4);
		mTileStore->save(tileReqNo, buf);//保存数据
		mReplies.removeOne(reply);
		reply->deleteLater();

		if (mReplies.isEmpty())
			finish();

	}
	else
	{
		mReplies.removeOne(reply);
		reply->deleteLater();

		if (mReplies.isEmpty())
			finish();
	}
	*/
}

void EdomDownloadHandler::CutImage(LONGLONG tileReqNo,QByteArray &buf)
{
	GEO_RECT tileRect = SilenTileUtils::GetGeoRectById(tileReqNo);
	CGeoPolygon img_polygon;
	CGeoPoint points;
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.miny));
	points.m_points.append(QgsPointXY(tileRect.minx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.maxy));
	points.m_points.append(QgsPointXY(tileRect.maxx, tileRect.miny));
	img_polygon.m_polyon.append(points);
	img_polygon.UpdateBound();

	//判断多边形是否相交
	if (pDataBoundPolygon->PolygonInPolygon(&img_polygon) == 1)
	{
		return;//区域块如果完全在多边形m_pDataBoundPolygon范围内，则不需要裁剪了
	}

	//要做区域的裁剪
	QImage image;
	if (!image.loadFromData(buf))
		return;

	double width = tileRect.maxx - tileRect.minx;
	double height = tileRect.maxy - tileRect.miny;
	QgsPointXY point;
	QImage bk_image(256,256, QImage::Format_ARGB32);//创建一个256*256的图片
	bk_image.fill(Qt::transparent);//这行代码可以使背景透明
	QPainter thePainter(&bk_image);
	thePainter.setRenderHint(QPainter::Antialiasing);
	thePainter.setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
	thePainter.setBrush(QBrush(QColor(255, 255, 255, 255), Qt::SolidPattern));

	int polycount = pDataBoundPolygon->m_polyon.size();
	for (int pc=0;pc<polycount;pc++)
	{
		CGeoPoint *pGeoPoints = &(pDataBoundPolygon->m_polyon[pc]);
		int lx, ly, point_count = pGeoPoints->m_points.size();
		QPolygon poly;
		for (int  n= 0; n < point_count; n++)
		{
			double dx = pGeoPoints->m_points[n].x();
			double dy = pGeoPoints->m_points[n].y();
			lx = (dx - tileRect.minx) * 256 / (tileRect.maxx - tileRect.minx);
			ly = (tileRect.maxy - dy) * 256 / (tileRect.maxy - tileRect.miny);
			poly.append(QPoint(lx, ly));
		}
		thePainter.drawPolygon(poly, Qt::WindingFill);
	}
	thePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	thePainter.drawImage(0, 0, image);

	//将图片转换为字节
	buf.clear();
	QBuffer buffer(&buf);
	buffer.open(QIODevice::WriteOnly);
	bk_image.save(&buffer, "PNG");

}

void EdomDownloadHandler::cancelled()
{
	QgsDebugMsg("Caught cancelled() signal");
	Q_FOREACH(QNetworkReply* reply, mReplies)
	{
		QgsDebugMsg("Aborting tiled network request");
		reply->abort();
	}
}


void EdomDownloadHandler::repeatTileRequest(QNetworkRequest const &oldRequest)
{
	/*	QgsWmsStatistics::Stat& stat = QgsWmsStatistics::statForUri(mProviderUri);

	if (stat.errors == 100)
	{
	QgsMessageLog::logMessage(tr("Not logging more than 100 request errors."), tr("WMS"));
	}

	QNetworkRequest request(oldRequest);

	QString url = request.url().toString();
	int tileReqNo = request.attribute(static_cast<QNetworkRequest::Attribute>(TileReqNo)).toInt();
	int tileNo = request.attribute(static_cast<QNetworkRequest::Attribute>(TileIndex)).toInt();
	int retry = request.attribute(static_cast<QNetworkRequest::Attribute>(TileRetry)).toInt();
	retry++;

	QSettings s;
	int maxRetry = s.value("/qgis/defaultTileMaxRetry", "3").toInt();
	if (retry > maxRetry)
	{
	if (stat.errors < 100)
	{
	QgsMessageLog::logMessage(tr("Tile request max retry error. Failed %1 requests for tile %2 of tileRequest %3 (url: %4)")
	.arg(maxRetry).arg(tileNo).arg(tileReqNo).arg(url), tr("WMS"));
	}
	return;
	}

	mAuth.setAuthorization(request);
	if (stat.errors < 100)
	{
	QgsMessageLog::logMessage(tr("repeat tileRequest %1 tile %2(retry %3)")
	.arg(tileReqNo).arg(tileNo).arg(retry), tr("WMS"), QgsMessageLog::INFO);
	}
	QgsDebugMsg(QString("repeat tileRequest %1 %2(retry %3) for url: %4").arg(tileReqNo).arg(tileNo).arg(retry).arg(url));
	request.setAttribute(static_cast<QNetworkRequest::Attribute>(TileRetry), retry);

	QNetworkReply *reply = QgsNetworkAccessManager::instance()->get(request);
	mReplies << reply;
	connect(reply, SIGNAL(finished()), this, SLOT(tileReplyFinished()));
	*/
}







