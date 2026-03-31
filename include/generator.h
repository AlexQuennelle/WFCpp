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
	std::vector<int> adjacencies;
};

static const std::vector<Tile> testTiles = {
	{.ID = 0, .col = {.r = 0, .g = 0, .b = 0, .a = 255}, .adjacencies = {0, 1}},
	{.ID = 1,
	 .col = {.r = 120, .g = 120, .b = 120, .a = 255},
	 .adjacencies = {0, 1, 2}},
	{.ID = 2,
	 .col = {.r = 255, .g = 255, .b = 255, .a = 255},
	 .adjacencies = {1, 2}},
};
// static const std::vector<Tile> testTiles = {
// 	{.ID = 0, .col = {.r = 0, .g = 0, .b = 0, .a = 255}, .adjacencies = {0}},
// 	{.ID = 1,
// 	 .col = {.r = 255, .g = 255, .b = 255, .a = 255},
// 	 .adjacencies = {1}},
// };

class Cell
{
	public:
	Cell(const Cell&) = default;
	Cell(Cell&&) = delete;
	Cell& operator=(Cell&&) = delete;
	Cell(const std::vector<Tile>& init);

	auto Collapse(std::mt19937& rand) -> Tile;
	auto GetEntropy() const -> uint64_t;
	auto GetColor() const -> Color;
	auto GetPossibilities() const -> std::vector<Tile>;
	void SetPossibilities(std::vector<Tile> newPossibilities);

	Vector2 id;
	bool original{true};

	auto operator=(const Cell& other) -> Cell&
	{
		std::println("Assigning to cell ({}, {})", this->id.x, this->id.y);
		std::println("{}", other.possibilities.size());
		// raise(SIGTRAP);
		possibilities = other.possibilities;
		this->original = other.original;
		return *this;
	}

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
