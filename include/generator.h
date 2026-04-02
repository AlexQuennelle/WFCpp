#pragma once

#include <csignal>
#include <cstdint>
#include <print>
#include <random>
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
		.S = {0},
		.E = {0},
		.W = {0},
		.NW = {0, 1},
		.SW = {0},
		.SE = {0},
		.NE = {0, 1},
	},
	{
		.ID = 1,
		.col = {.r = 120, .g = 120, .b = 120, .a = 255},
		.N = {1, 2},
		.S = {0, 1},
		.E = {1},
		.W = {1},
		.NW = {1, 2},
		.SW = {0, 1},
		.SE = {0, 1},
		.NE = {1, 2},
	},
	{
		.ID = 2,
		.col = {.r = 255, .g = 255, .b = 255, .a = 255},
		.N = {2},
		.S = {1, 2},
		.E = {2},
		.W = {2},
		.NW = {2},
		.SW = {1, 2},
		.SE = {1, 2},
		.NE = {2},
	},
};

class Cell
{
	public:
	Cell(const std::vector<Tile>& init);

	auto Collapse(std::mt19937& rand) -> Tile;
	auto GetEntropy() const -> uint64_t;
	auto GetColor() const -> Color;
	auto GetPossibilities() const -> std::vector<Tile>;
	void SetPossibilities(std::vector<Tile> newPossibilities);

	Vector2 id;

	private:
	std::vector<Tile> possibilities;
};

class CellRef
{
	public:
	CellRef(Cell* cell);

	auto Collapse(std::mt19937& rand) -> Tile;
	auto Propogate(const std::vector<int> posibilities, int depth) -> bool;
	void Reset();
	void Apply();

	auto operator->() -> Cell*;

	CellRef* N{};
	CellRef* W{};
	CellRef* S{};
	CellRef* E{};
	CellRef* NE{};
	CellRef* NW{};
	CellRef* SW{};
	CellRef* SE{};

	private:
	Cell* cell;
	Cell newCell;
};

class Chunk
{
	public:
	Chunk(const std::vector<Cell*>& data, const int size);

	auto Step(std::mt19937& rand) -> bool;

	private:
	auto CellAt(const int x, const int y) -> CellRef*;

	int size;
	std::vector<CellRef> area;
};

class Generator
{
	public:
	Generator(const int size, const int chunks = 1);

	void Step();
	void ToTex();
	void UpdateTex();
	auto GetTexture() const -> Texture2D { return this->texture; };

	private:
	std::random_device rd;
	std::mt19937 gen;
	std::vector<Cell> grid;
	std::vector<Chunk> chunks;
	Image img{};
	Texture2D texture{};
	int size;
};
