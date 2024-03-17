#ifndef GAME_SOLVER_H
#define GAME_SOLVER_H

#include <algorithm>
#include <numeric>
#include <cuddObj.hh>

#include "../safety-arena/SafetyArena.h"

class GameSolver
{
protected:
    const SafetyArena& _arena;
    const Cudd& _manager;

    BDD _controllable_cube;
    BDD _uncontrollable_cube;

    virtual std::vector<BDD> get_strategies(const BDD& winning_region) = 0;

public:
    GameSolver(const SafetyArena& arena, const Cudd& manager);
    virtual ~GameSolver() = default;

    virtual BDD solve() = 0;
    virtual aiger* synthesize(const BDD& winning_region) = 0;
};

#endif
