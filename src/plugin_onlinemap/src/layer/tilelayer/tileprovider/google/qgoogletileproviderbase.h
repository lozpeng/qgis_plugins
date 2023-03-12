#pragma once

#include <QObject>
#include <QString>

namespace geotile {
	/// <summary>
/// 地图图层服务基础类
/// </summary>

	typedef enum VisualStyle
	{
		None,
		Gray,
		Invert,
		Tint,
		Embossed,
		Mosaic,
	}VisualStyle;
	class qGoogleTileProviderBase : public QObject
	{
		Q_OBJECT

	public:
		qGoogleTileProviderBase(QObject* parent);
		~qGoogleTileProviderBase();
	private:
		QString Sec1;
		QString RouteUrlFormatPointLatLng;
		QString RouteUrlFormatStr;
		QString WalkingStr;
		QString RouteWithoutHighwaysStr;
		QString RouteStr;
		QString ReverseGeocoderUrlFormat;
		QString GeocoderUrlFormat;
		QString DirectionUrlFormatStr;
		QString DirectionUrlFormatPoint;
		QString DirectionUrlFormatWaypoint;
		QString DirectionUrlFormatWaypointStr;

		QString ServerAPIs;
		QString Server;
		QString ServerChina;
		QString ServerKorea;
		QString ServerKoreaKr;
		QString string_0;
		QString SecureWord;
		QString APIKeyA;
		//MapProvider[] overlays;
		bool TryCorrectVersion;
		bool init;
	};
}