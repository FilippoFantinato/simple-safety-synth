#include "./SimpleSafetySolver.h"

SimpleSafetySolver::SimpleSafetySolver(const SafetyArena& arena, const Cudd& manager) 
    : GameSolver(arena, manager)
{
}

BDD SimpleSafetySolver::solve()
{
    BDD fixpoint  = _manager.bddZero();
    BDD attractor = ~_arena.safety_condition();

    unsigned round = 0;
    while(fixpoint != attractor)
    {
        std::cout << "Round: " << ++round << std::endl;

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

// BDD SimpleSafetySolver::solve()
// {
//     BDD fixpoint    = _manager.bddZero();
//     BDD safe_states = _manager.bddOne();
//     const BDD& initial = _arena.initial();

//     unsigned round = 0;
//     while(fixpoint != safe_states)
//     {
//         std::cout << "Round: " << ++round << std::endl;

//         fixpoint = safe_states;
//         safe_states &= (safe_states.VectorCompose(_arena.compose()) & _arena.safety_condition())
//                         .UnivAbstract(_uncontrollable_cube)
//                         .ExistAbstract(_controllable_cube);
        
//         if(!(initial <= safe_states))
//         {
//             return _manager.bddZero();
//         }
//     }

//     return fixpoint;
// }

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

        BDD maybe_true   = winning_controllables.Cofactor(c);
        BDD maybe_false  = winning_controllables.Cofactor(~c);

        // BDD p = maybe_true * !maybe_false;
        // BDD n = maybe_false * !maybe_true;
        // for(const auto& u: _arena.uncontrollables())
        // {
        //     BDD p1 = p.ExistAbstract(u);
        //     BDD n1 = n.ExistAbstract(u);
        //     if((p1 & n1).IsZero()){
        //         p = p1;
        //         n = n1;
        //     }
        // }
        // maybe_true = p;
        // maybe_false = n;


        BDD must_be_true  = (~maybe_false) & maybe_true;
        BDD must_be_false = (~maybe_true) & maybe_false;
        BDD care_set      = must_be_true | must_be_false;

        BDD model = maybe_true.Restrict(care_set);

        strategies.push_back(model);
        nondeterministic_strategy &= c.Xnor(model);
    }

    return strategies;
}

