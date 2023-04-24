#pragma once

#include <map>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#define intern static

namespace core
{
	/* Set flags in binary notation. 0 for not set and 1 for set. */
	bool hasFlag(int flag, int flagList);

	std::string getUsername();

	long long getUUID();

	void setConfig(std::filesystem::path path, const std::map<std::wstring, std::wstring>& config);
	/* File must contain in each line: varName=varValue. */
	std::map<std::wstring, std::wstring> getConfig(std::filesystem::path path);
	std::vector<std::wstring> getConfigStrArr(std::wstring array);
	std::vector<fs::path> getConfigPathArr(std::wstring array);

	bool isSupportedAudioFile(fs::path filepath);

	void log(std::string message);
}