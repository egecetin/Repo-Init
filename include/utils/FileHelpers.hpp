#pragma once

#include <fstream>
#include <regex>
#include <string>
#include <vector>

/**
 * Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @param[out] lastWord Last word (space delimiter) of the first found line
 * @return std::vector<std::string> Matched lines
 */
inline std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern,
											 std::string &lastWord)
{
	const std::regex regExp(pattern);
	std::ifstream inFile(filePath);
	std::vector<std::string> matchedLines;

	std::string readLine;
	while (getline(inFile, readLine))
	{
		if (std::regex_search(readLine, regExp))
		{
			matchedLines.push_back(readLine);
		}
	}

	if (!matchedLines.empty())
	{
		auto pos = matchedLines.front().find_last_of(' ');
		if (pos != std::string::npos && pos != matchedLines.front().size())
		{
			lastWord = matchedLines.front().substr(pos + 1);
		}
	}

	return matchedLines;
}

/**
 * Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @return std::vector<std::string> Matched lines
 */
inline std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern)
{
	std::string lastWord;
	return findFromFile(filePath, pattern, lastWord);
}
