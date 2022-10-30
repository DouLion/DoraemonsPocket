#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <filesystem>
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
const char* conn_str = "driver=sql server;server=192.168.0.155;port=1433;uid=sa;pwd=3edc9ijn~;database=HUNAN_DYNAMIC_RULE";

std::string ptime_to_format_str(const boost::posix_time::ptime& time, const std::string& format)
{
	boost::posix_time::time_facet* tf = new boost::posix_time::time_facet(format.c_str());
	std::stringstream ss;
	ss.imbue(std::locale(std::cout.getloc(), tf));
	ss << time;
	return ss.str();
}

int main()
{
	OtlConnPool conn_pool;
	conn_pool.open(conn_str, 10, 10);
	cv::Mat lutRND = cv::Mat::zeros(256, 1, CV_8UC3);
	cv::Mat bound = cv::imread("mask.hunan.png", -1);;
	cv::Mat alphaMat = cv::Mat::ones(1160, 1160, CV_8UC1) * 0xaf;
	cv::bitwise_and(alphaMat, bound, alphaMat);
	// B G R  , 这种渲染方式只支持 单通道 或 三通道
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
	for (int i = 85; i < 90; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(209, 10, 121); }
	for (int i = 90; i < 255; ++i) { lutRND.at<cv::Vec3b>(i, 0) = cv::Vec3b(222, 0, 186); }
	lutRND.at<cv::Vec3b>(0, 0) = cv::Vec3b(255, 255, 255);
	// cv::imwrite("luthunan.png", lutRND);
	std::string startTimeStr = "2022-10-28 00:00";
	std::string endTimeStr = "2022-10-30 00:00";
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
			std::string querySqlStr = "select a.x, a.y, b.tswp from(select PID, round((lgtd - 108.65) / 0.05, 0)  x, round((30.25 - lttd) / 0.05, 0) y  from ForeRainPoint_B) a, (select PID, SWP * 100 tswp from TZX_SoilWater_R_WM90 where DataTime = '"+ dbTimeStr +"') b where a.PID = b.PID";
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
		cv::merge(dstChannels, target2);
		std::filesystem::path targetPath("./");
		targetPath.append(dirStr);
		if (!std::filesystem::exists(targetPath))
		{
			std::filesystem::create_directories(targetPath);
		}

		targetPath.append("SW" + fileStr + ".png");
		
		cv::imwrite(targetPath.string().c_str(), target2);
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