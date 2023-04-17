#include "Tools/ColoredStr.hpp"
#include <Windows.h>

core::ColoredStr::ColoredStr(std::string str, Color color) :
	str(str), 
	color(color)
{

}

void core::ColoredStr::operator= (std::string str)
{
	this->str = str;
}

std::ostream& operator<<(std::ostream& stream, const core::ColoredStr& print)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)print.color);
	std::cout << print.str;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	return stream;
}