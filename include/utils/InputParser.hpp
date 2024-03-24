#pragma once

#include <algorithm>
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
			this->tokens.emplace_back(argv[i]);
		}
		this->tokens.emplace_back("");
	}

	/**
	 * Gets single command line input
	 * @param[in] option Option to check
	 * @return const std::string& Found command line input. Empty string if not found
	 */
	const std::string &getCmdOption(const std::string &option) const
	{
		std::vector<std::string>::const_iterator itr;
		itr = std::find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end())
		{
			return *itr;
		}
		static const std::string empty_string;
		return empty_string;
	}

	/**
	 * Checks whether provided command line option exists.
	 * @param[in] option Option to check
	 * @return true If the provided option is found
	 * @return false If the provided option is not found
	 */
	bool cmdOptionExists(const std::string &option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}
};
