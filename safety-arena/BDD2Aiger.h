#ifndef BDD2AIGER_H
#define BDD2AIGER_H

#include <unordered_map>
#include <functional>
#include <cassert>

#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"
#include "../aiger/aiger.h"
#include "../safety-arena/aiger-utils.h"

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
    const Cudd& manager;
    aiger *aig;
    std::unordered_map<BDD, AigerLit, BDDHash> bdd_to_aigerlit;

public:
    BDD2Aiger(const Cudd& manager) : manager(manager)
    {
        aig = aiger_init(); aiger_reencode(aig);
        
        bdd_to_aigerlit[manager.bddOne()]  = aiger_true;
        bdd_to_aigerlit[manager.bddZero()] = aiger_false;
    }

    aiger* get_encoding()
    {
        return aig;
    }

    void add_input(const BDD& node, const std::string& name)
    {
        AigerLit lit = AigerUtils::next_var_index(aig);
        aiger_add_input(aig, lit, name.c_str());
        bdd_to_aigerlit[node] = lit;
    }
    
    void add_latch(const BDD& node, const std::string& name)
    {
        AigerLit lit = AigerUtils::next_var_index(aig);
        aiger_add_latch(aig, lit, 0, nullptr);
        bdd_to_aigerlit[node] = lit;
    }

    void add_output(const BDD& node, const std::string& name)
    {
        aiger_add_output(aig, translate_bdd_to_aig(node), name.c_str());
    }

    // void define_latch(const BDD& node, const BDD& next_node)
    // {
    //     AigerLit lit = bdd_to_aigerlit[node];
        
    //     aiger_symbol *symb = aiger_is_latch(aig, lit);
    //     if(symb != nullptr)
    //     {
    //         symb->next = translate_bdd_to_aig(next_node);
    //     }
    // }

    AigerLit translate_bdd_to_aig(const BDD& node)
    {
        if(node == manager.bddOne())  return aiger_true;
        if(node == manager.bddZero()) return aiger_false;

        bool negated = Cudd_IsComplement(node) == 1;
        BDD normalized_node = negated ? ~node : node;

        AigerLit node_lit = bdd_to_aigerlit.at(normalized_node);
        node_lit = negated ? aiger_not(node_lit) : node_lit;

        AigerLit then_lit = translate_bdd_to_aig(BDD(manager, Cudd_T(normalized_node.getNode())));
        AigerLit else_lit = translate_bdd_to_aig(BDD(manager, Cudd_E(normalized_node.getNode())));

        unsigned left_and  = AigerUtils::create_and(aig, node_lit, then_lit);
        unsigned right_and = AigerUtils::create_and(aig, aiger_not(node_lit), else_lit);

        // If it's broken, probably see here
        unsigned ite_lit = aiger_not(AigerUtils::create_and(aig, aiger_not(left_and), aiger_not(right_and)));
        bdd_to_aigerlit[normalized_node] = ite_lit;

        return negated ? aiger_not(ite_lit) : ite_lit;
    }
};

#endif
