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
		QString prjTxt;// 坐标系信息，采用proj4 的文本格式
		QString userid;//用名	
		QString	pwd;//密码
	};
	/// <summary>
	/// 渲染图像信息
	/// </summary>
	struct qRenderImageInfo {
		QImage* image;
		QRect  rect;
	};
}
#endif