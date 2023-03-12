#pragma once

#include <QObject>
class QgsPointXY;
class QPoint;

namespace geotile {
	class qGoogleEarthTile : public QObject
	{
		Q_OBJECT

	public:
		qGoogleEarthTile();
		~qGoogleEarthTile();
		double GetLevelResolution(int level);
		QgsPointXY FromPixelToLatLng(long x, long y, int zoom);
		QPoint FromLatLngToPixel(double lat, double lng, int zoom);
	};
}
