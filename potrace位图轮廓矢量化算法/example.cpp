#pragma once 

#include "opencv2/opencv.hpp"
#include "backend_geojson.h"
#include "bitmap.h"
#include <filesystem>

int main()
{
	std::filesystem::path pngPath("../../test.png");
	std::cout << std::filesystem::absolute(pngPath).string() << std::endl;
	cv::Mat pngMat = cv::imread(pngPath.string());
	potrace_bitmap_s* bitmap = bm_new(pngMat.cols, pngMat.rows);
	// potrace_bitmap_s 格式简要说明
	/*
	    0 1 1 1 1 0 1 0  |  0 1 1
		0 0 1 1 0 0 1 1  |  1 1 0
		0 0 1 1 0 0 1 1	 |  1 1 0 
		0 0 1 1 0 0 1 1	 |  1 1 0
		0 0 1 1 0 0 1 1  |  1 0 0
第一行	0 0 1 1 0 0 1 1  |  0 0 0
	   | -----第一列---  | --- 第二列 ---- | ....

	   宽 11 高  6

	从 下到上  从 左到右 
	potrace_word *map 

	map[0]  =  0x33   第一行第一列  二进制值对应的数
	map[1]  = 0x00	  第一行第二列  二进制值对应的数  空出来的部分认为是0 
	map[2] = 0x33     第二行第一列
	map[3] = 0x80     第二行第二列
	.
	.
	.
	以此类推,二值化图像的黑白都用1位表示
	*/


	bm_clear(bitmap, 0);
	for (int r = pngMat.rows; r > 0; --r)
	{
		for (int c = 0; c * 8 < pngMat.cols; ++c)
		{
			unsigned int byd = 0;
			for (int kk = 0; kk < 8; ++kk)
			{
				byd <<= 1;	// 左移一位, 最后一位置零
				if ((c * 8 + kk) < pngMat.cols)
				{
					if (pngMat.ptr<unsigned int>(r - 1, c * 8 + kk)[0] > 0)
					{
						byd |= 1; // 最后一位置 1
					}
				}
			}
			byd ^= 255; // 不知道为啥, 应该跟后面算法的实现有关系, 面内用1表示 面外用0表示
			*bm_index(bitmap, c * 8, pngMat.rows - r) |= ((potrace_word)byd) << (8 * (BM_WORDSIZE - 1 - (c % BM_WORDSIZE)));;
		}
		std::cout << std::endl;
	}
	potrace_param_t* param_ = potrace_param_default();
	potrace_state_s* states = potrace_trace(param_, bitmap);
	PoTracePathTransToGeoJson ptgeo(states->plist, pngMat.cols, pngMat.rows);

	Json::Value  geojsonValue = ptgeo.ToJsonFormat();
	std::string gstr = Json::FastWriter().write(geojsonValue);
	FILE* pf = fopen("../../test.geojson", "wb");
	if (pf)
	{
		fwrite(gstr.c_str(), 1, gstr.length(), pf);
		fclose(pf);
	}
	return 0;
}
