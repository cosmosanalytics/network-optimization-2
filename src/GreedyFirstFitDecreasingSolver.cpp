#include "GreedyFirstFitDecreasingSolver.h"

#include <algorithm>

namespace netopt2 {

PackingSolution GreedyFirstFitDecreasingSolver::solve(const PackingProblem& problem) {
    std::vector<Shipment> sorted = problem.shipments();
    std::sort(sorted.begin(), sorted.end(), [](const Shipment& a, const Shipment& b) {
        return a.weight() > b.weight();
    });

    const Truck& truck = problem.truckType();
    PackingSolution solution;

    for (const Shipment& s : sorted) {
        bool placed = false;
        for (Bin& bin : solution.bins) {
            const double newWeight = bin.usedWeight + s.weight();
            const double newVolume = bin.usedVolume + s.volume();
            if (newWeight <= truck.weightCapacity() + 1e-9 &&
                newVolume <= truck.volumeCapacity() + 1e-9) {
                bin.shipmentIds.push_back(s.id());
                bin.usedWeight = newWeight;
                bin.usedVolume = newVolume;
                placed = true;
                break;
            }
        }
        if (!placed) {
            Bin bin;
            bin.shipmentIds.push_back(s.id());
            bin.usedWeight = s.weight();
            bin.usedVolume = s.volume();
            solution.bins.push_back(std::move(bin));
        }
    }

    problem.validate(solution);
    return solution;
}

} // namespace netopt2
