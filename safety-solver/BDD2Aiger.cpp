#include "BDD2Aiger.h"

BDD2Aiger::BDD2Aiger(const Cudd& manager) : _manager(manager)
{
    _aig = aiger_init(); aiger_reencode(_aig);
    
    _bdd2lit[_manager.bddOne()]  = aiger_true;
}

// BDD2Aiger::~BDD2Aiger()
// {
//     delete _aig;
// }

aiger* BDD2Aiger::get_encoding()
{
    return _aig;
}

void BDD2Aiger::add_input(const BDD& node, AigerLit name)
{
    AigerLit lit = Utils::Aiger::next_var_index(_aig);
    aiger_add_input(_aig, lit, std::to_string(name).c_str());
    _bdd2lit[node] = lit;
    _bddidx2lit[node.NodeReadIndex()] = lit;
}

void BDD2Aiger::add_latch(const BDD& node, AigerLit name)
{
    AigerLit lit = Utils::Aiger::next_var_index(_aig);
    aiger_add_latch(_aig, lit, 0, nullptr);
    _bdd2lit[node] = lit;
    _bddidx2lit[node.NodeReadIndex()] = lit;
}

void BDD2Aiger::add_output(const BDD& node, AigerLit name)
{
    aiger_add_output(_aig, translate_bdd2aig(node), std::to_string(name).c_str());
}

AigerLit BDD2Aiger::translate_bdd2aig(const BDD& node)
{
    bool negated = Cudd_IsComplement(node.getNode()) == 1;
    BDD normalized_node = negated ? ~node : node;

    if(_bdd2lit.count(normalized_node) != 0) 
    {
        return negated ? aiger_not(_bdd2lit.at(normalized_node)) : _bdd2lit.at(normalized_node);
    }

    if(normalized_node == _manager.bddOne()) throw std::runtime_error("ERROR: equals to T");

    AigerLit normalized_node_lit = _bddidx2lit.at(normalized_node.NodeReadIndex());

    AigerLit then_lit = translate_bdd2aig(BDD(_manager, Cudd_T(normalized_node.getNode())));
    AigerLit else_lit = translate_bdd2aig(BDD(_manager, Cudd_E(normalized_node.getNode())));

    unsigned left_and  = Utils::Aiger::create_and(_aig, normalized_node_lit, then_lit);
    unsigned right_and = Utils::Aiger::create_and(_aig, aiger_not(normalized_node_lit), else_lit);
    unsigned ite_lit = aiger_not(Utils::Aiger::create_and(_aig, aiger_not(left_and), aiger_not(right_and)));

    _bdd2lit[normalized_node] = ite_lit;

    return negated ? aiger_not(ite_lit) : ite_lit;
}
