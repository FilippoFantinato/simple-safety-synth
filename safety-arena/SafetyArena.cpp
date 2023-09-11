#include "SafetyArena.h"

SafetyArena::SafetyArena(aiger *aig, const Cudd& manager) : _manager(manager)
{
    std::unordered_map<AigerLit, BDD> cache = {
        {aiger_false, manager.bddZero()}};
    unsigned index = 0;

    this->_initial = manager.bddOne();
    this->_safety_condition = manager.bddOne();

    for (unsigned i = 0; i < aig->num_inputs; ++i)
    {
        aiger_symbol *symb = aig->inputs + i;

        BDD node = manager.bddVar(index++);

        std::string name(symb->name);
        if(AigerUtils::is_controllable(name))
        {
            name.erase(0, CONTROLLABLE_PREFIX_LENGTH);
            _controllables.push_back(node);
            _controllables_names.push_back(name);
        }
        else
        {
            _uncontrollables.push_back(node);
            _uncontrollables_names.push_back(name);
        }

        cache[symb->lit] = node;
        _compose.push_back(node);

        Cudd_bddSetPiVar(manager.getManager(), node.NodeReadIndex());
    }

    for (unsigned i = 0; i < aig->num_latches; ++i)
    {
        aiger_symbol *symb = aig->latches + i;

        BDD node = manager.bddVar(index++);

        switch (symb->reset)
        {
        case 0:
            _initial &= ~node;
            break;
        case 1:
            _initial &= node;
            break;
        default:
            throw std::runtime_error("ERROR");
        }

        cache[symb->lit] = node;
        _latches.push_back(node);
        _latches_names.push_back(std::to_string(symb->lit));

        Cudd_bddSetPsVar(manager.getManager(), node.NodeReadIndex());
    }

    for (unsigned i = 0; i < aig->num_ands; ++i)
    {
        aiger_and *a = aig->ands + i;

        cache[a->lhs] = lookup_literal(a->rhs0, cache) & lookup_literal(a->rhs1, cache);
    }

    for (unsigned i = 0; i < aig->num_latches; ++i)
    {
        aiger_symbol *symb = aig->latches + i;
        BDD f = lookup_literal(symb->next, cache);
        _compose.push_back(f);
    }

    for (unsigned i = 0; i < aig->num_outputs; ++i)
    {
        aiger_symbol *symb = aig->outputs + i;

        _safety_condition &= ~lookup_literal(symb->lit, cache);
    }
}

BDD SafetyArena::lookup_literal(AigerLit node, const std::unordered_map<AigerLit, BDD>& cache)
{
    auto normalized = AigerUtils::normalize(node);

    return normalized.first ? ~cache.at(normalized.second) : cache.at(normalized.second);
}

const BDD& SafetyArena::initial() const
{
    return _initial;
}

const BDD& SafetyArena::safety_condition() const
{
    return _safety_condition;
}

const std::vector<std::string>& SafetyArena::controllables_names() const
{
    return _controllables_names;
}

const std::vector<std::string>& SafetyArena::uncontrollables_names() const
{
    return _uncontrollables_names;
}

const std::vector<std::string>& SafetyArena::latches_names() const
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
