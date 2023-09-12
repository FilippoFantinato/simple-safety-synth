#ifndef UTILS_AIGER_H
#define UTILS_AIGER_H

#include <cstdio>
#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../aiger/aiger.h"
#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"

#define CONTROLLABLE_PREFIX "controllable_"

typedef unsigned AigerLit;
typedef std::pair<bool, AigerLit> NegatedNormalized;

namespace Utils::Aiger
{
    aiger* open_aiger(char const *filename);
    void write_aiger(aiger *aig, char const *filename);

    NegatedNormalized normalize(AigerLit lit);
    AigerLit next_var_index(aiger *aig);

    bool is_controllable(const std::string& name);
    unsigned create_and(aiger *aig, AigerLit lhs, AigerLit rhs);
    unsigned translate_lit(aiger *aig, unsigned offset, unsigned lit);

    aiger* merge_arena_strategy(aiger* arena, aiger *strategy);
}

#endif
