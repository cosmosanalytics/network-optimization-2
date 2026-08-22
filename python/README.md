# Network Optimization 2 (Python) — Truck-Load Packing as a Mixed-Integer Program

Python 3 port of the `netopt2` C++ project — a truck-load packing /
capacitated bin-packing solver: assign shipments to trucks to minimize the
number of trucks used, subject to per-truck weight and volume capacity.

## Problem

Given a set of shipments (each with a weight and a volume) and a fleet of
truck types (each with a weight capacity, a volume capacity, and a cost per
trip), assign every shipment to exactly one truck instance so that:

- No truck instance exceeds its weight or volume capacity.
- The total cost (number of trucks used × cost per trip) is minimized.

This is a classic **bin-packing / vehicle-loading mixed-integer program**:

```
minimize   sum_k  cost_k * y_k
subject to sum_i  w_i * x_ik <= W_k * y_k        for every truck k
           sum_i  v_i * x_ik <= V_k * y_k        for every truck k
           sum_k  x_ik = 1                        for every shipment i
           x_ik, y_k in {0, 1}
```

where `x_ik = 1` if shipment `i` is assigned to truck `k`, and `y_k = 1` if
truck `k` is used at all.

## Design

- `netopt2/shipment.py`, `netopt2/truck.py` — plain dataclasses describing
  the problem instance.
- `netopt2/problem.py` — `PackingProblem` owns the shipment list and the
  available truck type, plus `Bin`/`PackingSolution`. `validate()`
  independently recomputes feasibility and cost for any candidate solution
  from scratch (capacity checks, duplicate/missing-shipment checks) — it
  never trusts what a solver claims about its own output.
- `netopt2/solver.py` — `PackingSolver` abstract base class (Strategy
  pattern), so the algorithm used to solve an instance can be swapped
  without touching calling code.
- `netopt2/greedy_solver.py` — `GreedyFirstFitDecreasingSolver`, a fast
  first-fit-decreasing heuristic upper bound.
- `netopt2/exact_solver.py` — `BranchAndBoundSolver`, an exact solver for
  small/medium instances: seeds its incumbent from the greedy solver and
  prunes with a `ceil(remaining weight / capacity)` lower bound.
- `netopt2/pulp_solver.py` — `PuLPMipSolver`, the production-scale path.

The greedy and branch-and-bound solvers are **dependency-free** (Python
standard library only) and are what the test suite exercises by default.
This mirrors the C++ project's `BranchAndBoundSolver`, which ships as the
default so the project builds with no external dependencies.

`PuLPMipSolver` mirrors the C++ project's `CbcMipSolver.h` — a *documentary*
header there, compiled only behind a `USE_CBC` build flag because it needs
the COIN-OR CBC dev libraries. Here the equivalent step is just
`pip install pulp` (PuLP calls the bundled CBC binary under the hood). Same
decision variables (`x_ik`, `y_k`), same constraints, same objective. The
test that exercises it is automatically skipped if `pulp` is not installed,
so `netopt2` itself never requires it to be importable.

## Build & run

```bash
pip install -r requirements.txt   # optional: only needed for the PuLP/CBC solver and its test
python3 -m unittest discover -s tests -v
python3 main.py
```

## Tests

`tests/test_packing.py` ports every case from the C++ suite with the same
hand-verified numbers:

- Feasibility checking (capacity constraints respected / violated).
- Correctness of the greedy solver on a known instance.
- Correctness of the branch-and-bound solver against a hand-verified
  optimum (3 trucks, matching the bin-packing lower bound exactly).
- Branch-and-bound never does worse than greedy on the same instance.
- Edge cases: a single oversized shipment, zero shipments, exact-capacity
  fit.
- (When `pulp` is installed) `PuLPMipSolver` matches the exact solver's
  cost on a small instance.
