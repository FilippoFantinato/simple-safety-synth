#include "SafetySolver.h"

SafetySolver::SafetySolver(const SafetyArena& arena, const Cudd& manager)
    : _arena(arena), _manager(manager)
{
    const auto& controllables   = arena.controllables();
    const auto& uncontrollables = arena.uncontrollables();

    _controllable_cube   = std::accumulate(controllables.begin(), controllables.end(), manager.bddOne(), [](const BDD& acc, const BDD& el){return acc&el;});
    _uncontrollable_cube = std::accumulate(uncontrollables.begin(), uncontrollables.end(), manager.bddOne(), [](const BDD& acc, const BDD& el){return acc&el;});
}

