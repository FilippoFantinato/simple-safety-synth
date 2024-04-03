#include "./SimpleCoSafetySolver2.h"

SimpleCoSafetySolver2::SimpleCoSafetySolver2(const SafetyArena& arena, const Cudd& manager) 
    : GameSolver(arena, manager)
{
}

BDD SimpleCoSafetySolver2::solve()
{
    const auto& initial  = _arena.initial();
    const auto& compose = _arena.compose();
    auto fixpoint  = _manager.bddZero();
    auto attractor = ~_arena.safety_condition();
    
    while(fixpoint != attractor)
    {
        fixpoint = attractor;

        _attractors.insert(_attractors.begin(), attractor);

        BDD cpre = attractor.VectorCompose(compose)
                            .ExistAbstract(_controllable_cube)
                            .UnivAbstract(_uncontrollable_cube);
        
        attractor = attractor | cpre;
    }

    BDD arena = attractor;

    return (initial & arena) != initial ? 
            _manager.bddZero() :
            arena;
}

aiger* SimpleCoSafetySolver2::synthesize(const BDD& winning_region)
{
    std::vector<BDD> strategies = get_strategies(winning_region);

    BDD2Aiger encoder(_manager);

    const auto& uncontrollables       = _arena.uncontrollables();
    const auto& uncontrollables_names = _arena.uncontrollables_names();
    for(size_t i = 0; i < uncontrollables.size(); ++i)
    {
        encoder.add_input(uncontrollables[i], uncontrollables_names[i]);
    }

    const auto& latches       = _arena.latches();
    const auto& latches_names = _arena.latches_names();
    for(size_t i = 0; i < latches.size(); ++i)
    {
        encoder.add_input(latches[i], latches_names[i]);
    }

    const auto& controllables_names = _arena.controllables_names();
    for(size_t i = 0; i < controllables_names.size(); ++i)
    {
        encoder.add_output(strategies[i], controllables_names[i]);
    }

    return encoder.get_encoding();
}

std::vector<BDD> SimpleCoSafetySolver2::get_strategies(const BDD& winning_region)
{
    const auto& compose = _arena.compose();
    const auto& controllables = _arena.controllables();

    std::map<int, BDD> controllable_strategy;
    for(const BDD& c : controllables)
    {
        controllable_strategy[c.NodeReadIndex()] = _manager.bddOne();
    }

    for(unsigned i = _attractors.size() - 1; i > 0; --i)
    {
        const BDD& attractor = _attractors[i];
        const BDD& pre_attractor =  _attractors[i - 1];
        BDD arena = attractor.VectorCompose(compose);

        for(const BDD& c : controllables)
        {
            BDD other_controllables = std::accumulate(
                        controllables.begin(),
                        controllables.end(),
                        _manager.bddOne(),
                        [&c](const BDD& acc, const BDD& el){
                            return c != el ? acc&el : acc;
                        }
                    );
            BDD winning_controllables = arena.ExistAbstract(other_controllables);

            BDD maybe_true  = winning_controllables.Cofactor(c);
            BDD maybe_false = winning_controllables.Cofactor(~c);
            BDD must_be_true  = (~maybe_false) & maybe_true;
            BDD must_be_false = (~maybe_true) & maybe_false;
            BDD care_set      = must_be_true | must_be_false;
            BDD model = maybe_true.Restrict(care_set);

            BDD rule = (!(pre_attractor & !attractor)) | model;

            controllable_strategy[c.NodeReadIndex()] &= rule;

            arena &= c.Xnor(model);
        }
    }

    std::vector<BDD> strategies;
    for(auto& p : controllable_strategy)
    {
        strategies.push_back(p.second);
    }

    return strategies;
}
