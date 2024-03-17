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
    auto pre_cpre = attractor;

    std::cout << "Controllable cube: " << _controllable_cube << std::endl;
    std::cout << "Uncontrollable cube: " << _uncontrollable_cube << std::endl;
    std::cout << std::endl;
    
    unsigned round = 0;
    while(fixpoint != attractor)
    {
        std::cout << "Round: " << ++round << std::endl;
        // std::cout << "ATTRACTOR: " << attractor << std::endl;

        fixpoint = attractor;

        _attractors.insert(_attractors.begin(),
            attractor.IsOne() ? initial : attractor
        );

        BDD cpre = attractor.VectorCompose(compose)
                            .UnivAbstract(_uncontrollable_cube)
                            .ExistAbstract(_controllable_cube);

        // std::cout << "CPRE: " << cpre << std::endl;
        
        attractor = attractor | cpre;
    }

    BDD arena = attractor;

    std::cout << std::endl;
    std::cout << "INITIAL: " << initial << std::endl;
    std::cout << "ARENA: " << arena << std::endl;
    std::cout << "I&A: " << (!(initial <= arena) ? "true" : "false") << " " << (initial & arena) << std::endl;
    std::cout << "OPPOSITE GAME: " << ((~attractor & initial).IsZero() ? "Unrealizable": "Realizable") << std::endl;
    std::cout << std::endl;

    return (initial & arena) != initial ? 
            _manager.bddZero() :
            arena;
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

std::vector<BDD> SimpleCoSafetySolver::get_strategies(const BDD& winning_region)
{
    const BDD& initial  = _arena.initial();
    const auto& compose = _arena.compose();
    const auto& controllables = _arena.controllables();

    std::map<int, BDD> controllable_strategy;
    for(const BDD& c : controllables)
    {
        controllable_strategy[c.NodeReadIndex()] = _manager.bddOne();
    }

    std::cout << "COMPUTING STRATEGIES" << std::endl;

    std::vector<BDD> tmp;

    std::cout << "INITIAL: " << initial << std::endl;

    for(unsigned i = 0; i < (_attractors.size() - 1); ++i)
    {
        std::cout << "Round: " << i << std::endl;
        
        BDD attractor = _attractors[i];
        BDD next_attractor =  _attractors[i + 1];
        BDD arena = attractor.VectorCompose(compose);

        // std::cout << "ATTRACTOR: " << attractor << std::endl;
        // std::cout << "ARENA: " << arena  << std::endl;
        // std::cout << ((attractor & initial) == initial) << std::endl;

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

            // BDD model = maybe_true;
            BDD rule = (~(attractor & !next_attractor)) | model;

            std::cout << "C: " << c << std::endl;
            // std::cout << "WC: " << winning_controllables << std::endl;
            // std::cout << "IN NEXT ATTR: " << (winning_controllables & next_attractor) << std::endl; 
            // std::cout << "OTHER Cs: " << other_controllables << std::endl;
            // std::cout << "MT: " << maybe_true << std::endl;
            // std::cout << "MF: " << maybe_false << std::endl;
            std::cout << "RULE: " << rule << std::endl;

            controllable_strategy[c.NodeReadIndex()] &= rule;

            arena &= c.Xnor(model);
        }

        std::cout << std::endl;
    }

    std::vector<BDD> strategies;
    for(auto& p : controllable_strategy)
    {
        strategies.push_back(p.second);
    }

    return strategies;
}
