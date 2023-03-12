#ifndef Q_PLUGIN_TILE_LAYER_H_
#define Q_PLUGIN_TILE_LAYER_H_
#pragma once

#include <QObject>
#include <QObject>
#include <qgspluginlayer.h>
#include <qgsrendercontext.h>
#include <qgisinterface.h>
#include <qgsrasterinterface.h>
#include <qgsdataprovider.h>
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

#include "tiledb/qtiledb.h"
#include "tileprovider/qTileProvider.h"
#include "tileprovider/qtileproviderutils.h"

#include "qTiledLayerDef.h"
namespace geotile {

	/// <summary>
	/// 瓦片图层类型
	/// </summary>
	class SILEN_TILE_EXPORT qPluginTileLayerType : public QgsPluginLayerType
	{
		public:
			qPluginTileLayerType(QgisInterface* qgsInterface, qTileLayerDef lyrDef);
			~qPluginTileLayerType();

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
			qTileLayerDef  m_lyrDef;
	};
	class qTileLayerRender;
	class qTileLayerRenderFeedBack :public QgsRasterBlockFeedback
	{
		Q_OBJECT
	public:
		//! Create feedback object based on our layer renderer
		explicit qTileLayerRenderFeedBack(qTileLayerRender* r);

		//! when notified of new data in data provider it launches a preview draw of the raster
		void onNewData() override;
	private:
		qTileLayerRender* mR = nullptr;   //!< Parent renderer instance
		int mMinimalPreviewInterval;  //!< In milliseconds
		QTime mLastPreview;           //!< When last preview has been generated
	};
	class qgsCustomizedTileLayer;
	/// <summary>
	/// 
	/// </summary>
	class SILEN_TILE_EXPORT qTileLayerRender :public QgsMapLayerRenderer
	{
	public:
		qTileLayerRender(qgsCustomizedTileLayer* layer, QgsRenderContext& rendererContext);
		~qTileLayerRender() override;

		bool render() override;
		QgsFeedback* feedback() const override;

		QList<qRenderImageInfo>	m_RenderData;

		QgsRenderContext& RenderContext() {
			return mContext;
		}
	private:

		//	QPainter *mPainter = nullptr;
		const QgsMapToPixel* mMapToPixel = nullptr;

		QgsRenderContext& mContext;
		//! feedback class for cancellation and preview generation
		qTileLayerRenderFeedBack* mFeedback = nullptr;

		friend class qTileLayerRenderFeedBack;
	};
	/// <summary>
	/// 插件扩展的瓦片图层
	/// </summary>
	class SILEN_TILE_EXPORT qgsCustomizedTileLayer :public QgsPluginLayer
	{
		Q_OBJECT
	public:
		qgsCustomizedTileLayer(QgisInterface* qgsInterface, qTileLayerDef lyrDef);
		~qgsCustomizedTileLayer();

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
		bool render(QgsRenderContext& rendererContext, qTileLayerRender* pRender);
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

		//! Returns the extent of the layer.
		virtual QgsRectangle extent() const;
		/// <summary>
		/// 
		/// </summary>
		/// <param name=""></param>
		virtual void setTransformContext(const QgsCoordinateTransformContext&);
		/**!强制返回null，如果不指定则在关闭时会发生错误，这里最好下一步定义
		   \否则在在地图工程保存，再打开时存在问题。
		*/
		QgsDataProvider* dataProvider() override { return nullptr; };

	signals:
		void tileDraw(qTileLayerRender* pRender); // 数据渲染消息
	private:
		qTileLayerDef mTileLyrDef;
		QgisInterface* mQgsInterface;
		qTileProviderUtils* mpTileUtils;
		bool mUserLastZoomForPrint;
		int mCancasLastZoom;
		double mDownloadTimeOut;

		qTileProvider* mTileProvider;
		EV_DB_Header	m_header;

	private slots:
		void slot_requestBlockFinished(bool bSuccess, LONGLONG tid, const QByteArray& data);
		void slot_requestHeaderFinished(bool bSuccess, const QByteArray& data);
		void slot_requestBlock(LONGLONG tid);
		//!瓦片渲染事件
		void on_TileDraw(qTileLayerRender* pRender);
	};
}
#endif