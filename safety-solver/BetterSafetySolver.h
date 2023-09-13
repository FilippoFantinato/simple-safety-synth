#ifndef BETTER_SAFETY_SOLVER_H
#define BETTER_SAFETY_SOLVER_H

#include <iostream>

#include "./BDD2Aiger.h"
#include "./SafetySolver.h"

class BetterSafetySolver : public SafetySolver
{
private:
    BDD pre(const BDD& states);
    std::vector<BDD> get_strategies(const BDD& winning_region) override;

public:
    BetterSafetySolver(const SafetyArena& arena, const Cudd& manager);

    BDD solve() override;
    aiger* synthesize(const BDD& winning_region) override;
};

#endif
