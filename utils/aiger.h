#ifndef UTILS_AIGER_H
#define UTILS_AIGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <cuddObj.hh>
#include <aiger.h>

#define CONTROLLABLE_PREFIX "controllable_"
#define CONTROLLABLE_PREFIX_LEN 13
#define OUTPUT_FORMULA "formula"

typedef unsigned AigerLit;
typedef std::pair<bool, AigerLit> NegatedNormalized;

namespace Utils::Aiger
{
    aiger* open_aiger(char const *filename);
    bool is_negated(AigerLit lit);
    AigerLit normalize(AigerLit lit);
    AigerLit next_var_index(aiger *aig);
    
    bool is_controllable(const std::string& name);
    unsigned create_and(aiger *aig, AigerLit lhs, AigerLit rhs);
    unsigned translate_lit(aiger *aig, unsigned offset, unsigned lit);

    aiger* merge_arena_strategy(aiger *arena, aiger *strategy);

    void write_smv(std::ostream& outfile, aiger *aig, bool submodule);
}

#endif
