#pragma once

#include <QObject>
#include <QFileInfo>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QThread>
#include <QTime>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QVariant>

#include <qgscoordinatetransform.h>
#include <qgsmapcanvas.h>
#include <qgsrectangle.h>
#include <QgsGeometry.h>
#include <qgspolygon.h>

#include "../tiledb/qtiledb.h"

#include "../tileprovider/qtileprovider.h"
#include "../tileprovider/qtiledbprivate.h"
#include "../tileprovider/qtileproviderutils.h"

namespace geotile {
	class qCutEdomThread;
	/// <summary>
	/// ����㹫ʽ����Ҫ���ж϶���ι�ϵ��ͼƬ����
	/// </summary>
	class qTileBlockThread :public QThread {
		Q_OBJECT
	public:
		qTileBlockThread(qCutEdomThread* pCutEdom, QObject* parent);
		~qTileBlockThread();

		void Init(QgsPolygonXY* DataBoundPolygon, qEdomTileProviderUtils* pUtils,
						EV_DB_Header* header, qTileDbPrivateWriter* TileStore);
		bool AddData(LONGLONG tid);
		bool IsProcessging() { return mDataValid; };
		void Fini();
	private:
		qCutEdomThread* m_pCutEdom;
		QgsPolygonXY* m_pDataBoundPolygon;
		LONGLONG	m_tid;
		qEdomTileProviderUtils* m_pUtils;
		EV_DB_Header* m_header;
		qTileDbPrivateWriter* mTileStore;
		bool		mDataValid;//�����Ƿ�ʼ��Ч
		bool		mStopThread;
		int Block_Polygon(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId, qEdomTileProviderUtils* pTileUtils);
		void ProcessBlockData(QgsPolygonXY* pDataBoundPolygon,
						LONGLONG tileId, qEdomTileProviderUtils* pTileUtils, EV_DB_Header* header);
		void CutImage(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId, qEdomTileProviderUtils* pTileUtils, QImage* outmap);
		void DelBlockData(LONGLONG tileId, EV_DB_Header* header);
		void DelChildBlockData(LONGLONG tileId, EV_DB_Header* header);
	protected:
		void run();
	};
	/// <summary>
	/// 
	/// </summary>
	class qCutEdomThread :public QThread {
		Q_OBJECT
	public:
		qCutEdomThread(QString edompath, QgsPolygonXY* DataBoundPolygon, int ProcessThreadCount, QObject* parent);
		~qCutEdomThread();

		void UnitBlock(qTileDbPrivateWriter* pTileDb, LONGLONG tid, QImage bk_image);//������Ҫ�ϲ������ݿ�
		void DelBlock(qTileDbPrivateWriter* pTileDb, LONGLONG tid);//�����Ҫɾ�������ݿ�
		bool ExitTile(qTileDbPrivateWriter* pTileDb, LONGLONG tid);
	private:
		int				m_ProcessThreadCount;
		QString			m_EdomPath;
		QgsPolygonXY* m_pDataBoundPolygon;//���ݵı߽緶Χ��ֻ����������,���ڲü�ʹ��
		bool			mStop;
		QList<qTileBlockThread*>	m_BlockThreads;
		QList<LONGLONG>			m_UnitTIDs, m_DelTIDs;
		QList<QImage>			m_UnitImages;
		QMutex					mBlockDBMutex;//��CWriteTileDb����������

		void DelBlockData(qTileDbPrivateWriter* pTileDb, bool all = false);
		void UnitImage(LONGLONG tileId, qTileDbPrivateWriter* pTileDb, QImage* bk_map);
	protected:
		void run();
	signals:
		void signalProcess(int percent);
		void signalFinish();
		void signalMessage(QString msg);

	public slots:
		void on_stop() { mStop = true; }

	};
	class qTileMultiThread;
	class qCutTileThread : public QThread
	{
		Q_OBJECT

		QPainter mpainter;
		QgsMapSettings mSettings, mAlphaSettings;//mAlphaSettingsΪalpahͼ�����
		QImage mPixmap, mAlphaPixmap;
		qTileMultiThread* m_Parent;
	private:
		bool	m_bRun;
		bool	m_working;//�������߳��Ƿ��ڹ���
		QList<qCutTileStru>		mOutRects;//��Ҫ��������ݿ鷶Χ
		QMutex					mOutRectMutex;//mOutRects����������
		qTileProviderUtils* m_pTileUtils;//��ǰ��ļ��㹫ʽ
		void InitAlphaSettings();
		void InitAlphaData(QList<QgsMapLayer*>& Layers);
		void AddTreeGroup(QString GroupName, QList<QgsMapLayer*>& Layers);
		void AddMapLayer(QString LayerName, QList<QgsMapLayer*>& Layers);


		void ShapeImage(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId, QImage* outmap);
		bool IsValid(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId);
		void saveTileData(LONGLONG tileId, QgsRectangle MapRect, QImage& pixMap);
		void KillThread() { m_bRun = false; };//kill�̣߳������ݴ�����ɺ�������
		void BeginWork() { m_working = true; };
		void EndWork() { m_working = false; };
	protected:
		void run();

	public:
		qCutTileThread(qTileProviderUtils* utils, QObject* parent);
		~qCutTileThread();

		bool working() { return m_working; };
		void Init(qTileMultiThread* parent, QgsMapCanvas* ifaceCanvas);
		void Fini();

		bool AddData(QgsRectangle outRect, int minx, int miny, int maxx, int maxy,
			int level, int minIndX, int minIndY, int maxIndX, int maxIndY);
		int GetListCount() { return mOutRects.size(); };
	};

	/// <summary>
	/// 
	/// </summary>
	class qTileMultiThread : public QThread
	{
		Q_OBJECT

		EV_DB_Header mTileInfo;
		qTileDb* mTileStore;
		QString mtileDbPath;//��Ƭ���ݿ�·��
		QList<qCutTileThread*>	m_CutTileThreads;

		QgsCoordinateTransform* mTransform;//�������Ƭ��Χ���ͼ���ݷ�Χת��
		bool mStop;
		bool mIsCreateNewFile;

		void RenderBlockByLevel(int level);
		void saveTileData(LONGLONG tileId, QgsRectangle MapRect, QPixmap& pixMap);
	public:
		bool	m_bCompressTile;//�Ƿ�ѹ��ͼƬ��=TRUE��ʾҪѹ��ͼƬ��Ĭ��Ϊfalse
		QString	m_AlphaTreeGroupName;//͸����ͼ�����ƣ�Ϊ�ձ�ʾ����Ҫ
	private:
		QObject* m_pObject;
		QList<qSaveBlockStru>	m_SaveList;//��Ҫ��������ݿ�����
		QMutex					mSaveListMutex;//��m_SaveList����������
		QgsPolygonXY* m_pDataBoundPolygon;//���ݵı߽緶Χ��ֻ����������,���ڲü�ʹ��
		qTileProviderUtils* m_pTileUtils;
		QList<QgsRectangle>		m_ValidateBound;//���ݵ���Ч����Χ

		void CreateCutTileThread(QObject* parent, QgsMapCanvas* ifaceCanvas);
		void FreeCutTileThread();
		bool AddThread(QgsRectangle outRect, int minx, int miny, int maxx, int maxy,
			int level, int minIndX, int minIndY, int maxIndX, int maxIndY);//�����������̣߳��ɹ�����TRUE��ʧ�ܷ���FALSE
		void SaveTileToDB(bool AllCommit);
		void ShowThreadInfo();
		void ReadBound(QString shp_file);//��ȡ��Ҫ�ü��Ķ���η�Χ

		//�������ļ�����������û����ܵ�ǰ�������ݴ�����ȵ������
		LONGLONG		m_TileCount;
		LONGLONG		m_CurTileStep;
		QString m_msg;//��Ϣ
		//////////////////////////////////////////////
		int		m_ProcessCount;//��̨�߳�����
	public:
		QString myear, mgdcode, mname, malias, mzt;

		bool ConnectDBSucceed();
		void SaveData(LONGLONG id, QByteArray bytes, string format);
		int GetProcessCount() { return m_ProcessCount; };
		QgsPolygonXY* GetBoundPolygon() {
			return m_pDataBoundPolygon;
		};
		void AddValidateBound(QgsRectangle bound) { m_ValidateBound.append(bound); };
		//�ж�ĳ��ID�Ƿ���������Ч��Χ��
		bool IDInValidateBound(ENVELOPE tileRect);
	protected:
		void run();
		void saveTileData(LONGLONG id, QgsRectangle MapRect, QImage& image);

	signals:
		void signalThreadPercent(int percent);
		void signalMessage(QString msg);
		void signalProcess(int percent);
		void signalFinish();
		void DataFinish(QString _t1);

	public slots:
		void on_stop() { mStop = true; }
	public:
		qTileMultiThread(QObject* parent
			, QgsMapCanvas* ifaceCanvas
			, const EV_DB_Header& tileInfo
			, const  QString tileDbPath
			, bool CreateNewFile
			, int	ProcessNum
			, QString PolygoFilePath
			, QString AlphaName);
		~qTileMultiThread();
	};

	/////////////////////////////
	class qTileDownloadThread : public QThread
	{//�����߳�
		Q_OBJECT
	public:
		qTileDownloadThread(QObject* parent);
		~qTileDownloadThread(void);

		int m_type; //=1Ϊedom����=2Ϊ���ͼ����,=4Ϊ���ң�е�ͼ
		QString m_url;
		int m_minlevel, m_maxlevel;
		EV_DB_Header	mTileInfo;
		qTileDb* mTileStore;
		QString			mtileDbPath;//��Ƭ���ݿ�·��
		QString			mUserID;
		QString			mPwd;
		QgsPolygonXY    mDownloadGeometry;//��Ҫ���ص�����Χ��Ϊ����η�Χ
		void stop() { mStop = true; };
	protected:
		void run();

	private:
		bool	mStop;

		void DownloadData(int level, int min_lx, int max_lx, int min_y, int max_y);
		QString GetTDBaseUrl(int downloadlevel, int i, int j);
		QString GetCHBaseUrl(int downloadlevel, int i, int j);
		QString GetURL(LONGLONG id);

	signals:
		void signalThreadPercent(int curlevel, int percent);
		void signalMessage(QString msg);
		void signalFinish();

	public slots:
		void slot_requestFinished(bool bSuccess, char type, LONGLONG tid, const QByteArray& data);
	};

	/////////////////////////////////////////////
	//CAddTopTileThread�޲�������������ݵ��߳�
	class CAddTopTileThread : public QThread
	{
		Q_OBJECT
	public:
		CAddTopTileThread(QObject* parent, const QString tileDbPath);
		~CAddTopTileThread(void);

	private:
		EV_DB_Header mTileInfo;
		qTileDbPrivateWriter* mTileStore;
		QString mtileDbPath;//��Ƭ���ݿ�·��
		bool		mStop;

		QImage* createImage(int grid) const;

	signals:
		void signalThreadPercent(int curlevel, int percent);
		void signalMessage(QString msg);
		void signalFinish();

	protected:
		void run();

	public slots:
		void on_stop() { mStop = true; }

	};

	/////////////////////////////////////////////
	//�ϲ�edom���ݵ��߳�
	class CUnitTileThread : public QThread
	{
		Q_OBJECT
	public:
		CUnitTileThread(QObject* parent, const QString tileDbPath, const QStringList edomPaths);
		~CUnitTileThread(void);

	private:

		bool		mStop;

		QString		m_SaveDBPath;
		QStringList	m_DataList;

		void UnitData(qTileDb* SaveDB, EV_DB_Header* SavaHeader, QString DataFile);
		bool MergeData(QByteArray* read_bytes, QByteArray* save_bytes, QByteArray* out_bytes);
	signals:
		void signalProcess(int percent);
		void signalMessage(QString msg);
		void signalFinish();

	protected:
		void run();

	public slots:
		void on_stop() { mStop = true; }

	};

	/////////////////////////////////////////////
	//�ϲ�edom���ݵ��߳�
	class CTransparentThread : public QThread
	{
		Q_OBJECT
	public:
		CTransparentThread(QObject* parent, const QString ImgPath, const QVector<QColor> colors);
		~CTransparentThread(void);

	private:

		bool		mStop;

		QString		m_ImgPath;
		QVector<QColor>	m_colors;

	signals:
		void signalProcess(int percent);
		void signalMessage(QString msg);
		void signalFinish();

	protected:
		void run();
		bool FindFile(const QString& path);
		void TransparentImage(QString FileName);
		bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);

	public slots:
		void on_stop() { mStop = true; }

	};

	//DEMȥ����߳�
	class CDemNoiseThread : public QThread
	{
		Q_OBJECT
	public:
		CDemNoiseThread(QObject* parent, const QStringList Paths, double MinValue, double MaxValue);
		~CDemNoiseThread(void);

	private:
		QStringList	mPaths;
		bool		mStop;
		double		m_MinAlt, m_MaxAlt;

	signals:
		void signalProcess(int percent);
		void signalMessage(QString msg);
		void signalFinish();

	protected:
		void run();

	public slots:
		void on_stop() { mStop = true; }

	};
}

