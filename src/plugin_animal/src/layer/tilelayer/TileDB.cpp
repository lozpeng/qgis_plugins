#include "TileDB.h"
#include "QObject"

CTileDB::CTileDB(QObject *parent) : QObject(parent)
{
}


CTileDB::~CTileDB(void)
{
}


///////////////////////////////
CEDOMTileDB::CEDOMTileDB(QObject *parent) : CTileDB(parent)
{
	m_db = NULL;
}


CEDOMTileDB::~CEDOMTileDB(void)
{
	close();
}

void CEDOMTileDB::close()
{
	if(m_db)
	{
		m_db->close();
		delete m_db;
		m_db = NULL;
	}
}

//数据的初始化连接
bool CEDOMTileDB::InitConnection(QString connStr)
{
	m_connStr = connStr;
	//打开数据
	m_db = new CReadTileDb();
	//本地文件
	if(!m_db->open(m_connStr))
		return false;

	return true;
}

bool CEDOMTileDB::IsConnection()
{
	if(!m_db || !m_db->IsConnection())
		return false;

	return true;
}

QByteArray CEDOMTileDB::ReadHead()
{
	return m_db->ReadHeaderData();
}

QByteArray CEDOMTileDB::ReadBlock(LONGLONG id)
{
	return m_db->read(id);
}

void CEDOMTileDB::RequestHead()
{
}

void CEDOMTileDB::RequestBlock(LONGLONG id)
{
}

///////////////////////////////
CMDOMTileDB::CMDOMTileDB(QObject *parent) : CTileDB(parent)
{
	m_db = NULL;
}


CMDOMTileDB::~CMDOMTileDB(void)
{
	close();
}

void CMDOMTileDB::close()
{
	if (m_db)
	{
		m_db->close();
		delete m_db;
		m_db = NULL;
	}
}

//数据的初始化连接
bool CMDOMTileDB::InitConnection(QString connStr)
{
	m_connStr = connStr;
	//打开数据
	m_db = new CReadTileDb();
	//本地文件
	if (!m_db->open(m_connStr))
		return false;

	return true;
}

bool CMDOMTileDB::IsConnection()
{
	if (!m_db || !m_db->IsConnection())
		return false;

	return true;
}

QByteArray CMDOMTileDB::ReadHead()
{
	return m_db->ReadHeaderData();
}

QByteArray CMDOMTileDB::ReadBlock(LONGLONG id)
{
	return m_db->read(id);
}

void CMDOMTileDB::RequestHead()
{
}

void CMDOMTileDB::RequestBlock(LONGLONG id)
{
}

///////////////////////////////
CEviaServerTileDB::CEviaServerTileDB(QObject *parent) : CTileDB(parent)
{

}


CEviaServerTileDB::~CEviaServerTileDB(void)
{
	close();
}

bool CEviaServerTileDB::IsConnection()
{
	return true;
}

void CEviaServerTileDB::close()
{
}

//数据的初始化连接
bool CEviaServerTileDB::InitConnection(QString connStr)
{
	m_connStr = connStr;
	return true;
}

QByteArray CEviaServerTileDB::ReadHead()
{

	return NULL;
}

QByteArray CEviaServerTileDB::ReadBlock(LONGLONG id)
{
	QMap<LONGLONG,QByteArray>::iterator iter = m_DataList.find(id);
	if(iter == m_DataList.end())
	{
		return NULL;
	}
	return iter.value();
}

void CEviaServerTileDB::RequestHead()
{
	//请求的是头信息
	HttpFun *http = new HttpFun(1,0);
	QObject::connect(http,SIGNAL(signal_requestFinished(bool,char,LONGLONG,const QByteArray&)), //http请求结束信号
            this,SLOT(slot_requestFinished(bool,char,LONGLONG, const QByteArray&)));

	http->sendRequest(QString("%1&tid=SYSINFO").arg(m_connStr));


}

void CEviaServerTileDB::RequestBlock(LONGLONG id)
{
	//请求的是数据块信息
	HttpFun *http = new HttpFun(2,id,this);
	QObject::connect(http,SIGNAL(signal_requestFinished(bool,char,LONGLONG,const QByteArray&)), //http请求结束信号
            this,SLOT(slot_requestFinished(bool,char,LONGLONG,const QByteArray&)));


	http->sendRequest(QString("%1&tid=%2").arg(m_connStr).arg(id,4,16,QLatin1Char('0')));
}

//请求结束
void CEviaServerTileDB::slot_requestFinished(bool bSuccess, char type,LONGLONG tid, const QByteArray &data)
{
	switch(type)
	{
	case 1:
		emit signal_requestHeaderFinished(bSuccess,data);
		break;
	case 2:
		m_DataList[tid] = data;
		emit signal_requestBlockFinished(bSuccess,tid,data);
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

QString CEviaServerTileDB::GetURL(LONGLONG id)
{
	return (QString("%1&tid=%2").arg(m_connStr).arg(id, 4, 16, QLatin1Char('0')));
}

///////////////////////////////
CTDServerTileDB::CTDServerTileDB(QObject *parent) : CTileDB(parent)
{
}

CTDServerTileDB::~CTDServerTileDB(void)
{
}

//数据的初始化连接
bool CTDServerTileDB::InitConnection(QString connStr)
{
	m_connStr = connStr;
	return false;
}

QByteArray CTDServerTileDB::ReadHead()
{
	return NULL;
}

QByteArray CTDServerTileDB::ReadBlock(LONGLONG id)
{
	return NULL;
}

void CTDServerTileDB::RequestHead()
{
}

void CTDServerTileDB::RequestBlock(LONGLONG id)
{
}

//根据不同地图类型，获取下载的url 地址
QString CTDServerTileDB::GetTDBaseUrl(int downloadlevel, int i, int j)
{
	//http://t5.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=17&TileRow=18240&TileCol=107913&style=default&format=tiles
	//http://t6.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=11&TileRow=284&TileCol=1687&style=default&format=tiles
	// requesturl = "http://t7.tianditu.cn/img_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=img&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + maplevel;
	//requesturl = "http://t5.tianditu.cn/img_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=img&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	return  m_connStr + QString("&x=%1&y=%2&l=%3&tk=d5b24677cd5d66023be61f408eeaf45a").arg(i).arg(j).arg(downloadlevel);
//	requesturl = "http://t2.tianditu.cn/ter_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=ter&tileMatrixSet=c&TileMatrix=" + +downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t7.tianditu.cn/wat_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=wat&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t5.tianditu.cn/raw_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=raw&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t6.tianditu.cn/cva_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=cva&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t5.tianditu.cn/eva_c/wmts?service=wmts&request=GetTile&version=1.0.0&LAYER=eva&tileMatrixSet=c&TileMatrix=" + downloadlevel + "&TileRow=" + j + "&TileCol=" + i + "&style=default&format=tiles";
	//requesturl = "http://t6.tianditu.com/cia_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=cia&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + downloadlevel;
	//requesturl = " http://t0.tianditu.com/cta_c/wmts?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0&LAYER=cta&STYLE=default&TILEMATRIXSET=c&FORMAT=tiles&TILECOL=" + i + "&TILEROW=" + j + "&TILEMATRIX=" + downloadlevel;
}

QString CTDServerTileDB::GetURL(LONGLONG id)
{
	//将易景的瓦片规则转换成天地图的规则
	int level = BLOCK_ID_LEVEL(id);
	int lx = BLOCK_ID_X(id);
	int ly = BLOCK_ID_Y(id);
	/////////////////
	return GetTDBaseUrl( level + 2, lx + (1 << (level + 1)),(1 << level)-ly-1);
}

//////////////////////////////////////////////////////////
CCHServerTileDB::CCHServerTileDB(QObject *parent) : CTileDB(parent)
{
}

CCHServerTileDB::~CCHServerTileDB(void)
{
}

//数据的初始化连接
bool CCHServerTileDB::InitConnection(QString connStr)
{
	m_connStr = connStr;
	return false;
}

QByteArray CCHServerTileDB::ReadHead()
{
	return NULL;
}

QByteArray CCHServerTileDB::ReadBlock(LONGLONG id)
{
	return NULL;
}

void CCHServerTileDB::RequestHead()
{
}

void CCHServerTileDB::RequestBlock(LONGLONG id)
{
}

//根据不同地图类型，获取下载的url 地址
QString CCHServerTileDB::GetCHBaseUrl(int downloadlevel, int i, int j)
{
//http://60.205.227.207:7090/onemap17/rest/wmts?ACCOUNT=ghyforestry&PASSWD=ghyforestry&layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileMatrix=10&TileCol=1664&TileRow=358
	QString url =  m_connStr + QString("/rest/wmts?layer=satImage&style=default&tilematrixset=satImage&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&TileCol=%1&TileRow=%2&TileMatrix=%3").arg(i).arg(j).arg(downloadlevel);
	if (!m_UserID.isEmpty())
		url += QString("&ACCOUNT=%1").arg(m_UserID);
	if(!m_Password.isEmpty())
		url += QString("&PASSWD=%1").arg(m_Password);
	return url;
}

QString CCHServerTileDB::GetURL(LONGLONG id)
{
	//将易景的瓦片规则转换成测绘地图的规则
	int level = BLOCK_ID_LEVEL(id);
	int lx = BLOCK_ID_X(id);
	int ly = BLOCK_ID_Y(id);
	/////////////////
	return GetCHBaseUrl(level + 1, lx + (1 << (level + 1)), (1 << level) - ly - 1);
}

