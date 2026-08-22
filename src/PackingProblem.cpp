#include "PackingProblem.h"

#include <algorithm>
#include <set>
#include <stdexcept>

namespace netopt2 {

const Shipment& PackingProblem::shipmentById(int id) const {
    auto it = std::find_if(shipments_.begin(), shipments_.end(),
                            [id](const Shipment& s) { return s.id() == id; });
    if (it == shipments_.end()) {
        throw std::invalid_argument("Unknown shipment id: " + std::to_string(id));
    }
    return *it;
}

bool PackingProblem::fitsInOneBin(const std::vector<int>& shipmentIds) const {
    double totalWeight = 0.0;
    double totalVolume = 0.0;
    for (int id : shipmentIds) {
        const Shipment& s = shipmentById(id);
        totalWeight += s.weight();
        totalVolume += s.volume();
    }
    return totalWeight <= truckType_.weightCapacity() + 1e-9 &&
           totalVolume <= truckType_.volumeCapacity() + 1e-9;
}

bool PackingProblem::validate(PackingSolution& solution) const {
    // Every shipment must appear in exactly one bin.
    std::set<int> seen;
    for (const Bin& bin : solution.bins) {
        if (!fitsInOneBin(bin.shipmentIds)) {
            solution.feasible = false;
            return false;
        }
        for (int id : bin.shipmentIds) {
            if (!seen.insert(id).second) {
                solution.feasible = false; // duplicate assignment
                return false;
            }
        }
    }
    if (seen.size() != shipments_.size()) {
        solution.feasible = false; // some shipment left unassigned
        return false;
    }

    solution.totalCost = solution.truckCount() * truckType_.costPerTrip();
    solution.feasible = true;
    return true;
}

} // namespace netopt2
