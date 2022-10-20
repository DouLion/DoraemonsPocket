#pragma once

#include <iostream>
#include <vector>

#include <math.h>
// GDAL.
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "gdal_alg.h"
#include "ogr_api.h"

// cairo
#include <cairo/cairo.h>

const char* mask_png = "mask.png";
const char* mask_bin_png = "mask.bin.png";

std::string filePath;
double _param_width = 0, _param_height = 0;
double _param_left = 0, _param_top = 0, _param_bottom = 0, _param_right = 0;

double _x_geo_step = 0, _y_geo_step = 0;

// 过滤目标字段的名称 与 值, 仅支持字符串类型的字段
std::string _param_target_segment;
std::string _param_target_value;


/// <summary>
/// 此结构以左上角为原点
/// </summary>
struct ScreenPoint
{
	ScreenPoint(void) :x(0), y(0) {}
	ScreenPoint(double _x, double _y) :x(_x), y(_y){}
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

std::vector<ScreenPaths> ReadGeomertry(const std::string& path);
void _cairo_draw_paths(cairo_t* cr, ScreenPaths& paths);
void to_bin_image();
int main(void)
{
	filePath = "geoj_province.geojson";
	_param_target_segment = "省";
	_param_target_value = "河北省";
	// 119.9	111.9	42.8	34.8
	_param_left = 111.9;
	_param_right = 119.9;
	_param_top = 42.8;
	_param_bottom = 34.8;
	
	_param_width = 3220;
	_param_height = 3220;
	_x_geo_step = (_param_right - _param_left) / _param_width;
	_y_geo_step = (_param_top - _param_bottom) / _param_height;
	cairo_surface_t* surface;
	cairo_t* cr;

	surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, _param_width, _param_height);
	cr = cairo_create(surface);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	auto parts = ReadGeomertry(filePath);
	std::cout << _param_target_value << std::endl;
	for (auto paths : parts)
	{
		_cairo_draw_paths(cr, paths);
	}
	cairo_surface_write_to_png(surface, mask_png);

	cairo_destroy(cr);

	cairo_surface_destroy(surface);
	to_bin_image();
	return 0;
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
		OuterPath.path.push_back(ScreenPoint((point.getX()-_param_left)/_x_geo_step, (_param_top - point.getY())/_y_geo_step));
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
			// InnerPath.path.push_back(ScreenPoint((point.getX() - _param_left) / _x_geo_step, (point.getY() - _param_bottom) / _y_geo_step));
			InnerPath.path.push_back(ScreenPoint((point.getX() - _param_left) / _x_geo_step, (_param_top - point.getY()) / _y_geo_step));
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
	CPLSetConfigOption("SHAPE_ENCODING", "");	// 编码支持,自动识别, 目前好像使用.shp文件无效, 所以先转换为GeoJSON
	GDALDataset* poDSr = (GDALDataset*)GDALOpenEx(path.c_str(), GDAL_OF_ALL | GDAL_OF_READONLY, NULL, NULL, NULL);
	OGRLayer* poLayer_r = poDSr->GetLayer(0);
	int iEntities = poLayer_r->GetFeatureCount();
	OGRFeature* pFeature_r = NULL;
	std::vector<ScreenPaths>  polygons;
	while ((pFeature_r = poLayer_r->GetNextFeature()) != NULL)
	{
		OGRGeometry* poGeometry_r = pFeature_r->GetGeometryRef();
		int fieldCnt = pFeature_r->GetFieldCount();
		bool isMatch = false;
		for (int fc = 0; fc < fieldCnt; ++fc)
		{
			OGRFieldDefn* def = pFeature_r->GetFieldDefnRef(fc);

			if (def->GetType() == OFTString)
			{
				std::string segName = def->GetNameRef();
				if (segName == _param_target_segment)
				{
					std::string identify = pFeature_r->GetFieldAsString(def->GetNameRef());
					
					if (identify == _param_target_value)
					{
						std::cout << identify << std::endl;
						isMatch = true;
					}
				}
			}
		}
		if (!isMatch)
		{
			OGRFeature::DestroyFeature(pFeature_r);
			continue;
		}
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
	cairo_set_line_width(cr, 6);

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
		cairo_move_to(cr, path.path.at(9).x, path.path.at(9).y);
		for (auto point : path.path)
		{
			cairo_line_to(cr, point.x, point.y);
		}
		// 闭合
		cairo_close_path(cr);

	}

	cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_set_source_rgb(cr, 0, 0.7, 0); cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, 0, 0, 0); cairo_stroke(cr);
	//cairo_restore(cr);
	return;
}
#include <opencv2/opencv.hpp>
// 将彩色图转为灰度图
void to_bin_image()
{
	cv::Mat rbgMat = cv::imread(mask_png, -1);

	cv::Mat binMat;
	cv::cvtColor(rbgMat, binMat, cv::COLOR_RGB2GRAY);
	cv::imwrite(mask_bin_png,binMat);
}