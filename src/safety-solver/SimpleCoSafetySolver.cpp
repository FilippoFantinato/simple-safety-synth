#include "./SimpleCoSafetySolver.h"

SimpleCoSafetySolver::SimpleCoSafetySolver(const SafetyArena& arena, const Cudd& manager) 
    : GameSolver(arena, manager)
{
}

BDD SimpleCoSafetySolver::solve()
{
    const auto& initial  = _arena.initial();
    const auto& compose = _arena.compose();
    auto fixpoint  = _manager.bddZero();
    auto attractor = ~_arena.safety_condition();
    
    unsigned round = 0;
    while(fixpoint != attractor)
    {
        fixpoint = attractor;

        _attractors.insert(_attractors.begin(),
            attractor.IsOne() ? initial : attractor
        );

        BDD cpre = attractor.VectorCompose(compose)
                            .UnivAbstract(_uncontrollable_cube)
                            .ExistAbstract(_controllable_cube);
        
        attractor = attractor | cpre;
    }

    BDD arena = attractor;

    // return (initial & arena) != initial ? 
    //         _manager.bddZero() :
    //         arena;
    return (initial & arena) != initial ? 
            _manager.bddZero() :
            get_wining_region();
}

BDD SimpleCoSafetySolver::get_wining_region()
{
    const auto& compose = _arena.compose();
    auto winning_region = _manager.bddZero();

    for(int i = 1; i < _attractors.size(); ++i)
    {
        const BDD& pre_attractor = _attractors[i-1];
        const BDD& attractor = _attractors[i];

        winning_region = winning_region | attractor.VectorCompose(compose);
    }

    return winning_region;
}


aiger* SimpleCoSafetySolver::synthesize(const BDD& winning_region)
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

// std::vector<BDD> SimpleCoSafetySolver::get_strategies(const BDD& winning_region)
// {
//     const BDD& initial  = _arena.initial();
//     const auto& compose = _arena.compose();
//     const auto& controllables = _arena.controllables();

//     std::map<int, BDD> controllable_strategy;
//     for(const BDD& c : controllables)
//     {
//         controllable_strategy[c.NodeReadIndex()] = _manager.bddOne();
//     }

//     for(unsigned i = 0; i < (_attractors.size() - 1); ++i)
//     {
//         std::cout << "Round: " << i << std::endl;
        
//         BDD attractor = _attractors[i];
//         BDD next_attractor =  _attractors[i + 1];
//         BDD arena = attractor.VectorCompose(compose);
//         // std::cout << "ATTRACTOR: " << attractor << std::endl;
//         // std::cout << "ARENA: " << arena  << std::endl;
//         // std::cout << ((attractor & initial) == initial) << std::endl;

//         for(const BDD& c : controllables)
//         {
//             BDD other_controllables = std::accumulate(
//                         controllables.begin(),
//                         controllables.end(),
//                         _manager.bddOne(),
//                         [&c](const BDD& acc, const BDD& el){
//                             return c != el ? acc&el : acc;
//                         }
//                     );
//             BDD winning_controllables = arena.ExistAbstract(other_controllables);

//             BDD maybe_true  = winning_controllables.Cofactor(c);
//             BDD maybe_false = winning_controllables.Cofactor(~c);
//             BDD must_be_true  = (~maybe_false) & maybe_true;
//             BDD must_be_false = (~maybe_true) & maybe_false;
//             BDD care_set      = must_be_true | must_be_false;
//             BDD model = maybe_true.Restrict(care_set);

//             BDD rule = (~(attractor & !next_attractor)) | model;

//             controllable_strategy[c.NodeReadIndex()] &= rule;

//             arena &= c.Xnor(model);
//         }

//         std::cout << std::endl;
//     }

//     std::vector<BDD> strategies;
//     for(auto& p : controllable_strategy)
//     {
//         strategies.push_back(p.second);
//     }

//     return strategies;
// }


std::vector<BDD> SimpleCoSafetySolver::get_strategies(const BDD& winning_region)
{
    BDD nondeterministic_strategy = winning_region;
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
