#pragma once
// 需要定义 _CRT_SECURE_NO_WARNINGS  _WINDOWS  IS_WIN32 三个宏
// 只支持64位系统
// 在项目属性 c++ -> 常规 -> 附加包含目录添加 include文件夹
// 在项目属性 连接器-> 输入 -> 附加依赖项 添加 对应的Debug/Release  TzxUtils.lib
#define _CRT_SECURE_NO_WARNINGS
#ifndef _WINDOWS
#define _WINDOWS
#endif
#ifndef IS_WIN32
#define IS_WIN32
#endif
//#include <string.h>
#include "Database/TFF_DbFactory.h"
#include "File/TFF_FileUtils.h"

// GDAL.
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "gdal_alg.h"
#include "ogr_api.h"
struct VertexInfo
{
	int size;
	TFFBYTE* data;
};
void  ReadGeomertry(const std::string& path, const std::string& idName, std::map<std::string, VertexInfo>& id2vertex);
otl_connect db;
int main()
{
	std::map<std::string, VertexInfo> id2vertex;
	ReadGeomertry("Gg2.geojson", "ENTITYID", id2vertex);
	int count = 0;
	try
	{
		otl_connect::otl_initialize();

		// 数据库 通过odbc连接  Driver后面是对应的ODBC 驱动名称, 查看系统64位odbc
		std::string mssqlStr = "driver=sql server;server=192.168.0.150;uid=sa;pwd=3edc9ijn~;database=RewServiceDB_HB;";
		db.rlogon(mssqlStr.c_str());
		db.set_max_long_size(1500000);
		otl_stream otlStream; // defined an otl_stream variable
		//otlStream.set_lob_stream_mode(true); // set the "lob stream mode" flag
		std::string insertSqlStr = "INSERT into DA values (:f1<char[100]>, :f2<raw_long>);";
		otlStream.open(1, insertSqlStr.c_str(),db);
		
		otlStream.set_lob_stream_mode(true);
		otlStream.setBufSize(1500000);
		for (auto [_id_, _vertexInfo_] : id2vertex)
		{

			/*if (!("hebeix_W040190" == _id_ || "hebeix_W040630" == _id_))
			{
				continue;
			}*/
			/*if (("hebeix_W040190" == _id_ || "hebeix_W040630" == _id_))
			{
				continue;
			}*/
			
			std::cout << _id_ << std::endl;
			otlStream << _id_.c_str();

			otl_long_string _ols((const void*)_vertexInfo_.data, 1499999, _vertexInfo_.size);
			/*otl_lob_stream lob;
			lob.set_len(_vertexInfo_.size);
			lob << (char*)_vertexInfo_.data;
			lob.close();;
			otlStream << lob;*/
			otlStream << _ols;
			otlStream.flush();
			count++;
			
		}
		db.commit();
		for (auto [_id_, _vertexInfo_] : id2vertex)
		{
			TFF_SAFE_DELETE(_vertexInfo_.data);
		}
	}
	catch (otl_exception& p) { // intercept OTL exceptions
		cerr << p.msg << endl; // print out error message
		cerr << p.sqlstate << endl; // print out SQLSTATE 
		cerr << p.stm_text << endl; // print out SQL that caused the error
		cerr << p.var_info << endl; // print out the variable that caused the error
	}
	db.logoff();
	
	return 0;
}

void ReadGeomertry(const std::string& path, const std::string& idName, std::map<std::string, VertexInfo>& id2vertex)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");  
	OGRRegisterAll();
	CPLSetConfigOption("SHAPE_ENCODING", "");
	GDALDataset* poDSr = (GDALDataset*)GDALOpenEx(path.c_str(), GDAL_OF_ALL | GDAL_OF_READONLY, NULL, NULL, NULL);
	OGRLayer* poLayer_r = poDSr->GetLayer(0);
	int iEntities = poLayer_r->GetFeatureCount();
	OGRFeature* pFeature_r = NULL;
	int maxXXX = 0;
	while ((pFeature_r = poLayer_r->GetNextFeature()) != NULL)
	{
		OGRGeometry* poGeometry_r = pFeature_r->GetGeometryRef();
		int fieldCnt = pFeature_r->GetFieldCount();
		std::string identify = "";
		for (int fc = 0; fc < fieldCnt; ++fc)
		{
			OGRFieldDefn* def = pFeature_r->GetFieldDefnRef(fc);

			if (def->GetType() == OFTString)
			{
				std::string segName = def->GetNameRef();
				if (segName == idName)
				{
					identify = pFeature_r->GetFieldAsString(def->GetNameRef());
					std::cout << def->GetNameRef() << ": ";
					std::cout << pFeature_r->GetFieldAsString(def->GetNameRef()) << std::endl;
					break;
				}
			}
		}
		if (identify.empty())
		{
			continue;
		}
		VertexInfo info;
		OGRPoint point;
		OGRPolygon* poPolygon = nullptr;
		if (poGeometry_r != NULL && wkbFlatten(poGeometry_r->getGeometryType()) == wkbPolygon)
		{
			poPolygon = (OGRPolygon*)poGeometry_r;
		}
		else if (poGeometry_r != NULL && wkbFlatten(poGeometry_r->getGeometryType()) == wkbMultiPolygon)
		{
		
			OGRMultiPolygon* pMultiPolygon = (OGRMultiPolygon*)poGeometry_r;
			//部位数
			int GeometryNum = pMultiPolygon->getNumGeometries();
			if (GeometryNum > 1)
			{
				std::cerr << "不持支多部分组成的面" << std::endl;
				continue;
			}
			for (int k = 0; k < GeometryNum; k++)
			{
				poPolygon = (OGRPolygon*)(pMultiPolygon->getGeometryRef(k));
				break;
			}
		}

		OGRLinearRing* pOGRLinearRing = poPolygon->getExteriorRing();
		int InnerRingCout = poPolygon->getNumInteriorRings();
		int RingCout = InnerRingCout + 1;// 内环个数 + 外环
		TFFInt32 OutPointNum = pOGRLinearRing->getNumPoints();//外环点数
		int totalPointNum = OutPointNum;
		for (int i = 0; i < InnerRingCout; ++i)
		{
			OGRLinearRing* pInterRing = poPolygon->getInteriorRing(i);
			totalPointNum += pInterRing->getNumPoints();
		}

		info.size = (1 + RingCout + totalPointNum * 2) * sizeof(double);
		// std::cout << info.size << std::endl;
		maxXXX = TFF_MAX(info.size, maxXXX);
		double* data = new double[1 + RingCout + totalPointNum * 2];

		data[0] = RingCout;
		data[1] = OutPointNum;
		int aa = 2;

		for (int i = 0; i < OutPointNum; i++)//外环
		{
			pOGRLinearRing->getPoint(i, &point);
			data[aa] = point.getX();
			data[aa + 1] = point.getY();
			aa += 2;
		}

		for (int j = 0; j < InnerRingCout; ++j)
		{
			OGRLinearRing* tmpInterRing = poPolygon->getInteriorRing(j);
			int InnerPointNum = tmpInterRing->getNumPoints();
			data[aa] = (double)InnerPointNum;
			++aa;
			for (int i = 0; i < InnerPointNum; i++)//外环
			{
				tmpInterRing->getPoint(i, &point);
				data[aa] = point.getX();
				data[aa + 1] = point.getY();
				aa += 2;
			}
		}
		info.data = (TFFBYTE*)data;
		id2vertex.insert({identify,info});
	}
	std::cout << maxXXX << std::endl;
	OGRCleanupAll();
}

void bbb()
{
otl_long_string f2(6000); // define long string variable
	otl_stream o; // defined an otl_stream variable
	o.set_lob_stream_mode(true); // set the "lob stream mode" flag
	o.open(1, // buffer size has to be set to 1 for operations with LOBs
		"insert into test_tab values(:f1<int>,:f2<varchar_long>, "
		":f3<varchar_long>) ",
		// SQL statement
		db // connect object
	);
	o.set_commit(0); // setting stream "auto-commit" to "off". It is required
					 // when LOB stream mode is used.
	int i, j;
	otl_lob_stream lob; // LOB stream for reading/writing unlimited number
						// of bytes regardless of the buffer size.
	otl_lob_stream lob2; // LOB stream for reading/writing unlimited number
						// of bytes regardless of the buffer size.

	for (i = 1; i <= 20; ++i) {
		o << i;
		o << lob; // Initialize otl_lob_stream by writing it
				// into otl_stream.
		o << lob2; // Initialize otl_lob_stream by writing it
				// into otl_stream.

		for (j = 0; j < 5000; ++j)
			f2[j] = '*';
		f2[5000] = '?';
		f2.set_len(5001);

		lob.set_len(5001 + 2123); // setting the total  size of
								// the TEXT to be written.

		lob << f2; // writing first chunk of the TEXT into lob


		f2[2122] = '?';
		f2.set_len(2123); // setting the size of the second chunk

		lob << f2; // writing the second chunk of the TEXT into lob
		lob.close(); // closing the otl_lob_stream

		for (j = 0; j < 5000; ++j)
			f2[j] = '*';
		f2[5000] = '?';
		f2.set_len(5001);
		lob2.set_len(5001 + 2123); // setting the total  size of
								// the TEXT to be written.

		lob2 << f2; // writing first chunk of the TEXT into lob

		f2[2122] = '?';
		f2.set_len(2123); // setting the size of the second chunk

		lob2 << f2; // writing the second chunk of the TEXT into lob
		lob2.close(); // closing the otl_lob_stream

	}

	db.commit(); // committing transaction.

}
