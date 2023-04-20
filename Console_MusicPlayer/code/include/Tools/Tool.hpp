#pragma once

#include <map>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

namespace core
{
	/* Set flags in binary notation. 0 for not set and 1 for set. */
	bool hasFlag(int flag, int flagList);

	void clearScreen();

	void setWindowSize(unsigned short x = 100, unsigned short y = 100);

	void setWindowPos(unsigned short x, unsigned short y);

	long long getUUID();

	void setConfig(std::filesystem::path path, const std::map<std::string, std::string>& config);
	/* File must contain in each line: varName=varValue. */
	std::map<std::string, std::string> getConfig(std::filesystem::path path);
	std::vector<std::string> getConfigStrArr(std::string array);
	std::vector<fs::path> getConfigPathArr(std::string array);

	bool isSupportedAudioFile(fs::path filepath);

	void log(std::string message);
}