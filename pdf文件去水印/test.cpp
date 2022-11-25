#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <filesystem>
#include <cairo/cairo.h>
#define OTL_ODBC
#define OTL_ANSI_CPP							// ����֧��
#define OTL_STREAM_READ_ITERATOR_ON				// image �ȴ������ݲ����֧��	
#define OTL_STL			
#define OTL_CONNECT_POOL_ON
#define OTL_ANSI_CPP_11_VARIADIC_TEMPLATES
// #define OTL_CPP_17_ON
#define OTL_CPP_11_ON
#include <otl/otlv4.h>
#pragma  comment(lib, "odbc32.lib")
using OtlConnPool = otl_connect_pool<otl_connect, otl_exception>;
const char* conn_str = "driver=sql server;server=192.168.0.155;port=1433;uid=sa;pwd=3edc9ijn~;database=HUNAN_DYNAMIC_RULE";

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


static void MercatorToGeo(const float_t x, const float_t y, float_t& lgtd, float_t& lttd)
{
	lgtd = x / 20037508.34 * 180;
	lttd = y / 20037508.34 * 180;
	lttd = 180 / PI * (2 * atan(exp(lttd * PI / 180)) - PI / 2);
}


void draw_colors(std::vector<long> colors)
{
	cairo_set_line_width(cr, 6);

	cairo_rectangle(cr, 12, 12, 232, 70);

}

int main()
{

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
	OtlConnPool conn_pool;
	conn_pool.open(conn_str, 10, 10);
	cv::Mat lutRND = cv::Mat::zeros(256, 1, CV_8UC3);
	cv::Mat bound = cv::imread("mask.hunan.png", -1);;
	cv::Mat alphaMat = cv::Mat::ones(1160, 1160, CV_8UC1) * 0xaf;
	//cv::bitwise_and(alphaMat, bound, alphaMat);
	std::vector<int> compression_params_with_alpha;
	compression_params_with_alpha.push_back(cv::IMWRITE_PNG_COMPRESSION);
	compression_params_with_alpha.push_back(9);
	//for (int r =0; r < alphaMat.rows; ++r)
	//{
	//	for (int c = 0; c < alphaMat.cols; ++c)
	//	{
	//		if (alphaMat.ptr<unsigned char>(r, c)[0] > 0)
	//		{
	//			std::cout << (int)alphaMat.ptr<unsigned char>(r, c)[0] << std::endl;
	//			return 0;
	//		}
	//	}
	//}
	//cv::imwrite("aplha.png", alphaMat);
	//return 0;
	// B G R  , ������Ⱦ��ʽֻ֧�� ��ͨ�� �� ��ͨ��
	/*lutRND.at<cv::Vec3b>(13, 0) = cv::Vec3b(0xaa, 0x5c, 0x2a);
	lutRND.at<cv::Vec3b>(14, 0) = cv::Vec3b(0x00, 0xe0, 0xff);
	lutRND.at<cv::Vec3b>(15, 0) = cv::Vec3b(0x21, 0x6c, 0xf3);
	lutRND.at<cv::Vec3b>(16, 0) = cv::Vec3b(0x36, 0x41, 0xef);*/

	//for (int i = 0; i < 25; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(118, 7, 221); }
	//for (int i = 25; i < 40; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(58, 61, 253); }
	//for (int i = 40; i < 45; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(46, 133, 232); }
	//for (int i = 45; i < 50; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(49, 175, 230); }
	//for (int i = 50; i < 55; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(47, 219, 228); }
	//for (int i = 55; i < 60; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(51, 230, 162); }
	//for (int i = 60; i < 65; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(0, 221, 0); }
	//for (int i = 65; i < 70; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(138, 209, 0); }
	//for (int i = 70; i < 75; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(189, 201, 0); }
	//for (int i = 75; i < 80; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(252, 158, 0); }
	//for (int i = 80; i < 85; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(255, 60, 30); }
	//for (int i = 85; i < 90; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(209, 10, 121); }
	//for (int i = 90; i < 255; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(222, 0, 186); }

	for (int i = 0; i < 5; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(118, 7, 221); }
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
	cv::imwrite("scope.png", lutRND);
	return 0;
	// lutRND.at<cv::Vec3b>(0, 0) = cv::Vec3b(255, 255, 255);
	std::string startTimeStr = "2022-10-08 01:00";
	std::string endTimeStr = "2022-10-28 00:00";
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
		cv::Mat target = cv::Mat::ones(116, 116, CV_8UC1);

		try {
			std::string querySqlStr = "select a.x, a.y, b.tswp from(select PID, round((lgtd - 108.65) / 0.05, 0)  x, round((30.25 - lttd) / 0.05, 0) y  from ForeRainPoint_B) a, (select PID, SWP * 100 tswp from TZX_SoilWater_R where DataTime = '" + dbTimeStr + "') b where a.PID = b.PID";
			rStream.open(1, querySqlStr.c_str(), *fromPool);

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
		cv::medianBlur(target, target, 5);
		cv::resize(target, target, cv::Size(1160, 1160), cv::INTER_CUBIC);

		cv::bitwise_and(target, bound, target);
		cv::applyColorMap(target, target2, lutRND);
		std::vector<cv::Mat> srcChannels;
		std::vector<cv::Mat> dstChannels;
		cv::split(target2, srcChannels);
		dstChannels.push_back(srcChannels[0]);
		dstChannels.push_back(srcChannels[1]);
		dstChannels.push_back(srcChannels[2]);
		dstChannels.push_back(alphaMat);
		// ���͸����
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