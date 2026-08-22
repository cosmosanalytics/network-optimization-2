"""Port of Shipment.h."""

from dataclasses import dataclass


@dataclass(frozen=True)
class Shipment:
    """A single shipment that must be loaded onto exactly one truck."""

    id: int
    weight: float
    volume: float
