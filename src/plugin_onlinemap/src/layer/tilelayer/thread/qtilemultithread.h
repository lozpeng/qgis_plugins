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
	/// 块计算公式，主要是判断多边形关系和图片处理
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
		bool		mDataValid;//数据是否开始有效
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

		void UnitBlock(qTileDbPrivateWriter* pTileDb, LONGLONG tid, QImage bk_image);//加入需要合并的数据块
		void DelBlock(qTileDbPrivateWriter* pTileDb, LONGLONG tid);//添加需要删除的数据块
		bool ExitTile(qTileDbPrivateWriter* pTileDb, LONGLONG tid);
	private:
		int				m_ProcessThreadCount;
		QString			m_EdomPath;
		QgsPolygonXY* m_pDataBoundPolygon;//数据的边界范围，只能是面数据,用于裁剪使用
		bool			mStop;
		QList<qTileBlockThread*>	m_BlockThreads;
		QList<LONGLONG>			m_UnitTIDs, m_DelTIDs;
		QList<QImage>			m_UnitImages;
		QMutex					mBlockDBMutex;//对CWriteTileDb操作的锁定

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
		QgsMapSettings mSettings, mAlphaSettings;//mAlphaSettings为alpah图层对象
		QImage mPixmap, mAlphaPixmap;
		qTileMultiThread* m_Parent;
	private:
		bool	m_bRun;
		bool	m_working;//表明本线程是否在工作
		QList<qCutTileStru>		mOutRects;//需要输出的数据块范围
		QMutex					mOutRectMutex;//mOutRects变量的锁定
		qTileProviderUtils* m_pTileUtils;//当前块的计算公式
		void InitAlphaSettings();
		void InitAlphaData(QList<QgsMapLayer*>& Layers);
		void AddTreeGroup(QString GroupName, QList<QgsMapLayer*>& Layers);
		void AddMapLayer(QString LayerName, QList<QgsMapLayer*>& Layers);


		void ShapeImage(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId, QImage* outmap);
		bool IsValid(QgsPolygonXY* pDataBoundPolygon, LONGLONG tileId);
		void saveTileData(LONGLONG tileId, QgsRectangle MapRect, QImage& pixMap);
		void KillThread() { m_bRun = false; };//kill线程，在数据处理完成后再运行
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
		QString mtileDbPath;//瓦片数据库路径
		QList<qCutTileThread*>	m_CutTileThreads;

		QgsCoordinateTransform* mTransform;//计算的瓦片范围向地图数据范围转换
		bool mStop;
		bool mIsCreateNewFile;

		void RenderBlockByLevel(int level);
		void saveTileData(LONGLONG tileId, QgsRectangle MapRect, QPixmap& pixMap);
	public:
		bool	m_bCompressTile;//是否压缩图片，=TRUE表示要压缩图片，默认为false
		QString	m_AlphaTreeGroupName;//透明的图层名称，为空表示不需要
	private:
		QObject* m_pObject;
		QList<qSaveBlockStru>	m_SaveList;//需要保存的数据块序列
		QMutex					mSaveListMutex;//对m_SaveList操作的锁定
		QgsPolygonXY* m_pDataBoundPolygon;//数据的边界范围，只能是面数据,用于裁剪使用
		qTileProviderUtils* m_pTileUtils;
		QList<QgsRectangle>		m_ValidateBound;//数据的有效区域范围

		void CreateCutTileThread(QObject* parent, QgsMapCanvas* ifaceCanvas);
		void FreeCutTileThread();
		bool AddThread(QgsRectangle outRect, int minx, int miny, int maxx, int maxy,
			int level, int minIndX, int minIndY, int maxIndX, int maxIndY);//将工作加入线程，成果返回TRUE，失败返回FALSE
		void SaveTileToDB(bool AllCommit);
		void ShowThreadInfo();
		void ReadBound(QString shp_file);//读取需要裁剪的多边形范围

		//进度条的计算变量，让用户感受当前进行数据处理进度的情况。
		LONGLONG		m_TileCount;
		LONGLONG		m_CurTileStep;
		QString m_msg;//消息
		//////////////////////////////////////////////
		int		m_ProcessCount;//后台线程数量
	public:
		QString myear, mgdcode, mname, malias, mzt;

		bool ConnectDBSucceed();
		void SaveData(LONGLONG id, QByteArray bytes, string format);
		int GetProcessCount() { return m_ProcessCount; };
		QgsPolygonXY* GetBoundPolygon() {
			return m_pDataBoundPolygon;
		};
		void AddValidateBound(QgsRectangle bound) { m_ValidateBound.append(bound); };
		//判断某个ID是否在区域有效范围内
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
	{//下载线程
		Q_OBJECT
	public:
		qTileDownloadThread(QObject* parent);
		~qTileDownloadThread(void);

		int m_type; //=1为edom服务，=2为天地图服务,=4为测绘遥感地图
		QString m_url;
		int m_minlevel, m_maxlevel;
		EV_DB_Header	mTileInfo;
		qTileDb* mTileStore;
		QString			mtileDbPath;//瓦片数据库路径
		QString			mUserID;
		QString			mPwd;
		QgsPolygonXY    mDownloadGeometry;//需要下载的区域范围，为多边形范围
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
	//CAddTopTileThread修补顶层金字塔数据的线程
	class CAddTopTileThread : public QThread
	{
		Q_OBJECT
	public:
		CAddTopTileThread(QObject* parent, const QString tileDbPath);
		~CAddTopTileThread(void);

	private:
		EV_DB_Header mTileInfo;
		qTileDbPrivateWriter* mTileStore;
		QString mtileDbPath;//瓦片数据库路径
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
	//合并edom数据的线程
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
	//合并edom数据的线程
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

	//DEM去噪点线程
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

