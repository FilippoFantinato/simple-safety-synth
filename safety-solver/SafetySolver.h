#ifndef SAFETY_SOLVER_H
#define SAFETY_SOLVER_H

#include <algorithm>
#include <numeric>

#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"
#include "../safety-arena/SafetyArena.h"

class SafetySolver
{
protected:
    const SafetyArena& _arena;
    const Cudd& _manager;

    BDD _controllable_cube;
    BDD _uncontrollable_cube;

    virtual std::vector<BDD> get_strategies(const BDD& winning_region) = 0;

public:
    SafetySolver(const SafetyArena& arena, const Cudd& manager);
    virtual ~SafetySolver() = default;

    virtual BDD solve() = 0;
    virtual aiger* synthesize(const BDD& winning_region) = 0;
};

#endif
