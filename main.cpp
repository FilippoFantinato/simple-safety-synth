#include <iostream>

#include "./cudd/include/cuddObj.hh"

#include "./safety-arena/SafetyArena.h"
#include "./safety-solver/SafetySolver.h"

int main(int argc, char const *argv[])
{
    aiger *aig_arena = Utils::Aiger::open_aiger(argv[1]);
    Cudd manager;

    SafetyArena arena(aig_arena, manager);
    SafetySolver solver(arena, manager);

    BDD winning_region = solver.solve();

    if(winning_region != manager.bddZero())
    {
        std::cout << "Realizable" << std::endl;

        aiger *strategy = solver.synthesize(winning_region);

        // aiger_write_to_file(strategy, aiger_ascii_mode, stdout);

        aiger *combined = Utils::Aiger::merge_arena_strategy(aig_arena, strategy);

        aiger_write_to_file(combined, aiger_ascii_mode, stdout);
    }
    else
    {
        std::cout << "Unrealizable" << std::endl;
    }

    delete aig_arena;

    return 0;
}
