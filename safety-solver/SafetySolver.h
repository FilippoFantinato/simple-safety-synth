#ifndef SAFETY_SOLVER_H
#define SAFETY_SOLVER_H

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cstring>

#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"
#include "../safety-arena/SafetyArena.h"

#include "./BDD2Aiger.h"

class SafetySolver
{
private:
    const SafetyArena& _arena;
    const Cudd& _manager;

    BDD controllable_cube;
    BDD uncontrollable_cube;

    BDD pre(const BDD& states);
    std::vector<BDD> get_strategies(const BDD& winning_region);

public:
    SafetySolver(const SafetyArena& arena, const Cudd& manager);

    BDD solve();
    aiger *synthesize(const BDD& winning_region);
};

#endif
