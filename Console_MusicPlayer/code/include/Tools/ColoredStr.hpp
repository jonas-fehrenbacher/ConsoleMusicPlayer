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
		// ...256
	};

	/* cout string */
	class ColoredStr
	{
	public:
		ColoredStr(std::string str, Color color);

		std::string str;
		Color       color;

		void operator= (std::string str);
	};
}

std::ostream& operator<<(std::ostream& stream, const core::ColoredStr& print);
inline bool operator==(const core::ColoredStr& lhs, const std::string& rhs) { return lhs.str == rhs; }
inline bool operator!=(const core::ColoredStr& lhs, const std::string& rhs) { return !operator==(lhs, rhs); }