#pragma once
#include <iostream>
#include <string>

namespace core
{
	enum class Color
	{
		Black,         
		Blue,			
		Green,			
		Aqua,			
		Red,				
		Purple,			
		Yellow,			
		White, // default		
		Gray,			
		Light_Blue,		
		Light_Green,	  
		Light_Aqua,		
		Light_Red,		
		Light_Purple,	
		Light_Yellow,	
		Bright_White
		// Max: 256 (16*16), 16 different foregrounds on 16 different backgrounds.
	};

	/* std::cout colored string. If no color is specified the default will be taken. */
	class ColoredStr
	{
	public:
		std::string str;
		Color       fgcolor;
		Color       bgcolor;

		ColoredStr(std::string str, Color fgcolor = Color::White, Color bgColor = Color::Black);
		void operator= (std::string str);
	};
}

std::ostream& operator<<(std::ostream& stream, const core::ColoredStr& print);
std::wostream& operator<<(std::wostream& stream, const core::ColoredStr& print);
inline bool operator==(const core::ColoredStr& lhs, const std::string& rhs) { return lhs.str == rhs; }
inline bool operator!=(const core::ColoredStr& lhs, const std::string& rhs) { return !operator==(lhs, rhs); }