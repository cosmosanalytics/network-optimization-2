#include "BranchAndBoundSolver.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace netopt2 {

PackingSolution BranchAndBoundSolver::solve(const PackingProblem& problem) {
    if (static_cast<int>(problem.shipments().size()) > maxShipments_) {
        throw std::invalid_argument(
            "BranchAndBoundSolver: instance too large for exact search ("
            + std::to_string(problem.shipments().size()) + " shipments, limit "
            + std::to_string(maxShipments_) + "). Use CbcMipSolver or "
            "GreedyFirstFitDecreasingSolver instead.");
    }

    // Seed the incumbent with the fast heuristic so pruning is effective
    // from the very first branch-and-bound node.
    GreedyFirstFitDecreasingSolver greedy;
    PackingSolution best = greedy.solve(problem);

    std::vector<Shipment> sorted = problem.shipments();
    std::sort(sorted.begin(), sorted.end(), [](const Shipment& a, const Shipment& b) {
        return a.weight() > b.weight();
    });

    std::vector<Bin> openBins;
    recurse(problem, sorted, 0, openBins, best);

    problem.validate(best);
    return best;
}

void BranchAndBoundSolver::recurse(const PackingProblem& problem,
                            const std::vector<Shipment>& sortedShipments,
                            std::size_t index,
                            std::vector<Bin>& openBins,
                            PackingSolution& best) {
    if (index == sortedShipments.size()) {
        if (openBins.size() < best.bins.size()) {
            best.bins = openBins;
        }
        return;
    }

    // Lower bound: bins already open, plus the minimum number of additional
    // bins the remaining (unplaced) shipments could possibly fit into.
    double remainingWeight = 0.0;
    for (std::size_t i = index; i < sortedShipments.size(); ++i) {
        remainingWeight += sortedShipments[i].weight();
    }
    const double capacity = problem.truckType().weightCapacity();
    const int lowerBoundAdditional =
        capacity > 0.0 ? static_cast<int>(std::ceil(remainingWeight / capacity - 1e-9)) : 0;

    if (static_cast<int>(openBins.size()) + lowerBoundAdditional >=
        static_cast<int>(best.bins.size())) {
        return; // cannot possibly beat the incumbent from here — prune
    }

    const Shipment& shipment = sortedShipments[index];

    // Branch 1..k: try placing the shipment into each already-open bin.
    // Simple symmetry-break: skip a bin whose (weight, volume) load exactly
    // matches a bin we already tried and rejected/accepted at this level,
    // since trying it again cannot produce a new distinct solution.
    double lastTriedWeight = -1.0, lastTriedVolume = -1.0;
    for (Bin& bin : openBins) {
        if (bin.usedWeight == lastTriedWeight && bin.usedVolume == lastTriedVolume) {
            continue;
        }
        lastTriedWeight = bin.usedWeight;
        lastTriedVolume = bin.usedVolume;

        const double newWeight = bin.usedWeight + shipment.weight();
        const double newVolume = bin.usedVolume + shipment.volume();
        const Truck& truck = problem.truckType();
        if (newWeight <= truck.weightCapacity() + 1e-9 &&
            newVolume <= truck.volumeCapacity() + 1e-9) {
            bin.shipmentIds.push_back(shipment.id());
            const double prevWeight = bin.usedWeight, prevVolume = bin.usedVolume;
            bin.usedWeight = newWeight;
            bin.usedVolume = newVolume;

            recurse(problem, sortedShipments, index + 1, openBins, best);

            bin.shipmentIds.pop_back();
            bin.usedWeight = prevWeight;
            bin.usedVolume = prevVolume;
        }
    }

    // Branch k+1: open a brand-new bin for this shipment.
    Bin fresh;
    fresh.shipmentIds.push_back(shipment.id());
    fresh.usedWeight = shipment.weight();
    fresh.usedVolume = shipment.volume();
    openBins.push_back(std::move(fresh));

    recurse(problem, sortedShipments, index + 1, openBins, best);

    openBins.pop_back();
}

} // namespace netopt2
