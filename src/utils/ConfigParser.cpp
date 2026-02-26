#include "utils/ConfigParser.hpp"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <format>
#include <fstream>

namespace
{
	template <typename T> std::string stringifyRapidjson(const T &obj)
	{
		rapidjson::StringBuffer sbuffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);
		obj.Accept(writer);
		return sbuffer.GetString();
	}
} // namespace

bool ConfigParser::readJson()
{
	std::ifstream inFile(_configPath);
	if (!inFile.is_open())
	{
		_lastError = std::format("{} {}", "Can't open config file:", _configPath.string());
		return false;
	}

	// Scope the wrapper to ensure proper destruction order
	rapidjson::Document doc;
	{
		rapidjson::IStreamWrapper fStreamWrapper(inFile);
		doc.ParseStream(fStreamWrapper);
	}

	// Check for parse errors
	if (doc.HasParseError())
	{
		_lastError = std::format("{} {}", "JSON parse error at offset", std::to_string(doc.GetErrorOffset()));
		return false;
	}

	// Check is there any data
	if (doc.IsNull() || !doc.IsObject())
	{
		_lastError = "Config is empty or not a JSON object";
		return false;
	}

	// Parse the configuration file
	for (const auto &entry : doc.GetObject())
	{
		_configMap[entry.name.GetString()] =
			entry.value.IsString() ? entry.value.GetString() : stringifyRapidjson(entry.value);
	}

	return true;
}

void ConfigParser::writeJson() const
{
	std::ofstream outFile(_configPath);
	rapidjson::OStreamWrapper fStreamWrapper(outFile);

	rapidjson::Document doc;
	doc.SetObject();

	for (const auto &[keyVal, valueVal] : _configMap)
	{
		rapidjson::Value key(keyVal.c_str(), doc.GetAllocator());
		rapidjson::Value value(valueVal.c_str(), doc.GetAllocator());
		doc.AddMember(key, value, doc.GetAllocator());
	}

	rapidjson::Writer<rapidjson::OStreamWrapper> writer(fStreamWrapper);
	doc.Accept(writer);
}

ConfigParser::ConfigParser(std::filesystem::path configPath) : _configPath(std::move(configPath)) { load(); }

std::string ConfigParser::get(const std::string &key) const
{
	auto itr = _configMap.find(key);
	return itr == _configMap.end() ? "" : itr->second;
}

void ConfigParser::set(const std::string &key, const std::string_view &value) { _configMap[key] = value; }

void ConfigParser::remove(const std::string &key) { _configMap.erase(key); }

void ConfigParser::save() const { writeJson(); }

bool ConfigParser::load()
{
	_configMap.clear();
	_isValid = readJson();
	return _isValid;
}
