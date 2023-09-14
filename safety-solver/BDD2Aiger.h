#ifndef BDD2AIGER_H
#define BDD2AIGER_H

#include <unordered_map>
#include <functional>
#include <cassert>

#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"
#include "../aiger/aiger.h"
#include "../utils/aiger.h"

struct BDDHash
{
    size_t operator()(const BDD& node) const
    {
        return std::hash<unsigned>{}(node.NodeReadIndex());
    }
};

class BDD2Aiger
{
private:
    const Cudd& _manager;
    aiger *_aig;
    std::unordered_map<BDD, AigerLit, BDDHash> _bdd2lit;
    std::unordered_map<unsigned, AigerLit> _bddidx2lit;

public:
    BDD2Aiger(const Cudd& manager);
    // ~BDD2Aiger();

    aiger* get_encoding();

    void add_input(const BDD& node, AigerLit name);
    void add_latch(const BDD& node, AigerLit name);
    void add_output(const BDD& node, AigerLit name);
    
    // void define_latch(const BDD& node, const BDD& next_node)
    // {
    //     AigerLit lit = _bdd2lit[node];
    //     aiger_symbol *symb = aiger_is_latch(aig, lit);
    //     if(symb != nullptr)
    //     {
    //         symb->next = translate_bdd2aig(next_node);
    //     }
    // }

    AigerLit translate_bdd2aig(const BDD& node);
};

#endif
