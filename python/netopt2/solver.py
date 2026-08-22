"""Port of PackingSolver.h -- strategy interface for packing algorithms."""

from abc import ABC, abstractmethod

from .problem import PackingProblem, PackingSolution


class PackingSolver(ABC):
    """Any algorithm capable of solving a PackingProblem implements this.

    Lets callers swap solvers (greedy heuristic, exact branch-and-bound, or
    an external MIP solver such as CBC/PuLP) without changing calling code.
    """

    @abstractmethod
    def solve(self, problem: PackingProblem) -> PackingSolution:
        ...

    @abstractmethod
    def name(self) -> str:
        ...
