#ifndef Q_TILE_PROVIDER_H_
#define Q_TILE_PROVIDER_H_

#include <QObject>
#include <QString>

#include <QMap>
#include <QVector>

#include "../qtileinfo.h"
#include "../qtileutils.h"

#include "qtiledbprivate.h"
namespace geotile
{
	//瓦片数据源类型
	enum qTileSourceType {
		tUnKnown = 0,		//未知格式 可能不做处理
		tEdomDb = 1,		//Evia Edom本地瓦片
		tMBTiles = 2,		//MBTiles格式数据，也是一个sqlite格式的数据库
		tLocalDb = 3,		//本地林草瓦片数据格式
		tWebMecatorDb = 4, //web Mecator 瓦片数据库
		tEdomServer = 5, //林草数据在线服务
		tXYZTileServer = 6,//在线XYZ瓦片数据格式
		tTdtMapServer = 7, //天地图在线地图
		tCHJMapServer = 8, //测绘局地图服务
	};

	/*
	* 瓦片数据存储基础类
	*/
	class qTileProvider :public QObject
	{
		Q_OBJECT

	public:
		qTileProvider(QObject* parent);
		virtual ~qTileProvider(void);
		virtual qTileSourceType GetType() = 0;
		virtual void close() = 0;
		//数据的初始化连接
		virtual bool InitConnection(QString connStr) = 0;
		virtual bool IsConnection() = 0;
		virtual QByteArray ReadHead() = 0;
		virtual QByteArray ReadBlock(LONGLONG id) = 0;

		virtual void RequestHead() = 0;
		virtual void RequestBlock(LONGLONG id) = 0;
		virtual QString GetURL(LONGLONG id) = 0;
	protected:
		QString m_connStr;// 数据连接字符串，可能是本地也可能是服务器地址
	};
	/// <summary>
	/// 本地edom文件
	/// </summary>
	class qEdomTileProvider : public qTileProvider
	{
	private:
		qTileDbPrivateReader* m_db;

	public:
		qEdomTileProvider(QObject* parent);
		virtual ~qEdomTileProvider();

		qTileSourceType GetType() { return qTileSourceType::tEdomDb; }
		//数据的初始化连接
		bool InitConnection(QString connStr);
		bool IsConnection();
		void close();
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id) { return ""; };
	};

	/// <summary>
	/// 本地墨卡托投影的文件
	/// </summary>
	class qWebMecatorLocalDbProvider : public qTileProvider
	{
	private:
		qTileDbPrivateReader* m_db;

	public:
		qWebMecatorLocalDbProvider(QObject* parent);
		virtual ~qWebMecatorLocalDbProvider();

		qTileSourceType GetType() { return qTileSourceType::tWebMecatorDb; }
		//数据的初始化连接
		bool InitConnection(QString connStr);
		bool IsConnection();
		void close();
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id) { return ""; };
	};

	/// <summary>
	/// 易景服务
	/// </summary>
	class qEviaServerTileProvider : public qTileProvider
	{
		Q_OBJECT

	private:
		map<LONGLONG, QByteArray>	m_DataList;

	public:
		qEviaServerTileProvider(QObject* parent);
		virtual ~qEviaServerTileProvider();
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		qTileSourceType GetType() { return qTileSourceType::tEdomServer; }
		//数据的初始化连接
		bool InitConnection(QString connStr);
		bool IsConnection();
		void close();
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id);

	signals:
		void signal_requestHeaderFinished(bool bSuccess, const QByteArray& strResult); //http请求结束
		void signal_requestBlockFinished(bool bSuccess, LONGLONG tid, const QByteArray& data);

	private slots:
		void slot_requestFinished(bool bSuccess, char type, LONGLONG tid, const QByteArray& data);

	};
	/// <summary>
	/// 天地图服务
	/// </summary>
	class qTdtServerTileProvider : public qTileProvider
	{
	public:
		qTdtServerTileProvider(QObject* parent);
		virtual ~qTdtServerTileProvider();

		qTileSourceType GetType() { return qTileSourceType::tTdtMapServer; }
		void close() {};
		//数据的初始化连接
		bool InitConnection(QString connStr);

		bool IsConnection() {
			return true;
		};
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id);

	private:
		QString GetTDBaseUrl(int downloadlevel, int i, int j);

	};

	/// <summary>
	/// 测绘局的服务地址 天地图服务 ,主要是对接一版图
	/// </summary>
	class qCeHuiServerTileProvider : public qTileProvider
	{
	private:
		QString		m_UserID;//用户名
		QString		m_Password;//密码
	public:
		qCeHuiServerTileProvider(QObject* parent);
		virtual ~qCeHuiServerTileProvider();

		void SetUSerID(QString user, QString ps) { m_UserID = user; m_Password = ps; }
		qTileSourceType GetType() { return qTileSourceType::tCHJMapServer; }
		void close() {};
		//数据的初始化连接
		bool InitConnection(QString connStr);
		bool IsConnection() {
			return true;
		}
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id);

	private:
		QString GetCHBaseUrl(int downloadlevel, int i, int j);
	};
}
#endif