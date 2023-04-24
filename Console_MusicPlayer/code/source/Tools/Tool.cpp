#include "Tools/Tool.hpp"
#include "Tools/ColoredStr.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <codecvt>
#include <locale>

namespace core 
{
	const std::wstring DEFAULT_UNICODE_FONTNAME = L"Lucida Sans Unicode";
	const std::wstring DEFAULT_FONTNAME = L"Consolas";
}

bool core::hasFlag(int flag, int flagList)
{
	return flag == (flag & flagList);
}

std::string core::getUsername()
{
	return getenv("username");
}

long long core::getUUID()
{
	static long long id = 0;
	return id++;
}

void core::setConfig(std::filesystem::path path, const std::map<std::wstring, std::wstring>& config)
{
	// Debug:
	return;

	std::wofstream ofs(path, std::ios::out);
	for (auto& [name, value] : config) {
		ofs << name << " = " << value << "\n";
	}
	ofs.close();
}

std::map<std::wstring, std::wstring> core::getConfig(std::filesystem::path path)
{
	std::map<std::wstring, std::wstring> config;
	std::wifstream ifs(path, std::ios::in);
	ifs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstring line;

	while (std::getline(ifs, line))
	{
		// Note: Spaces may not be removed on 'line', because strings can contain spaces (e.g. paths).
		std::wstring varName = line.substr(0, line.find('='));
		// remove spaces:
		std::wstring::iterator newEndPos = std::remove(varName.begin(), varName.end(), ' ');
		varName.erase(newEndPos, varName.end());
		// Alternative works not for utf-8: varName.erase(std::remove_if(varName.begin(), varName.end(), isspace), varName.end());
		int varValueIndex = line.find('=') + 1;
		for (; line[varValueIndex] == ' '; ++varValueIndex);
		std::wstring varValue = line.substr(varValueIndex);
		config.insert(std::pair<std::wstring, std::wstring>(varName, varValue));
	}
	ifs.close();

	return config;
}

std::vector<std::wstring> core::getConfigStrArr(std::wstring strarr)
{
	// Example: musicDirs = "music", "C:/Users/Jonas/Music"
	std::vector<std::wstring> arr;
	for (int end = 0; strarr.find(L"\"", end + 1) != -1;) { // +1 because at the end I need to check for the next occurrence
		int begin = strarr.find(L"\"", end == 0 ? 0 : end + 1);
		end = strarr.find(L"\"", begin + 1);
		arr.push_back(strarr.substr(begin + 1, end - begin - 1));
	}
	return arr;
}

std::vector<fs::path> core::getConfigPathArr(std::wstring strarr)
{
	std::vector<fs::path> pathArr;
	std::vector<std::wstring> arr = getConfigStrArr(strarr);
	std::transform(arr.begin(), arr.end(), std::back_inserter(pathArr),
		[](const std::wstring& str) { return str; });
	return pathArr;
}

bool core::isSupportedAudioFile(fs::path filepath)
{
	static const std::vector<std::string> sdlSupportedExtentions{
		".flac", ".mp3", ".ogg", ".voc", ".wav", ".midi", ".mod", ".opus"
	};
	std::string audioFormat = filepath.extension().string();
	std::transform(audioFormat.begin(), audioFormat.end(), audioFormat.begin(),
		[](unsigned char c) { return std::tolower(c); }); // e.g. .WAV is also valid
	bool isSupported = false;
	for (auto& sdlSupportedExtention : sdlSupportedExtentions) {
		if (audioFormat == sdlSupportedExtention) {
			isSupported = true;
			break;
		}
	}
	return isSupported;
}

void core::log(std::string message)
{
	std::ofstream ofs("data/log.txt", std::ios_base::app);
	ofs << message << "\n";
	ofs.close();
}