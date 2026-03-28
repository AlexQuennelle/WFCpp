#pragma once

#include <cstdint>
#include <random>
#include <raylib.h>
#include <vector>

struct Tile
{
	int ID;
	Color col;
	std::vector<int> adjacencies;
};

static const std::vector<Tile> testTiles = {
	{.ID = 0, .col = {.r = 0, .g = 0, .b = 0, .a = 255}, .adjacencies = {1}},
	{.ID = 1,
	 .col = {.r = 220, .g = 220, .b = 220, .a = 220},
	 .adjacencies = {0, 2}},
	{.ID = 2,
	 .col = {.r = 255, .g = 255, .b = 255, .a = 255},
	 .adjacencies = {1}},
};

class Cell
{
	public:
	Cell(const std::vector<Tile>& init);

	void Collapse(std::mt19937& rand);
	auto GetEntropy() const -> uint64_t;
	auto GetColor() const -> Color;

	private:
	std::vector<Tile> possibilities;
};

class CellRef
{
	public:
	CellRef(Cell* cell);

	void Collapse(std::mt19937& rand);
	void Reset();
	void Apply();

	auto operator->() -> Cell*;

	CellRef* N;
	CellRef* W;
	CellRef* S;
	CellRef* E;
	CellRef* NE;
	CellRef* NW;
	CellRef* SW;
	CellRef* SE;

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
