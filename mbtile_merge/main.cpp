/*
 * @copyright: Beijing tianzhixiang Information Technology Co., Ltd. All rights reserved.
 * @website: http://www.tianzhixiang.com.cn/
 * @author: dou li yang
 * @date: 2022/10/29 9:48
 * @version: 1.0.1
 * @description:
 */

#pragma once
#include <sqlite3.h>
#include "mapbox_tile_sets.hpp"
#include <iostream>
#include <filesystem>
#include <json/json.h>
#include <fstream>
#include <thread>
#include <chrono>
#define DETECT_LEAK 0
#if DETECT_LEAK		// 检查没有内存泄漏
#include <vld.h>
#endif


#if SINGLE_THREAD_MODE

#endif
namespace tzxutils	// 防止有些函数名 或者 .. 重复
{
	// 逗号 字符
	const char COMMA_CHAR = ',';
	// 建表语句
	const char* SQL_CREATE_TILES = "CREATE TABLE tiles (zoom_level integer, tile_column integer, tile_row integer, tile_data blob);";
	const char* SQL_CREATE_METADATA = "CREATE TABLE metadata (name text, value text);";
	// 查询语句
	const char* SQL_QUERY_TILES = "select zoom_level, tile_column, tile_row, tile_data  from tiles;";
	const char* SQL_QUERY_METADATA = "select name, value from metadata;";
	// 插入语句
	const char* SQL_INSERT_TILES = "insert into tiles (zoom_level, tile_column, tile_row, tile_data) values (?,?,?,?);";
	const char* SQL_INSERT_METADAT = "insert into metadata (name, value) values (?,?);";


	namespace mtv_tile_string  // 压缩后的矢量字符串, 与结构化的矢量 区分开
	{
		typedef std::map<int, std::map<int, std::map<int, std::string>>> LCRData;
		typedef std::map<int, std::map<int, std::string>> LevelData;
	}

	// 切分字符串
	static std::vector<std::string> string_split(const std::string& strIn, const char delim = COMMA_CHAR)
	{
		char* str = const_cast<char*>(strIn.c_str());
		std::string s;
		s.append(1, delim);
		std::vector<std::string> elems;
		char* splitted = strtok(str, s.c_str());
		while (splitted != NULL)
		{
			elems.push_back(std::string(splitted));
			splitted = strtok(NULL, s.c_str());
		}
		return elems;
	}

	// 读取blob 数据
	static std::string read_blob_data(sqlite3_stmt* stat, int index)
	{
		std::string ret_data;
		unsigned char* tmpBlock = (unsigned char*)sqlite3_column_blob(stat, index);
		int len = sqlite3_column_bytes(stat, index);

		ret_data.resize(len);
		memcpy(&ret_data[0], tmpBlock, len);
		return ret_data;
	}

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

		std::map<std::string, std::string> to_map()
		{
			std::map<std::string, std::string> retData;
			retData.insert({ "name", name });
			retData.insert({ "description", description });
			retData.insert({ "version", std::to_string(version) });
			retData.insert({ "minzoom", std::to_string(minzoom) });
			retData.insert({ "maxzoom", std::to_string(maxzoom) });
			retData.insert({ "center", center });
			retData.insert({ "bounds", bounds });
			retData.insert({ "type", type });
			retData.insert({ "format", format });
			retData.insert({ "generator", generator });
			retData.insert({ "generator_options", generator_options });
			retData.insert({ "json", json });

			return retData;
		}

	};

	struct TaskInfo
	{
		TaskInfo()
		{
			output_name = "";
			output_name = {};
			tile_data = {};
			meta_data = {};
			_thread_count = 0;
			max_thread = 16;
		}
		std::string output_name;
		std::vector<std::string> input_names;
		std::map<std::string, mtv_tile_string::LCRData>	tile_data;
		std::map<std::string, MetaInfo> meta_data;
		int max_thread;

		mtv_tile_string::LCRData read_mbtiles(const std::string& mbtile_path, MetaInfo& info)
		{
			mtv_tile_string::LCRData retData;
			sqlite3* pDB = NULL;
			if (SQLITE_OK != sqlite3_open(mbtile_path.c_str(), &pDB))
			{
				std::cout << "Open database fail: " << sqlite3_errmsg(pDB) << std::endl;
				sqlite3_close(pDB);
				return retData;
			}

			sqlite3_stmt* tile_stat;		// 确认这个是否需要释放
			sqlite3_stmt* meta_info_stat;
			const char* pzTail = NULL;

			// 读取 metadata 数据
			int state = sqlite3_prepare(pDB, SQL_QUERY_METADATA, -1, &meta_info_stat, &pzTail);
			int cols = sqlite3_column_count(meta_info_stat);
			int result = sqlite3_step(meta_info_stat);
			while (result == SQLITE_ROW)
			{
				std::string name = read_blob_data(meta_info_stat, 0);
				std::string value = read_blob_data(meta_info_stat, 1);
				if (name == "name") { info.name = value; }
				else if (name == "description") { info.description = value; }
				else if (name == "version") { info.version = stoi(value); }
				else if (name == "minzoom") { info.minzoom = stoi(value); }
				else if (name == "maxzoom") { info.maxzoom = stoi(value); }
				else if (name == "center") { info.center = value; }
				else if (name == "bounds") { info.bounds = value; }
				else if (name == "type") { info.type = value; }
				else if (name == "format") { info.format = value; }
				else if (name == "generator") { info.generator = value; }
				else if (name == "generator_options") { info.generator_options = value; }
				else if (name == "json") { info.json = value; }
				result = sqlite3_step(meta_info_stat);
			}

			// 先全读完, 看是不是索引序号错误
				// 109.293965,24.954316,113.892880,29.955873
			std::vector<std::string> bound_str_vec = string_split(info.bounds);
			if (bound_str_vec.size() != 4)
			{
				std::cerr << mbtile_path << " 边界信息错误, " << info.bounds << std::endl;
				sqlite3_close(pDB);
				retData.clear();
				return retData;
			}
			float _left = stof(bound_str_vec[0]);
			float _bottom = stof(bound_str_vec[1]);
			float _right = stof(bound_str_vec[2]);
			float _top = stof(bound_str_vec[3]);

			_max_lgtd = std::max(_right, _max_lgtd);
			_min_lgtd = std::min(_left, _min_lgtd);
			_max_lttd = std::max(_top, _max_lttd);
			_min_lttd = std::min(_bottom, _min_lttd);

			// 读取 tiles 数据
			sqlite3_prepare(pDB, SQL_QUERY_TILES, -1, &tile_stat, &pzTail);
			cols = sqlite3_column_count(tile_stat);
			result = sqlite3_step(tile_stat);
			while (result == SQLITE_ROW)
			{
				int levle = sqlite3_column_int(tile_stat, 0);
				int col = sqlite3_column_int(tile_stat, 1);
				int row = sqlite3_column_int(tile_stat, 2);
				_max_level = std::max(levle, _max_level);	//  最大最小层级
				_min_level = std::min(levle, _min_level);
				if (l_c_r_record[levle][col].empty())
				{
					l_c_r_record[levle][col] = {};
				}
				l_c_r_record[levle][col][row] = 1;
				std::string info = read_blob_data(tile_stat, 3);
				if (!info.empty())	// 一般不会有这个情况
				{
					retData[levle][col][row] = info;
				}
				else
				{
					std::cerr << levle << "-" << col << "-" << row << std::endl;
				}
				result = sqlite3_step(tile_stat);
			}

			sqlite3_close(pDB);
			return retData;
		}

		bool read_data()
		{
			for (auto tile_name : input_names)
			{
				// 文件 存在
				std::filesystem::path _tile_path(tile_name);
				if (!std::filesystem::exists(_tile_path))
				{
					std::cout << "文件: " << std::filesystem::absolute(_tile_path) << "不存在" << std::endl;
					continue;
				}
				std::string _tile_short_name = _tile_path.string(); // 只用文件名,调试方便看

				MetaInfo _meta_info;
				auto _tile_data = read_mbtiles(tile_name, _meta_info);
				tile_data[_tile_short_name] = _tile_data;
				meta_data[_tile_short_name] = _meta_info;

			}
			return true;

		}

		bool merge_tile_data()
		{
			mtv_tile_string::LCRData target_medata_data;

			// 考虑 使用多线程 提高效率
			for (auto [l, c_r] : l_c_r_record)
			{
				for (auto [c, rs] : c_r)
				{
					for (auto [r, oc]: rs)
					{
						std::thread _merge_thread(&TaskInfo::merge_tile_thread, this, l, c, r, std::ref(target_medata_data));
						_thread_count++;
						while (_thread_count > max_thread)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(2));
						}
						_merge_thread.detach();
					}
					
				}
			}

			while (_thread_count > 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			MetaInfo merge_meta_info = merge_meta_data();
			return insert_tile_data(target_medata_data, merge_meta_info);
		}

		MetaInfo merge_meta_data()
		{
			MetaInfo merge_meta_info;
			std::filesystem::path merge_path(output_name);
			if (!std::filesystem::exists(merge_path.parent_path()))
			{
				std::filesystem::create_directories(merge_path.parent_path());
			}
			if (std::filesystem::exists(merge_path))
			{
				std::filesystem::remove(merge_path);
			}
			std::string name = merge_path.filename().string();
			Json::Value jsonValue = Json::objectValue;
			jsonValue["vector_layers"] = Json::arrayValue;
			jsonValue["tilestats"] = Json::objectValue;
			jsonValue["tilestats"]["layerCount"] = 0;
			jsonValue["tilestats"]["layers"] = Json::arrayValue;
			for (auto [_name, _info] : meta_data)
			{
				Json::Reader reader;
				Json::Value root;
				if (reader.parse(_info.json, root))
				{
					if (root["vector_layers"].isNull() || root["tilestats"].isNull())
					{
						continue;
					}
					if (root["vector_layers"].size() != root["tilestats"]["layers"].size())
					{
						continue;
					}

					for (int i = 0; i < root["vector_layers"].size(); ++i)
					{
						jsonValue["vector_layers"].append(root["vector_layers"][i]);
					}
					for (int i = 0; i < root["tilestats"]["layers"].size(); ++i)
					{
						jsonValue["tilestats"]["layers"].append(root["tilestats"]["layers"][i]);
					}
					jsonValue["tilestats"]["layerCount"] = jsonValue["tilestats"]["layerCount"].asInt() + root["vector_layers"].size();
				}

			}

			merge_meta_info.name = output_name;
			merge_meta_info.description = output_name;
			merge_meta_info.version = 2;
			merge_meta_info.minzoom = _min_level;
			merge_meta_info.maxzoom = _max_level;
			float centerx = (_max_lgtd + _min_lgtd) / 2;
			float centery = (_max_lttd + _min_lttd) / 2;
			merge_meta_info.center = std::to_string(centerx) + "," + std::to_string(centery);
			merge_meta_info.bounds = std::to_string(_min_lgtd) + "," + std::to_string(_min_lttd) + "," + std::to_string(_max_lgtd) + "," + std::to_string(_max_lttd);
			merge_meta_info.type = "overlay";
			merge_meta_info.format = "pbf";
			merge_meta_info.generator = "tzx tile merge";
			merge_meta_info.generator_options = "MBTileMerge";
			merge_meta_info.json = Json::FastWriter().write(jsonValue);

			return merge_meta_info;
		}
	private:

		/// <summary>
		/// 合并tile的任务线程
		/// </summary>
		/// <param name="level"></param>
		/// <param name="col"></param>
		/// <param name="row"></param>
		/// <param name="write_data"></param>
		void merge_tile_thread(const int& level, const int& col, const int& row, mtv_tile_string::LCRData& write_data)
		{
			mvt_tile _merge_mvt_tile;
			for (auto [_name, _lcrdata] : tile_data)
			{
				std::string data_str = _lcrdata[level][col][row]; // 检查不存在的是否补齐为空字符串
				if (data_str.empty())
				{
					continue;
				}

				mvt_tile _tmp_tile;
				bool isCps = true;
				if (_tmp_tile.decode(data_str, isCps)) {
					for (auto _tlayer : _tmp_tile.layers)
					{
						_merge_mvt_tile.layers.emplace_back(_tlayer);
					}
				}
			}
			if (_merge_mvt_tile.layers.size() > 0)
			{
				write_data[level][col][row] = _merge_mvt_tile.encode();
			}
			else
			{
				std::cerr << level << "-" << col << "-" << row << " 空" << std::endl;;
			}
			_thread_count--;
		}

		/// <summary>
		/// 将压缩后的数据写回
		/// </summary>
		/// <param name="merge_tile_data"></param>
		/// <param name="merge_meta_inf0"></param>
		/// <returns></returns>
		bool insert_tile_data(const mtv_tile_string::LCRData& merge_tile_data, MetaInfo& merge_meta_inf0)
		{
			sqlite3* pDB = NULL;
			if (SQLITE_OK != sqlite3_open(output_name.c_str(), &pDB))
			{
				std::cout << "Open database fail: " << sqlite3_errmsg(pDB) << std::endl;
				sqlite3_close(pDB);
				return false;
			}
			char* errMsg;
			if (SQLITE_OK != sqlite3_exec(pDB, SQL_CREATE_METADATA, NULL, NULL, &errMsg))
			{

				std::cout << "创建 metadata 表失败: " << sqlite3_errmsg(pDB) << std::endl;
				sqlite3_free(errMsg);
				sqlite3_close(pDB);
				return false;
			}
			sqlite3_free(errMsg);
			if (SQLITE_OK != sqlite3_exec(pDB, SQL_CREATE_TILES, NULL, NULL, &errMsg))
			{

				std::cout << "创建 tiles 表失败: " << sqlite3_errmsg(pDB) << std::endl;
				sqlite3_free(errMsg);
				sqlite3_close(pDB);
				return false;
			}
			sqlite3_free(errMsg);
			sqlite3_mutex_enter(sqlite3_db_mutex(pDB));
			sqlite3_exec(pDB, "PRAGMA synchronous=OFF", NULL, NULL, &errMsg);
			sqlite3_exec(pDB, "PRAGMA count_changes=OFF", NULL, NULL, &errMsg);
			sqlite3_exec(pDB, "PRAGMA journal_mode=MEMORY", NULL, NULL, &errMsg);
			sqlite3_exec(pDB, "PRAGMA temp_store=MEMORY", NULL, NULL, &errMsg);

			//sqlite3_exec(pDB, "BEGIN TRANSACTION", NULL, NULL, &errMsg);
			sqlite3_stmt* inset_meta_stmt;
			sqlite3_stmt* inset_tile_stmt;
			const char* pzTail = NULL;
			int rc = 0;
			rc = sqlite3_prepare(pDB, SQL_INSERT_METADAT, -1, &inset_meta_stmt, &pzTail);

			if (rc == SQLITE_OK) {

				std::map<std::string, std::string> meta_info_map = merge_meta_inf0.to_map();
				for (auto [_name, _value] : meta_info_map)
				{
					std::cout << _name << std::endl;
					sqlite3_bind_text(inset_meta_stmt, 1, _name.c_str(), strlen(_name.c_str()), 0);
					sqlite3_bind_text(inset_meta_stmt, 2, _value.c_str(), strlen(_value.c_str()), 0);
					int retVal = sqlite3_step(inset_meta_stmt);
					if (retVal != SQLITE_DONE)
					{
						printf("Commit Failed! %d\n", retVal);
					}

					sqlite3_reset(inset_meta_stmt);
				}
				sqlite3_finalize(inset_meta_stmt);
			}
			else {
				fprintf(stderr, "SQL error: %s\n", errMsg);
				sqlite3_free(errMsg);
			}

			rc = sqlite3_prepare(pDB, SQL_INSERT_TILES, -1, &inset_tile_stmt, &pzTail);

			if (rc == SQLITE_OK) {
				for (auto [l, b1] : merge_tile_data)
				{
					for (auto [c, b2] : b1)
					{
						for (auto [r, tdata] : b2)
						{
							sqlite3_bind_int(inset_tile_stmt, 1, l);
							sqlite3_bind_int(inset_tile_stmt, 2, c);
							sqlite3_bind_int(inset_tile_stmt, 3, r);
							sqlite3_bind_blob(inset_tile_stmt, 4, tdata.c_str(), strlen(tdata.c_str()), 0);
							int retVal = sqlite3_step(inset_tile_stmt);
							if (retVal != SQLITE_DONE)
							{
								printf("Commit Failed! %d\n", retVal);
							}

							sqlite3_reset(inset_tile_stmt);

						}
					}
				}
				sqlite3_finalize(inset_tile_stmt);
			}
			else {
				fprintf(stderr, "SQL error: %s\n", errMsg);
				sqlite3_free(errMsg);
			}

			sqlite3_mutex_leave(sqlite3_db_mutex(pDB));
			sqlite3_close(pDB);
			return false;
		}
		// 多个mbtiles的范围
		int _max_level = -10e6;
		int _min_level = 10e6;
		std::map<int, std::map<int, std::map<int, int>>> l_c_r_record;
		// 最大经纬范围包围框
		float _max_lgtd = -10e6;
		float _min_lgtd = 10e6;
		float _max_lttd = -10e6;
		float _min_lttd = 10e6;
		std::atomic<unsigned int> _thread_count;
	};

	/// <summary>
	/// 解析参数文件
	/// </summary>
	/// <returns></returns>
	std::map<std::string, std::vector<std::string>> phrase_param_file_task(int& max_thread_limit)
	{
		std::map<std::string, std::vector<std::string>> retData;
		Json::Reader reader;
		Json::Value root;
		std::fstream input;
		std::cout << std::filesystem::absolute(std::filesystem::path("param.json").string()) << std::endl;;
		input.open("param.json", std::ios::binary | std::ios::in);
		if (reader.parse(input, root))
		{
			if (root["thread_limit"].isNull())
			{
				max_thread_limit = 16;
			}
			else
			{
				max_thread_limit = root["thread_limit"].asInt();
			}
			for (int i = 0; i < root["tasks"].size(); ++i)
			{
				std::string _output = root["tasks"][i]["output"].asString();
				std::vector<std::string> _inputs;
				for (int j = 0; j < root["tasks"][i]["inputs"].size(); ++j)
				{
					_inputs.push_back(root["tasks"][i]["inputs"][j].asString());
				}
				retData.insert({ _output, _inputs });
			}
		}
		return retData;
	}

}

using namespace tzxutils;
int main()
{
	int thread_limit = 0;
	std::map<std::string, std::vector<std::string>> tasks_params = phrase_param_file_task(thread_limit);
	for (auto [_output_name, _input_names] : tasks_params)
	{
		TaskInfo _task_info;
		_task_info.max_thread = thread_limit;
		_task_info.input_names = _input_names;
		_task_info.output_name = _output_name;
		_task_info.read_data();
		_task_info.merge_tile_data();

	}
	return 0;
}