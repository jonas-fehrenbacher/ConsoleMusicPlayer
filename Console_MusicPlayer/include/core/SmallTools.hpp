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
	namespace uc
	{
		// Search here: https://www.compart.com/en/unicode/U+2588
		// Note unicode characters can be stored in an char by using u8"".
		extern const char* fullBlock;
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
	}

	void log(std::string message);
	/* Set flags in binary notation. 0 for not set and 1 for set. */
	std::string getUsername();
	long long getUUID();
	/* Convert core::Time to an string (3min -> "3:00") */
	std::string getTimeStr(core::Time time, core::Time limit = 0ns);
	bool isSupportedAudioFile(fs::path filepath);
	bool hasFlag(int flag, int flagList);

	void setConfig(std::filesystem::path path, const std::map<std::wstring, std::wstring>& config);
	/* File must contain in each line: varName=varValue. */
	std::map<std::wstring, std::wstring> getConfig(std::filesystem::path path);
	std::vector<std::wstring> getConfigStrArr(std::wstring array);
	std::vector<fs::path> getConfigPathArr(std::wstring array);

	void testUCPrint();
	void testColors();
}