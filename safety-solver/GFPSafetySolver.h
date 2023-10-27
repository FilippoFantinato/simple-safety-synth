#ifndef GFP_SAFETY_SOLVER_H
#define GFP_SAFETY_SOLVER_H

#include <iostream>

#include "./BDD2Aiger.h"
#include "./GameSolver.h"

class GFPSafetySolver : public GameSolver
{
private:
    std::vector<BDD> get_strategies(const BDD& winning_region) override;

public:
    GFPSafetySolver(const SafetyArena& arena, const Cudd& manager);

    BDD solve() override;
    aiger* synthesize(const BDD& winning_region) override;
};

#endif
