#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <filesystem>
#include <cairo/cairo.h>
// GDAL.
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "gdal_alg.h"
#include "ogr_api.h"

#define OTL_ODBC
#define OTL_ANSI_CPP							// 中文支持
#define OTL_STREAM_READ_ITERATOR_ON				// image 等大量数据插入的支持	
#define OTL_STL			
#define OTL_CONNECT_POOL_ON
#define OTL_ANSI_CPP_11_VARIADIC_TEMPLATES
// #define OTL_CPP_17_ON
#define OTL_CPP_11_ON
#include <otl/otlv4.h>
#pragma  comment(lib, "odbc32.lib")
using OtlConnPool = otl_connect_pool<otl_connect, otl_exception>;
const char* conn_str = "Driver={MySQL ODBC 8.0 ANSI Driver};Server=192.168.0.156;Port=3306;Database=rwdb_xj;User=tzx;Password=3edc9ijn~;Option=3;";
double _param_width = 0, _param_height = 0;
double _param_left = 0, _param_top = 0, _param_bottom = 0, _param_right = 0;
std::string ptime_to_format_str(const boost::posix_time::ptime& time, const std::string& format)
{
	boost::posix_time::time_facet* tf = new boost::posix_time::time_facet(format.c_str());
	std::stringstream ss;
	ss.imbue(std::locale(std::cout.getloc(), tf));
	ss << time;
	return ss.str();
}
#define PI (3.1415926535897932384626433832795028841971693993751f)
static void GeoToMercator(const float_t lgtd, const float_t lttd, float_t& x, float_t& y)
{
	x = lgtd * 20037508.34 / 180;
	y = log(tan((90 + lttd) * PI / 360)) / (PI / 180);
	y = y * 20037508.34 / 180;

}
/// <summary>
/// 此结构以左上角为原点
/// </summary>
struct ScreenPoint
{
	ScreenPoint(void) :x(0), y(0) {}
	ScreenPoint(double _x, double _y) :x(_x), y(_y) {}
	double  x;
	double	y;
};

/// <summary>
/// 单环 或者 线 
/// </summary>
struct ScreenPath
{
	std::vector<ScreenPoint> path;
};

/// <summary>
///	多面 或者 多线
/// </summary>
struct ScreenPaths
{
	std::vector<ScreenPath> paths;
};

static void MercatorToGeo(const float_t x, const float_t y, float_t& lgtd, float_t& lttd)
{
	lgtd = x / 20037508.34 * 180;
	lttd = y / 20037508.34 * 180;
	lttd = 180 / PI * (2 * atan(exp(lttd * PI / 180)) - PI / 2);
}
ScreenPaths GetPathsFromPolygon(OGRPolygon* poPolygon)
{
	ScreenPaths retPaths;
	OGRPoint point;
	OGRLinearRing* pOGRLinearRing = poPolygon->getExteriorRing();
	int InnerRingCout = poPolygon->getNumInteriorRings();
	int RingCout = InnerRingCout + 1;// 内环个数 + 外环
	int OutPointNum = pOGRLinearRing->getNumPoints();//外环点数
	int totalPointNum = OutPointNum;
	ScreenPath OuterPath;


	for (int i = 0; i < OutPointNum; i++)//外环
	{
		pOGRLinearRing->getPoint(i, &point);
#if MERCATOR
		double _mx = 0, _my = 0;
		GeoToMercator(point.getX(), point.getY(), _mx, _my);
		OuterPath.path.push_back(ScreenPoint((_mx - _mercator_left) / _x_mercator_step, (_mercator_top - _my) / _y_mercator_step));
#else
		OuterPath.path.push_back(ScreenPoint((point.getX() - _param_left) / _x_geo_step, (_param_top - point.getY()) / _y_geo_step));
#endif
	}
	retPaths.paths.push_back(OuterPath);
	for (int j = 0; j < InnerRingCout; ++j)
	{
		OGRLinearRing* tmpInterRing = poPolygon->getInteriorRing(j);
		int InnerPointNum = tmpInterRing->getNumPoints();
		ScreenPath InnerPath;
		for (int i = 0; i < InnerPointNum; i++)//外环
		{
			tmpInterRing->getPoint(i, &point);
#if MERCATOR
			double _mx = 0, _my = 0;
			GeoToMercator(point.getX(), point.getY(), _mx, _my);
			InnerPath.path.push_back(ScreenPoint((_mx - _mercator_left) / _x_mercator_step, (_mercator_top - _my) / _y_mercator_step));
#else
			InnerPath.path.push_back(ScreenPoint((point.getX() - _param_left) / _x_geo_step, (_param_top - point.getY()) / _y_geo_step));
#endif
		}
		retPaths.paths.push_back(InnerPath);
	}

	return retPaths;
}

std::vector<ScreenPaths> ReadGeomertry(const std::string& path)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO"); // 支持中文路径
	OGRRegisterAll();
	CPLSetConfigOption("SHAPE_ENCODING", "");	// 编码支持
	GDALDataset* poDSr = (GDALDataset*)GDALOpenEx(path.c_str(), GDAL_OF_ALL | GDAL_OF_READONLY, NULL, NULL, NULL);
	OGRLayer* poLayer_r = poDSr->GetLayer(0);
	int iEntities = poLayer_r->GetFeatureCount();
	OGRFeature* pFeature_r = NULL;
	std::vector<ScreenPaths>  polygons;
	while ((pFeature_r = poLayer_r->GetNextFeature()) != NULL)
	{
		OGRGeometry* poGeometry_r = pFeature_r->GetGeometryRef();
		OGRPoint point;
		OGRPolygon* poPolygon = nullptr;
		if (poGeometry_r != NULL && wkbFlatten(poGeometry_r->getGeometryType()) == wkbPolygon)
		{
			poPolygon = (OGRPolygon*)poGeometry_r;
			polygons.push_back(GetPathsFromPolygon(poPolygon));
		}
		else if (poGeometry_r != NULL && wkbFlatten(poGeometry_r->getGeometryType()) == wkbMultiPolygon)
		{

			OGRMultiPolygon* pMultiPolygon = (OGRMultiPolygon*)poGeometry_r;
			//部位数
			int GeometryNum = pMultiPolygon->getNumGeometries();

			for (int k = 0; k < GeometryNum; k++)
			{
				poPolygon = (OGRPolygon*)(pMultiPolygon->getGeometryRef(k));
				polygons.push_back(GetPathsFromPolygon(poPolygon));
			}
		}
		OGRFeature::DestroyFeature(pFeature_r);
	}
	OGRCleanupAll();

	return polygons;
}


void _cairo_draw_paths(cairo_t* cr, ScreenPaths& paths)
{
	cairo_set_line_width(cr, 1);

	int firstLine = true;
	for (auto path : paths.paths)
	{
		if (!firstLine)
		{
			cairo_new_sub_path(cr);
		}
		else
		{
			firstLine = false;
		}
		cairo_move_to(cr, path.path.at(0).x, path.path.at(0).y);
		for (auto point : path.path)
		{
			cairo_line_to(cr, point.x, point.y);
		}
		// 闭合
		cairo_close_path(cr);

	}

	cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
	cairo_set_source_rgba(cr, 0, 0, 0, 1); cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr, 0, 0, 0, 0); cairo_stroke(cr);
	//cairo_restore(cr);
	return;
}


int main()
{
	_param_left = 73;
	_param_right = 97;
	_param_top = 50;
	_param_bottom = 34;
	_param_width = 400;
	_param_height = 350;
	std::vector<long> colors = { 0x8552a1
,0xf58f98
,0xaa363d
,0xef4136
,0xc63c26
,0xf26522
,0xc88400
,0xffe600
,0x225a1f
,0x78a355
,0x00ff00
,0xc4fdb9
,0x2a5caa
,0x00ffff
,0xbdeeef };
	auto parts = ReadGeomertry(filePath);
	OtlConnPool conn_pool;
	conn_pool.open(conn_str, 10, 10);
	cv::Mat lutRND = cv::Mat::zeros(256, 1, CV_8UC3);
	cv::Mat bound = cv::imread("mask.xinjiang.400_350.png", -1);;
	cv::Mat alphaMat = cv::Mat::ones(350, 400, CV_8UC1) * 0xaf;
	cv::bitwise_and(alphaMat, bound, alphaMat);
	std::vector<int> compression_params_with_alpha;
	compression_params_with_alpha.push_back(cv::IMWRITE_PNG_COMPRESSION);
	compression_params_with_alpha.push_back(9);
	
	for (int i = 0; i < 5; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(0x7e, 0x0e, 0xe7); }
	for (int i = 5; i < 10; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(58, 61, 253); }
	for (int i = 10; i < 15; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(46, 133, 232); }
	for (int i = 15; i < 20; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(49, 175, 230); }
	for (int i = 20; i < 30; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(47, 219, 228); }
	for (int i = 30; i < 40; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(51, 230, 162); }
	for (int i = 40; i < 50; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(0, 221, 0); }
	for (int i = 50; i < 60; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(138, 209, 0); }
	for (int i = 60; i < 75; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(189, 201, 0); }
	for (int i = 75; i < 80; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(252, 158, 0); }
	for (int i = 80; i < 85; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(255, 60, 30); }
	/*for (int i = 85; i < 90; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(209, 10, 121); }
	for (int i = 90; i < 255; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(222, 0, 186); }*/
	for (int i = 85; i < 90; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(0xb3, 0x6a, 0x42); }
	for (int i = 90; i < 255; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(0x6a, 0x2b, 0x10); }
	
	// lutRND.at<cv::Vec3b>(0, 0) = cv::Vec3b(255, 255, 255);
	std::string startTimeStr = "2021-06-11 20:00";
	std::string endTimeStr = "2021-06-13 20:00";
	auto p_start_time = boost::posix_time::time_from_string(startTimeStr);
	auto p_end_time = boost::posix_time::time_from_string(endTimeStr);
	boost::posix_time::time_duration h1our(1, 0, 0, 0);

	while (1)
	{
		std::string dbTimeStr = ptime_to_format_str(p_start_time, "%Y-%m-%d %H:%M");
		std::string dirStr = ptime_to_format_str(p_start_time, "%Y%m%d");
		std::string fileStr = ptime_to_format_str(p_start_time, "%Y%m%d%H%M");
		if (dbTimeStr >= endTimeStr)
		{
			break;
		}
		OtlConnPool::connect_ptr fromPool = conn_pool.get();
		otl_stream rStream;
		rStream.set_lob_stream_mode(true);
		cv::Mat target = cv::Mat::ones(350, 400, CV_8UC1);

		try {
			std::string querySqlStr = "select a.x, a.y, b.tswp from(select PID, round((lgtd - 73) / 0.05999)  x, round((50 - lttd) / 0.0457) y  from ForeRainPoint_B) a, (select PID, SWP * 75 tswp from TZX_SoilWater_R  where DataTime = '" + dbTimeStr + "') b where a.PID = b.PID";
			rStream.open(1, querySqlStr.c_str(), *fromPool);
			std::cout << querySqlStr << std::endl;

			for (auto& _ts : rStream)
			{
				otl_value<double> x; otl_value<double> y; otl_value<double> spw; otl_value<double> drrp;
				otl_read_row(_ts, x, y, spw);
				int sx = std::round(x.v);
				int sy = std::round(y.v);
				int val = std::round(spw.v);
				target.ptr<unsigned char>(sy, sx)[0] = val;
				
			}
		}
		catch (otl_exception& p)
		{
			std::cout << "otl error " << std::endl;
			p_start_time += h1our;
			continue;
			//return 0;
		}
		conn_pool.put(std::move(fromPool));
		cv::Mat target2;
		cv::medianBlur(target, target, 3);
		// cv::resize(target, target, cv::Size(1160, 1160), cv::INTER_CUBIC);

		//cv::bitwise_and(target, bound, target);
		cv::applyColorMap(target, target2, lutRND);
		std::vector<cv::Mat> srcChannels;
		std::vector<cv::Mat> dstChannels;
		cv::split(target2, srcChannels);
		dstChannels.push_back(srcChannels[0]);
		dstChannels.push_back(srcChannels[1]);
		dstChannels.push_back(srcChannels[2]);
		dstChannels.push_back(alphaMat);
		// 添加透明度
		cv::Mat target3;
		cv::merge(dstChannels, target3);
		std::filesystem::path targetPath("./");
		targetPath.append(dirStr);
		if (!std::filesystem::exists(targetPath))
		{
			std::filesystem::create_directories(targetPath);
		}

		targetPath.append("SW" + fileStr + ".png");

		cv::imwrite(targetPath.string().c_str(), target3, compression_params_with_alpha);
		std::cout << dbTimeStr << std::endl;
		p_start_time += h1our;
	}

	/*cv::Mat src = cv::imread("src.png", -1);
	cv::Mat dst;
	cv::applyColorMap(src, dst, lutRND);
	cv::imshow("Render Image", dst);
	cv::namedWindow("Render Image");
	cv::waitKey(0);
	cv::imwrite("convert.png", dst);*/
	return 0;

}