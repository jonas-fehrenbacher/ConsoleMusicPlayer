#include "Console.hpp"
#include "Tools/Tool.hpp"
#include "Tools/ColoredStr.hpp"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ostream>
#include <iostream>
#include <sstream>

namespace core::console
{
	enum Options {
		Size         = 1 << 0,
		ScreenBuffer = 1 << 1,
		Font         = 1 << 2
	};

	const std::wstring DEFAULT_UNICODE_FONTNAME = L"Lucida Sans Unicode";
	const std::wstring DEFAULT_FONTNAME = L"Consolas";
	RECT                       defaultWinRect;
	CONSOLE_SCREEN_BUFFER_INFO defaultScreenBufferInfo;
	CONSOLE_FONT_INFOEX        defaultFontInfo;
	HANDLE                     hOut = NULL;
	Options                    options = (Options)0;
}

void core::console::init()
{
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
}

void core::console::clearScreen()
{
	// See: https://cplusplus.com/articles/4z18T05o/

#if 0
	HANDLE                     hStdOut = hOut;
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

#if 1
	// Clear rest of the console (what core::endl couldn't do):
	// Can cause screen flicker if you draw one line to much.
	int undrawnLineCount = core::console::getCharCount().y - core::console::getCursorPos().y;
	std::string line(core::console::getCharCount().x, ' ');
	for (int i = 0; i < undrawnLineCount; ++i) {
		std::cout << line << (i < undrawnLineCount - 1 ? "\n" : "");
	}

	// Sets the cursor to the beginning and starts writing here. This means everything previous in the buffer is
	// still there and visible if not over drawn.
	COORD Position = { (SHORT)0, (SHORT)0 }; // X, Y
	SetConsoleCursorPosition(hOut, Position);
#endif

#if 0
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

core::Vec2 core::console::getCharCount()
{
	CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
	GetConsoleScreenBufferInfo(hOut, &scrBufferInfo);
	return { scrBufferInfo.dwSize.X, scrBufferInfo.dwSize.Y };
}

void core::console::setSize(unsigned int width, unsigned int height, bool adjustScreenbuffer)
{
	// Set width and height without changing width screen buffer:
	// SetConsoleWindowInfo() sets width and height with characters row and column count.
	// good size: 90, 35
	SMALL_RECT windowSize = { 0, 0, width, height }; // left, top, width, height
	SetConsoleWindowInfo(hOut, TRUE, &windowSize);

	// Set console size:
	// size: 1000, 900
	//HWND console = GetConsoleWindow();
	//RECT winRect;
	//GetWindowRect(console, &winRect);
	//defaultWinRect = winRect;
	//int status = MoveWindow(console, winRect.left, winRect.top, width, height, TRUE); // also sets wisth screen buffer (no scrollbar)
	//if (status == 0) {
	//	log(std::string("MoveWindow() failed! Reason : ") + std::to_string(GetLastError()));
	//	__debugbreak();
	//}

	if (adjustScreenbuffer) {
		// Changing screen buffer:
		// dwSize: A COORD structure that specifies the new size of the console screen buffer, in character rows and columns. 
		//         The specified width and height cannot be less than the width and height of the console screen buffer's window.
		// IMPORTANT: srWindow.Bottom+1, +1 is required otherwise it failes, because character height would be so little, that
		//            console could not be fully filled with characters.
		// See: https://stackoverflow.com/questions/3471520/how-to-remove-scrollbars-in-console-windows-c
		CONSOLE_SCREEN_BUFFER_INFO scrBufferInfo;
		GetConsoleScreenBufferInfo(hOut, &scrBufferInfo);
		defaultScreenBufferInfo = scrBufferInfo;
		//int status = SetConsoleScreenBufferSize(hOut, COORD{ (short)(scrBufferInfo.dwSize.X), (short)(scrBufferInfo.srWindow.Bottom + 1) }); // buffer is already adjusted in x axis
		int status = SetConsoleScreenBufferSize(hOut, COORD{ (short)(scrBufferInfo.srWindow.Right + 1), (short)(scrBufferInfo.srWindow.Bottom + 1) }); // buffer is not adjusted
		if (status == 0) {
			log(std::string("SetConsoleScreenBufferSize() failed! Reason : ") + std::to_string(GetLastError()));
			__debugbreak();
		}
		options = (Options)(options | Options::ScreenBuffer);
	}

	options = (Options)(options | Options::Size);
}

void core::console::setPos(unsigned int x, unsigned int y)
{
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void core::console::reset()
{
	if (options & Options::Size) {
		// MoveWindow takes to much time (console freezes after pressing close btn):
		//MoveWindow(GetConsoleWindow(), defaultWinRect.left, defaultWinRect.top, defaultWinRect.right, defaultWinRect.bottom, TRUE);
		SMALL_RECT windowSize = { 0, 0, defaultScreenBufferInfo.dwSize.X, defaultScreenBufferInfo.srWindow.Bottom };
		SetConsoleWindowInfo(hOut, TRUE, &windowSize);
	}
	if (options & Options::ScreenBuffer) {
		int Status = SetConsoleScreenBufferSize(hOut, defaultScreenBufferInfo.dwSize);
		if (Status == 0) {
			log(std::string("SetConsoleScreenBufferSize() failed! Reason : ") + std::to_string(GetLastError()));
			__debugbreak();
		}
	}
	if (options & Options::Font) {
		SetCurrentConsoleFontEx(hOut, false, &defaultFontInfo);
	}

	// Show cursor:
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.bVisible = true;
	SetConsoleCursorInfo(hOut, &cursorInfo);

	options = (Options)0;
}

void core::console::setFont(std::wstring fontName)
{
	// Get old font info:
	GetCurrentConsoleFontEx(hOut, false, &defaultFontInfo);

	CONSOLE_FONT_INFOEX fontInfo;
	fontInfo.cbSize = sizeof(fontInfo);
	fontInfo.nFont = 0;
	fontInfo.dwFontSize.X = 0;                   // Width of each character in the font
	fontInfo.dwFontSize.Y = 24;                  // Height
	fontInfo.FontFamily = FF_DONTCARE;
	fontInfo.FontWeight = FW_NORMAL;
	std::wcscpy(fontInfo.FaceName, fontName.c_str()); // Choose your font ("Consolas"); "Lucida Sans Unicode" is a unicode font! fontName.c_str()
	SetCurrentConsoleFontEx(hOut, false, &fontInfo);

	options = (Options)(options | Options::Font);
}

void core::console::setTitle(std::string title)
{
	SetConsoleTitle(title.c_str());
}

core::Vec2 core::console::getCursorPos()
{
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	if (!GetConsoleScreenBufferInfo(hOut, &cbsi)) {
		log(std::string("GetConsoleScreenBufferInfo() failed! Reason : ") + std::to_string(GetLastError()));
		__debugbreak();
	}

	return { cbsi.dwCursorPosition.X, cbsi.dwCursorPosition.Y };
}

void core::console::hideCursor()
{
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hOut, &cursorInfo);
	cursorInfo.bVisible = false; // set the cursor visibility
	SetConsoleCursorInfo(hOut, &cursorInfo);
}

std::ostream& operator<<(std::ostream& stream, const core::endl& endl)
{
	int fillLength = core::console::getCharCount().x - core::console::getCursorPos().x;
	std::string postStr(fillLength, ' ');
	std::cout << postStr << "\n";
	return stream;
}

std::wostream& operator<<(std::wostream& stream, const core::endl& endl)
{
	int fillLength = core::console::getCharCount().x - core::console::getCursorPos().x;
	std::wstring postStr(fillLength, ' ');
	std::wcout << postStr << "\n";
	return stream;
}

// Maybe useful: ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

intern COORD getScreenSize()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(hOut, &bufferInfo);
	const auto newScreenWidth = bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1;
	const auto newscreenHeight = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;

	return COORD{ static_cast<short>(newScreenWidth), static_cast<short>(newscreenHeight) };
}

intern void setConsoleColor(core::Color bg, core::Color fg)
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

intern void createConsole()
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