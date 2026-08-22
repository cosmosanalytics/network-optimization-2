"""Port of PackingProblem.h / PackingProblem.cpp."""

from dataclasses import dataclass, field
from typing import List

from .shipment import Shipment
from .truck import Truck

_EPS = 1e-9


@dataclass
class Bin:
    """A single bin (one truck instance) and the shipments loaded onto it."""

    shipment_ids: List[int] = field(default_factory=list)
    used_weight: float = 0.0
    used_volume: float = 0.0


@dataclass
class PackingSolution:
    """The result of solving a PackingProblem."""

    bins: List[Bin] = field(default_factory=list)
    total_cost: float = 0.0
    feasible: bool = False

    def truck_count(self) -> int:
        return len(self.bins)


class PackingProblem:
    """Problem instance: shipments that all must be loaded, plus a single
    (homogeneous) truck type available in unlimited supply.

    Modeling a homogeneous fleet keeps this a textbook-exact bin-packing
    MIP, which is the version implemented exactly by BranchAndBoundSolver.
    A heterogeneous fleet is supported at the data-model level via Truck,
    and left to the greedy solver / external MIP backend rather than the
    exact solver.
    """

    def __init__(self, shipments: List[Shipment], truck_type: Truck) -> None:
        self._shipments = list(shipments)
        self._truck_type = truck_type
        self._by_id = {s.id: s for s in self._shipments}

    @property
    def shipments(self) -> List[Shipment]:
        return self._shipments

    @property
    def truck_type(self) -> Truck:
        return self._truck_type

    def _shipment_by_id(self, shipment_id: int) -> Shipment:
        try:
            return self._by_id[shipment_id]
        except KeyError:
            raise ValueError(f"Unknown shipment id: {shipment_id}") from None

    def fits_in_one_bin(self, shipment_ids: List[int]) -> bool:
        """True if the given group of shipment ids can legally share one
        truck instance (weight and volume capacity both respected)."""
        total_weight = 0.0
        total_volume = 0.0
        for sid in shipment_ids:
            s = self._shipment_by_id(sid)
            total_weight += s.weight
            total_volume += s.volume
        return (
            total_weight <= self._truck_type.weight_capacity + _EPS
            and total_volume <= self._truck_type.volume_capacity + _EPS
        )

    def validate(self, solution: PackingSolution) -> bool:
        """Validates a full solution: every shipment assigned exactly once,
        and every bin within capacity. Also (re)computes total_cost/feasible
        on the solution in place. Independently recomputes feasibility from
        scratch -- does not trust anything the solver claims."""
        seen = set()
        for b in solution.bins:
            if not self.fits_in_one_bin(b.shipment_ids):
                solution.feasible = False
                return False
            for sid in b.shipment_ids:
                if sid in seen:
                    solution.feasible = False  # duplicate assignment
                    return False
                seen.add(sid)

        if len(seen) != len(self._shipments):
            solution.feasible = False  # some shipment left unassigned
            return False

        solution.total_cost = solution.truck_count() * self._truck_type.cost_per_trip
        solution.feasible = True
        return True
