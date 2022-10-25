#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <json/json.h>

int main()
{
	std::filesystem::path  root("E:/dly/workspace/vcpkg/ports");
	for (auto iter : std::filesystem::directory_iterator(root))
	{
		std::cout << iter.path().filename().string();
		std::cout << "\t\t\t\t\t\t";
		std::filesystem::path vcpkgJsonPath = iter.path();
		vcpkgJsonPath = vcpkgJsonPath.append(std::string("vcpkg.json"));
		std::ifstream input;
		input.open(vcpkgJsonPath.string());
		Json::Reader reader;
		Json::Value root;
		reader.parse(input, root);

		std::string desc;
		if (root["description"].isString())
		{
			desc = root["description"].asString();
			
		}
		else if (root["description"].isArray())
		{
			for (int i = 0; i < root[""].size(); ++i)
			{
				desc += root["description"][i].asString();
				desc += "; ";
			}
		}
		else
		{
			//std::cout << iter.path().filename().string() << std::endl;
		}
		std::cout << desc << std::endl;
		
	}

	return 0;
}