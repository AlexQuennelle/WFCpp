#pragma once

#include <raylib.h>
#include <vector>

struct Tile
{
	int ID;
	Color col;
	std::vector<int> N;
	std::vector<int> S;
	std::vector<int> E;
	std::vector<int> W;
	std::vector<int> NW;
	std::vector<int> SW;
	std::vector<int> SE;
	std::vector<int> NE;
};

static const std::vector<Tile> testTiles = {
	{
		.ID = 0,
		.col = {.r = 0, .g = 0, .b = 0, .a = 255},
		.N = {0, 1},
		.S = {0, 1},
		.E = {0, 1},
		.W = {0, 1},
		.NW = {0, 1},
		.SW = {0, 1},
		.SE = {0, 1},
		.NE = {0, 1},
	},
	{
		.ID = 1,
		.col = {.r = 120, .g = 120, .b = 120, .a = 255},
		.N = {0, 1, 2},
		.S = {0, 1, 2},
		.E = {0, 1, 2},
		.W = {0, 1, 2},
		.NW = {0, 1, 2},
		.SW = {0, 1, 2},
		.SE = {0, 1, 2},
		.NE = {0, 1, 2},
	},
	{
		.ID = 2,
		.col = {.r = 255, .g = 255, .b = 255, .a = 255},
		.N = {1, 2},
		.S = {1, 2},
		.E = {1, 2},
		.W = {1, 2},
		.NW = {1, 2},
		.SW = {1, 2},
		.SE = {1, 2},
		.NE = {1, 2},
	},
};
