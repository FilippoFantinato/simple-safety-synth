#include <iostream>
#include <argparse/argparse.hpp>

#include "./cudd/include/cuddObj.hh"

#include "./safety-arena/SafetyArena.h"
#include "./safety-solver/SimpleSafetySolver.h"
#include "./safety-solver/BetterSafetySolver.h"

int main(int argc, char const *argv[])
{
    argparse::ArgumentParser args("simple-safety-synth");

    args.add_argument("input")
           .help("Input file in either aag or aig format")
           .required();
    args.add_argument("-s", "--synthesize")
           .help("Whether synthesizing the strategy or not")
           .default_value(false)
           .implicit_value(true);
    args.add_argument("--smv")
           .help("Output SMV format")
           .default_value(false)
           .implicit_value(true);
    args.add_argument("-o", "--output")
           .help("Output file in either SMV or aag format");

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
    SafetySolver *solver = new BetterSafetySolver(arena, manager);

    BDD winning_region = solver->solve();

    if(winning_region != manager.bddZero())
    {
        std::cout << "Realizable" << std::endl;

        if(args.get<bool>("--synthesize"))
        {
            aiger *strategy = solver->synthesize(winning_region);
            aiger *combined = Utils::Aiger::merge_arena_strategy(aig_arena, strategy);

            auto output = args.present("--output");

            if(args.get<bool>("--smv"))
            {
                std::ostream *outfile;
                if(output)
                    outfile = new std::ofstream(*output);
                else
                    outfile = &std::cout;

                Utils::Aiger::write_aiger_to_smv(*outfile, combined);

                outfile->flush();
            }
            else
            {   
                FILE *outfile = output ? fopen(output->c_str(), "w") : stdout;
                aiger_write_to_file(combined, aiger_ascii_mode, outfile);
                fclose(outfile);
            }

            delete strategy;
            delete combined;
        }
    }
    else
    {
        std::cout << "Unrealizable" << std::endl;
    }

    delete aig_arena;
    delete solver;

    return 0;
}
