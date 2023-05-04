#pragma once

#include <string>

namespace core 
{
	struct Vec2
	{
		int x, y;
	};
}

namespace core::console
{
	extern const std::wstring DEFAULT_UNICODE_FONTNAME;
	extern const std::wstring DEFAULT_FONTNAME;

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
		Bright_White,
		// Max: 256 (16*16), 16 different foregrounds on 16 different backgrounds.

		None // If Color is None, then the default console color will be used.
	};

	/*
	 * Use this when consoled is "cleared" by setting the console cursor to the top.
	 * Actually console is not cleared at all and cursor is just at the top, ready to overwrite everything previously
	 * in the screen buffer. core::endl() adds white spaces ' ' to the line till the end, so that everything that was
	 * previously in the buffer is overwritten. Though, the one thing that is left is, that if there are less rows written
	 * on the console than on the previous print, then they are not overwritten. But that is automatically handled in
	 * clearScreen().
	 * Colored endl: std::cout << core::endl{ core::Color::White };
	 */
	struct endl
	{
		int   count; // < how many end lines it should make.
		Color bgcolor;

		endl();
		endl(int count, Color bgcolor = Color::None);
		endl(Color bgcolor);
	};
	/*
	 * A normal tab (\t) draws nothing and the console cursor just jumps forward. With my clear routine this would allow
	 * old draws to be seen. core::tab() just draws eight white spaces, thus overdrawing old frames / text.
	 * Colored tab: std::cout << core::tab{ core::Color::White };
	 */
	struct tab
	{
		static int size;
		Color bgcolor;

		tab();
		tab(Color bgcolor);
	};

	/* std::cout colored string. If no color is specified the default will be taken. */
	class Text
	{
	public:
		std::string str;
		Color       fgcolor;
		Color       bgcolor;

		Text();
		Text(std::string str, Color fgcolor = Color::None, Color bgColor = Color::None);
		void operator= (std::string str);
	};

	class WText
	{
	public:
		std::wstring str;
		Color        fgcolor;
		Color        bgcolor;

		WText();
		WText(std::wstring str, Color fgcolor = Color::None, Color bgColor = Color::None);
		void operator= (std::wstring str);
	};

	void init();
	void reset();
	void clearScreen();
	/* Is expensive and causes screen stutter if called frequently. */
	void hardClearScreen();
	/* adjustScreenbuffer: Adjust screen buffer, so that it fits the window size (otherwise there might be a scrollbar). */
	void setSize(unsigned int width, unsigned int height, bool adjustScreenbuffer);
	void setPos(unsigned int x, unsigned int y);
	void setCursorPos(Vec2 pos);
	/* Font name can be anything windows knows: "Lucida Sans Unicode" for unicode or "Consolas". */
	void setFont(std::wstring fontName, short size = 27);
	void setTitle(std::string title);
	void setFgColor(Color color);
	void setBgColor(Color color);
	/* return screen buffer character count (row and column count). If there is a scrollbar, then this is
		not equal to the window character count. */
	Vec2 getCharCount();
	/* retun console cursor position in character row and column count. Note that is not the same as the mouse cursor. */
	Vec2 getCursorPos();
	Color getFgColor();
	Color getBgColor();
	void hideCursor();
}

std::ostream& operator<<(std::ostream& stream, const core::console::endl& endl);
std::wostream& operator<<(std::wostream& stream, const core::console::endl& endl);
std::ostream& operator<<(std::ostream& stream, const core::console::tab& tab);
std::wostream& operator<<(std::wostream& stream, const core::console::tab& tab);

std::ostream& operator<<(std::ostream& stream, const core::console::Text& text);
std::wostream& operator<<(std::wostream& stream, const core::console::Text& text);
inline bool operator==(const core::console::Text& lhs, const std::string& rhs) { return lhs.str == rhs; }
inline bool operator!=(const core::console::Text& lhs, const std::string& rhs) { return !operator==(lhs, rhs); }
std::wostream& operator<<(std::wostream& stream, const core::console::WText& text);
inline bool operator==(const core::console::WText& lhs, const std::string& rhs) { return lhs.str == rhs; }
inline bool operator!=(const core::console::WText& lhs, const std::string& rhs) { return !operator==(lhs, rhs); }

namespace core
{
	// Otherwise its to verbose..
	using console::Color;
	using console::tab;
	using console::endl;
	using console::Text;
	using console::WText;
}