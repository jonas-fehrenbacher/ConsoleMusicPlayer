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

	extern const std::wstring DEFAULT_UNICODE_FONTNAME;
	extern const std::wstring DEFAULT_FONTNAME;
	/* Font name can be anything windows knows: "Lucida Sans Unicode" for unicode or "Consolas". */
	void setConsoleFont(std::wstring fontName);

	long long getUUID();

	void setConfig(std::filesystem::path path, const std::map<std::wstring, std::wstring>& config);
	/* File must contain in each line: varName=varValue. */
	std::map<std::wstring, std::wstring> getConfig(std::filesystem::path path);
	std::vector<std::wstring> getConfigStrArr(std::wstring array);
	std::vector<fs::path> getConfigPathArr(std::wstring array);

	bool isSupportedAudioFile(fs::path filepath);

	void log(std::string message);
}