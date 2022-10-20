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
	bm_clear(bitmap, 0);
	for (int r = pngMat.rows; r > 0; --r)
	{
		for (int c = 0; c * 8 < pngMat.cols; ++c)
		{
			unsigned int byd = 0;
			for (int kk = 0; kk < 8; ++kk)
			{
				byd <<= 1;
				if ((c * 8 + kk) < pngMat.cols)
				{
					if (pngMat.ptr<unsigned int>(r - 1, c * 8 + kk)[0] > 0)
					{
						byd |= 1;
					}
				}
			}
			byd ^= 255;
			std::cout << byd << " ";

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
