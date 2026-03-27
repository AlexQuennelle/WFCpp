#include "generator.h"

#include <csignal>
#include <ranges>

namespace r = std::ranges;
namespace rv = std::ranges::views;

Cell::Cell(const std::vector<Tile>& init) : possibilities(init) { }
auto Cell::GetEntropy() const -> uint64_t { return this->possibilities.size(); }

Chunk::Chunk(const std::vector<Cell*> data) { }

Generator::Generator(const int size,
					 const int chunks) //:
									   // grid(size * size, testTiles)
{
	const std::vector<Tile> testTiles = {
		{.ID = 0,
		 .col = {.r = 0, .g = 0, .b = 0, .a = 255},
		 .adjacencies = {1}},
		{.ID = 1,
		 .col = {.r = 220, .g = 220, .b = 220, .a = 220},
		 .adjacencies = {0, 2}},
		{.ID = 2,
		 .col = {.r = 255, .g = 255, .b = 255, .a = 255},
		 .adjacencies = {1}},
	};
	this->grid = std::vector<Cell>(size * size, testTiles);

	int chunkSize = size; // TODO: Fix this when multithreading
	int chunkStride = chunks / size;

	for (auto [xc, yc] :
		 rv::cartesian_product(rv::iota(0, chunks), rv::iota(0, chunks)))
	{
		std::vector<Cell*> chunkData;
		int startIndex = ((xc * chunkStride) * size) + (yc * chunkStride);
		for (auto [x, y] : rv::cartesian_product(rv::iota(0, chunkSize),
												 rv::iota(0, chunkSize)))
		{
			chunkData.push_back(&this->grid[startIndex + ((x * size) + y)]);
		}
		this->chunks.emplace_back(chunkData);
	}
}
