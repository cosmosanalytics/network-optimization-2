"""PuLP + CBC MIP solver -- mirrors CbcMipSolver.h's formulation.

Production-scale path: solves the same MIP the C++ project documents via
COIN-OR CBC's C++ API, here via PuLP's CBC binding.

Variables: x_ik in {0,1} (shipment i assigned to bin k), y_k in {0,1}
(bin k used). `max_bins` is an upper bound on bins needed -- the greedy
solution's bin count is a safe, tight default.

pulp is imported lazily inside solve() so importing the `netopt2` package
never requires pulp to be installed.
"""

from typing import Optional

from .greedy_solver import GreedyFirstFitDecreasingSolver
from .problem import Bin, PackingProblem, PackingSolution
from .solver import PackingSolver


class PuLPMipSolver(PackingSolver):
    def __init__(self, max_bins: Optional[int] = None) -> None:
        self._max_bins = max_bins

    def solve(self, problem: PackingProblem) -> PackingSolution:
        try:
            import pulp
        except ImportError as exc:
            raise RuntimeError(
                "PuLPMipSolver requires the 'pulp' package. Install it with "
                "`pip install pulp` (see requirements.txt)."
            ) from exc

        shipments = problem.shipments
        n = len(shipments)
        truck = problem.truck_type

        k_bins = self._max_bins
        if k_bins is None:
            k_bins = GreedyFirstFitDecreasingSolver().solve(problem).truck_count()
        k_bins = max(k_bins, 1)

        model = pulp.LpProblem("netopt2_bin_packing", pulp.LpMinimize)

        x = {
            (i, k): pulp.LpVariable(f"x_{i}_{k}", cat="Binary")
            for i in range(n)
            for k in range(k_bins)
        }
        y = {k: pulp.LpVariable(f"y_{k}", cat="Binary") for k in range(k_bins)}

        model += pulp.lpSum(truck.cost_per_trip * y[k] for k in range(k_bins))

        # sum_k x_ik = 1 for every shipment i.
        for i in range(n):
            model += pulp.lpSum(x[(i, k)] for k in range(k_bins)) == 1

        # sum_i w_i*x_ik <= W*y_k, and same for volume, per bin k.
        for k in range(k_bins):
            model += (
                pulp.lpSum(shipments[i].weight * x[(i, k)] for i in range(n))
                <= truck.weight_capacity * y[k]
            )
            model += (
                pulp.lpSum(shipments[i].volume * x[(i, k)] for i in range(n))
                <= truck.volume_capacity * y[k]
            )

        model.solve(pulp.PULP_CBC_CMD(msg=False))

        result = PackingSolution()
        result.bins = [Bin() for _ in range(k_bins)]
        for i in range(n):
            for k in range(k_bins):
                if pulp.value(x[(i, k)]) > 0.5:
                    result.bins[k].shipment_ids.append(shipments[i].id)

        # Drop empty bins, then validate/cost the rest.
        result.bins = [b for b in result.bins if b.shipment_ids]
        for b in result.bins:
            b.used_weight = sum(s.weight for s in shipments if s.id in b.shipment_ids)
            b.used_volume = sum(s.volume for s in shipments if s.id in b.shipment_ids)

        problem.validate(result)
        return result

    def name(self) -> str:
        return "PuLP-CBC-MIP"
