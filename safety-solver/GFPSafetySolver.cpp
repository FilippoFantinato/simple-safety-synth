#include "./GFPSafetySolver.h"

GFPSafetySolver::GFPSafetySolver(const SafetyArena& arena, const Cudd& manager) 
    : GameSolver(arena, manager)
{
}

BDD GFPSafetySolver::solve()
{
    const BDD& initial          = _arena.initial();
    const auto& compose         = _arena.compose();
    const BDD& safety_condition = _arena.safety_condition();
    
    BDD fixpoint    = _manager.bddZero();
    BDD safe_states = _manager.bddOne();

    unsigned round = 0;
    while(fixpoint != safe_states)
    {
        std::cout << "Round: " << ++round << std::endl;

        fixpoint = safe_states;
        BDD cpre = safe_states.VectorCompose(compose)
                              .AndAbstract(safety_condition, _controllable_cube)
                              .UnivAbstract(_uncontrollable_cube);
        safe_states = safe_states & cpre;
    }

    return (safe_states & initial) != initial ?
            _manager.bddZero() :
            safe_states;
}

aiger* GFPSafetySolver::synthesize(const BDD& winning_region)
{
    std::vector<BDD> strategies = get_strategies(winning_region);

    BDD2Aiger encoder(_manager);

    const auto& uncontrollables       = _arena.uncontrollables();
    const auto& uncontrollables_names = _arena.uncontrollables_names();
    for(size_t i = 0; i < uncontrollables.size(); ++i)
    {
        encoder.add_input(uncontrollables[i], uncontrollables_names[i]);
    }

    const auto& latches = _arena.latches();
    for(size_t i = 0; i < latches.size(); ++i)
    {
        encoder.add_latch(latches[i]);
    }

    const auto& controllables_names = _arena.controllables_names();
    for(size_t i = 0; i < controllables_names.size(); ++i)
    {
        encoder.add_output(strategies[i], controllables_names[i]);
    }

    return encoder.get_encoding();
}

std::vector<BDD> GFPSafetySolver::get_strategies(const BDD& winning_region)
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

        BDD maybe_true   = winning_controllables.Cofactor(c);
        BDD maybe_false  = winning_controllables.Cofactor(~c);
        BDD must_be_true  = (~maybe_false) & maybe_true;
        BDD must_be_false = (~maybe_true) & maybe_false;
        BDD care_set      = must_be_true | must_be_false;

        BDD model = maybe_true.Restrict(care_set);

        strategies.push_back(model);
        nondeterministic_strategy &= c.Xnor(model);
    }

    return strategies;
}
