#include "SafetyArena.h"

SafetyArena::SafetyArena(aiger *aig, const Cudd& manager) : _manager(manager)
{
    std::unordered_map<AigerLit, BDD> cache = {
        {aiger_false, manager.bddZero()}
    };

    this->_initial = manager.bddOne();
    this->_safety_condition = manager.bddOne();

    for(unsigned i = 0; i < aig->num_inputs; ++i)
    {
        aiger_symbol *input = aig->inputs + i;
        add_input(input, &cache);
    }

    for (unsigned i = 0; i < aig->num_latches; ++i)
    {
        aiger_symbol *latch = aig->latches + i;
        add_latch(latch, &cache);
    }

    {
        std::unordered_map<AigerLit, aiger_and*> lit2symb;
        std::unordered_map<AigerLit, bool> visited_ands;

        for (unsigned i = 0; i < aig->num_ands; ++i)
        {
            aiger_and *a = aig->ands + i;
            lit2symb[a->lhs] = a;
            visited_ands[a->lhs] = false;
        }

        for (unsigned i = 0; i < aig->num_ands; ++i)
        {
            aiger_and *a = aig->ands + i;
            add_and(a, lit2symb, &visited_ands, &cache);
        }
    }

    for (unsigned i = 0; i < aig->num_latches; ++i)
    {
        aiger_symbol *latch = aig->latches + i;
        const BDD& f = lookup_literal(latch->next, cache);
        _compose.push_back(f);
    }

    for (unsigned i = 0; i < aig->num_outputs; ++i)
    {
        aiger_symbol *output = aig->outputs + i;
        _safety_condition &= ~lookup_literal(output->lit, cache);
    }
}

void SafetyArena::add_input(aiger_symbol *input, std::unordered_map<AigerLit, BDD> *cache)
{
    BDD node = _manager.bddVar(_var_index++);

    if(Utils::Aiger::is_controllable(input->name))
    {
        _controllables.push_back(node);
        _controllables_names.push_back(input->lit);
    }
    else
    {
        _uncontrollables.push_back(node);
        _uncontrollables_names.push_back(input->lit);
    }

    (*cache)[input->lit] = node;
    _compose.push_back(node);
    Cudd_bddSetPiVar(_manager.getManager(), node.NodeReadIndex());
}

void SafetyArena::add_latch(aiger_symbol *latch, std::unordered_map<AigerLit, BDD> *cache)
{
    BDD node = _manager.bddVar(_var_index++);

    Cudd_bddSetPsVar(_manager.getManager(), node.NodeReadIndex());

    switch (latch->reset)
    {
    case 0:
        _initial &= ~node;
        break;
    case 1:
        _initial &= node;
        break;
    default:
        throw std::runtime_error("Error in AIGER input: only 0 and 1 is allowed for initial latch values.");
    }

    (*cache)[latch->lit] = node;
    _latches.push_back(node);
    _latches_names.push_back(latch->lit);
}

void SafetyArena::add_and(aiger_and *symb,
                          const std::unordered_map<AigerLit, aiger_and*>& lit2symb,
                          std::unordered_map<AigerLit, bool> *visited, 
                          std::unordered_map<AigerLit, BDD> *cache)
{
    if(visited->at(symb->lhs) && cache->count(symb->lhs) == 0) throw std::runtime_error("Cyclic dependency.");

    (*visited)[symb->lhs] = true;

    auto rhs0 = Utils::Aiger::normalize(symb->rhs0);
    if(cache->count(rhs0.second) == 0) add_and(lit2symb.at(rhs0.second), lit2symb, visited, cache);

    auto rhs1 = Utils::Aiger::normalize(symb->rhs1);
    if(cache->count(rhs1.second) == 0) add_and(lit2symb.at(rhs1.second), lit2symb, visited, cache);

    (*cache)[symb->lhs] = lookup_literal(rhs0.first, rhs0.second, *cache) & lookup_literal(rhs1.first, rhs1.second, *cache);
}

BDD SafetyArena::lookup_literal(AigerLit node, const std::unordered_map<AigerLit, BDD>& cache)
{
    auto negated_normalized = Utils::Aiger::normalize(node);

    return lookup_literal(negated_normalized.first, negated_normalized.second, cache);
}

BDD SafetyArena::lookup_literal(bool negated, AigerLit normalized, const std::unordered_map<AigerLit, BDD>& cache)
{
    return negated ? ~cache.at(normalized) : cache.at(normalized);
}

const BDD& SafetyArena::initial() const
{
    return _initial;
}

const BDD& SafetyArena::safety_condition() const
{
    return _safety_condition;
}

const std::vector<AigerLit>& SafetyArena::controllables_names() const
{
    return _controllables_names;
}

const std::vector<AigerLit>& SafetyArena::uncontrollables_names() const
{
    return _uncontrollables_names;
}

const std::vector<AigerLit>& SafetyArena::latches_names() const
{
    return _latches_names;
}

const std::vector<BDD>& SafetyArena::controllables() const
{
    return _controllables;
}

const std::vector<BDD>& SafetyArena::uncontrollables() const
{
    return _uncontrollables;
}

const std::vector<BDD>& SafetyArena::latches() const
{
    return _latches;
}

const std::vector<BDD>& SafetyArena::compose() const
{
    return _compose;
}
