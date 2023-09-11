#ifndef SAFETY_ARENA_H
#define SAFETY_ARENA_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdio>

#include "../aiger/aiger.h"
#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"

#include "./aiger-utils.h"

class SafetyArena
{
private:
    const Cudd& _manager;

    BDD _initial;
    BDD _safety_condition;
    std::vector<BDD> _compose;

    std::vector<std::string> _controllables_names;
    std::vector<std::string> _uncontrollables_names;
    std::vector<std::string> _latches_names;

    std::vector<BDD> _controllables;
    std::vector<BDD> _uncontrollables;
    std::vector<BDD> _latches;

    static BDD lookup_literal(AigerLit node, const std::unordered_map<AigerLit, BDD>& cache);

public:
    SafetyArena(aiger *aig, const Cudd& manager);

    const BDD& initial() const;
    const BDD& safety_condition() const;
    const std::vector<BDD>& compose() const;

    const std::vector<std::string>& controllables_names() const;
    const std::vector<std::string>& uncontrollables_names() const;
    const std::vector<std::string>& latches_names() const;

    const std::vector<BDD>& controllables() const;
    const std::vector<BDD>& uncontrollables() const;
    const std::vector<BDD>& latches() const;
};

#endif
