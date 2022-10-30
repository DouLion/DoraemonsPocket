#pragma once
#include <sqlite3.h>
#include "mapbox_tile_sets.hpp"
#include <iostream>
#include <filesystem>
#include <json/json.h>
namespace tzxutils
{
	const std::string SQL_CREATE_TILES = "CREATE TABLE tiles (zoom_level integer, tile_column integer, tile_row integer, tile_data blob);";
	const std::string SQL_CREATE_METADATA = "CREATE TABLE metadata (name text, description text, version, minzoom, maxzoom, center text, bounds text, type text, format text, generator text, generator_options text,json text);";
	typedef std::map<int, std::map<int, std::map<int, std::string>>> LCRData;


	struct MetaInfo
	{
		std::string name;
		std::string description;
		int version;
		int minzoom;
		int maxzoom;
		std::string center;
		std::string bounds;
		std::string type;
		std::string format;
		std::string generator;
		std::string generator_options;
		std::string json;

	};

	struct TaskInfo
	{
		std::string output_name;
		std::vector<std::string> input_names;
		std::map<std::string, LCRData>	tile_data;

		LCRData read_mbtiles(const std::string& mbtile_path)
		{
			LCRData retData;
			sqlite3* pDB = NULL;
			if (SQLITE_OK != sqlite3_open(mbtile_path.c_str(), &pDB))
			{
				std::cout << "Open database fail: " << sqlite3_errmsg(pDB) << std::endl;
				sqlite3_close(pDB);
				return retData;
			}
			// char* cErrMsg;
			//char* prepSql = "select zoom_level, tile_column, tile_row, tile_data  from tiles where zoom_level < 4;";
			const char* prepSql = "select  tile_data  from tiles limit 1;";
			//int res = sqlite3_exec(pDB, "select CAST(length(tile_data) as int), zoom_level, tile_column, tile_row, tile_data  from tiles where zoom_level < 4;", UserResult , 0 , &cErrMsg);  
			sqlite3_stmt* stat;
			const char* pzTail = NULL;

			sqlite3_prepare(pDB, prepSql, -1, &stat, &pzTail);
			int cols = sqlite3_column_count(stat);
			int result = sqlite3_step(stat);
			while (result == SQLITE_ROW) /* sqlite3_step() has another row ready */
			{
				std::string info;
				unsigned char* tmpBlock = (unsigned char*)sqlite3_column_blob(stat, 0);
				int len = sqlite3_column_bytes(stat, 0);
				info.resize(len);
				memcpy(&info[0], tmpBlock, len);
				mvt_tile tile;
				bool isCps = true;
				if (tile.decode(info, isCps)) {
					int bbb = 0;
				}
				for (int k = 0; k < tile.layers.size(); ++k)
				{
					std::cout << tile.layers[k].name << std::endl;
					/*for (std::string key : tile.layers[k].keys) {
						std::cout << key << std::endl;
					}*/
				}
				result = sqlite3_step(stat);
			}
			sqlite3_close(pDB);
			return retData;
		}

		bool read_data()
		{
			return false;

		}

		bool merge_data()
		{
			return false;
		}
	};

	/// <summary>
	/// 解析单个任务 的输入文件 和 输出文件
	/// </summary>
	/// <param name="inputs"></param>
	/// <param name="output"></param>
	/// <returns></returns>
	int phrase_param_file_task(std::vector<std::string>& inputs, std::string& output)
	{

		return 1;
	}


	
}
int main()
{
	std::filesystem::path mbtpath1("../../../data/line_10_50river_s5000.mbtiles");
	std::filesystem::path mbtpath2("../../../data/line_10_50river_s5000.mbtiles");
	std::map<int, std::map<int, std::map<int, std::string>>> layer_col_row_data;
	return 0;
}