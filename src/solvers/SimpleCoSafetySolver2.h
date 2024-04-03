#ifndef SIMPLE_COSAFETY_SOLVER_2_H
#define SIMPLE_COSAFETY_SOLVER_2_H

#include <iostream>
#include <vector>
#include <map>

#include "./BDD2Aiger.h"
#include "./GameSolver.h"

class SimpleCoSafetySolver2 : public GameSolver
{
private:
    std::vector<BDD> _attractors;

    std::vector<BDD> get_strategies(const BDD& winning_region) override;

    BDD get_wining_region();

public:
    SimpleCoSafetySolver2(const SafetyArena& arena, const Cudd& manager);

    BDD solve() override;
    aiger* synthesize(const BDD& winning_region) override;
};

#endif
