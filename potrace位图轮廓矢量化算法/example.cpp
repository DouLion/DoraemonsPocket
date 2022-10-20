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
	// potrace_bitmap_s ��ʽ��Ҫ˵��
	/*
	    0 1 1 1 1 0 1 0  |  0 1 1
		0 0 1 1 0 0 1 1  |  1 1 0
		0 0 1 1 0 0 1 1	 |  1 1 0 
		0 0 1 1 0 0 1 1	 |  1 1 0
		0 0 1 1 0 0 1 1  |  1 0 0
��һ��	0 0 1 1 0 0 1 1  |  0 0 0
	   | -----��һ��---  | --- �ڶ��� ---- | ....

	   �� 11 ��  6

	�� �µ���  �� ���� 
	potrace_word *map 

	map[0]  =  0x33   ��һ�е�һ��  ������ֵ��Ӧ����
	map[1]  = 0x00	  ��һ�еڶ���  ������ֵ��Ӧ����  �ճ����Ĳ�����Ϊ��0 
	map[2] = 0x33     �ڶ��е�һ��
	map[3] = 0x80     �ڶ��еڶ���
	.
	.
	.
	�Դ�����,��ֵ��ͼ��ĺڰ׶���1λ��ʾ
	*/


	bm_clear(bitmap, 0);
	for (int r = pngMat.rows; r > 0; --r)
	{
		for (int c = 0; c * 8 < pngMat.cols; ++c)
		{
			unsigned int byd = 0;
			for (int kk = 0; kk < 8; ++kk)
			{
				byd <<= 1;	// ����һλ, ���һλ����
				if ((c * 8 + kk) < pngMat.cols)
				{
					if (pngMat.ptr<unsigned int>(r - 1, c * 8 + kk)[0] > 0)
					{
						byd |= 1; // ���һλ�� 1
					}
				}
			}
			byd ^= 255; // ��֪��Ϊɶ, Ӧ�ø������㷨��ʵ���й�ϵ, ������1��ʾ ������0��ʾ
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
