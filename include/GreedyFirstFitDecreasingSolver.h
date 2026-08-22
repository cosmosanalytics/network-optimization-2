#pragma once

#include "PackingSolver.h"

namespace netopt2 {

// Classic first-fit-decreasing bin-packing heuristic:
//   1. Sort shipments by weight, largest first.
//   2. For each shipment, place it in the first already-open bin it fits
//      in (weight AND volume); if it fits in none, open a new bin.
//
// Fast (O(n log n + n * open_bins)) and gives a solution within a small,
// well-known factor of optimal — used here both as a standalone fast
// solver and as the initial incumbent for BranchAndBoundSolver's pruning.
class GreedyFirstFitDecreasingSolver : public PackingSolver {
public:
    PackingSolution solve(const PackingProblem& problem) override;
    std::string name() const override { return "GreedyFirstFitDecreasing"; }
};

} // namespace netopt2
