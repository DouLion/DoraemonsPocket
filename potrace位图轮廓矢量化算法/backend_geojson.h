/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#ifndef BACKEND_GEO_H
#define BACKEND_GEO_H

#include "potracelib.h"
#include "trans.h"
#include "json/json.h"

typedef std::vector<std::vector<dpoint_t>> dpolygon_t;

struct imginfo_s {
	int pixwidth;        /* width of input pixmap */
	int pixheight;       /* height of input pixmap */
	double width;        /* desired width of image (in pt or pixels) */
	double height;       /* desired height of image (in pt or pixels) */
	double lmar, rmar, tmar, bmar;   /* requested margins (in pt) */
	trans_t trans;        /* specify relative position of a tilted rectangle */
};

class PoTracePathTransToGeoJson
{
public:
	PoTracePathTransToGeoJson(potrace_path_t* plist, int width, int height)
	{
		m_pList = plist;
		m_pWidth = width;
		m_pHeight = m_pHeight;
		m_pImginfo = nullptr;
		CreateDefaultImageInfo();
	}

	Json::Value ToJsonFormat();
	std::vector<dpolygon_t> ToPolygonVec();
private:

	
	void CreateDefaultImageInfo();
	dpoint_t geojson_moveto(dpoint_t p);

	dpoint_t  geojson_lineto(dpoint_t p);
	std::vector<dpoint_t> geojson_curveto(dpoint_t p1, dpoint_t p2, dpoint_t p3);
	Json::Value GeojsonJsonPath(potrace_curve_t* curve);
	std::vector<dpoint_t> GeojsonPolyPath(potrace_curve_t* curve);
	void WritePolyPolygon(potrace_path_t* tree, int first);
	void WriteJsonPolygons(potrace_path_t* tree, int first);

private:
	std::vector<dpolygon_t> m_pAllPoygon;
	dpoint_t m_pCur;
	potrace_path_t* m_pList;
	imginfo_s* m_pImginfo;
	int m_pWidth;
	int m_pHeight;
	trans_t m_pTrans;
	Json::Value	m_pAllFeatures;

};

#endif /* BACKEND_GEO_H */

