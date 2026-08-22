#pragma once

#include <vector>

#include "Shipment.h"
#include "Truck.h"

namespace netopt2 {

// A single bin (one truck instance) and the shipments loaded onto it.
struct Bin {
    std::vector<int> shipmentIds;
    double usedWeight = 0.0;
    double usedVolume = 0.0;
};

// The result of solving a PackingProblem: a set of bins (truck instances)
// and the total cost of using them.
struct PackingSolution {
    std::vector<Bin> bins;
    double totalCost = 0.0;
    bool feasible = false;

    int truckCount() const { return static_cast<int>(bins.size()); }
};

// Problem instance: a list of shipments that all must be loaded, and a
// single (homogeneous) truck type available in unlimited supply. Modeling a
// homogeneous fleet keeps this a textbook-exact bin-packing MIP, which is
// the version implemented exactly by BranchAndBoundSolver. A heterogeneous
// fleet (multiple truck types) is supported at the data-model level via
// Truck, and is left to the greedy solver / external MIP backend (see
// CbcMipSolver.h) rather than the exact solver, since exact multi-bin-size
// bin packing needs a materially different bounding strategy.
class PackingProblem {
public:
    PackingProblem(std::vector<Shipment> shipments, Truck truckType)
        : shipments_(std::move(shipments)), truckType_(std::move(truckType)) {}

    const std::vector<Shipment>& shipments() const { return shipments_; }
    const Truck& truckType() const { return truckType_; }

    // True if the given group of shipment ids can legally share one truck
    // instance (weight and volume capacity both respected).
    bool fitsInOneBin(const std::vector<int>& shipmentIds) const;

    // Validates a full solution: every shipment assigned exactly once, and
    // every bin within capacity. Also (re)computes totalCost/feasible on the
    // solution in place.
    bool validate(PackingSolution& solution) const;

private:
    const Shipment& shipmentById(int id) const;

    std::vector<Shipment> shipments_;
    Truck truckType_;
};

} // namespace netopt2
