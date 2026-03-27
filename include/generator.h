#pragma once

#include <cstdint>
#include <raylib.h>
#include <vector>

struct Tile
{
	int ID;
	Color col;
	std::vector<int> adjacencies;
};

// static const std::vector<Tile> testTiles;

enum class CellState : uint8_t
{
	Unresolved,
	Resolved,
	Error
};

class Cell
{
	public:
	Cell(const std::vector<Tile>& init);

	auto GetEntropy() const -> uint64_t;

	private:
	CellState state{CellState::Unresolved};
	std::vector<Tile> possibilities;
};

class Chunk
{
	public:
	Chunk(const std::vector<Cell*> data);

	private:
	std::vector<Cell*> area;
};

class Generator
{
	public:
	Generator(const int size, const int chunks = 1);

	private:
	std::vector<Cell> grid;
	std::vector<Chunk> chunks;
};
