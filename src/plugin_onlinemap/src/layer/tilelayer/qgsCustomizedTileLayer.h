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
	/// ��Ƭͼ������
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
	/// �����չ����Ƭͼ��
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
		/**!ǿ�Ʒ���null�������ָ�����ڹر�ʱ�ᷢ���������������һ������
		   \�������ڵ�ͼ���̱��棬�ٴ�ʱ�������⡣
		*/
		QgsDataProvider* dataProvider() override { return nullptr; };

	signals:
		void tileDraw(qTileLayerRender* pRender); // ������Ⱦ��Ϣ
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
		//!��Ƭ��Ⱦ�¼�
		void on_TileDraw(qTileLayerRender* pRender);
	};
}
#endif