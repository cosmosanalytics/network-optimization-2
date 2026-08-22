#include <iomanip>
#include <iostream>
#include <vector>

#include "BranchAndBoundSolver.h"
#include "GreedyFirstFitDecreasingSolver.h"
#include "PackingProblem.h"

using namespace netopt2;

namespace {

void printSolution(const std::string& solverName, const PackingSolution& solution) {
    std::cout << "\n--- " << solverName << " ---\n";
    std::cout << "Feasible: " << (solution.feasible ? "yes" : "no") << "\n";
    std::cout << "Trucks used: " << solution.truckCount() << "\n";
    std::cout << "Total cost: $" << std::fixed << std::setprecision(2)
              << solution.totalCost << "\n";
    for (std::size_t k = 0; k < solution.bins.size(); ++k) {
        const Bin& bin = solution.bins[k];
        std::cout << "  Truck " << (k + 1) << " (load " << bin.usedWeight << "): shipments {";
        for (std::size_t i = 0; i < bin.shipmentIds.size(); ++i) {
            std::cout << bin.shipmentIds[i] << (i + 1 < bin.shipmentIds.size() ? ", " : "");
        }
        std::cout << "}\n";
    }
}

} // namespace

int main() {
    // A representative daily shipment batch: weights in tons, volumes in m^3.
    std::vector<Shipment> shipments = {
        Shipment(101, 6.2, 12.0), Shipment(102, 5.1, 9.0),  Shipment(103, 4.0, 7.5),
        Shipment(104, 3.3, 6.0),  Shipment(105, 2.8, 5.0),  Shipment(106, 2.1, 4.0),
        Shipment(107, 7.5, 14.0), Shipment(108, 1.9, 3.5),  Shipment(109, 3.6, 6.5),
        Shipment(110, 5.5, 10.0),
    };
    Truck standardTruck(/*typeId=*/1, "26ft Box Truck", /*weightCapacity=*/10.0,
                         /*volumeCapacity=*/20.0, /*costPerTrip=*/275.0);

    PackingProblem problem(shipments, standardTruck);

    std::cout << "Network Optimization 2 (C++) — Truck-Load Packing MIP\n";
    std::cout << shipments.size() << " shipments, truck capacity "
              << standardTruck.weightCapacity() << "t / " << standardTruck.volumeCapacity()
              << "m^3, $" << standardTruck.costPerTrip() << "/trip\n";

    GreedyFirstFitDecreasingSolver greedy;
    printSolution(greedy.name(), greedy.solve(problem));

    BranchAndBoundSolver exact;
    printSolution(exact.name() + " (exact)", exact.solve(problem));

    return 0;
}
