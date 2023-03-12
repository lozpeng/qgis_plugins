#include "qgoogletileproviderbase.h"

namespace geotile {
	qGoogleTileProviderBase::qGoogleTileProviderBase(QObject* parent)
		: QObject(parent)
	{
		Sec1 = "&s=";
		RouteUrlFormatPointLatLng = "http://maps.{6}/maps?f=q&output=dragdir&doflg=p&hl={0}{1}&q=&saddr=@{2},{3}&daddr=@{4},{5}";
		RouteUrlFormatStr = "http://maps.{4}/maps?f=q&output=dragdir&doflg=p&hl={0}{1}&q=&saddr=@{2}&daddr=@{3}";
		WalkingStr = "&mra=ls&dirflg=w";
		RouteWithoutHighwaysStr = "&mra=ls&dirflg=dh";
		RouteStr = "&mra=ls&dirflg=d";
		ReverseGeocoderUrlFormat = "http://maps.{0}/maps/api/geocode/xml?latlng={1},{2}&language={3}&sensor=false";
		GeocoderUrlFormat = "http://maps.{0}/maps/api/geocode/xml?address={1}&language={2}&sensor=false";
		DirectionUrlFormatStr = "http://maps.{7}/maps/api/directions/xml?origin={0}&destination={1}&sensor={2}&language={3}{4}{5}{6}";
		DirectionUrlFormatPoint = "http://maps.{9}/maps/api/directions/xml?origin={0},{1}&destination={2},{3}&sensor={4}&language={5}{6}{7}{8}";
		DirectionUrlFormatWaypoint = "http://maps.{8}/maps/api/directions/xml?origin={0},{1}&waypoints={2}&sensor={3}&language={4}{5}{6}{7}";
		DirectionUrlFormatWaypointStr = "http://maps.{7}/maps/api/directions/xml?origin={0}&waypoints={1}&sensor={2}&language={3}{4}{5}{6}";

		//ServerAPIs /* ;}~~ */ = GString(/*{^_^}*/Resources.ServerAPIs/*d{'_'}b*/);
		//Server /* ;}~~~~ */ = GString(/*{^_^}*/Resources.Server/*d{'_'}b*/);
		//ServerChina /* ;}~ */ = GString(/*{^_^}*/Resources.ServerChina/*d{'_'}b*/);
		//ServerKorea /* ;}~~ */ = GString(/*{^_^}*/Resources.ServerKorea/*d{'_'}b*/);
		//ServerKoreaKr /* ;}~ */ = GString(/*{^_^}*/Resources.ServerKoreaKr/*d{'_'}b*/);
		string_0 = "googleapis.com";

		APIKeyA = "ABQIAAAAWaQgWiEBF3lW97ifKnAczhRAzBk5Igf8Z5n2W3hNnMT0j2TikxTLtVIGU7hCLLHMAuAMt-BO5UrEWA";
		//private MapProvider[] overlays;
		TryCorrectVersion = true;
		init = false;
	}

	qGoogleTileProviderBase::~qGoogleTileProviderBase()
	{
	}
}
