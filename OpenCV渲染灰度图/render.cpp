#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

int main()
{

	cv::Mat lutRND = cv::Mat::zeros(256, 1, CV_8UC3);
	// B G R  , 这种渲染方式只支持 单通道 或 三通道
	lutRND.at<cv::Vec3b>(13, 0) = cv::Vec3b(0xaa, 0x5c, 0x2a);
	lutRND.at<cv::Vec3b>(14, 0) = cv::Vec3b(0x00, 0xe0, 0xff);
	lutRND.at<cv::Vec3b>(15, 0) = cv::Vec3b(0x21, 0x6c, 0xf3);
	lutRND.at<cv::Vec3b>(16, 0) = cv::Vec3b(0x36, 0x41, 0xef);
	cv::Mat src = cv::imread("src.png", -1);
	cv::Mat dst;
	cv::applyColorMap(src, dst, lutRND);
	cv::imshow("Render Image", dst);
	cv::namedWindow("Render Image");
	cv::waitKey(0);
	cv::imwrite("convert.png", dst);
	return 0;

}