"""Port of BranchAndBoundSolver.h / .cpp.

Exact solver for the homogeneous-fleet truck-load packing problem
(equivalent to classic bin packing: minimize the number of bins used).

Approach: depth-first branch-and-bound over "place shipment i into an
already-open bin, or open a new one," processing shipments largest-first.

  - Incumbent: seeded with GreedyFirstFitDecreasingSolver's result, so the
    search starts with a good upper bound and prunes aggressively from the
    first node onward.
  - Lower bound at each node: (bins already open) + ceil(remaining total
    weight / truck capacity) -- a standard, valid bin-packing bound. A
    branch is pruned as soon as this bound cannot beat the incumbent.

Complexity is exponential in the worst case (this is an NP-hard problem),
so this is intended for small/medium instances (roughly up to a few dozen
shipments) -- exactly the scale of a single truck-route planning decision.
`max_shipments` guards against accidentally running it on something much
larger; see pulp_solver.py for the production-scale MIP-solver path.
"""

import copy
import math
from typing import List

from .greedy_solver import GreedyFirstFitDecreasingSolver
from .problem import Bin, PackingProblem, PackingSolution
from .shipment import Shipment
from .solver import PackingSolver

_EPS = 1e-9


class BranchAndBoundSolver(PackingSolver):
    def __init__(self, max_shipments: int = 40) -> None:
        self._max_shipments = max_shipments

    def solve(self, problem: PackingProblem) -> PackingSolution:
        if len(problem.shipments) > self._max_shipments:
            raise ValueError(
                "BranchAndBoundSolver: instance too large for exact search "
                f"({len(problem.shipments)} shipments, limit "
                f"{self._max_shipments}). Use pulp_solver or "
                "GreedyFirstFitDecreasingSolver instead."
            )

        # Seed the incumbent with the fast heuristic so pruning is
        # effective from the very first branch-and-bound node.
        greedy = GreedyFirstFitDecreasingSolver()
        best = greedy.solve(problem)

        sorted_shipments = sorted(problem.shipments, key=lambda s: s.weight, reverse=True)

        open_bins: List[Bin] = []
        self._recurse(problem, sorted_shipments, 0, open_bins, best)

        problem.validate(best)
        return best

    def _recurse(
        self,
        problem: PackingProblem,
        sorted_shipments: List[Shipment],
        index: int,
        open_bins: List[Bin],
        best: PackingSolution,
    ) -> None:
        if index == len(sorted_shipments):
            if len(open_bins) < len(best.bins):
                best.bins = copy.deepcopy(open_bins)
            return

        # Lower bound: bins already open, plus the minimum number of
        # additional bins the remaining (unplaced) shipments could
        # possibly fit into.
        remaining_weight = sum(s.weight for s in sorted_shipments[index:])
        capacity = problem.truck_type.weight_capacity
        if capacity > 0.0:
            lower_bound_additional = math.ceil(remaining_weight / capacity - _EPS)
        else:
            lower_bound_additional = 0

        if len(open_bins) + lower_bound_additional >= len(best.bins):
            return  # cannot possibly beat the incumbent from here -- prune

        shipment = sorted_shipments[index]

        # Branch 1..k: try placing the shipment into each already-open
        # bin. Simple symmetry-break: skip a bin whose (weight, volume)
        # load exactly matches a bin we already tried and
        # rejected/accepted at this level, since trying it again cannot
        # produce a new distinct solution.
        last_tried_weight = -1.0
        last_tried_volume = -1.0
        for b in open_bins:
            if b.used_weight == last_tried_weight and b.used_volume == last_tried_volume:
                continue
            last_tried_weight = b.used_weight
            last_tried_volume = b.used_volume

            new_weight = b.used_weight + shipment.weight
            new_volume = b.used_volume + shipment.volume
            truck = problem.truck_type
            if (
                new_weight <= truck.weight_capacity + _EPS
                and new_volume <= truck.volume_capacity + _EPS
            ):
                b.shipment_ids.append(shipment.id)
                prev_weight, prev_volume = b.used_weight, b.used_volume
                b.used_weight = new_weight
                b.used_volume = new_volume

                self._recurse(problem, sorted_shipments, index + 1, open_bins, best)

                b.shipment_ids.pop()
                b.used_weight = prev_weight
                b.used_volume = prev_volume

        # Branch k+1: open a brand-new bin for this shipment.
        fresh = Bin(shipment_ids=[shipment.id], used_weight=shipment.weight,
                    used_volume=shipment.volume)
        open_bins.append(fresh)

        self._recurse(problem, sorted_shipments, index + 1, open_bins, best)

        open_bins.pop()

    def name(self) -> str:
        return "BranchAndBound"
