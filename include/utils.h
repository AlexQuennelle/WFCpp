#pragma once

#include <iostream>
#include <raylib.h>
#include <string>

static constexpr Color ERROR{220, 0, 0, 0};
static constexpr Color SUCCESS{0, 220, 0, 0};
static constexpr Color INFO{200, 200, 100, 0};

/** Resets all terminal styles and colours. */
inline void ClearStyles() { std::cout << "\033[0m"; }

/**
 * Sets the foreground colour of the output terminal to the provided colour
 * using ANSI escape codes.
 */
inline void SetTextColor(Color col)
{
	std::cout << "\033[38;2;" << std::to_string(col.r) << ';'
			  << std::to_string(col.g) << ';' << std::to_string(col.b) << 'm';
}
