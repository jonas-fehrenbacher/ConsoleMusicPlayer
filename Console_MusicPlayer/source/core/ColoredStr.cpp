#include "core/ColoredStr.hpp"
#include <Windows.h>

core::ColoredStr::ColoredStr(std::string str, Color fgcolor /*= Color::White*/, Color bgcolor /*= Color::Black*/) :
	str(str), 
	fgcolor(fgcolor),
	bgcolor(bgcolor)
{

}

void core::ColoredStr::operator=(std::string str)
{
	this->str = str;
}

std::ostream& operator<<(std::ostream& stream, const core::ColoredStr& print)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)((int)print.bgcolor * 16 + (int)print.fgcolor));
	std::cout << print.str;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	return stream;
}

std::wostream& operator<<(std::wostream& stream, const core::ColoredStr& print)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)((int)print.bgcolor * 16 + (int)print.fgcolor));
	std::cout << print.str;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	return stream;
}