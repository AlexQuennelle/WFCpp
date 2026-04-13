#include "generator.h"

#include <algorithm>
#include <csignal>
#include <print>
#include <random>
#include <ranges>
#include <utility>

static constexpr int MAXPROPAGATION{1000};

namespace r = std::ranges;
namespace rv = std::ranges::views;

Cell::Cell(const std::vector<Tile>& init) : possibilities(init) { }
auto Cell::Collapse(std::mt19937& rand) -> Tile
{
	std::uniform_int_distribution<> distrib(0, this->GetEntropy() - 1);
	this->possibilities
		= std::vector<Tile>(1, this->possibilities[distrib(rand)]);

	this->isDone = true;
	return this->possibilities[0];
}
auto Cell::GetEntropy() const -> uint64_t { return this->possibilities.size(); }
auto Cell::GetColor() const -> Color
{
	if (this->possibilities.size() <= 1)
		return this->possibilities[0].col;

	Vector3 col{.x = 0.0f, .y = 0.0f, .z = 0.0f};
	for (const auto& tile : this->possibilities)
	{
		auto tileCol{ColorToHSV(tile.col)};
		col.x += tileCol.x;
		col.y += tileCol.y;
		col.z += tileCol.z;
	}
	col.x /= static_cast<float>(this->possibilities.size());
	col.y /= static_cast<float>(this->possibilities.size());
	col.z /= static_cast<float>(this->possibilities.size());

	return ColorFromHSV(col.x, col.y, col.z);
}
auto Cell::GetPossibilities() const -> std::vector<Tile>
{
	return this->possibilities;
}
void Cell::SetPossibilities(std::vector<Tile> newPossibilities)
{
	this->possibilities = std::move(newPossibilities);
}

CellRef::CellRef(Cell* cell) : cell(cell), newCell(*this->cell) { }
auto CellRef::Collapse(std::mt19937& rand) -> Tile
{
	return this->newCell.Collapse(rand);
}
auto CellRef::Propogate(const std::vector<int> possibilities, int depth) -> bool
{
	if (--depth == 0)
	{
		return true;
	}

	auto initPossibilities = this->newCell.GetPossibilities();
	auto GetOverlap = [&possibilities](Tile a) -> bool
	{
		auto pred = [a](auto b) -> bool
		{
			return a.ID == b;
		};
		return r::any_of(possibilities, pred);
	};
	auto newPossibilites = initPossibilities
						   | rv::filter(GetOverlap)
						   | r::to<std::vector<Tile>>();
	this->newCell.SetPossibilities(newPossibilites);
	if (newPossibilites.size() == initPossibilities.size())
	{
		if (depth != MAXPROPAGATION - 1)
		{
			return true;
		}
	}
	if (newPossibilites.size() == 0)
	{
		std::println("Error state");
		return false;
	}

	std::vector<int> Nconstraints;
	std::vector<int> Sconstraints;
	std::vector<int> Econstraints;
	std::vector<int> Wconstraints;
	std::vector<int> NWconstraints;
	std::vector<int> SWconstraints;
	std::vector<int> NEconstraints;
	std::vector<int> SEconstraints;
	for (const auto& tile : newPossibilites)
	{
		Nconstraints.insert_range(Nconstraints.end(), tile.N);
		Sconstraints.insert_range(Sconstraints.end(), tile.S);
		Econstraints.insert_range(Econstraints.end(), tile.E);
		Wconstraints.insert_range(Wconstraints.end(), tile.W);
		NWconstraints.insert_range(NWconstraints.end(), tile.NW);
		SWconstraints.insert_range(SWconstraints.end(), tile.SW);
		SEconstraints.insert_range(SEconstraints.end(), tile.SE);
		NEconstraints.insert_range(NEconstraints.end(), tile.NE);
	}

	bool successful{true};
	if (N != nullptr)
		successful &= N->Propogate(Nconstraints, depth);
	if (W != nullptr)
		successful &= W->Propogate(Wconstraints, depth);
	if (S != nullptr)
		successful &= S->Propogate(Sconstraints, depth);
	if (E != nullptr)
		successful &= E->Propogate(Econstraints, depth);
	if (NW != nullptr)
		successful &= NW->Propogate(NWconstraints, depth);
	if (NE != nullptr)
		successful &= NE->Propogate(NEconstraints, depth);
	if (SE != nullptr)
		successful &= SE->Propogate(SEconstraints, depth);
	if (SW != nullptr)
		successful &= SW->Propogate(SWconstraints, depth);
	return successful;
}
void CellRef::Reset() { newCell = *cell; }
void CellRef::Apply() { *cell = newCell; }
auto CellRef::operator->() -> Cell* { return this->cell; }

Chunk::Chunk(const std::vector<Cell*>& data, const int size) : size(size)
{
	this->area = data
				 | rv::transform([](auto& cell) { return CellRef(cell); })
				 | r::to<std::vector<CellRef>>();
	for (auto x : rv::iota(0, size))
	{
		for (auto y : rv::iota(0, size))
		{
			auto& cell = *this->CellAt(x, y);
			cell.N = this->CellAt(x, y - 1);
			cell.W = this->CellAt(x - 1, y);
			cell.S = this->CellAt(x, y + 1);
			cell.E = this->CellAt(x + 1, y);
			cell.NE = this->CellAt(x + 1, y - 1);
			cell.NW = this->CellAt(x - 1, y - 1);
			cell.SW = this->CellAt(x - 1, y + 1);
			cell.SE = this->CellAt(x + 1, y + 1);
		}
	}
}
auto Chunk::Step(std::mt19937& rand) -> bool
{
	auto sortFunc = [](auto& a, auto& b) -> bool
	{
		return (*a)->GetEntropy() < (*b)->GetEntropy();
	};
	auto sorted
		= this->area
		  | rv::filter([](auto& cell) { return !cell->isDone; })
		  // | rv::filter([](auto& cell) { return cell->GetEntropy() > 1; })
		  | rv::transform([](auto& cell) { return &cell; })
		  | r::to<std::vector<CellRef*>>();
	if (sorted.size() == 0)
	{
		this->isDone = true;
		return true;
	}
	r::sort(sorted, sortFunc);
	auto GetLowest = [ref = (*sorted[0])->GetEntropy()](auto* cell) -> bool
	{
		return (*cell)->GetEntropy() == ref;
	};
	auto lowest
		= sorted | rv::take_while(GetLowest) | r::to<std::vector<CellRef*>>();
	std::uniform_int_distribution<> distrib(0, lowest.size() - 1);
	CellRef& toCollapse = *lowest[distrib(rand)];

	auto selectedTile = toCollapse.Collapse(rand);

	if (!toCollapse.Propogate({selectedTile.ID}, MAXPROPAGATION))
	{
		for (auto& cell : this->area)
		{
			cell.Reset();
		}
		return false;
	}

	for (auto& cell : this->area)
	{
		cell.Apply();
	}
	return true;
}
auto Chunk::CellAt(const int x, const int y) -> CellRef*
{
	if (x < 0 || x >= this->size)
		return nullptr;
	if (y < 0 || y >= this->size)
		return nullptr;

	return &area[(x * this->size) + y];
}

Generator::Generator(const int size, const int chunkCount) :
	grid(size * size, testTiles), size(size)
{
	this->gen = std::mt19937(rd());
	int chunkSize = (size / chunkCount) + (chunkCount - 1);
	int chunkStride = (size / chunkCount) - 1;

	for (auto [xc, yc] : rv::cartesian_product(rv::iota(0, chunkCount),
											   rv::iota(0, chunkCount)))
	{
		std::vector<Cell*> chunkData;
		int startIndex = ((xc * chunkStride) * size) + (yc * chunkStride);
		for (auto [x, y] : rv::cartesian_product(rv::iota(0, chunkSize),
												 rv::iota(0, chunkSize)))
		{
			// if ((x + (xc * chunkStride) >= size)
			// 	|| (y + (yc * chunkStride) >= size))
			// {
			// 	chunkData.push_back(nullptr);
			// 	continue;
			// }
			bool isInit
				= (this->grid[startIndex + ((x * size) + y)].id.x < 1.0f)
				  && (this->grid[startIndex + ((x * size) + y)].id.y < 1.0f);
			if (isInit)
				this->grid[startIndex + ((x * size) + y)].id
					= {.x = static_cast<float>(x + (xc * chunkStride)),
					   .y = static_cast<float>(y + (yc * chunkStride))};
			chunkData.push_back(&this->grid[startIndex + ((x * size) + y)]);
		}
		this->chunks.emplace_back(chunkData, chunkSize);
	}
}
void Generator::Step()
{
	while (!this->chunks[0].Step(this->gen))
		;
	// TODO: Fix how chunks are invoked
	// for (auto& chunk : this->chunks)
	// {
	// 	// TODO: Add conditions to check when chunk is done.
	// 	while (!chunk.Step(this->gen))
	// 		;
	// }
}
void Generator::ToTex()
{
	this->img
		= {.data = nullptr, .width = 0, .height = 0, .mipmaps = 0, .format = 0};
	this->img = GenImageColor(this->size, this->size, BLACK);
	for (auto [x, y] : rv::cartesian_product(rv::iota(0, this->size),
											 rv::iota(0, this->size)))
	{
		ImageDrawPixel(&this->img, x, y, this->grid[(x * size) + y].GetColor());
	}
	this->texture = LoadTextureFromImage(this->img);
}
void Generator::UpdateTex()
{
	for (auto [x, y] : rv::cartesian_product(rv::iota(0, this->size),
											 rv::iota(0, this->size)))
	{
		ImageDrawPixel(&this->img, x, y, this->grid[(x * size) + y].GetColor());
	}
	UpdateTexture(this->texture, this->img.data);
}
auto Generator::GetTexture() const -> Texture2D { return this->texture; }
auto Generator::GetSize() const -> float
{
	return static_cast<float>(this->size);
};
