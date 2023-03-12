#include "qgoogleearthtile.h"
#include <qgspoint.h>
#include <math.h>

namespace geotile {
#define GOOGLEEARTH_orignX  -180
#define GOOGLEEARTH_orignY  90
#define GOOGLEEARTH_MinLatitude  -90.0
#define GOOGLEEARTH_MaxLatitude  90.0
#define GOOGLEEARTH_MinLongitude  -180.0
#define GOOGLEEARTH_MaxLongitude  180.0
	qGoogleEarthTile::qGoogleEarthTile()
	{
	}

	qGoogleEarthTile::~qGoogleEarthTile()
	{
	}

	double qGoogleEarthTile::GetLevelResolution(int level)
	{
		double num = 1.40625;
		for (int i = 0; i < level; i++)
		{
			num /= 2.0;
		}
		return num;

	}

	QgsPointXY qGoogleEarthTile::FromPixelToLatLng(long x, long y, int zoom)
	{
		double tileMatrixResolution = GetLevelResolution(zoom);
		QgsPointXY empty;
		empty.setY(GOOGLEEARTH_orignY - (double)y * tileMatrixResolution);
		empty.setX((double)x * tileMatrixResolution + GOOGLEEARTH_orignX);
		if (empty.y() < GOOGLEEARTH_MinLatitude)
		{
			empty.setY(GOOGLEEARTH_MinLatitude);
		}
		if (empty.y() > GOOGLEEARTH_MaxLatitude)
		{
			empty.setY(GOOGLEEARTH_MaxLatitude);
		}
		if (empty.x() < GOOGLEEARTH_MinLongitude)
		{
			empty.setX(GOOGLEEARTH_MinLongitude);
		}
		if (empty.x() > GOOGLEEARTH_MaxLongitude)
		{
			empty.setX(GOOGLEEARTH_MaxLongitude);
		}
		return empty;
	}

	QPoint qGoogleEarthTile::FromLatLngToPixel(double lat, double lng, int zoom)
	{
		QPoint vPoint;
		double tileMatrixResolution = GetLevelResolution(zoom);
		vPoint.setX((long)round((lng - GOOGLEEARTH_orignX) / tileMatrixResolution));
		vPoint.setY((long)round((GOOGLEEARTH_orignY - lat) / tileMatrixResolution));
		return vPoint;
	}
}
