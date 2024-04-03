#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

/**
 * @class InputParser
 * Parses command line inputs
 */
class InputParser {
  private:
	std::vector<std::string> tokens;

  public:
	/**
	 * Constructs a new InputParser object
	 * @param[in] argc Number of input arguments
	 * @param[in] argv Input arguments
	 */
	InputParser(const int &argc, char **argv)
	{
		for (int i = 1; i < argc; ++i)
		{
			tokens.emplace_back(argv[i]);
		}
	}

	/**
	 * Gets single command line input
	 * @param[in] option Option to check
	 * @return const std::string& Found command line input. Empty string if not found
	 */
	const std::string &getCmdOption(const std::string &option) const
	{
		std::vector<std::string>::const_iterator itr;
		itr = std::find(tokens.begin(), tokens.end(), option);
		if (itr != tokens.end() && ++itr != tokens.end())
		{
			return *itr;
		}
		static const std::string empty_string;
		return empty_string;
	}

	/**
	 * Gets all command line options
	 * @return std::vector<std::pair<std::string, std::string>> All command line options
	 */
	std::vector<std::pair<std::string, std::string>> getCmdOptions() const
	{
		std::vector<std::pair<std::string, std::string>> options;
		for (auto itr = tokens.begin(); itr != tokens.end(); ++itr)
		{
			if (!itr->empty() && itr->at(0) == '-')
			{
				auto nextItr = std::next(itr);
				if (nextItr != tokens.end() && !nextItr->empty() && nextItr->at(0) != '-')
				{
					options.emplace_back(*itr, *(nextItr));
				}
				else
				{
					options.emplace_back(*itr, "");
				}
			}
		}
		return options;
	}

	/**
	 * Checks whether provided command line option exists.
	 * @param[in] option Option to check
	 * @return true If the provided option is found
	 * @return false If the provided option is not found
	 */
	bool cmdOptionExists(const std::string &option) const
	{
		return std::find(tokens.begin(), tokens.end(), option) != tokens.end();
	}
};
