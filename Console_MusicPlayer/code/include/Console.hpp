#pragma once

#include <string>

namespace core 
{
	struct Vec2
	{
		int x, y;
	};

	/*
	 * Use this when consoled is "cleared" by setting the console cursor to the top.
	 * Actually console is not cleared at all and cursor is just at the top, ready to overwrite everything previously
	 * in the screen buffer. core::endl() adds white spaces ' ' to the line till the end, so that everything that was
	 * previously in the buffer is overwritten. Though, the one thing that is left is, that if there are less rows written
	 * on the console than on the previous print, then they are not overwritten. But that is automatically handled in
	 * clearScreen().
	 */
	struct endl {};
}

namespace core::console
{
	extern const std::wstring DEFAULT_UNICODE_FONTNAME;
	extern const std::wstring DEFAULT_FONTNAME;

	void init();
	void clearScreen();
	/* adjustScreenbuffer: Adjust screen buffer, so that it fits the window size (otherwise there might be a scrollbar). */
	void setSize(unsigned int width, unsigned int height, bool adjustScreenbuffer);
	void setPos(unsigned int x, unsigned int y);
	void reset();
	/* return screen buffer character count (row and column count). If there is a scrollbar, then this is
		not equal to the window character count. */
	Vec2 getCharCount();
	/* retun console cursor position in character row and column count. Note that is not the same as the mouse cursor. */
	Vec2 getCursorPos();
	void hideCursor();
	/* Font name can be anything windows knows: "Lucida Sans Unicode" for unicode or "Consolas". */
	void setFont(std::wstring fontName);
	void setTitle(std::string title);
}

std::ostream& operator<<(std::ostream& stream, const core::endl& endl);
std::wostream& operator<<(std::wostream& stream, const core::endl& endl);