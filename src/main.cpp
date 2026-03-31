#include "program.h"

auto main() -> int
{
	Program program;
	program.Run();

	// NOTE: Passint unit test thingy
	// Cell cell1{testTiles};
	// Cell cell2{testTiles};

	// CellRef ref1{&cell1};
	// CellRef ref2{&cell2};
	// ref1.N = &ref2;
	// ref2.S = &ref1;

	// std::mt19937 rand{0};
	// ref1.Collapse(rand);
	// ref1.Propogate({0}, 1000);

	// ref1.Apply();
	// ref2.Apply();
	// assert(cell2.GetPossibilities().size() == 2);
}
