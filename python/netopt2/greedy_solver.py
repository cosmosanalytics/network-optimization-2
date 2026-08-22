"""Port of GreedyFirstFitDecreasingSolver.h / .cpp."""

from .problem import Bin, PackingProblem, PackingSolution
from .solver import PackingSolver

_EPS = 1e-9


class GreedyFirstFitDecreasingSolver(PackingSolver):
    """Classic first-fit-decreasing bin-packing heuristic:

    1. Sort shipments by weight, largest first.
    2. For each shipment, place it in the first already-open bin it fits
       in (weight AND volume); if it fits in none, open a new bin.

    Fast (O(n log n + n * open_bins)) and gives a solution within a small,
    well-known factor of optimal -- used here both as a standalone fast
    solver and as the initial incumbent for BranchAndBoundSolver's pruning.
    """

    def solve(self, problem: PackingProblem) -> PackingSolution:
        sorted_shipments = sorted(problem.shipments, key=lambda s: s.weight, reverse=True)
        truck = problem.truck_type
        solution = PackingSolution()

        for s in sorted_shipments:
            placed = False
            for b in solution.bins:
                new_weight = b.used_weight + s.weight
                new_volume = b.used_volume + s.volume
                if (
                    new_weight <= truck.weight_capacity + _EPS
                    and new_volume <= truck.volume_capacity + _EPS
                ):
                    b.shipment_ids.append(s.id)
                    b.used_weight = new_weight
                    b.used_volume = new_volume
                    placed = True
                    break
            if not placed:
                solution.bins.append(
                    Bin(shipment_ids=[s.id], used_weight=s.weight, used_volume=s.volume)
                )

        problem.validate(solution)
        return solution

    def name(self) -> str:
        return "GreedyFirstFitDecreasing"
