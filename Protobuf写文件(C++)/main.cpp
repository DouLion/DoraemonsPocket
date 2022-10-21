#define _CRT_SECURE_NO_WARNINGS
#include <json/json.h>

#include <protozero/pbf_builder.hpp>
#include <protozero/pbf_message.hpp>
#include <protozero/pbf_reader.hpp>
#include <protozero/pbf_writer.hpp>

#include "ee.pb.h"
#include <iostream>
#include <fstream>
std::string read_file(const std::string& filename)
{
	std::string content;
	FILE* pf = fopen(filename.c_str(), "rb");
	if (pf)
	{
		char buffer[1024 * 10] = { 0 };
		auto cur = fread(buffer, 1, sizeof(buffer), pf);
		auto pos = 0;
		while (cur > 0)
		{
			content.resize(content.size() + cur);
			memcpy(&content[pos], buffer, cur);
			pos += cur;
			memset(buffer, 0, sizeof(buffer));
			cur = fread(buffer, 1, sizeof(buffer), pf);
		}
		fclose(pf);

	}
	return content;
}

bool write_file(const std::string& filename, const std::string& content)
{
	FILE* fw = fopen(filename.c_str(), "wb");
	if (!fw)
	{
		fwrite(content.c_str(), 1, content.length(), fw);
		fclose(fw);
		return true;
	}
		
	return false;
}

int main()
{
	topy_resp::RegionResp resp;
	std::string content = read_file("./test.json");

	Json::Reader _reader;
	Json::Value pRoot;
	if (_reader.parse(content, pRoot))
	{
		topy_resp::RegionResp_Header* header = resp.mutable_header();
		header->set_return_code(pRoot["header"]["return_code"].asInt());
		header->set_return_msg(pRoot["header"]["return_msg"].asString());

		topy_resp::RegionResp_Body* body = resp.mutable_body();
		for (int i = 0; i < pRoot["body"]["coordinates"].size();++i)
		{
			topy_resp::RegionResp_Body_Polygon* polygon = body->add_coordinates();
			for (int j = 0; j < pRoot["body"]["coordinates"][i].size();++j)
			{
				auto linestring = polygon->add_line_string();
				for (int k = 0; k < pRoot["body"]["coordinates"][i][j].size(); ++k)
				{

						
					auto point = linestring->add_points();
					point->set_x(pRoot["body"]["coordinates"][i][j][k][0].asFloat());
					point->set_y(pRoot["body"]["coordinates"][i][j][k][1].asFloat());
				}
			}
		}
		for (int i = 0; i < pRoot["body"]["station"].size(); ++i)
		{
			auto _station =	body->add_stations();
			_station->set_code(pRoot["body"]["station"]["code"].asString());
			_station->set_lgtd(pRoot["body"]["station"]["lgtd"].asFloat());
			_station->set_lttd(pRoot["body"]["station"]["lttd"].asFloat());
			_station->set_name(pRoot["body"]["station"]["name"].asString());
			_station->set_type(pRoot["body"]["station"]["type"].asString());
		}
		body->set_id(pRoot["body"]["id"].asString());
	}

	
	std::string out;
	
	resp.SerializeToString(&out);
	std::cout << out.size() << std::endl;
	// write_file("./test_region.pbf", out);
	std::ofstream outStream{ "./test_region.pbf", std::ios_base::out | std::ios_base::binary };
	outStream << out;

	return 0;
}