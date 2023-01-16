// Contribution by: Abe Tusk https://github.com/abetusk
// To compile:
// gcc -Wall -Weverything -Wno-float-equal src/examples/simple.c -Isrc -o simple
//
// About:
//
// This example outputs 10 random 2D coordinates, and all the generated edges, to standard output.
// Note that the edges have duplicates, but you can easily filter them out.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#define BOOST_ALLOW_DEPRECATED_HEADERS

#include <boost/timer.hpp>
#ifndef NDEBUG
#include <vld.h>
#endif

#define M_PI       3.14159265358979323846
#define JC_VORONOI_IMPLEMENTATION
// If you wish to use doubles
//#define JCV_REAL_TYPE double
//#define JCV_FABS fabs
//#define JCV_ATAN2 atan2
//#define JCV_CEIL ceil
//#define JCV_FLOOR floor
//#define JCV_FLT_MAX 1.7976931348623157E+308
#include "jc_voronoi.h"
#include <cairo/cairo.h>
#include <vector>
#include <map>

#include <polyclipping/clipper.hpp>

#define NPOINT 5000


#define SCREEN_WIDTH  2560
#define SCREEN_HEIGHT  1440


double polygonarea(std::vector<jcv_point>& polygon, int N)
{
	int i, j;
	double area = 0;
	for (i = 0; i < N; i++) {
		j = (i + 1) % N;
		area += polygon[i].x * polygon[j].y;
		area -= polygon[i].y * polygon[j].x;
	}
	area /= 2;
	return(area < 0 ? -area : area);
}

using namespace ClipperLib;
ClipperLib::Paths clip(const Path& p)
{
	ClipperLib::Clipper clipPlatform;
	ClipperLib::Paths toBeClip, clipPolygon, clipResult;
	ClipperLib::Path tt;
	tt.push_back(IntPoint(10e8, 10e8));
	tt.push_back(IntPoint(10e8, 9*10e8));
	tt.push_back(IntPoint(9* 10e8, 9* 10e8));
	tt.push_back(IntPoint(9* 10e8, 10e8));
	tt.push_back(IntPoint(10e8, 10e8));

	clipPlatform.Clear();
	clipPlatform.AddPath(p, ClipperLib::ptSubject, true);	// 被切割的
	clipPlatform.AddPath(tt, ClipperLib::ptClip, true);		// 切割的

	// 第一个参数 设置切割方式为 取相交部分
	clipPlatform.Execute(ClipperLib::ctIntersection, clipResult, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
	return clipResult;

}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;
	while (1)
	{
		int i;
		jcv_rect bounding_box = { { 0.0f, 0.0f }, { SCREEN_WIDTH, SCREEN_HEIGHT } };
		jcv_diagram diagram;
		jcv_point points[NPOINT];
		const jcv_site* sites;
		

		memset(&diagram, 0, sizeof(jcv_diagram));

		srand(0);
		for (i = 0; i < NPOINT; i++) {
			points[i].x = ((float)rand() / (1.0f + (float)RAND_MAX)) * SCREEN_WIDTH;
			points[i].y = ((float)rand() / (1.0f + (float)RAND_MAX)) * SCREEN_HEIGHT;
		}
		boost::timer t;

		jcv_diagram_generate(NPOINT, (const jcv_point*)points, &bounding_box, 0, &diagram);
		sites = jcv_diagram_get_sites(&diagram);
		// double* areas = (double*)malloc(NPOINT * sizeof(double));
		std::map<int, double>  areas;
		double total = 0;
		
		
		// 遍历泰森多边形 并且切割
		for (i = 0; i < diagram.numsites; i++) {

			std::vector<jcv_point> polygon;
			jcv_graphedge* graph_edge  = sites[i].edges;
			jcv_graphedge* graph_edge_last = graph_edge;
			Path pp;
			while (graph_edge) {		// 这里取的时每个边, 连续的两个边之间有一个点是重复的,所以只取一个
				
				polygon.push_back(graph_edge->pos[0]);
				pp.push_back(IntPoint((cInt)graph_edge->pos[0].x*10e6, (cInt)graph_edge->pos[0].y*10e6));
				//pp.push_back(IntPoint(graph_edge->pos[0].x * 10e6, graph_edge->pos[0].y * 10e6));
			
				graph_edge_last = graph_edge;
				graph_edge = graph_edge->next;
			}
			polygon.push_back(graph_edge_last->pos[1]);
			pp.push_back(IntPoint(graph_edge_last->pos[1].x * 10e6, graph_edge_last->pos[1].y * 10e6));
			Paths clipPaths = clip(pp);
			
			double area = polygonarea(polygon, polygon.size());
			areas[sites[i].index] = area;
			total += area;
		}
		
		std::map<int, double> areas_percent;
		for (i = 0; i < NPOINT; ++i)
		{
			areas_percent[i] = areas[i] / total;
		}
	
		std::cout << "now time elapsed:" << t.elapsed() << "s" << std::endl;

		jcv_diagram_free(&diagram);
	}
	
	return 0;
}
