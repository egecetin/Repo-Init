#include "utils/ConfigParser.hpp"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <fstream>

template <typename T> std::string stringifyRapidjson(const T &obj)
{
	rapidjson::StringBuffer sbuffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);
	obj.Accept(writer);
	return sbuffer.GetString();
}

void ConfigParser::readJson()
{
	std::ifstream inFile(_configPath);
	if (!inFile.is_open())
	{
		throw std::invalid_argument("Can't open config file");
	}

	rapidjson::IStreamWrapper fStreamWrapper(inFile);

	rapidjson::Document doc;
	doc.ParseStream(fStreamWrapper);

	// Check is there any data
	if (doc.IsNull())
	{
		throw std::invalid_argument("Read config is empty or invalid JSON format");
	}

	// Parse the configuration file
	for (const auto &entry : doc.GetObject())
	{
		_configMap[entry.name.GetString()] = entry.value.IsString() ? entry.value.GetString() : stringifyRapidjson(entry.value);
	}
}

void ConfigParser::writeJson()
{
	std::ofstream outFile(_configPath);
	rapidjson::OStreamWrapper fStreamWrapper(outFile);

	rapidjson::Document doc;
	doc.SetObject();

	for (const auto &entry : _configMap)
	{
		rapidjson::Value key(entry.first.c_str(), doc.GetAllocator());
		rapidjson::Value value(entry.second.c_str(), doc.GetAllocator());
		doc.AddMember(key, value, doc.GetAllocator());
	}

	rapidjson::Writer<rapidjson::OStreamWrapper> writer(fStreamWrapper);
	doc.Accept(writer);
}

ConfigParser::ConfigParser(std::string configPath) : _configPath(std::move(configPath)) { load(); }

std::string ConfigParser::get(const std::string &key) const
{
	auto itr = _configMap.find(key);
	return itr == _configMap.end() ? "" : itr->second;
}

void ConfigParser::set(const std::string &key, const std::string &value) { _configMap[key] = value; }

void ConfigParser::save()
{
	writeJson();
}

void ConfigParser::load()
{
	_configMap.clear();
	readJson();
}
