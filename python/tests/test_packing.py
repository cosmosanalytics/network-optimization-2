"""Port of tests/PackingTests.cpp -- every case, same hand-verified numbers."""

import math
import unittest
from typing import List

from netopt2 import (
    BranchAndBoundSolver,
    GreedyFirstFitDecreasingSolver,
    PackingProblem,
    Shipment,
    Truck,
)

try:
    import pulp  # noqa: F401

    PULP_AVAILABLE = True
except ImportError:
    PULP_AVAILABLE = False


def make_truck(weight_capacity: float, volume_capacity: float = 1000.0) -> Truck:
    """A truck with generous volume capacity, so these tests exercise the
    weight constraint in isolation unless stated otherwise."""
    return Truck(
        type_id=1,
        name="StandardTruck",
        weight_capacity=weight_capacity,
        volume_capacity=volume_capacity,
        cost_per_trip=250.0,
    )


def lower_bound_bins(shipments: List[Shipment], capacity: float) -> int:
    total = sum(s.weight for s in shipments)
    return math.ceil(total / capacity - 1e-9)


class PackingTests(unittest.TestCase):
    def test_feasibility_respects_capacity(self):
        shipments = [Shipment(1, 4.0, 1.0), Shipment(2, 5.0, 1.0)]
        problem = PackingProblem(shipments, make_truck(10.0))
        self.assertTrue(problem.fits_in_one_bin([1, 2]))  # 4 + 5 = 9 <= 10

    def test_feasibility_detects_violation(self):
        shipments = [Shipment(1, 6.0, 1.0), Shipment(2, 7.0, 1.0)]
        problem = PackingProblem(shipments, make_truck(10.0))
        self.assertFalse(problem.fits_in_one_bin([1, 2]))  # 6 + 7 = 13 > 10

    def test_greedy_produces_feasible_solution(self):
        shipments = [
            Shipment(1, 6.0, 1.0), Shipment(2, 5.0, 1.0), Shipment(3, 4.0, 1.0),
            Shipment(4, 3.0, 1.0), Shipment(5, 2.0, 1.0), Shipment(6, 2.0, 1.0),
        ]
        problem = PackingProblem(shipments, make_truck(10.0))
        solver = GreedyFirstFitDecreasingSolver()
        solution = solver.solve(problem)

        self.assertTrue(solution.feasible)
        self.assertGreaterEqual(solution.truck_count(), lower_bound_bins(shipments, 10.0))

    def test_branch_and_bound_achieves_known_optimum(self):
        # Hand-verified optimum: {6,4}=10, {5,3,2}=10, {2} -> 3 bins, matching
        # the ceil(total weight / capacity) lower bound exactly, so 3 is
        # provably optimal (not just "as good as we found").
        shipments = [
            Shipment(1, 6.0, 1.0), Shipment(2, 5.0, 1.0), Shipment(3, 4.0, 1.0),
            Shipment(4, 3.0, 1.0), Shipment(5, 2.0, 1.0), Shipment(6, 2.0, 1.0),
        ]
        problem = PackingProblem(shipments, make_truck(10.0))
        solver = BranchAndBoundSolver()
        solution = solver.solve(problem)

        self.assertTrue(solution.feasible)
        self.assertEqual(solution.truck_count(), 3)
        self.assertEqual(solution.truck_count(), lower_bound_bins(shipments, 10.0))

    def test_branch_and_bound_never_worse_than_greedy(self):
        shipments = [
            Shipment(1, 8.0, 1.0), Shipment(2, 7.0, 1.0), Shipment(3, 6.0, 1.0),
            Shipment(4, 5.0, 1.0), Shipment(5, 4.0, 1.0), Shipment(6, 3.0, 1.0),
            Shipment(7, 2.0, 1.0),
        ]
        problem = PackingProblem(shipments, make_truck(12.0))

        greedy = GreedyFirstFitDecreasingSolver()
        exact = BranchAndBoundSolver()
        greedy_solution = greedy.solve(problem)
        exact_solution = exact.solve(problem)

        self.assertTrue(greedy_solution.feasible)
        self.assertTrue(exact_solution.feasible)
        self.assertLessEqual(exact_solution.truck_count(), greedy_solution.truck_count())

    def test_edge_case_single_oversized_shipment_is_infeasible(self):
        shipments = [Shipment(1, 15.0, 1.0)]
        problem = PackingProblem(shipments, make_truck(10.0))
        self.assertFalse(problem.fits_in_one_bin([1]))  # no truck can ever carry this alone

    def test_edge_case_zero_shipments(self):
        shipments: List[Shipment] = []
        problem = PackingProblem(shipments, make_truck(10.0))

        greedy = GreedyFirstFitDecreasingSolver()
        solution = greedy.solve(problem)

        self.assertTrue(solution.feasible)
        self.assertEqual(solution.truck_count(), 0)
        self.assertEqual(solution.total_cost, 0.0)

    def test_edge_case_exact_capacity_fit(self):
        shipments = [Shipment(1, 10.0, 1.0)]
        problem = PackingProblem(shipments, make_truck(10.0))
        self.assertTrue(problem.fits_in_one_bin([1]))  # exactly at capacity should still fit

    @unittest.skipUnless(PULP_AVAILABLE, "pulp not installed")
    def test_pulp_solver_matches_exact_solver_cost(self):
        from netopt2.pulp_solver import PuLPMipSolver

        shipments = [
            Shipment(1, 6.0, 1.0), Shipment(2, 5.0, 1.0), Shipment(3, 4.0, 1.0),
            Shipment(4, 3.0, 1.0), Shipment(5, 2.0, 1.0), Shipment(6, 2.0, 1.0),
        ]
        problem = PackingProblem(shipments, make_truck(10.0))

        exact_solution = BranchAndBoundSolver().solve(problem)
        pulp_solution = PuLPMipSolver().solve(problem)

        self.assertTrue(pulp_solution.feasible)
        self.assertAlmostEqual(pulp_solution.total_cost, exact_solution.total_cost)


if __name__ == "__main__":
    unittest.main()
