# Network Optimization 2 (C++) — Truck-Load Packing as a Mixed-Integer Program

This project re-implements, in C++, the truck-load packing / logistics optimization
model described on my resume as **"Network Optimization 2"** (originally built in
Python using COIN-OR/CBC, GLPK, and PuLP). The goal of this rewrite is to demonstrate
hands-on C++ proficiency — object-oriented design, data structures and algorithms,
and unit testing — applied to a real optimization problem I have already formulated,
solved, and deployed, rather than a generic textbook exercise.

## Problem

Given a set of shipments (each with a weight and a volume) and a fleet of truck
types (each with a weight capacity, a volume capacity, and a cost per trip),
assign every shipment to exactly one truck instance so that:

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

where `x_ik = 1` if shipment `i` is assigned to truck `k`, and `y_k = 1` if truck
`k` is used at all.

## Design

- `Shipment`, `Truck` — plain value types describing the problem instance.
- `PackingProblem` — owns the shipment list and available truck types, and
  validates a candidate solution against the capacity constraints.
- `PackingSolver` — abstract interface (**Strategy pattern**), so the exact
  algorithm used to solve an instance can be swapped without touching the rest
  of the codebase. Two implementations are provided:
  - `GreedyFirstFitDecreasingSolver` — a fast heuristic upper bound.
  - `BranchAndBoundSolver` — an exact solver for small/medium instances,
    using the greedy solution as an initial incumbent and a
    continuous-relaxation lower bound (`total weight / truck capacity`,
    rounded up) to prune the search tree.

## Solver backends

This repository ships with the from-scratch `BranchAndBoundSolver` above so
the project builds and runs with **no external dependencies** — useful for
review on any machine with just a C++17 compiler and CMake.

In production, the same `PackingProblem` model is designed to be solved with a
commercial-grade MIP solver instead. A `CbcMipSolver` implementation (see
`include/CbcMipSolver.h`) is included as a header showing how the exact same
problem would be handed to **COIN-OR CBC's C++ API** (`OsiClpSolverInterface` +
`CbcModel`) — this mirrors how the equivalent Python version of this model
uses CBC/GLPK/PuLP. To build it, install the COIN-OR development libraries
and enable it in `CMakeLists.txt` (instructions inline):

```bash
# Debian/Ubuntu
sudo apt-get install coinor-libcbc-dev coinor-libclp-dev coinor-libosi-dev coinor-libcoinutils-dev
cmake -DUSE_CBC=ON -B build && cmake --build build
```

## Build & run

```bash
cmake -B build
cmake --build build
./build/network_opt2_demo        # runs a sample instance, prints the assignment
./build/network_opt2_tests       # runs the unit test suite
```

## Tests

`tests/` contains a small, dependency-free unit test harness (no external
test framework required — again, so this builds anywhere) covering:

- Feasibility checking (capacity constraints respected / violated).
- Correctness of the greedy solver on a known instance.
- Correctness of the branch-and-bound solver against a hand-verified optimum.
- Edge cases: a single oversized shipment, zero shipments, exact-capacity fit.
