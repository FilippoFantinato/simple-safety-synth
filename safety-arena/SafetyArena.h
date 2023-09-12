#ifndef SAFETY_ARENA_H
#define SAFETY_ARENA_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdio>

#include "../aiger/aiger.h"
#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"

#include "../utils/aiger.h"

class SafetyArena
{
private:
    unsigned _var_index = 0;
    const Cudd& _manager;

    BDD _initial;
    BDD _safety_condition;
    std::vector<BDD> _compose;

    std::vector<AigerLit> _controllables_names;
    std::vector<AigerLit> _uncontrollables_names;
    std::vector<AigerLit> _latches_names;

    std::vector<BDD> _controllables;
    std::vector<BDD> _uncontrollables;
    std::vector<BDD> _latches;
    
    void add_input(aiger_symbol *input, std::unordered_map<AigerLit, BDD> *cache);
    void add_latch(aiger_symbol *latch, std::unordered_map<AigerLit, BDD> *cache);
    void add_and(
        aiger_and *symb,
        const std::unordered_map<AigerLit, aiger_and*>& lit2symb,
        std::unordered_map<AigerLit, bool> *visited, 
        std::unordered_map<AigerLit, BDD> *cache
    );

    static BDD lookup_literal(AigerLit node, const std::unordered_map<AigerLit, BDD>& cache);
    static BDD lookup_literal(bool negated, AigerLit normalized, const std::unordered_map<AigerLit, BDD>& cache);

public:
    SafetyArena(aiger *aig, const Cudd& manager);

    const BDD& initial() const;
    const BDD& safety_condition() const;
    const std::vector<BDD>& compose() const;

    const std::vector<AigerLit>& controllables_names() const;
    const std::vector<AigerLit>& uncontrollables_names() const;
    const std::vector<AigerLit>& latches_names() const;

    const std::vector<BDD>& controllables() const;
    const std::vector<BDD>& uncontrollables() const;
    const std::vector<BDD>& latches() const;
};

#endif
