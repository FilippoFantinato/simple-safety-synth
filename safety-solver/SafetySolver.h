#ifndef SAFETY_SOLVER_H
#define SAFETY_SOLVER_H

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cstring>

#include "../cudd/include/cuddObj.hh"
#include "../cudd/include/cudd.h"

#include "../safety-arena/BDD2Aiger.h"
#include "../safety-arena/SafetyArena.h"

class SafetySolver
{
private:
    const SafetyArena& arena;
    const Cudd& manager;

    BDD exiscube;
    BDD univcube;

    static BDD bdd_and(const BDD& lhs, const BDD& rhs)
    {
        return lhs & rhs;
    }

    BDD pre(BDD states)
    {   
        return states.VectorCompose(arena.compose())
              .AndAbstract(arena.safety_condition(), exiscube)
              .UnivAbstract(univcube);
    }

    std::vector<BDD> get_strategies(const BDD& winning_region)
    {
        BDD care_set = winning_region;
        BDD nondeterministic_strategy = winning_region.VectorCompose(arena.compose()) & arena.safety_condition();
        std::vector<BDD> strategies;
        const std::vector<BDD>& controllables = arena.controllables();

        for(auto c: controllables)
        {
            BDD winning_controllables = nondeterministic_strategy;

            std::vector<BDD> other_controllables;
            for(auto o_c: controllables) if(o_c != c) other_controllables.push_back(o_c);

            if(other_controllables.size() > 0)
            {
                winning_controllables = winning_controllables.ExistAbstract(
                    std::accumulate(other_controllables.begin(), other_controllables.end(), manager.bddOne(), bdd_and)
                );
            }

            BDD canBeTrue = winning_controllables.Cofactor(c);
            BDD canBeFalse = winning_controllables.Cofactor(~c);
            BDD mustBeTrue = ~canBeFalse & canBeTrue;
            BDD mustBeFalse = ~canBeTrue & canBeFalse;

            BDD local_care_set = care_set & (mustBeTrue | mustBeFalse);

            BDD model_true = mustBeTrue.Restrict(local_care_set);
            BDD model_false = (~mustBeFalse).Restrict(local_care_set);

            BDD model = model_true.nodeCount() < model_false.nodeCount() ? model_true : model_false;

            strategies.push_back(model);
            
            nondeterministic_strategy &= c.Xnor(model);
        }

        return strategies;
    }

public:
    SafetySolver(const SafetyArena& arena, const Cudd& manager) 
        : arena(arena), manager(manager)
    {
        const auto& controllables   = arena.controllables();
        const auto& uncontrollables = arena.uncontrollables();

        exiscube = std::accumulate(controllables.begin(), controllables.end(), manager.bddOne(), bdd_and);
        univcube = std::accumulate(uncontrollables.begin(), uncontrollables.end(), manager.bddOne(), bdd_and);
    }

    BDD solve() 
    {
        BDD fixpoint = manager.bddZero();
        BDD safeStates = manager.bddOne();

        unsigned round = 0;
        while(fixpoint != safeStates)
        {
            std::cout << "Round: " << ++round << std::endl;

            fixpoint = safeStates;
            safeStates &= pre(safeStates);
            if(arena.initial() > safeStates)
            {
                return manager.bddZero();
            }
        }

        return fixpoint;
    }


    aiger *synthesize(BDD winning_region)
    {
        std::vector<BDD> strategies = get_strategies(winning_region);

        BDD2Aiger encoder(manager);

        const auto& uncontrollables       = arena.uncontrollables();
        const auto& uncontrollables_names = arena.uncontrollables_names();
        for(size_t i = 0; i < uncontrollables.size(); ++i)
        {
            encoder.add_input(uncontrollables[i], uncontrollables_names[i]);
        }

        const auto& latches       = arena.latches();
        const auto& latches_names = arena.latches_names();
        for(size_t i = 0; i < arena.latches().size(); ++i)
        {
            encoder.add_latch(latches[i], latches_names[i]);
        }

        const auto& controllables_names = arena.controllables_names();
        for(size_t i = 0; i < controllables_names.size(); ++i)
        {
            encoder.add_output(strategies[i], controllables_names[i]);
        }

        return encoder.get_encoding();
    }
};

#endif
