#pragma once

#include "tile.h"

#include <array>
#include <cstdint>
#include <random>
#include <raylib.h>
#include <vector>

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
	bool isDone{false};

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
	bool isDone{false};
};

class ChunkPtr
{
	public:
	static void Next();

	auto operator->() -> Chunk*;

	private:
	std::array<Chunk, 4> chunks;

	static uint8_t index;
};

class Generator
{
	public:
	Generator(const int size, const int chunks = 1);

	void Step();
	void ToTex();
	void UpdateTex();
	auto GetTexture() const -> Texture2D;
	auto GetSize() const -> float;

	private:
	std::random_device rd;
	std::mt19937 gen;
	std::vector<Cell> grid;
	std::vector<Chunk> chunks;
	Image img{};
	Texture2D texture{};
	int size;
};
