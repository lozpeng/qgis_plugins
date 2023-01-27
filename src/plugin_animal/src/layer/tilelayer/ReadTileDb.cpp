#include "ReadTileDb.h"
#include <qobject.h>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QFile>
#include <qmessagebox.h>
#include <Shlwapi.h> 
#include "xfileio.h"


#define d_nameInnerName  "02A4A7B8-D465-43c2-8AE0-787FDFCFE2EB"
#define d_nameOuterName  "3339F5A0-54CF-4be2-8DAA-53201F131464"
#define d_nameSize  "block_count"

CReadTileDb::CReadTileDb(void)
{
}


CReadTileDb::~CReadTileDb(void)
{
	close();
}

bool CReadTileDb::open(QString dbPath)
{
	mDbPath = dbPath;
	if(!QSqlDatabase::contains(mDbPath)) //如果没有改连接名称的连接则添加
	{
		//添加数据库时指定名称以支持多个数据库连接
		m_database = QSqlDatabase::addDatabase("QSQLITE",mDbPath);
		m_database.setDatabaseName(mDbPath);
		if(!m_database.open())
			return false;
	}
	else 
		m_database = QSqlDatabase::database(mDbPath);

	if(!m_database.isValid())return false;

	return true;
}

QByteArray CReadTileDb::ReadHeaderData()
{
	//读取头信息
	QSqlQuery query(QString("SELECT * FROM info WHERE name='%1';").arg(d_nameOuterName),m_database);
	query.exec();
	if(!query.first())
		return NULL;

	return query.value(1).toByteArray();
}

bool CReadTileDb::ReadHeader(EV_DB_Header& tileInfo)
{
	memset((void*)&tileInfo,0,sizeof(EV_DB_Header));
	//读取头信息
	QSqlQuery query(QString("SELECT * FROM info WHERE name='%1';").arg(d_nameOuterName),m_database);
	query.exec();
	if(!query.first())
		return false;

	QByteArray header_data = query.value(1).toByteArray();
	if(header_data.size()<1)
		return false;
		
	CXFileIOMem buffer;
	buffer.putData((const void*)header_data.data(),header_data.size());
	buffer.read((char*)&tileInfo.size,			sizeof(tileInfo.size));
	buffer.read((char*)&tileInfo.version,		sizeof(tileInfo.version));
	buffer.read((char*)&tileInfo.dbtype,		sizeof(tileInfo.dbtype));
	buffer.read((char*)&tileInfo.tileGeoSize,	sizeof(tileInfo.tileGeoSize));
	buffer.read((char*)&tileInfo.tileGridSize,	sizeof(tileInfo.tileGridSize));
	buffer.read((char*)&tileInfo.jpgQuality,	sizeof(tileInfo.jpgQuality));
	buffer.read((char*)&tileInfo.minlevel,		sizeof(tileInfo.minlevel));
	buffer.read((char*)&tileInfo.maxlevel,		sizeof(tileInfo.maxlevel));
	buffer.seek(sizeof(char)*2,CXFileIO::XFILEIO_SEEK_CUR);
	//buffer.read((char*)&tileInfo.reserved,		sizeof(char)*2);
	buffer.read((char*)&tileInfo.minx,			sizeof(double));
	buffer.read((char*)&tileInfo.miny,			sizeof(double));
	buffer.read((char*)&tileInfo.maxx,			sizeof(double));
	buffer.read((char*)&tileInfo.maxy,			sizeof(double));
	buffer.read((char*)&tileInfo.factor,		sizeof(tileInfo.factor));
	buffer.read((char*)&tileInfo.lowvalue,		sizeof(float));
	buffer.read((char*)&tileInfo.highvalue,		sizeof(float));
	buffer.read((char*)&tileInfo.dfactor,		sizeof(double));

	return true;
}

bool CReadTileDb::close()
{
	if(!m_database.isOpen() && !m_database.open())
		return false;

	m_database.close();
	QSqlDatabase::removeDatabase(mDbPath);
	return true;
}

QByteArray CReadTileDb::read(LONGLONG tileId)
{
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if(!m_database.isOpen()&&!m_database.open())
		return NULL;

	QSqlQuery query(QString("SELECT * FROM blocks WHERE id='%1'").arg(tileId),m_database);
	query.exec();
	if(query.first())
	{
		QVariant vdata = query.value(1);
		return vdata.toByteArray();
	}
	return NULL;
}


bool CReadTileDb::IsConnection()
{
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if(!m_database.isOpen()&&!m_database.open())
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////
//CWriteTileDb
CWriteTileDb::CWriteTileDb(void)
{
}


CWriteTileDb::~CWriteTileDb(void)
{
	close();
}

bool CWriteTileDb::open(QString dbPath)
{
	mDbPath = dbPath;
	if (!QSqlDatabase::contains(mDbPath)) //如果没有改连接名称的连接则添加
	{
		//添加数据库时指定名称以支持多个数据库连接
		m_database = QSqlDatabase::addDatabase("QSQLITE", mDbPath);
		m_database.setDatabaseName(mDbPath);
		if (!m_database.open())
			return false;
	}
	else
		m_database = QSqlDatabase::database(mDbPath);

	if (!m_database.isValid())return false;

	SetDBParams();
	return true;
}

void CWriteTileDb::SetDBParams()
{
	ExeSQL("PRAGMA page_size = 1024;");
	ExeSQL("PRAGMA cache_size = 1000;");
	ExeSQL("PRAGMA synchronous = 0;");
	ExeSQL("PRAGMA journal_mode = MEMORY;");
}

void CWriteTileDb::ExeSQL(QString sql)
{
	if (!m_database.isOpen() && !m_database.open())
		return;

	QSqlQuery query(sql, m_database);
	query.exec();
}

QByteArray CWriteTileDb::ReadHeaderData()
{
	//读取头信息
	QSqlQuery query(QString("SELECT * FROM info WHERE name='%1';").arg(d_nameOuterName), m_database);
	query.exec();
	if (!query.first())
		return NULL;

	return query.value(1).toByteArray();
}

bool CWriteTileDb::ReadHeader(EV_DB_Header& tileInfo)
{
	memset((void*)&tileInfo, 0, sizeof(EV_DB_Header));
	//读取头信息
	QSqlQuery query(QString("SELECT * FROM info WHERE name='%1';").arg(d_nameOuterName), m_database);
	query.exec();
	if (!query.first())
		return false;

	QByteArray header_data = query.value(1).toByteArray();
	if (header_data.size()<1)
		return false;

	CXFileIOMem buffer;
	buffer.putData((const void*)header_data.data(), header_data.size());
	buffer.read((char*)&tileInfo.size, sizeof(tileInfo.size));
	buffer.read((char*)&tileInfo.version, sizeof(tileInfo.version));
	buffer.read((char*)&tileInfo.dbtype, sizeof(tileInfo.dbtype));
	buffer.read((char*)&tileInfo.tileGeoSize, sizeof(tileInfo.tileGeoSize));
	buffer.read((char*)&tileInfo.tileGridSize, sizeof(tileInfo.tileGridSize));
	buffer.read((char*)&tileInfo.jpgQuality, sizeof(tileInfo.jpgQuality));
	buffer.read((char*)&tileInfo.minlevel, sizeof(tileInfo.minlevel));
	buffer.read((char*)&tileInfo.maxlevel, sizeof(tileInfo.maxlevel));
	buffer.seek(sizeof(char) * 2, CXFileIO::XFILEIO_SEEK_CUR);
	//buffer.read((char*)&tileInfo.reserved,		sizeof(char)*2);
	buffer.read((char*)&tileInfo.minx, sizeof(double));
	buffer.read((char*)&tileInfo.miny, sizeof(double));
	buffer.read((char*)&tileInfo.maxx, sizeof(double));
	buffer.read((char*)&tileInfo.maxy, sizeof(double));
	buffer.read((char*)&tileInfo.factor, sizeof(tileInfo.factor));
	buffer.read((char*)&tileInfo.lowvalue, sizeof(float));
	buffer.read((char*)&tileInfo.highvalue, sizeof(float));
	buffer.read((char*)&tileInfo.dfactor, sizeof(double));

	return true;
}

bool CWriteTileDb::close()
{
	if (!m_database.isOpen() && !m_database.open())
		return false;

	m_database.commit();
	m_database.close();

	QSqlDatabase::removeDatabase(mDbPath);
	return true;
}

QByteArray CWriteTileDb::read(LONGLONG tileId)
{
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return NULL;

	QSqlQuery query(QString("SELECT * FROM blocks WHERE id='%1'").arg(tileId), m_database);
	query.exec();
	if (query.first())
	{
		QVariant vdata = query.value(1);
		return vdata.toByteArray();
	}
	return NULL;
}


bool CWriteTileDb::IsConnection()
{
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	return true;
}

//更新数据，直接更新数据块，调用该方法要确保原有的数据记录以及存在
bool CWriteTileDb::UpdateData(LONGLONG tileId, QByteArray td)
{
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	//UPDATE table_name
	//SET column1 = value1, column2 = value2, ...
	//WHERE some_column = some_value;
	QString sql = QString("UPDATE  blocks SET file=? WHERE id=%1;").arg(tileId);
	QSqlQuery query(sql, m_database);
	query.bindValue(0,QVariant(td));
	return query.exec();
}

bool CWriteTileDb::save(LONGLONG tileId, QByteArray td) {
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	QString sql = QString("INSERT INTO blocks(id,file) VALUES(%1,?);").arg(tileId);
	QSqlQuery query(sql, m_database);
	//	QString str_id = QString("%1").arg(tileId);
	//	query.bindValue(str_id, QVariant(td), QSql::Binary);
	query.addBindValue(QVariant(td));

	bool success = query.exec();
	if (success)
		mblock_count++;

	return success;
}
//根据层块编号删除一个瓦片
bool CWriteTileDb::del(LONGLONG tileId) {
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	QSqlQuery query(QString("DELETE FROM blocks WHERE id=%1").arg(tileId), m_database);
	return query.exec();
}

//根据ID号批量删除
bool CWriteTileDb::delDatas(LONGLONG *tileId, int count)
{
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	if (count < 1)
		return false;

	QString sql = QString("DELETE FROM blocks WHERE ");
	for (int i = 0; i < count; i++)
	{
		if (i > 0)
			sql += " or ";

		sql += QString("id=%1").arg(tileId[i]);
	}

	QSqlQuery query(sql, m_database);
	return query.exec();
}

bool CWriteTileDb::ExitTile(LONGLONG tileId)
{//判断数据块是否存在，如果已经存在了，则返回TRUE
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	QSqlQuery query(QString("SELECT * FROM blocks WHERE id=%1").arg(tileId), m_database);
	query.exec();
	return query.first();
}
bool CWriteTileDb::DelSysTile()
{
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	char szSql[256];
	QSqlQuery query(QString("DELETE FROM info WHERE name='%1';").arg(d_nameOuterName), m_database);
	return query.exec();
}

bool CWriteTileDb::SaveSysTile(const EV_DB_Header &tileInfo) {
	//数据库不是打开的且不能打开则返回false表明不能写入数据
	if (!m_database.isOpen() && !m_database.open())
		return false;

	char szSql[256];
	sprintf(szSql, "INSERT INTO info VALUES ('%s',?);", d_nameOuterName);
	QSqlQuery query(QString(szSql), m_database);

	QByteArray data;
	data.append((const char*)&tileInfo.size, sizeof(tileInfo.size));
	data.append((const char*)&tileInfo.version, sizeof(tileInfo.version));
	data.append((const char*)&tileInfo.dbtype, sizeof(tileInfo.dbtype));
	data.append((const char*)&tileInfo.tileGeoSize, sizeof(tileInfo.tileGeoSize));
	data.append((const char*)&tileInfo.tileGridSize, sizeof(tileInfo.tileGridSize));
	data.append((const char*)&tileInfo.jpgQuality, sizeof(tileInfo.jpgQuality));
	data.append((const char*)&tileInfo.minlevel, sizeof(tileInfo.minlevel));
	data.append((const char*)&tileInfo.maxlevel, sizeof(tileInfo.maxlevel));
	data.append((const char*)&tileInfo.reserved, sizeof(char) * 2);
	data.append((const char*)&tileInfo.minx, sizeof(tileInfo.minx));
	data.append((const char*)&tileInfo.miny, sizeof(tileInfo.miny));
	data.append((const char*)&tileInfo.maxx, sizeof(tileInfo.maxx));
	data.append((const char*)&tileInfo.maxy, sizeof(tileInfo.maxy));
	data.append((const char*)&tileInfo.factor, sizeof(tileInfo.factor));
	data.append((const char*)&tileInfo.lowvalue, sizeof(float));
	data.append((const char*)&tileInfo.highvalue, sizeof(float));
	data.append((const char*)&tileInfo.dfactor, sizeof(double));

	//	query.bindValue(g_nameOuterName, QVariant(data));
	query.addBindValue(QVariant(data));
	bool success = query.exec();
	return success;
}
