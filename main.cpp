#include <iostream>
#include <argparse/argparse.hpp>

#include "./cudd/include/cuddObj.hh"

#include "./safety-arena/SafetyArena.h"
#include "./safety-solver/SafetySolver.h"

int main(int argc, char const *argv[])
{
    argparse::ArgumentParser args("simple-safety-synth");

    args.add_argument("input")
           .help("Input file in either aag or aig format")
           .required();
    args.add_argument("--synthesize")
           .help("Whether synthesizing the strategy or not")
           .default_value(false)
           .implicit_value(true);
    args.add_argument("-o", "--output")
           .help("Output file in aag format");

    try
    {
        args.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) 
    {
        std::cerr << err.what() << std::endl;
        std::cerr << args;
        std::exit(1);
    }
    
    aiger *aig_arena = Utils::Aiger::open_aiger(args.get("input").c_str());
    Cudd manager;

    SafetyArena arena(aig_arena, manager);
    SafetySolver solver(arena, manager);

    BDD winning_region = solver.solve();

    if(winning_region != manager.bddZero())
    {
        std::cout << "Realizable" << std::endl;

        if(args.get<bool>("--synthesize"))
        {
            aiger *strategy = solver.synthesize(winning_region);
            aiger *combined = Utils::Aiger::merge_arena_strategy(aig_arena, strategy);

            if(auto output = args.present("--output"))
            {
                Utils::Aiger::write_aiger(combined, output->c_str());
            }
            else
            {
                aiger_write_to_file(combined, aiger_ascii_mode, stdout);
            }
        }
    }
    else
    {
        std::cout << "Unrealizable" << std::endl;
    }

    delete aig_arena;

    return 0;
}
