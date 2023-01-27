#pragma once

#include <QObject>
#include <qgspluginlayer.h>
#include <qgsrendercontext.h>
#include <qgisinterface.h>
#include <qgsrasterinterface.h>

#include <QtCore/qglobal.h>

//#ifdef _SILEN_TILE_
# define SILEN_TILE_EXPORT Q_DECL_EXPORT
//#else
//# define SILEN_TILE_EXPORT Q_DECL_IMPORT
//#endif
//
#include <qgspluginlayer.h>
#include <qgspluginlayerregistry.h>
#include <QgsMapLayerRenderer.h>
#include <QgsMapLayerRenderer.h>
#include "ReadTileDb.h"
#include "TileDB.h"
class CTileLayerRender;
class qgsTiledCustomLayer;

typedef struct qgsTileLayerDef
{
	int type; //=1本地edom文件，2=evia瓦片服务，=3为天地图服务
	QString title;
	QString url;
	QString userid;//用名	
	QString	pwd;//密码
}qgsTileLayerDef;

class CTileUtils;
class SILEN_TILE_EXPORT PluginTileLayerType :public QgsPluginLayerType
{
public:
	PluginTileLayerType(QgisInterface* qgsInterface, qgsTileLayerDef lyrDef);
	~PluginTileLayerType() {}

	QString name();

	/** Return new layer of this type. Return NULL on error */
	virtual QgsPluginLayer* createLayer();

	/** Return new layer of this type, using layer URI (specific to this plugin layer type). Return NULL on error.
	 * @note added in 2.10
	 */
	virtual QgsPluginLayer* createLayer(const QString& uri);

	/** Show plugin layer properties dialog. Return false if the dialog cannot be shown. */
	virtual bool showLayerProperties(QgsPluginLayer* layer);
private:
	QgisInterface* mQgsInterface;
	qgsTileLayerDef	m_lyrDef;
};

/**
* \ingroup core
* Specific internal feedback class to provide preview of raster layer rendering.
* \note not available in Python bindings
* \since QGIS 3.0
*/
class CTileLayerRendererFeedback : public QgsRasterBlockFeedback
{
	Q_OBJECT
public:
	//! Create feedback object based on our layer renderer
	explicit CTileLayerRendererFeedback(CTileLayerRender* r);

	//! when notified of new data in data provider it launches a preview draw of the raster
	void onNewData() override;
private:
	CTileLayerRender* mR = nullptr;   //!< Parent renderer instance
	int mMinimalPreviewInterval;  //!< In milliseconds
	QTime mLastPreview;           //!< When last preview has been generated
};

///@endcond

typedef struct {
	QImage* image;
	QRect  rect;
}RenderImageInfo;
/**
* \ingroup core
* Implementation of threaded rendering for raster layers.
*
* \note not available in Python bindings
* \since QGIS 2.4
*/
class SILEN_TILE_EXPORT CTileLayerRender : public QgsMapLayerRenderer
{
public:
	CTileLayerRender(qgsTiledCustomLayer* layer, QgsRenderContext& rendererContext);
	~CTileLayerRender() override;

	bool render() override;
	QgsFeedback* feedback() const override;

	QList<RenderImageInfo>	m_RenderData;
private:

	//	QPainter *mPainter = nullptr;
	const QgsMapToPixel* mMapToPixel = nullptr;

	QgsRenderContext& mContext;
	//! feedback class for cancellation and preview generation
	CTileLayerRendererFeedback* mFeedback = nullptr;

	friend class CTileLayerRendererFeedback;
};

class qgsTiledCustomLayer : public QgsPluginLayer
{
	Q_OBJECT
public:
	qgsTiledCustomLayer(QgisInterface* qgsInterface, qgsTileLayerDef lyrDef);
	~qgsTiledCustomLayer();

	bool LoadHeader(QByteArray header_data);

	QgsPluginLayer* clone()const {
		return nullptr;
	}
	/**
	* Copies attributes like name, short name, ... into another layer.
	* \param layer The copy recipient
	* \since QGIS 3.0
	*/
	void clone(QgsMapLayer* layer) {};
	bool render(QgsRenderContext& rendererContext, CTileLayerRender* pRender);
	bool readSymbology(const QDomNode& node, QString& errorMessage,
		QgsReadWriteContext& context, StyleCategories categories = AllStyleCategories);
	bool writeSymbology(QDomNode& node, QDomDocument& doc, QString& errorMessage, const QgsReadWriteContext& context,
		StyleCategories categories = AllStyleCategories) const {
		return false;
	};
	/**
	* Synchronises with changes in the datasource
	*/
	virtual void reload() {};
	/**
	* Returns new instance of QgsMapLayerRenderer that will be used for rendering of given context
	* \since QGIS 2.4
	*/
	virtual QgsMapLayerRenderer* createMapRenderer(QgsRenderContext& rendererContext) SIP_FACTORY;
	/**
	* Sets the coordinate transform context to \a transformContext
	*
	* \since QGIS 3.8
	*/
	virtual void setTransformContext(const QgsCoordinateTransformContext& transformContext) {};

	//! Returns the extent of the layer.
	virtual QgsRectangle extent() const;

signals:
	void signal_requestDraw(QgsRenderContext& rendererContext, CTileLayerRender* pRender); //渲染请求

private:
	qgsTileLayerDef mTileLyrDef;
	QgisInterface* mQgsInterface;
	CTileUtils* mpTileUtils;
	bool mUserLastZoomForPrint;
	int mCancasLastZoom;
	double mDownloadTimeOut;

	CTileDB* m_db;
	EV_DB_Header	m_header;

private slots:
	void slot_requestBlockFinished(bool bSuccess, LONGLONG tid, const QByteArray& data);
private slots:
	void slot_requestHeaderFinished(bool bSuccess, const QByteArray& data);
private slots:
	void slot_requestBlock(LONGLONG tid);
private slots:
	void slot_requestDraw(QgsRenderContext& rendererContext, CTileLayerRender* pRender);
};


