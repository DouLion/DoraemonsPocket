#include "backend_geojson.h"


static Json::Value PointToJson(dpoint_t point)
{
	Json::Value ot = Json::arrayValue;
	ot.append(point.x);
	ot.append(point.y);
	return ot;
}

static double bezier(double t, double x0, double x1, double x2, double x3) {
	double s = 1 - t;
	return s * s * s * x0 + 3 * (s * s * t) * x1 + 3 * (t * t * s) * x2 + t * t * t * x3;
}

Json::Value PoTracePathTransToGeoJson::ToJsonFormat()
{
	Json::Value ot = Json::objectValue;
	write_polygons(m_pList, 0);
	ot["type"] = "FeatureCollection";
	ot["features"] = m_pAllFeatures;

	return ot;
}

dpoint_t PoTracePathTransToGeoJson::geojson_moveto(dpoint_t p)
{
	dpoint_t q;
	q = trans(p, m_pTrans);
	m_pCur = q;
	return q;
}

dpoint_t PoTracePathTransToGeoJson::geojson_lineto(dpoint_t p)
{
	dpoint_t q;
	q = trans(p, m_pTrans);
	m_pCur = q;
	return q;
}

std::vector<dpoint_t> PoTracePathTransToGeoJson::geojson_curveto(dpoint_t p1, dpoint_t p2, dpoint_t p3)
{
	std::vector<dpoint_t> retVec;
	dpoint_t q1;
	dpoint_t q2;
	dpoint_t q3;
	double step, t;
	int i;
	// double x, y;

	q1 = trans(p1, m_pTrans);
	q2 = trans(p2, m_pTrans);
	q3 = trans(p3, m_pTrans);

	step = 1.0 / 8.0;  // 这个参数可能跟平滑度有关, 贝塞尔插值数量

	for (i = 0, t = step; i < 8; i++, t += step) {
		dpoint_t tpoint;
		tpoint.x = bezier(t, m_pCur.x, q1.x, q2.x, q3.x);
		tpoint.y = bezier(t, m_pCur.y, q1.y, q2.y, q3.y);
		retVec.push_back(tpoint);
	}

	m_pCur = q3;
	return retVec;
}

Json::Value PoTracePathTransToGeoJson::geojson_path(potrace_curve_t* curve)
{

	Json::Value ringRoot = Json::arrayValue;
	int i;
	dpoint_t* c;
	int m = curve->n;


	c = curve->c[m - 1];
	dpoint_t firstP = geojson_moveto(c[2]);
	ringRoot.append(PointToJson(firstP));
	for (i = 0; i < m; i++) {
		c = curve->c[i];
		switch (curve->tag[i]) {
		case POTRACE_CORNER:
			dpoint_t p1 = geojson_lineto(c[1]);
			dpoint_t p2 = geojson_lineto(c[2]);
			ringRoot.append(PointToJson(p1));
			ringRoot.append(PointToJson(p2));
			break;
		case POTRACE_CURVETO:
			std::vector<dpoint_t> dV = geojson_curveto(c[0], c[1], c[2]);
			for (auto dp : dV)
			{
				ringRoot.append(PointToJson(dp));
			}
			break;
		}
	}
	return ringRoot;
}

void PoTracePathTransToGeoJson::write_polygons(potrace_path_t* tree, int first)
{
	potrace_path_t* p, * q;

	for (p = tree; p; p = p->sibling) {
		Json::Value featureRoot = Json::objectValue;
		featureRoot["type"] = "Feature";
		featureRoot["geometry"] = Json::objectValue;
		featureRoot["geometry"]["type"] = "Polygon";
		featureRoot["geometry"]["coordinates"] = Json::arrayValue;


		featureRoot["geometry"]["coordinates"].append(geojson_path(&p->curve));

		for (q = p->childlist; q; q = q->sibling) {
			featureRoot["geometry"]["coordinates"].append(geojson_path(&q->curve));
		}

		m_pAllFeatures.append(featureRoot);

		for (q = p->childlist; q; q = q->sibling) {
			write_polygons(q->childlist, 0);
		}

		first = 0;
	}
}

void PoTracePathTransToGeoJson::CreateDefaultImageInfo()
{
	// double dim_def;
	// double maxwidth, maxheight;// , sc;
	int default_scaling = 0;
	if (nullptr == m_pImginfo)
	{
		m_pImginfo = new imginfo_s;
	}

	m_pImginfo->pixwidth = m_pWidth;

	m_pImginfo->pixheight = m_pHeight;

	m_pImginfo->width = m_pWidth;
	m_pImginfo->height = m_pHeight;
	m_pImginfo->lmar = 0;
	m_pImginfo->rmar = 0;
	m_pImginfo->tmar = 0;
	m_pImginfo->bmar = 0;

	trans_from_rect(&m_pImginfo->trans, m_pImginfo->pixwidth, m_pImginfo->pixheight);



	m_pTrans.bb[0] = m_pImginfo->trans.bb[0] + m_pImginfo->lmar + m_pImginfo->rmar;
	m_pTrans.bb[1] = m_pImginfo->trans.bb[1] + m_pImginfo->tmar + m_pImginfo->bmar;
	m_pTrans.orig[0] = m_pImginfo->trans.orig[0] + m_pImginfo->lmar;
	m_pTrans.orig[1] = m_pImginfo->trans.orig[1] + m_pImginfo->bmar;
	m_pTrans.x[0] = m_pImginfo->trans.x[0];
	m_pTrans.x[1] = m_pImginfo->trans.x[1];
	m_pTrans.y[0] = m_pImginfo->trans.y[0];
	m_pTrans.y[1] = m_pImginfo->trans.y[1];


}
