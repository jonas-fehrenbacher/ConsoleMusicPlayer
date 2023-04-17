#include "Tools/Tool.hpp"
#include "Tools/ColoredStr.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>

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

long long core::getUUID()
{
	static long long id = 0;
	return id++;
}

void core::setConfig(std::filesystem::path path, const std::map<std::string, std::string>& config)
{
	std::ofstream ofs(path, std::ios::out);
	for (auto& [name, value] : config) {
		ofs << name << " = " << value << "\n";
	}
	ofs.close();
}

std::map<std::string, std::string> core::getConfig(std::filesystem::path path)
{
	std::map<std::string, std::string> config;
	std::ifstream ifs(path, std::ios::in);
	std::string line;

	while (std::getline(ifs, line))
	{
		line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end()); // remove spaces
		std::string varName = line.substr(0, line.find('='));
		std::string varValue = line.substr(line.find('=') + 1);
		config.insert(std::pair<std::string, std::string>(varName, varValue));
	}
	ifs.close();

	return config;
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