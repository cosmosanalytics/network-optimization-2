#include <cmath>
#include <vector>

#include "BranchAndBoundSolver.h"
#include "GreedyFirstFitDecreasingSolver.h"
#include "PackingProblem.h"
#include "TestFramework.h"

using namespace netopt2;

namespace {

// A truck with generous volume capacity, so these tests exercise the
// weight constraint in isolation unless stated otherwise.
Truck makeTruck(double weightCapacity, double volumeCapacity = 1000.0) {
    return Truck(/*typeId=*/1, "StandardTruck", weightCapacity, volumeCapacity,
                 /*costPerTrip=*/250.0);
}

int lowerBoundBins(const std::vector<Shipment>& shipments, double capacity) {
    double total = 0.0;
    for (const auto& s : shipments) total += s.weight();
    return static_cast<int>(std::ceil(total / capacity - 1e-9));
}

} // namespace

TEST(Feasibility_RespectsCapacity) {
    std::vector<Shipment> shipments = {Shipment(1, 4.0, 1.0), Shipment(2, 5.0, 1.0)};
    PackingProblem problem(shipments, makeTruck(10.0));
    CHECK(problem.fitsInOneBin({1, 2})); // 4 + 5 = 9 <= 10
}

TEST(Feasibility_DetectsViolation) {
    std::vector<Shipment> shipments = {Shipment(1, 6.0, 1.0), Shipment(2, 7.0, 1.0)};
    PackingProblem problem(shipments, makeTruck(10.0));
    CHECK(!problem.fitsInOneBin({1, 2})); // 6 + 7 = 13 > 10
}

TEST(Greedy_ProducesFeasibleSolution) {
    std::vector<Shipment> shipments = {
        Shipment(1, 6.0, 1.0), Shipment(2, 5.0, 1.0), Shipment(3, 4.0, 1.0),
        Shipment(4, 3.0, 1.0), Shipment(5, 2.0, 1.0), Shipment(6, 2.0, 1.0),
    };
    PackingProblem problem(shipments, makeTruck(10.0));
    GreedyFirstFitDecreasingSolver solver;
    PackingSolution solution = solver.solve(problem);

    CHECK(solution.feasible);
    CHECK(solution.truckCount() >= lowerBoundBins(shipments, 10.0));
}

TEST(BranchAndBound_AchievesKnownOptimum) {
    // Hand-verified optimum: {6,4}=10, {5,3,2}=10, {2} -> 3 bins, matching
    // the ceil(total weight / capacity) lower bound exactly, so 3 is
    // provably optimal (not just "as good as we found").
    std::vector<Shipment> shipments = {
        Shipment(1, 6.0, 1.0), Shipment(2, 5.0, 1.0), Shipment(3, 4.0, 1.0),
        Shipment(4, 3.0, 1.0), Shipment(5, 2.0, 1.0), Shipment(6, 2.0, 1.0),
    };
    PackingProblem problem(shipments, makeTruck(10.0));
    BranchAndBoundSolver solver;
    PackingSolution solution = solver.solve(problem);

    CHECK(solution.feasible);
    CHECK(solution.truckCount() == 3);
    CHECK(solution.truckCount() == lowerBoundBins(shipments, 10.0));
}

TEST(BranchAndBound_NeverWorseThanGreedy) {
    std::vector<Shipment> shipments = {
        Shipment(1, 8.0, 1.0), Shipment(2, 7.0, 1.0), Shipment(3, 6.0, 1.0),
        Shipment(4, 5.0, 1.0), Shipment(5, 4.0, 1.0), Shipment(6, 3.0, 1.0),
        Shipment(7, 2.0, 1.0),
    };
    PackingProblem problem(shipments, makeTruck(12.0));

    GreedyFirstFitDecreasingSolver greedy;
    BranchAndBoundSolver exact;
    PackingSolution greedySolution = greedy.solve(problem);
    PackingSolution exactSolution = exact.solve(problem);

    CHECK(greedySolution.feasible);
    CHECK(exactSolution.feasible);
    CHECK(exactSolution.truckCount() <= greedySolution.truckCount());
}

TEST(EdgeCase_SingleOversizedShipmentIsInfeasible) {
    std::vector<Shipment> shipments = {Shipment(1, 15.0, 1.0)};
    PackingProblem problem(shipments, makeTruck(10.0));
    CHECK(!problem.fitsInOneBin({1})); // no truck can ever carry this alone
}

TEST(EdgeCase_ZeroShipments) {
    std::vector<Shipment> shipments; // empty
    PackingProblem problem(shipments, makeTruck(10.0));

    GreedyFirstFitDecreasingSolver greedy;
    PackingSolution solution = greedy.solve(problem);

    CHECK(solution.feasible);
    CHECK(solution.truckCount() == 0);
    CHECK(solution.totalCost == 0.0);
}

TEST(EdgeCase_ExactCapacityFit) {
    std::vector<Shipment> shipments = {Shipment(1, 10.0, 1.0)};
    PackingProblem problem(shipments, makeTruck(10.0));
    CHECK(problem.fitsInOneBin({1})); // exactly at capacity should still fit
}
