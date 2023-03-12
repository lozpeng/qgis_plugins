#ifndef Q_TILED_LAYER_DEF_H_
#define Q_TILED_LAYER_DEF_H_
#include <QString>
#include <QImage>
#include <QRect>

#include "tiledb/qtiledb.h"
#include "tileprovider/qTileProvider.h"
namespace geotile {
	struct qTileLayerDef
	{
		qTileSourceType type;
		QString title;
		QString url;
		int servers = 0;
		QString prjTxt;// ����ϵ��Ϣ������proj4 ���ı���ʽ
		QString userid;//����	
		QString	pwd;//����
	};
	/// <summary>
	/// ��Ⱦͼ����Ϣ
	/// </summary>
	struct qRenderImageInfo {
		QImage* image;
		QRect  rect;
	};
}
#endif