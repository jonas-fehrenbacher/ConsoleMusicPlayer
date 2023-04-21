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

namespace core {
	const std::wstring DEFAULT_UNICODE_FONTNAME = L"Lucida Sans Unicode";
	const std::wstring DEFAULT_FONTNAME = L"Consolas";
}

bool core::hasFlag(int flag, int flagList)
{
	return flag == (flag & flagList);
}

void core::clearScreen()
{
	// See: https://cplusplus.com/articles/4z18T05o/

#if 0
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR)' ',
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
#endif

#if 0
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD Position = { (SHORT)0, (SHORT)0 }; // X, Y
	SetConsoleCursorPosition(hOut, Position);
#endif

#if 1
	// system("cls"):
	HANDLE h;
	CHAR_INFO v3;
	COORD v4;
	SMALL_RECT v5;
	CONSOLE_SCREEN_BUFFER_INFO v6;
	if ((h = (HANDLE)GetStdHandle(0xFFFFFFF5), (unsigned int)GetConsoleScreenBufferInfo(h, &v6)))
	{
		v5.Right = v6.dwSize.X;
		v5.Bottom = v6.dwSize.Y;
		v3.Char.UnicodeChar = 32;
		v4.Y = -v6.dwSize.Y;
		v3.Attributes = v6.wAttributes;
		v4.X = 0;
		*(DWORD*)&v5.Left = 0;
		ScrollConsoleScreenBufferW(h, &v5, 0, v4, &v3);
		v6.dwCursorPosition = { 0 };
		HANDLE v1 = GetStdHandle(0xFFFFFFF5);
		SetConsoleCursorPosition(v1, v6.dwCursorPosition);
	}
#endif

#if 0
	// Double buffer:
	// See: https://stackoverflow.com/questions/34842526/update-console-without-flickering-c
	constexpr int MAX_X = 256;
	constexpr int MAX_Y = 256;
	char prevCoutBuffer[MAX_X][MAX_Y];
	std::memset((char*)prevCoutBuffer, 0, MAX_X * MAX_Y);
	char coutBuffer[MAX_X][MAX_Y];
	std::memset((char*)coutBuffer, 0, MAX_X * MAX_Y);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// ...fill coutBuffer somehow

	for (int y = 0; y != MAX_Y; ++y)
	{
		for (int x = 0; x != MAX_X; ++x)
		{
			if (coutBuffer[x][y] == prevCoutBuffer[x][y]) {
				continue;
			}
			
			COORD position = { (SHORT)x, (SHORT)y }; // X, Y
			SetConsoleCursorPosition(hOut, position);
			std::cout << coutBuffer[x][y];
		}
	}
	std::cout.flush(); // write everything in the std::cout buffer to the console. Normally this is done with std::endl.
	std::memcpy((char*)prevCoutBuffer, (char const*)coutBuffer, MAX_X * MAX_Y);
	// do std::memset((char*)coutBuffer, 0, MAX_X * MAX_Y); ?
#endif
}

void core::setWindowSize(unsigned short x /*= 100*/, unsigned short y /*= 100*/)
{
	//Begin 113, 20, dann 113, 30
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SMALL_RECT windowSize = { 0, 0, x, y / 2 }; //3: Breite, 4: Länge
	SetConsoleWindowInfo(hOut, TRUE, &windowSize);
}

void core::setWindowPos(unsigned short x, unsigned short y)
{
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, NULL, 325, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void core::setConsoleFont(std::wstring fontName)
{
	
	CONSOLE_FONT_INFOEX fontInfo;
	fontInfo.cbSize = sizeof(fontInfo);
	fontInfo.nFont = 0;
	fontInfo.dwFontSize.X = 0;                   // Width of each character in the font
	fontInfo.dwFontSize.Y = 24;                  // Height
	fontInfo.FontFamily = FF_DONTCARE;
	fontInfo.FontWeight = FW_NORMAL;
	std::wcscpy(fontInfo.FaceName, fontName.c_str()); // Choose your font ("Consolas"); "Lucida Sans Unicode" is a unicode font! fontName.c_str()
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &fontInfo);
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

static COORD getScreenSize()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(hOut, &bufferInfo);
	const auto newScreenWidth = bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1;
	const auto newscreenHeight = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;

	return COORD{ static_cast<short>(newScreenWidth), static_cast<short>(newscreenHeight) };
}

static void setConsoleColor(core::Color bg, core::Color fg)
{
	// Set color for whole console.
	// SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7) still works, but its default background color is black.
	std::stringstream hex_bgcolor;
	hex_bgcolor << std::hex << (int)bg;
	std::stringstream hex_fgcolor;
	hex_fgcolor << std::hex << (int)fg;
	std::string cmd = "color " + hex_bgcolor.str() + hex_fgcolor.str();
	system(cmd.c_str());
}

static void createConsole()
{
	// Could be handy if using SDL2, which has by default no console.
	if (GetConsoleWindow() == NULL)
	{
		if (AllocConsole())
		{
			(void)freopen("CONIN$", "r", stdin);
			(void)freopen("CONOUT$", "w", stdout);
			(void)freopen("CONOUT$", "w", stderr);
	
			SetFocus(GetConsoleWindow());
		}
	}
}