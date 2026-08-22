#pragma once

#include <vector>

#include "GreedyFirstFitDecreasingSolver.h"
#include "PackingSolver.h"

namespace netopt2 {

// Exact solver for the homogeneous-fleet truck-load packing problem
// (equivalent to classic bin packing: minimize the number of bins used).
//
// Approach: depth-first branch-and-bound over "place shipment i into an
// already-open bin, or open a new one," processing shipments largest-first.
//
//   - Incumbent: seeded with GreedyFirstFitDecreasingSolver's result, so the
//     search starts with a good upper bound and prunes aggressively from
//     the first node onward.
//   - Lower bound at each node: (bins already open) + ceil(remaining total
//     weight / truck capacity) — a standard, valid bin-packing bound. A
//     branch is pruned as soon as this bound cannot beat the incumbent.
//
// Complexity is exponential in the worst case (this is an NP-hard problem),
// so this implementation is intended for small/medium instances (roughly
// up to a few dozen shipments) — exactly the scale of a single truck-route
// planning decision, as opposed to a whole network's worth of shipments at
// once. `maxShipments` guards against accidentally running it on something
// much larger; see CbcMipSolver.h for the production-scale MIP-solver path.
class BranchAndBoundSolver : public PackingSolver {
public:
    explicit BranchAndBoundSolver(int maxShipments = 40) : maxShipments_(maxShipments) {}

    PackingSolution solve(const PackingProblem& problem) override;
    std::string name() const override { return "BranchAndBound"; }

private:
    void recurse(const PackingProblem& problem,
                 const std::vector<Shipment>& sortedShipments,
                 std::size_t index,
                 std::vector<Bin>& openBins,
                 PackingSolution& best);

    int maxShipments_;
};

} // namespace netopt2
