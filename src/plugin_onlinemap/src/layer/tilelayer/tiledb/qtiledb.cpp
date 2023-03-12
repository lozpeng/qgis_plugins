#include "qtiledb.h"

#include <qobject.h>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QFile>
#include <QSqlError>
#include <QSqlDriver> 
#include <qmessagebox.h>
#include <qgslogger.h>
#include <QFileInfo>

namespace geotile {
	static const QString C_TILE_SQL = "CREATE TABLE IF NOT EXISTS %1(TI INTEGER,TD BLOB,PRIMARY KEY(TI) ON CONFLICT REPLACE)";
	static const QString C_TILEINFO_SQL = "CREATE TABLE IF NOT EXISTS VGSTLYRS(LYRNAME TEXT,LYRALIAS TEXT,LYRTHEM TEXT,TILESIZE INTEGER,MAXLEVEL INTEGER,MINLEVEL INTEGER,MINX NUMERIC,MINY NUMERIC,MAXX NUMERIC,MAXY NUMERIC,LYEAR TEXT,IMGTYPE TEXT,GBCODE TEXT,VERSION NUMERIC,ISPROJ NUMERIC,CONSTRAINT [] PRIMARY KEY ([LYRNAME]) ON CONFLICT REPLACE)";

#define C_BLOCK_SQL "CREATE TABLE blocks (id INTEGER IDENTITY PRIMARY KEY, file BLOB);"
#define C_INFO_SQL "CREATE TABLE info ( name TEXT PRIMARY KEY, file BLOB);"
#define C_BLOCK_INDEX_SQL "CREATE INDEX blocks_index on blocks(id);"
#define	TABLE_NAME		"propertydata"
#define	DATAINFO_NAME	"datainfo"


	qTileDb::qTileDb(QString dbPath, const EV_DB_Header& tileInfo, bool CreateNew)
		:mDbPath(dbPath), mTileInfo(tileInfo) {
		this->initdb(CreateNew);
		this->SaveSysTile(tileInfo);
		mblock_count = 0;
		SetDBParams();
	}

	qTileDb::~qTileDb()
	{
		close();
	}

	//�ر���Ƭ�洢���ݿ�
	bool qTileDb::close()
	{
		if (!m_database.isOpen() && !m_database.open())
			return false;
		//�������ݿ�����û�д�����,�����ύ��֪���᲻�ᷢ������
		//������
		QSqlQuery indexTile(m_database);
		indexTile.prepare(QString(C_BLOCK_INDEX_SQL));
		indexTile.exec();

		char szSql[256];
		sprintf(szSql, "INSERT INTO info values('block_count', %d);", mblock_count);
		QSqlQuery countTile(m_database);
		countTile.prepare(QString(szSql));
		countTile.exec();

		m_database.commit();
		m_database.close();
		QSqlDatabase::removeDatabase(mDbPath);
		return true;
	}
	//�������ݿ�д������
	bool qTileDb::startTransaction() {
		if (!m_database.isOpen() && !m_database.open())
			return false;
		return m_database.transaction();
	}
	//�رղ��ύ����
	bool qTileDb::endTransaction() {
		if (!m_database.isOpen() && !m_database.open())
			return false;
		return m_database.commit();
	}

	bool qTileDb::save(LONGLONG tileId, QByteArray td) {
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
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

#define MAX_INSERT_COUNT 100 //һ�β�����������¼��
	bool qTileDb::save(QList<qSaveBlockStru>* blocks)
	{//��������
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
		if (!m_database.isOpen() && !m_database.open())
			return false;

		//һ�β��������¼
		int n, count = blocks->size();
		if (count < 1)
			return true;

		QList<QVariant>	datalist;
		QString sql = "INSERT INTO blocks(id,file) VALUES";
		for (n = 0; n < count; n++)
		{
			if (datalist.size() > 0)
				sql += ",";

			sql += QString("(%1,?)").arg(blocks->at(n).id);
			datalist.append(QVariant(blocks->at(n).bytes));
			if (datalist.size() > MAX_INSERT_COUNT)
			{
				sql += ";";
				QSqlQuery query(sql, m_database);
				for (int k = 0; k < datalist.size(); k++)
					query.addBindValue(datalist[k]);

				bool success = query.exec();//
				if (success)
					mblock_count += count;
				else {
					QSqlError sql_error = query.lastError();
					QString msg = sql_error.text();
					QgsDebugMsg(msg);
				}

				sql = "INSERT INTO blocks(id,file) VALUES";
				datalist.clear();
			}
		}
		if (datalist.size() > 0)
		{//ʣ���
			sql += ";";
			QSqlQuery query(sql, m_database);
			for (int k = 0; k < datalist.size(); k++)
				query.addBindValue(datalist[k]);

			bool success = query.exec();//
			if (success)
				mblock_count += count;
			else {
				QSqlError sql_error = query.lastError();
				QString msg = sql_error.text();
				QgsDebugMsg(msg);
			}

			sql = "INSERT INTO blocks(id,file) VALUES";
			datalist.clear();
		}

		return true;
	}

	//���ݲ����ɾ��һ����Ƭ
	bool qTileDb::del(LONGLONG tileId) {
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
		if (!m_database.isOpen() && !m_database.open())
			return false;

		QSqlQuery query(QString("DELETE FROM blocks WHERE id=%1").arg(tileId), m_database);
		return query.exec();
	}

	bool qTileDb::ExitTile(LONGLONG tileId)
	{//�ж����ݿ��Ƿ���ڣ�����Ѿ������ˣ��򷵻�TRUE
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
		if (!m_database.isOpen() && !m_database.open())
			return false;

		QSqlQuery query(QString("SELECT * FROM blocks WHERE id=%1").arg(tileId), m_database);
		query.exec();
		return query.first();
	}

	QByteArray qTileDb::read(LONGLONG tileId)
	{
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
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

	const char* g_nameInnerName = "02A4A7B8-D465-43c2-8AE0-787FDFCFE2EB";
	const char* g_nameOuterName = "3339F5A0-54CF-4be2-8DAA-53201F131464";
	const char* g_nameSize = "block_count";
	bool qTileDb::SaveSysTile(const EV_DB_Header& tileInfo) {
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
		if (!m_database.isOpen() && !m_database.open())
			return false;

		char szSql[256];
		sprintf(szSql, "INSERT INTO info VALUES ('%s',?);", g_nameOuterName);
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

	bool qTileDb::DelSysTile()
	{
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
		if (!m_database.isOpen() && !m_database.open())
			return false;

		char szSql[256];
		QSqlQuery query(QString("DELETE FROM info WHERE name='%1';").arg(g_nameOuterName), m_database);
		return query.exec();
	}

	bool qTileDb::SaveDataInfo(QString myear, QString mgdcode, QString mname, QString malias, QString mzt)
	{
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
		if (!m_database.isOpen() && !m_database.open())
			return false;

		QString str = QString::fromWCharArray(L"ʸ����Ƭ");
		QString szSql = QString("INSERT INTO propertydata VALUES (1,'%1','%2','%3','%4','%5','%6');").arg(myear).arg(str).arg(mgdcode).arg(mname).arg(malias).arg(mzt);
		QSqlQuery query(szSql, m_database);
		bool success = query.exec();
		return success;
	}

	bool qTileDb::IsConnection()
	{
		//���ݿⲻ�Ǵ򿪵��Ҳ��ܴ��򷵻�false��������д������
		if (!m_database.isOpen() && !m_database.open())
			return false;

		return true;
	}

	//CreateNew�����Ƿ񴴽�һ���µ��ļ������ԭ�����ļ������򸲸�
	void qTileDb::initdb(bool CreateNew)
	{
		if (CreateNew)
		{
			QFileInfo fileInfo(mDbPath);
			if (fileInfo.exists() && fileInfo.isFile())
				QFile::remove(mDbPath);
		}
		if (!QSqlDatabase::contains(mDbPath)) //���û�и��������Ƶ����������
		{
			//������ݿ�ʱָ��������֧�ֶ�����ݿ�����
			m_database = QSqlDatabase::addDatabase("QSQLITE", mDbPath);
			m_database.setDatabaseName(mDbPath);
			if (!m_database.open())
			{
				return;
			}
		}
		else
			m_database = QSqlDatabase::database(mDbPath);

		if (!m_database.isValid())return;
		//��Ƭ���ݱ�
		char szSql[512];
		QSqlQuery blocInfo(m_database);
		blocInfo.prepare(QString(C_BLOCK_SQL));
		blocInfo.exec();

		//�����Ϣ
		QSqlQuery infoTile(m_database);
		infoTile.prepare(QString(C_INFO_SQL));
		infoTile.exec();


		//����������Ϣ�ļ�¼����¼�е��ַ�������������80����
		sprintf(szSql, "CREATE TABLE %s (name varchar(80) PRIMARY KEY,alias varchar(80));\n", DATAINFO_NAME);
		QSqlQuery datainfnameTile(m_database);
		datainfnameTile.prepare(QString(szSql));
		datainfnameTile.exec();

		sprintf(szSql, "CREATE TABLE %s (evia_id INTEGER IDENTITY PRIMARY KEY,year TEXT,imagetype TEXT,gbcode varchar(80),name TEXT,alias TEXT,subject TEXT);", TABLE_NAME);
		QSqlQuery tablenameTile(m_database);
		tablenameTile.prepare(QString(szSql));
		tablenameTile.exec();

		if (!CreateNew)
		{
			QSqlQuery delquery(QString("DELETE FROM datainfo"), m_database);
			delquery.exec();

			QSqlQuery delpropertydataquery(QString("DELETE FROM propertydata"), m_database);
			delpropertydataquery.exec();

			QSqlQuery delinfoquery(QString("DELETE FROM info"), m_database);
			delinfoquery.exec();
		}

		QString insertsql;
		QSqlQuery datainfoTile(m_database);

		wchar_t ss[256];
		swprintf(ss, L"INSERT INTO datainfo values('evia_id', '���')");
		datainfoTile.prepare(QString::fromWCharArray(ss));
		datainfoTile.exec();

		swprintf(ss, L"INSERT INTO datainfo values('year', '���')");
		datainfoTile.prepare(QString::fromWCharArray(ss));
		datainfoTile.exec();

		swprintf(ss, L"INSERT INTO datainfo values('imagetype', 'Ӱ������')");
		datainfoTile.prepare(QString::fromWCharArray(ss));
		datainfoTile.exec();

		swprintf(ss, L"INSERT INTO datainfo values('gbcode', '�������')");
		datainfoTile.prepare(QString::fromWCharArray(ss));
		datainfoTile.exec();

		swprintf(ss, L"INSERT INTO datainfo values('name', '����')");
		datainfoTile.prepare(QString::fromWCharArray(ss));
		datainfoTile.exec();

		swprintf(ss, L"INSERT INTO datainfo values('alias', '����')");
		datainfoTile.prepare(QString::fromWCharArray(ss));
		datainfoTile.exec();

		swprintf(ss, L"INSERT INTO datainfo values('subject', 'ר��')");
		datainfoTile.prepare(QString::fromWCharArray(ss));
		datainfoTile.exec();


		mblock_count = 0;
	}


	void qTileDb::SetDBParams()
	{
		ExeSQL("PRAGMA page_size = 1024;");
		ExeSQL("PRAGMA cache_size = 1000;");
		ExeSQL("PRAGMA synchronous = 0;");
		ExeSQL("PRAGMA journal_mode = MEMORY;");
	}

	void qTileDb::ExeSQL(QString sql)
	{
		if (!m_database.isOpen() && !m_database.open())
			return;

		QSqlQuery query(sql, m_database);
		query.exec();
	}
}
