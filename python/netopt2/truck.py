"""Port of Truck.h."""

from dataclasses import dataclass


@dataclass(frozen=True)
class Truck:
    """A truck TYPE available to the solver.

    The solver may instantiate as many copies ("truck instances") of a
    given type as needed; `cost_per_trip` is charged once per instance
    actually used.
    """

    type_id: int
    name: str
    weight_capacity: float
    volume_capacity: float
    cost_per_trip: float
