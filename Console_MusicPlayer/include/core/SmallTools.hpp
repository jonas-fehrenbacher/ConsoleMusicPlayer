#pragma once

#include "Time.hpp"
#include <map>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
using namespace std::string_literals;
using namespace std::chrono_literals;

#define intern static

namespace core
{
	// unicode characters:
	// Important: std::string can not handle unicode characters properly, so be aware of the following: 
	// std::string(u8"\u25BA").length() == 3; std::wstring(L"\u25BA").length() == 1
	namespace uc
	{
		// Search here: https://www.compart.com/en/unicode/U+2588
		// Note unicode characters can be stored in an char by using u8"".
		extern const char* fullBlock; // instead you can use ' ' and change the background color.
		extern const char* infinity;
		extern const char* latinSmallLetterOu; // shuffle
		extern const char* copticCapitalLetterGangia; // shuffle
		extern const char* combiningCyrillicMillionsSign; // shuffle
		extern const char* leftwardsArrow;
		extern const char* blackRightPointingPointer; // play
		extern const char* doubleVerticalLine; // pause
		extern const char* boxDrawingsDoubleVertical; // pause
		extern const char* latinLetterLateralClick; // pause
		extern const char* blackSquare; // stop
		extern const char* rightwardsArrow;
		extern const char* eighthNote;
		extern const char* beamedEighthNotes;
		extern const char* modifierLetterUpArrowhead;
		extern const char* modifierLetterDownArrowhead;
		extern const char* overline;
		extern const char* ballotBoxWithCheck;
		extern const char* ballotBox;
		extern const char* upwardsArrow;
		extern const char* downwardsArrow;
		extern const char* blackUpPointingTriangle;
		extern const char* blackDownPointingTriangle;
		extern const char* blackRightPointingTriangle;
		// Box drawing:
		extern const char* boxDrawingsLightHorizontal;
		extern const char* boxDrawingsHeavyHorizontal;
		extern const char* boxDrawingsLightVertical;
		extern const char* boxDrawingsHeavyVertical;
		extern const char* boxDrawingsLightDownAndRight;
		extern const char* boxDrawingsHeavyDownAndRight;
		extern const char* boxDrawingsLightDownAndLeft;
		extern const char* boxDrawingsHeavyDownAndLeft;
		extern const char* boxDrawingsLightUpAndRight;
		extern const char* boxDrawingsHeavyUpAndRight;
		extern const char* boxDrawingsLightUpAndLeft;
		extern const char* boxDrawingsHeavyUpAndLeft;
		extern const char* boxDrawingsLightArcDownAndRight;
		extern const char* boxDrawingsLightArcDownAndLeft;
		extern const char* boxDrawingsLightArcUpAndLeft;
		extern const char* boxDrawingsLightArcUpAndRight;
	}

	void log(std::string message);
	std::wstring toWStr(std::string str);
	std::string toStr(std::wstring wstr);
	/* Set flags in binary notation. 0 for not set and 1 for set. */
	std::string getUsername();
	long long getUUID();
	/* Convert core::Time to an string (3min -> "3:00") */
	std::string getTimeStr(core::Time time, core::Time limit = 0ns);
	/** utf8 strings can not return the real length, but this function helps with that. */
	size_t getStrLength(const std::string& utf8);
	size_t getUniCodeCharCount(const std::string& utf8);
	size_t getUniCodeCharCount(const std::wstring& utf8);
	bool isSupportedAudioFile(fs::path filepath);
	bool isAudioFile(fs::path filepath);
	bool hasFlag(int flag, int flagList);

	void setConfig(std::filesystem::path path, const std::map<std::wstring, std::wstring>& config);
	/* File must contain in each line: varName=varValue. */
	std::map<std::wstring, std::wstring> getConfig(std::filesystem::path path);
	std::vector<std::wstring> getConfigStrArr(std::wstring array);
	std::vector<fs::path> getConfigPathArr(std::wstring array);

	void testUCPrint();
	void testColors();
}