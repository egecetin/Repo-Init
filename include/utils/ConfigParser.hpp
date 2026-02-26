#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

/**
 * @class ConfigParser
 * Parses configuration files
 */
class ConfigParser {
  private:
	std::filesystem::path _configPath;
	std::unordered_map<std::string, std::string> _configMap;
	bool _isValid = false;
	std::string _lastError;

	// JSON helpers
	bool readJson();
	void writeJson() const;

  public:
	/**
	 * Construct a new Config Parser object from config file. Currently only json files are supported
	 * @param[in] configPath Path to the configuration file
	 */
	explicit ConfigParser(std::filesystem::path configPath);

	/**
	 * Check if the configuration was loaded successfully
	 * @return bool True if valid, false otherwise
	 */
	bool isValid() const { return _isValid; }

	/**
	 * Get the last error message
	 * @return std::string Error message
	 */
	const std::string& getLastError() const { return _lastError; }

	/**
	 * Get the value of a key from the configuration file. If the configuration file modified from outside
	 * the program, the changes will not be reflected. You should call load() to refresh the configuration see the
	 * changes.
	 * @param[in] key Key to search for
	 * @return std::string Value of the key
	 */
	std::string get(const std::string &key) const;

	/**
	 * Get the configuration map
	 * @return std::unordered_map<std::string, std::string> Configuration map
	 */
	const std::unordered_map<std::string, std::string> &getConfigMap() const { return _configMap; }

	/**
	 * Set the value of a key in the configuration file. If the key does not exist, it will be created
	 * If you don't save to the file, the changes will be lost.
	 * @param[in] key Key to set
	 * @param[in] value Value to set
	 */
	void set(const std::string &key, const std::string_view &value);

	/**
	 * Remove a key from the configuration file. If the key does not exist, nothing will happen
	 * If you don't save to the file, the changes will be lost.
	 * @param[in] key Key to remove
	 */
	void remove(const std::string &key);

	/**
	 * Save the configuration to the file
	 */
	void save() const;

	/**
	 * Load the configuration from the file
	 * @return bool True if valid, false otherwise
	 */
	bool load();
};
