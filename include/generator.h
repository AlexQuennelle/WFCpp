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

	/**
	 * @brief Collapses the cell by randomly choosing one of the tiles in the
	 *        possibilities vector.
	 *
	 * @param rand Mersenne Twister generator for random numbers.
	 */
	auto Collapse(std::mt19937& rand) -> Tile;
	/**
	 * @returns The size of the possibilities vector.
	 */
	auto GetEntropy() const -> uint64_t;
	/**
	 * @returns The average colour of all tiles in the possibilities vector.
	 */
	auto GetColor() const -> Color;
	/**
	 * @returns A copy of the possibilities vector.
	 */
	auto GetPossibilities() const -> std::vector<Tile>;
	/**
	 * @brief Replaces the possibilities vector
	 *
	 * @param newPossibilities A new vector of Tiles to replace the internal
	 *                         possibilities vector.
	 */
	void SetPossibilities(std::vector<Tile> newPossibilities);

	Vector2 id{};
	bool isDone{false};

	private:
	std::vector<Tile> possibilities;
};

/**
 * @brief Wrapper class that holds a pointer to a cell, a copy of that cell, and
 *        pointers to adjacent CellPtrs.
 *        Also provides an API for managing the original and copy cells.
 */
class CellPtr
{
	public:
	CellPtr(Cell* cell);

	/**
	 * @brief Calls the Collapse() method on the copy cell.
	 *
	 * @param rand Mersenne Twister generator for random numbers.
	 */
	auto Collapse(std::mt19937& rand) -> Tile;
	/**
	 * @brief Propagates constraints to neighbouring CellPtrs until there are no
	 *        changes or depth reaches 0.
	 *
	 * @param possibilities Vector of Tile IDs to filter the possibilities
	 *                      vector by.
	 * @param depth Current propagation depth. Returns early when depth is 0.
	 *
	 * @returns True if the copy cell is in a valid state after filtering.
	 */
	auto Propagate(const std::vector<int> posibilities, int depth) -> bool;
	/**
	 * @brief Reinitializes the copy cell with the original cell.
	 */
	void Reset();
	/**
	 * @brief Replaces the original cell with the copy cell, applying changes
	 *        made.
	 */
	void Apply();

	auto operator->() -> Cell*;

	CellPtr* N{};
	CellPtr* W{};
	CellPtr* S{};
	CellPtr* E{};
	CellPtr* NE{};
	CellPtr* NW{};
	CellPtr* SW{};
	CellPtr* SE{};

	private:
	Cell* cell;
	Cell newCell;
};

class Chunk
{
	public:
	Chunk(const std::vector<Cell*>& data, const int size);

	/**
	 * @brief Calls the Reset method on all the CellPtrs in the area vector.
	 *
	 * @note Also 'collapses' any cells with an entropy of 1 to properly
	 *       propagate their constraints.
	 */
	void Reset();
	/**
	 * @brief Steps the generations once.
	 *
	 * @param rand Mersenne Twister generator for random numbers.
	 *
	 * @returns Whether the step was successful or failed and had to be reset.
	 */
	auto Step(std::mt19937& rand) -> bool;

	/**
	 * @returns Whether the chunk has finished generating.
	 */
	auto CheckDone() const -> bool;

	private:
	auto CellAt(const int x, const int y) -> CellPtr*;

	int size;
	std::vector<CellPtr> area;
	bool isDone{false};
};

/**
 * @brief Wrapper class to bundle together all chunks associated with a single
 *        thread.
 */
class ChunkPtr
{
	public:
	ChunkPtr(std::array<Chunk, 4> chunks);
	/**
	 * @brief advances the internal index counter, changing which chunk is being
	 *        exposed through operator->().
	 *
	 * @returns True if the index is 4 or greater, indicating all chunks have
	 *          been seen. Otherwise returns false.
	 */
	static auto Next() -> bool;

	auto operator->() -> Chunk*;

	private:
	std::array<Chunk, 4> chunks;

	static uint8_t index;
};

class Generator
{
	public:
	Generator(const int size, const int chunks = 1);

	/**
	 * @brief Runs the generator from start to finish.
	 */
	void Run();
	/**
	 * @brief Steps the generator forward once.
	 *
	 * @returns True if all chunks have been successfully stepped once.
	 */
	auto Step() -> bool;
	/**
	 * @brief Initializes the internal texture with the contents of the grid.
	 */
	void ToTex();
	/**
	 * @brief Updates the texture with the contents of the grid.
	 */
	void UpdateTex();
	/**
	 * @returns The internal texture.
	 */
	auto GetTexture() const -> Texture2D;
	/**
	 * @returns The size of the square grid.
	 */
	auto GetSize() const -> float;

	private:
	std::random_device rd;
	std::mt19937 gen;
	std::vector<Cell> grid;
	std::vector<ChunkPtr> ptrs;
	Image img{};
	Texture2D texture{};
	int size;
};
