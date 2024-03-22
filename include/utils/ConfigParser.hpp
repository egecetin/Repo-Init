#pragma once

#include <string>
#include <unordered_map>

/**
 * @class ConfigParser
 * @brief Parses configuration files
 */
class ConfigParser {
  private:
	std::string configPath;
	std::unordered_map<std::string, std::string> configMap;

  public:
	/**
	 * @brief Construct a new Config Parser object from config file. Currently only json files are supported
	 * @param[in] configPath Path to the configuration file
	 */
	ConfigParser(const std::string &configPath);

	/**
	 * @brief Get the value of a key from the configuration file. If the configuration file modified from outside
	 * the program, the changes will not be reflected. You should call load() to refresh the configuration see the
	 * changes.
	 * @param[in] key Key to search for
	 * @return std::string Value of the key
	 */
	std::string get(const std::string &key);

	/**
	 * @brief Set the value of a key in the configuration file. If the key does not exist, it will be created
	 * If you don't save to the file, the changes will be lost.
	 * @param[in] key Key to set
	 * @param[in] value Value to set
	 */
	void set(const std::string &key, const std::string &value);

	/**
	 * @brief Save the configuration to the file
	 */
	void save();

	/**
	 * @brief Load the configuration from the file
	 */
	void load();

	/**
	 * @brief Print the configuration to the console
	 */
	void print();
};
