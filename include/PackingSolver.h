#pragma once

#include <string>

#include "PackingProblem.h"

namespace netopt2 {

// Strategy interface: any algorithm capable of solving a PackingProblem
// implements this. Lets the demo / tests swap solvers (greedy heuristic,
// exact branch-and-bound, or an external MIP solver such as CBC) without
// changing any calling code.
class PackingSolver {
public:
    virtual ~PackingSolver() = default;
    virtual PackingSolution solve(const PackingProblem& problem) = 0;
    virtual std::string name() const = 0;
};

} // namespace netopt2
