#pragma once

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QTimer>
#include <QSslConfiguration>
#include <QString>
#include <QByteArray>
#include <QEventLoop>
#define QT_NO_OPENSSL

#include "../qtileinfo.h"
#include "../qgsCustomizedTileLayer.h"
#include "../tileprovider/qtileproviderutils.h"
#include "../qtileutils.h"
#include "../qtileinfo.h"
#include "../tiledb/qtiledb.h"

namespace geotile{
	/// <summary>
	/// http请求，访问一个瓦片块
	/// </summary>
	class qHttpRequest : public QObject
	{
		Q_OBJECT

	public:
		explicit qHttpRequest(char type, LONGLONG id, QObject* parent = 0);
		void sendRequest(const QString& strUrl);//根据url发起http请求
		void postRequest(const QString& strUrl, const QString& posUrl);//根据url发起post请求
	signals:
		void signal_requestFinished(bool bSuccess, char type, LONGLONG id, const QByteArray& strResult); //http请求结束
	public slots:
		void slot_requestFinished(); //http请求结束
		void slot_requestTimeout();  //请求超时

	private:
		QNetworkAccessManager* m_pNetworkManager;//网络管理类
		QNetworkReply* m_pNetworkReply; //封装请求返回信息
		QString m_strUrl; //记录当前请求的url
		QTimer* m_pTimer;
		char m_type;//=1为头，=2为数据库，=0为错误
		LONGLONG m_tileid;
	};

	/// <summary>
	/// WMS服务授权信息
	/// </summary>
	struct qWmsAuthorization
	{
		qWmsAuthorization(const QString& userName = QString(), const QString& password = QString(), const QString& referer = QString(), const QString& authcfg = QString())
			: mUserName(userName)
			, mPassword(password)
			, mReferer(referer)
			, mAuthCfg(authcfg)
		{}

		bool setAuthorization(QNetworkRequest& request) const
		{
			if (!mAuthCfg.isEmpty())
			{
				//QgsAuthManager m;
				//return m.updateNetworkRequest(request, mAuthCfg);
			}
			else if (!mUserName.isEmpty() || !mPassword.isEmpty())
			{
				request.setRawHeader("Authorization", "Basic " + QString("%1:%2").arg(mUserName, mPassword).toLatin1().toBase64());
			}

			if (!mReferer.isEmpty())
			{
				request.setRawHeader("Referer", QString("%1").arg(mReferer).toLatin1());
			}
			return true;
		}
		//! set authorization reply
		bool setAuthorizationReply(QNetworkReply* reply) const
		{
			if (!mAuthCfg.isEmpty())
			{
				//		return QgsAuthManager::instance()->updateNetworkReply(reply, mAuthCfg);
			}
			return true;
		}

		//! Username for basic http authentication
		QString mUserName;

		//! Password for basic http authentication
		QString mPassword;

		//! Referer for http requests
		QString mReferer;

		//! Authentication configuration ID
		QString mAuthCfg;
	};

	//! Helper struct for tile requests
	typedef struct qTileRequest
	{
		qTileRequest(const QUrl& u, int t, const QRectF& r, LONGLONG id)
			: url(u)
			, type(t)
			, rect(r)
			, tile_id(id)
		{}
		int type;
		QUrl url;
		QRectF rect;
		LONGLONG	tile_id;
	}_qTileRequest;
	typedef QList<qTileRequest> qTileRequests;
	class qTileLayerRender;
	/** Handler for tiled WMS-C/WMTS requests, the data are written to the given image */
	class QgsWmsTiledImageDownloadHandler : public QObject
	{
		Q_OBJECT
	public:

		QgsWmsTiledImageDownloadHandler(const qTileRequests& requests,
										qTileLayerRender* pRender);
		~QgsWmsTiledImageDownloadHandler();

		void downloadBlocking();

	protected slots:
		void tileReplyFinished();
		void cancelled();

	protected:
		/**
		* \brief Relaunch tile request cloning previous request parameters and managing max repeat
		*
		* \param oldRequest request to clone to generate new tile request
		*
		* request is not launched if max retry is reached. Message is logged.
		*/
		void repeatTileRequest(QNetworkRequest const& oldRequest);

		void finish() { QMetaObject::invokeMethod(mEventLoop, "quit", Qt::QueuedConnection); }

		qWmsAuthorization mAuth;

		qTileLayerRender* mRender;
		QEventLoop* mEventLoop;

		//! Running tile requests
		QList<QNetworkReply*> mReplies;

	};

	/** Class keeping simple statistics for WMS provider - per unique URI */
	class QgsWmsStatistics
	{
	public:
		struct Stat
		{
			Stat()
				: errors(0)
				, cacheHits(0)
				, cacheMisses(0)
			{}
			int errors;
			int cacheHits;
			int cacheMisses;
		};

		//! get reference to layer's statistics - insert to map if does not exist yet
		static Stat& statForUri(const QString& uri) { return sData[uri]; }

	protected:
		static QMap<QString, Stat> sData;
	};

	class qEdomDownloadHandler : public QObject
	{
		Q_OBJECT
	public:

		qEdomDownloadHandler(QgsPolygonXY* geo, const qTileRequests& requests, qTileDb* mTileStore);
		~qEdomDownloadHandler();

		void downloadBlocking();

	protected slots:
		void tileReplyFinished();
		void cancelled();

	private:
		QgsPolygonXY* pDataBoundPolygon;
		void CutImage(LONGLONG tileReqNo, QByteArray& buf);
	protected:
		/**
		* \brief Relaunch tile request cloning previous request parameters and managing max repeat
		*
		* \param oldRequest request to clone to generate new tile request
		*
		* request is not launched if max retry is reached. Message is logged.
		*/
		void repeatTileRequest(QNetworkRequest const& oldRequest);

		void finish() { QMetaObject::invokeMethod(mEventLoop, "quit", Qt::QueuedConnection); }

		qWmsAuthorization mAuth;

		QEventLoop* mEventLoop;
		qTileDb* mTileStore;
		//! Running tile requests
		QList<QNetworkReply*> mReplies;
	};
}