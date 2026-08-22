#pragma once

// CbcMipSolver -- production-scale path using COIN-OR CBC's C++ API.
// BranchAndBoundSolver.h is the zero-dependency exact solver used by
// default; this documents how the same model maps onto CBC (the solver
// the Python version of this project actually uses via COIN-OR/CBC).
//
// Compiled only when NETOPT2_USE_CBC is defined (CMakeLists.txt's
// USE_CBC option), since it needs the COIN-OR CBC dev libraries:
//   sudo apt-get install coinor-libcbc-dev coinor-libclp-dev \
//                         coinor-libosi-dev coinor-libcoinutils-dev
//   cmake -DUSE_CBC=ON -B build && cmake --build build

#ifdef NETOPT2_USE_CBC

#include <CbcModel.hpp>
#include <CoinPackedMatrix.hpp>
#include <OsiClpSolverInterface.hpp>

#include "PackingSolver.h"

namespace netopt2 {

// Solves PackingProblem as the MIP formulation in the README, via CBC's
// Open Solver Interface (OSI). Variables: x_ik in {0,1} (shipment i in
// bin k), y_k in {0,1} (bin k used). maxBins is an upper bound on bins
// needed (the greedy solution's bin count is a safe, tight choice).
class CbcMipSolver : public PackingSolver {
public:
    explicit CbcMipSolver(int maxBins) : maxBins_(maxBins) {}

    PackingSolution solve(const PackingProblem& problem) override {
        const auto& shipments = problem.shipments();
        const int n = static_cast<int>(shipments.size());
        const int K = maxBins_;

        const int numXVars = n * K;
        const int numVars = numXVars + K;
        auto xIndex = [K](int i, int k) { return i * K + k; };

        OsiClpSolverInterface solver;

        std::vector<double> objective(numVars, 0.0);
        for (int k = 0; k < K; ++k) {
            objective[numXVars + k] = problem.truckType().costPerTrip();
        }

        std::vector<double> colLower(numVars, 0.0);
        std::vector<double> colUpper(numVars, 1.0);

        CoinPackedMatrix matrix(false, 0, 0);
        matrix.setDimensions(0, numVars);

        std::vector<double> rowLower;
        std::vector<double> rowUpper;

        // sum_k x_ik = 1 for every shipment i.
        for (int i = 0; i < n; ++i) {
            CoinPackedVector row;
            for (int k = 0; k < K; ++k) row.insert(xIndex(i, k), 1.0);
            matrix.appendRow(row);
            rowLower.push_back(1.0);
            rowUpper.push_back(1.0);
        }

        // sum_i w_i*x_ik - W*y_k <= 0, and same for volume, per bin k.
        for (int k = 0; k < K; ++k) {
            CoinPackedVector weightRow, volumeRow;
            for (int i = 0; i < n; ++i) {
                weightRow.insert(xIndex(i, k), shipments[i].weight());
                volumeRow.insert(xIndex(i, k), shipments[i].volume());
            }
            weightRow.insert(numXVars + k, -problem.truckType().weightCapacity());
            volumeRow.insert(numXVars + k, -problem.truckType().volumeCapacity());
            matrix.appendRow(weightRow);
            matrix.appendRow(volumeRow);
            rowLower.push_back(-COIN_DBL_MAX);
            rowUpper.push_back(0.0);
            rowLower.push_back(-COIN_DBL_MAX);
            rowUpper.push_back(0.0);
        }

        solver.loadProblem(matrix, colLower.data(), colUpper.data(),
                            objective.data(), rowLower.data(), rowUpper.data());
        for (int v = 0; v < numVars; ++v) solver.setInteger(v);

        CbcModel model(solver);
        model.setLogLevel(0);
        model.branchAndBound();

        PackingSolution result;
        result.bins.resize(K);
        const double* sol = model.solver()->getColSolution();
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < K; ++k) {
                if (sol[xIndex(i, k)] > 0.5) {
                    result.bins[k].shipmentIds.push_back(shipments[i].id());
                }
            }
        }
        // Drop empty bins, then validate/cost the rest.
        result.bins.erase(
            std::remove_if(result.bins.begin(), result.bins.end(),
                            [](const Bin& b) { return b.shipmentIds.empty(); }),
            result.bins.end());
        problem.validate(result);
        return result;
    }

    std::string name() const override { return "Cbc-MIP"; }

private:
    int maxBins_;
};

} // namespace netopt2

#endif // NETOPT2_USE_CBC
