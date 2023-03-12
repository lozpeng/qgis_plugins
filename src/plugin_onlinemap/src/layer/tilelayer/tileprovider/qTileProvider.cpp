#include "qTileProvider.h"


#include "../web/qhttprequest.h"
#include<ctime>

namespace geotile {	
	qTileProvider::qTileProvider(QObject* parent) {}
	qTileProvider::~qTileProvider(void) {}


	///////////////////////////////
	qEdomTileProvider::qEdomTileProvider(QObject* parent) : qTileProvider(parent)
	{
		m_db = NULL;
	}


	qEdomTileProvider::~qEdomTileProvider(void)
	{
		close();
	}

	void qEdomTileProvider::close()
	{
		if (m_db)
		{
			m_db->close();
			delete m_db;
			m_db = NULL;
		}
	}

	//数据的初始化连接
	bool qEdomTileProvider::InitConnection(QString connStr)
	{
		m_connStr = connStr;
		//打开数据
		m_db = new qTileDbPrivateReader();
		//本地文件
		if (!m_db->open(m_connStr))
			return false;

		return true;
	}

	bool qEdomTileProvider::IsConnection()
	{
		if (!m_db || !m_db->IsConnection())
			return false;

		return true;
	}

	QByteArray qEdomTileProvider::ReadHead()
	{
		return m_db->ReadHeaderData();
	}

	QByteArray qEdomTileProvider::ReadBlock(LONGLONG id)
	{
		return m_db->read(id);
	}

	void qEdomTileProvider::RequestHead()
	{
	}

	void qEdomTileProvider::RequestBlock(LONGLONG id)
	{
	}

	///////////////////////////////
	qWebMecatorLocalDbProvider::qWebMecatorLocalDbProvider(QObject* parent) : qTileProvider(parent)
	{
		m_db = NULL;
	}


	qWebMecatorLocalDbProvider::~qWebMecatorLocalDbProvider(void)
	{
		close();
	}

	void qWebMecatorLocalDbProvider::close()
	{
		if (m_db)
		{
			m_db->close();
			delete m_db;
			m_db = NULL;
		}
	}

	//数据的初始化连接
	bool qWebMecatorLocalDbProvider::InitConnection(QString connStr)
	{
		m_connStr = connStr;
		//打开数据
		m_db = new qTileDbPrivateReader();
		//本地文件
		if (!m_db->open(m_connStr))
			return false;

		return true;
	}

	bool qWebMecatorLocalDbProvider::IsConnection()
	{
		if (!m_db || !m_db->IsConnection())
			return false;

		return true;
	}

	QByteArray qWebMecatorLocalDbProvider::ReadHead()
	{
		return m_db->ReadHeaderData();
	}

	QByteArray qWebMecatorLocalDbProvider::ReadBlock(LONGLONG id)
	{
		return m_db->read(id);
	}

	void qWebMecatorLocalDbProvider::RequestHead()
	{
	}

	void qWebMecatorLocalDbProvider::RequestBlock(LONGLONG id)
	{
	}

	///////////////////////////////
	qEviaServerTileProvider::qEviaServerTileProvider(QObject* parent) : qTileProvider(parent)
	{

	}


	qEviaServerTileProvider::~qEviaServerTileProvider(void)
	{
		close();
	}

	bool qEviaServerTileProvider::IsConnection()
	{
		return true;
	}

	void qEviaServerTileProvider::close()
	{
	}

	//数据的初始化连接
	bool qEviaServerTileProvider::InitConnection(QString connStr)
	{
		m_connStr = connStr;
		return true;
	}

	QByteArray qEviaServerTileProvider::ReadHead()
	{

		return NULL;
	}

	QByteArray qEviaServerTileProvider::ReadBlock(LONGLONG id)
	{
		map<LONGLONG, QByteArray>::iterator iter = m_DataList.find(id);
		if (iter == m_DataList.end())
		{
			return NULL;
		}
		return iter->second;
	}

	void qEviaServerTileProvider::RequestHead()
	{
		//请求的是头信息
		qHttpRequest* http = new qHttpRequest(1, 0);
		QObject::connect(http, SIGNAL(signal_requestFinished(bool, char, LONGLONG, const QByteArray&)), //http请求结束信号
			this, SLOT(slot_requestFinished(bool, char, LONGLONG, const QByteArray&)));

		http->sendRequest(QString("%1&tid=SYSINFO").arg(m_connStr));
	}

	void qEviaServerTileProvider::RequestBlock(LONGLONG id)
	{
		//请求的是数据块信息
		qHttpRequest* http = new qHttpRequest(2, id, this);
		QObject::connect(http, SIGNAL(signal_requestFinished(bool, char, LONGLONG, const QByteArray&)), //http请求结束信号
			this, SLOT(slot_requestFinished(bool, char, LONGLONG, const QByteArray&)));


		http->sendRequest(QString("%1&tid=%2").arg(m_connStr).arg(id, 4, 16, QLatin1Char('0')));
	}

	//请求结束
	void qEviaServerTileProvider::slot_requestFinished(bool bSuccess, char type, LONGLONG tid, const QByteArray& data)
	{
		switch (type)
		{
		case 1:
			emit signal_requestHeaderFinished(bSuccess, data);
			break;
		case 2:
			m_DataList[tid] = data;
			emit signal_requestBlockFinished(bSuccess, tid, data);
			break;
		}
		//if(bSuccess)
		//{
		// //   ui->resultEdit->setPlainText(strResult); //纯文本显示
		//}
		//else
		//{
		////    ui->resultEdit->setPlainText(tr("请求失败！"));
		//}

	}

	QString qEviaServerTileProvider::GetURL(LONGLONG id)
	{
		return (QString("%1&tid=%2").arg(m_connStr).arg(id, 4, 16, QLatin1Char('0')));
	}

	///////////////////////////////
	qTdtServerTileProvider::qTdtServerTileProvider(QObject* parent) : qTileProvider(parent)
	{
	}

	qTdtServerTileProvider::~qTdtServerTileProvider(void)
	{

	}

	//数据的初始化连接
	bool qTdtServerTileProvider::InitConnection(QString connStr)
	{
		m_connStr = connStr;
		return false;
	}

	QByteArray qTdtServerTileProvider::ReadHead()
	{
		return NULL;
	}

	QByteArray qTdtServerTileProvider::ReadBlock(LONGLONG id)
	{
		return NULL;
	}

	void qTdtServerTileProvider::RequestHead()
	{
	}

	void qTdtServerTileProvider::RequestBlock(LONGLONG id)
	{
	}

	//根据不同地图类型，获取下载的url 地址
	QString qTdtServerTileProvider::GetTDBaseUrl(int downloadlevel, int i, int j)
	{
		//http://t5.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=17&TileRow=18240&TileCol=107913&style=default&format=tiles
		//http://t6.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=11&TileRow=284&TileCol=1687&style=default&format=tiles
		// requesturl = "http://t7.tianditu.cn/img_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=img&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + maplevel;
		//requesturl = "http://t5.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
		//return  m_connStr + QString("&x=%1&y=%2&l=%3&tk=d5b24677cd5d66023be61f408eeaf45a").arg(i).arg(j).arg(downloadlevel);
		//	requesturl = "http://t2.tianditu.cn/ter_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=ter&tileMatrixSet=c&TileMatrix=" + +downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
			//requesturl = "http://t7.tianditu.cn/wat_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=wat&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
			//requesturl = "http://t5.tianditu.cn/raw_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=raw&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
			//requesturl = "http://t6.tianditu.cn/cva_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=cva&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
			//requesturl = "http://t5.tianditu.cn/eva_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=eva&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
			//requesturl = "http://t6.tianditu.com/cia_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=cia&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + downloadlevel;
			//requesturl = " http://t0.tianditu.com/cta_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=cta&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + downloadlevel;
		QString url = m_connStr;
		if (url.contains("%1"))
		{
			qsrand(time(0));
			int n = qrand() % 8;
			url = url.arg(n);
		}
		return url + QString("&x=%1&y=%2&l=%3").arg(i).arg(j).arg(downloadlevel);
	}

	QString qTdtServerTileProvider::GetURL(LONGLONG id)
	{
		//将易景的瓦片规则转换成天地图的规则
		int level = BLOCK_ID_LEVEL(id);
		int lx = BLOCK_ID_X(id);
		int ly = BLOCK_ID_Y(id);
		/////////////////
		return GetTDBaseUrl(level + 2, lx + (1 << (level + 1)), (1 << level) - ly - 1);
	}

	//////////////////////////////////////////////////////////
	qCeHuiServerTileProvider::qCeHuiServerTileProvider(QObject* parent) : qTileProvider(parent)
	{
	}

	qCeHuiServerTileProvider::~qCeHuiServerTileProvider(void)
	{
	}

	//数据的初始化连接
	bool qCeHuiServerTileProvider::InitConnection(QString connStr)
	{
		m_connStr = connStr;
		return false;
	}

	QByteArray qCeHuiServerTileProvider::ReadHead()
	{
		return NULL;
	}

	QByteArray qCeHuiServerTileProvider::ReadBlock(LONGLONG id)
	{
		return NULL;
	}

	void qCeHuiServerTileProvider::RequestHead()
	{
	}

	void qCeHuiServerTileProvider::RequestBlock(LONGLONG id)
	{
	}

	//根据不同地图类型，获取下载的url 地址
	QString qCeHuiServerTileProvider::GetCHBaseUrl(int downloadlevel, int i, int j)
	{
		//http://60.205.227.207:7090/onemap17/rest/wmts?ACCOUNT=ghyforestry&PASSWD=ghyforestry&layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileMatrix=10&TileCol=1664&TileRow=358
		QString url = m_connStr + QString("/rest/wmts?layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileCol=%1&TileRow=%2&TileMatrix=%3").arg(i).arg(j).arg(downloadlevel);
		if (!m_UserID.isEmpty())
			url += QString("&ACCOUNT=%1").arg(m_UserID);
		if (!m_Password.isEmpty())
			url += QString("&PASSWD=%1").arg(m_Password);
		return url;
	}
	/// <summary>
	/// 
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	QString qCeHuiServerTileProvider::GetURL(LONGLONG id)
	{
		//将易景的瓦片规则转换成测绘地图的规则
		int level = BLOCK_ID_LEVEL(id);
		int lx = BLOCK_ID_X(id);
		int ly = BLOCK_ID_Y(id);
		/////////////////
		return GetCHBaseUrl(level + 1, lx + (1 << (level + 1)), (1 << level) - ly - 1);
	}
}