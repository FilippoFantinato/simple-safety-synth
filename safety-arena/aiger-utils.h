#ifndef AIGER_UTILS_H
#define AIGER_UTILS_H

#include <cstdio>
#include <vector>
#include <cassert>
#include <iostream>

#include "../aiger/aiger.h"
#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"

#define CONTROLLABLE_PREFIX "controllable_"
#define CONTROLLABLE_PREFIX_LENGTH 13

typedef unsigned AigerLit;
typedef std::pair<bool, AigerLit> NegatedNormalized;

namespace AigerUtils
{
    aiger* open_aiger(char const *filename);

    NegatedNormalized normalize(AigerLit lit);
    AigerLit next_var_index(aiger *aig);

    bool is_controllable(const std::string& name);
    unsigned create_and(aiger *aig, AigerLit lhs, AigerLit rhs);
    unsigned translate_lit(aiger *aig, unsigned offset, unsigned lit);

    aiger* merge_arena_strategy(aiger* arena, aiger *strategy);
}

#endif
