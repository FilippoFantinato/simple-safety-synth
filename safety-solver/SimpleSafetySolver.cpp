#include "./SimpleSafetySolver.h"

SimpleSafetySolver::SimpleSafetySolver(const SafetyArena& arena, const Cudd& manager) 
    : SafetySolver(arena, manager)
{
}

BDD SimpleSafetySolver::solve()
{
    BDD fixpoint  = _manager.bddZero();
    BDD attractor = ~_arena.safety_condition();

    // unsigned round = 0;
    while(fixpoint != attractor)
    {
        // std::cout << "Round: " << ++round << std::endl;

        fixpoint = attractor;
        
        BDD controlled_predecessor = attractor.VectorCompose(_arena.compose())
            .UnivAbstract(_controllable_cube)
            .ExistAbstract(_uncontrollable_cube);
        
        attractor = attractor | controlled_predecessor;
    }

    BDD arena = ~attractor;
    const BDD& initial = _arena.initial();

    return (initial & arena) != initial ? 
            _manager.bddZero() : 
            arena;
}

aiger* SimpleSafetySolver::synthesize(const BDD& winning_region)
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
    for(size_t i = 0; i < _arena.latches().size(); ++i)
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

std::vector<BDD> SimpleSafetySolver::get_strategies(const BDD& winning_region)
{
    BDD nondeterministic_strategy = winning_region.VectorCompose(_arena.compose());
    const std::vector<BDD>& controllables = _arena.controllables();

    std::vector<BDD> strategies;
    for(const auto& c: controllables)
    {
        BDD winning_controllables = 
            nondeterministic_strategy.ExistAbstract(
                std::accumulate(
                    controllables.begin(), 
                    controllables.end(), 
                    _manager.bddOne(), 
                    [&c](const BDD& acc, const BDD& el){
                        return c != el ? acc&el : acc;
                    }
                )
            );

        BDD canBeTrue   = winning_controllables.Cofactor(c);
        BDD canBeFalse  = winning_controllables.Cofactor(~c);
        BDD mustBeTrue  = (~canBeFalse) & canBeTrue;
        BDD mustBeFalse = (~canBeTrue) & canBeFalse;
        
        BDD care_set = mustBeTrue | mustBeFalse;
        BDD model    = mustBeTrue.Restrict(care_set);

        strategies.push_back(model);
        nondeterministic_strategy &= c.Xnor(model);
    }

    return strategies;
}

