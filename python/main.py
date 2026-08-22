"""Demo script: mirrors src/main.cpp's sample data and prints a comparison
of the available solvers."""

from netopt2 import (
    BranchAndBoundSolver,
    GreedyFirstFitDecreasingSolver,
    PackingProblem,
    PackingSolution,
    Shipment,
    Truck,
)


def print_solution(solver_name: str, solution: PackingSolution) -> None:
    print(f"\n--- {solver_name} ---")
    print(f"Feasible: {'yes' if solution.feasible else 'no'}")
    print(f"Trucks used: {solution.truck_count()}")
    print(f"Total cost: ${solution.total_cost:.2f}")
    for k, b in enumerate(solution.bins, start=1):
        ids = ", ".join(str(i) for i in b.shipment_ids)
        print(f"  Truck {k} (load {b.used_weight:g}): shipments {{{ids}}}")


def main() -> None:
    # A representative daily shipment batch: weights in tons, volumes in m^3.
    shipments = [
        Shipment(101, 6.2, 12.0), Shipment(102, 5.1, 9.0), Shipment(103, 4.0, 7.5),
        Shipment(104, 3.3, 6.0), Shipment(105, 2.8, 5.0), Shipment(106, 2.1, 4.0),
        Shipment(107, 7.5, 14.0), Shipment(108, 1.9, 3.5), Shipment(109, 3.6, 6.5),
        Shipment(110, 5.5, 10.0),
    ]
    standard_truck = Truck(
        type_id=1, name="26ft Box Truck", weight_capacity=10.0,
        volume_capacity=20.0, cost_per_trip=275.0,
    )

    problem = PackingProblem(shipments, standard_truck)

    print("Network Optimization 2 (Python) -- Truck-Load Packing MIP")
    print(
        f"{len(shipments)} shipments, truck capacity "
        f"{standard_truck.weight_capacity}t / {standard_truck.volume_capacity}m^3, "
        f"${standard_truck.cost_per_trip}/trip"
    )

    greedy = GreedyFirstFitDecreasingSolver()
    print_solution(greedy.name(), greedy.solve(problem))

    exact = BranchAndBoundSolver()
    print_solution(exact.name() + " (exact)", exact.solve(problem))

    try:
        from netopt2.pulp_solver import PuLPMipSolver

        import pulp  # noqa: F401
    except ImportError:
        print("\n--- PuLP-CBC-MIP ---")
        print("Skipped: pulp is not installed (pip install -r requirements.txt to enable).")
        return

    pulp_solver = PuLPMipSolver()
    print_solution(pulp_solver.name(), pulp_solver.solve(problem))


if __name__ == "__main__":
    main()
