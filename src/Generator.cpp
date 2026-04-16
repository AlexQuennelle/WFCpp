#include "generator.h"

#include <algorithm>
#include <barrier>
#include <random>
#include <ranges>
#include <thread>
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

CellPtr::CellPtr(Cell* cell) : cell(cell), newCell(*this->cell) { }
auto CellPtr::Collapse(std::mt19937& rand) -> Tile
{
	return this->newCell.Collapse(rand);
}
auto CellPtr::Propagate(const std::vector<int> possibilities, int depth) -> bool
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
		successful &= N->Propagate(Nconstraints, depth);
	if (W != nullptr)
		successful &= W->Propagate(Wconstraints, depth);
	if (S != nullptr)
		successful &= S->Propagate(Sconstraints, depth);
	if (E != nullptr)
		successful &= E->Propagate(Econstraints, depth);
	if (NW != nullptr)
		successful &= NW->Propagate(NWconstraints, depth);
	if (NE != nullptr)
		successful &= NE->Propagate(NEconstraints, depth);
	if (SE != nullptr)
		successful &= SE->Propagate(SEconstraints, depth);
	if (SW != nullptr)
		successful &= SW->Propagate(SWconstraints, depth);
	return successful;
}
void CellPtr::Reset() { newCell = *cell; }
void CellPtr::Apply() { *cell = newCell; }
auto CellPtr::operator->() -> Cell* { return this->cell; }

Chunk::Chunk(const std::vector<Cell*>& data, const int size) : size(size)
{
	this->area = data // NOLINTNEXTLINE
				 | rv::transform([](auto& cell) { return CellPtr(cell); })
				 | r::to<std::vector<CellPtr>>();
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
void Chunk::Reset()
{
	for (auto& cell : this->area)
	{
		cell.Reset();
	}
	std::random_device rd{}; // NOLINT
	std::mt19937 rand{rd()};
	// Collapse any cells that are already collapsed to properly propagate
	// changes made by adjacent chunks
	for (auto& cell : this->area)
	{
		if (cell->GetEntropy() == 1)
		{
			cell->Collapse(rand);
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
		  | rv::filter([](auto& cell) { return !cell->isDone; }) // NOLINT
		  | rv::transform([](auto& cell) { return &cell; })		 // NOLINT
		  | r::to<std::vector<CellPtr*>>();
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
		= sorted | rv::take_while(GetLowest) | r::to<std::vector<CellPtr*>>();
	std::uniform_int_distribution<> distrib(0, lowest.size() - 1);
	CellPtr& toCollapse = *lowest[distrib(rand)];

	auto selectedTile = toCollapse.Collapse(rand);

	if (!toCollapse.Propagate({selectedTile.ID}, MAXPROPAGATION))
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
auto Chunk::CheckDone() const -> bool { return this->isDone; }

auto Chunk::CellAt(const int x, const int y) -> CellPtr*
{
	if (x < 0 || x >= this->size)
		return nullptr;
	if (y < 0 || y >= this->size)
		return nullptr;

	return &area[(x * this->size) + y];
}

ChunkPtr::ChunkPtr(std::array<Chunk, 4> chunks) : chunks(std::move(chunks)) { }
auto ChunkPtr::Next() -> bool
{
	index++;
	return index >= 4;
};
auto ChunkPtr::operator->() -> Chunk* { return &this->chunks[this->index]; }
uint8_t ChunkPtr::index = 0;

Generator::Generator(const int size, const int chunkCount) :
	grid(size * size, testTiles), size(size)
{
	this->gen = std::mt19937(rd());
	int chunkSize = (size / chunkCount) + (chunkCount - 1);
	int chunkStride = (size / chunkCount) - 1;

	for (auto [xc, yc] : rv::cartesian_product(rv::iota(0, chunkCount / 2),
											   rv::iota(0, chunkCount / 2)))
	{
		xc *= 2;
		yc *= 2;
		std::vector<Cell*> chunk1;
		std::vector<Cell*> chunk2;
		std::vector<Cell*> chunk3;
		std::vector<Cell*> chunk4;
		int start1 = ((xc * chunkStride) * size) + (yc * chunkStride);
		int start2 = (((xc + 1) * chunkStride) * size) + (yc * chunkStride);
		int start3 = ((xc * chunkStride) * size) + ((yc + 1) * chunkStride);
		int start4
			= (((xc + 1) * chunkStride) * size) + ((yc + 1) * chunkStride);
		for (auto [x, y] : rv::cartesian_product(rv::iota(0, chunkSize),
												 rv::iota(0, chunkSize)))
		{
			bool isInit
				= (this->grid[start1 + ((x * size) + y)].id.x < 1.0f)
				  && (this->grid[start1 + ((x * size) + y)].id.y < 1.0f);
			if (isInit)
				this->grid[start1 + ((x * size) + y)].id
					= {.x = static_cast<float>(x + (xc * chunkStride)),
					   .y = static_cast<float>(y + (yc * chunkStride))};
			chunk1.push_back(&this->grid[start1 + ((x * size) + y)]);
			chunk2.push_back(&this->grid[start2 + ((x * size) + y)]);
			chunk3.push_back(&this->grid[start3 + ((x * size) + y)]);
			chunk4.push_back(&this->grid[start4 + ((x * size) + y)]);
		}
		this->ptrs.push_back(ChunkPtr({{{chunk1, chunkSize},
										{chunk2, chunkSize},
										{chunk3, chunkSize},
										{chunk4, chunkSize}}}));
	}
}
void Generator::Run()
{
	bool done{false};
	auto onSync = [&done]() noexcept -> void
	{
		done = ChunkPtr::Next();
	};
	std::barrier syncPoint(this->ptrs.size(), onSync);
	auto task = [&done, this, &syncPoint](ChunkPtr* ptr) -> void
	{
		while (!done)
		{
			(*ptr)->Reset();
			while (true)
			{
				while (!(*ptr)->Step(this->gen))
					;
				if ((*ptr)->CheckDone())
					break;
			}
			syncPoint.arrive_and_wait();
		}
	};
	std::vector<std::jthread> threads;
	threads.reserve(this->ptrs.size());
	for (auto& ptr : this->ptrs)
		threads.emplace_back(task, &ptr);
}
auto Generator::Step() -> bool
{
	bool done{true};
	for (auto& chunk : this->ptrs)
	{
		while (!chunk->Step(this->gen))
			;
		done &= chunk->CheckDone();
	}
	return done;
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
