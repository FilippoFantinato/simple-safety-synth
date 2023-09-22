#include "aiger.h"

namespace Utils::Aiger
{
    aiger* open_aiger(char const *filename)
    {
        FILE *input = fopen(filename, "r");
        aiger *aig = aiger_init();
        aiger_read_from_file(aig, input);
        fclose(input);
        return aig;
    }

    AigerLit normalize(AigerLit lit)
    {
        return lit & ~1;
    }

    bool is_negated(AigerLit lit)
    {
        return lit & 1;
    } 

    AigerLit next_var_index(aiger *aig)
    {
        return (aig->maxvar + 1) * 2;
    }

    bool is_controllable(const std::string& name)
    {
        return name.find(CONTROLLABLE_PREFIX) != std::string::npos;
    }

    unsigned create_and(aiger *aig, AigerLit lhs, AigerLit rhs)
    {
        if(lhs == 0 || rhs == 0) return 0; 
        if(lhs == 1) return rhs;
        if(rhs == 1) return lhs;
        
        assert(lhs > 1 && rhs > 1);
        unsigned lit = Utils::Aiger::next_var_index(aig);
        aiger_add_and(aig, lit, lhs, rhs);
        return lit;
    }

    unsigned translate_lit(aiger *aig, unsigned offset, unsigned lit)
    {
        bool negated = is_negated(lit);
        AigerLit normalized = normalize(lit);
        switch (aiger_lit2tag(aig, normalized))
        {
        case 0: // Constant
            return lit;

        case 1: { // Input 
            AigerLit translated_lit = atoi(aiger_is_input(aig, normalized)->name);
            return negated ? aiger_not(translated_lit) : translated_lit;
        }

        case 3: // And
            return lit + offset;
        
        default:
            throw std::runtime_error("ERROR");
        }
    }

    aiger* merge_arena_strategy(aiger *arena, aiger *strategy)
    {
        aiger *aig = aiger_init();
        std::unordered_map<AigerLit, char*> lit2name;

        // inputs
        for(unsigned i = 0; i < arena->num_inputs; ++i)
        {
            aiger_symbol *input = arena->inputs + i;
            if(Utils::Aiger::is_controllable(input->name))
            {
                lit2name[input->lit] = input->name;
            }
            else
            {
                aiger_add_input(aig, input->lit, input->name);
            }
        }

        // latches
        for(unsigned i = 0; i < arena->num_latches; ++i)
        {
            aiger_symbol *latch = arena->latches + i;
            aiger_add_latch(aig, latch->lit, latch->next, latch->name);
        }

        // outputs
        for(unsigned i = 0; i < arena->num_outputs; ++i)
        {
            aiger_symbol *output = arena->outputs + i;
            aiger_add_output(aig, aiger_not(output->lit), OUTPUT_FORMULA);
        }

        for(unsigned i = 0; i < arena->num_ands; ++i)
        {
            aiger_and *a = arena->ands + i;
            aiger_add_and(aig, a->lhs, a->rhs0, a->rhs1);
        }

        unsigned offset = next_var_index(arena);
        
        for(unsigned i = 0; i < strategy->num_ands; ++i)
        {
            aiger_and *a  = strategy->ands + i;
            unsigned lhs  = translate_lit(strategy, offset, a->lhs);
            unsigned rhs0 = translate_lit(strategy, offset, a->rhs0);    
            unsigned rhs1 = translate_lit(strategy, offset, a->rhs1);    
            aiger_add_and(aig, lhs, rhs0, rhs1);
        }

        for(unsigned i = 0; i < strategy->num_outputs; ++i)
        {
            aiger_symbol *output = strategy->outputs + i;
            
            unsigned rhs0       = translate_lit(strategy, offset, output->lit);
            unsigned output_lit = atoi(output->name);

            aiger_add_and(aig, output_lit, rhs0, 1);
            aiger_add_output(aig, output_lit, lit2name.at(output_lit));
        }

        return aig;
    }

    std::string print_literal(aiger *aig, AigerLit lit)
    {
        if(lit == 0) return "FALSE";
        if(lit == 1) return "TRUE";
        if(is_negated(lit))  return "!" + print_literal(aig, normalize(lit));
        if(auto name = aiger_get_symbol(aig, lit)) return name;

        std::string prefix;
        if(aiger_is_input(aig, lit))
            prefix = "i";
        else if(aiger_is_latch(aig, lit))
            prefix = "l";
        else if(aiger_is_and(aig, lit))
            prefix = "a";
        else 
            prefix = "o";

        return prefix + std::to_string(lit);
    }

    void write_aiger_to_smv(std::ostream& outfile, aiger *aig, bool submodule)
    {
        if(submodule)
        {
            std::stringstream inputs;
            for(unsigned i = 0; i < aig->num_inputs; ++i)
            {
                aiger_symbol *input = aig->inputs + i;
                inputs << input->name;
                if((i+1) != (aig->num_inputs))
                {
                    inputs << ",";
                }
            }
            
            outfile << "MODULE controller" << "(" << inputs.str() << ")" << std::endl;
            outfile << "--latches" << std::endl;
            outfile << "VAR" << std::endl;
        }
        else
        {
            outfile << "MODULE main" << std::endl;
            outfile << "--latches" << std::endl;
            outfile << "VAR" << std::endl;

            for(unsigned i = 0; i < aig->num_inputs; ++i)
            {
                aiger_symbol *input = aig->inputs + i;
                outfile << print_literal(aig, input->lit) << " : boolean;" << std::endl;
            }
        }
        
        for(unsigned i = 0; i < aig->num_latches; ++i)
        {
            aiger_symbol *latch = aig->latches + i;
            outfile << print_literal(aig, latch->lit) << " : boolean;" << std::endl;
        }

        outfile << "ASSIGN" << std::endl;
        for(unsigned i = 0; i < aig->num_latches; ++i)
        {
            aiger_symbol *latch = aig->latches + i;

            if(latch->reset != latch->lit)
            {
                outfile << "init(" << print_literal(aig, latch->lit) << ") := " 
                        << print_literal(aig, latch->reset) << ";" << std::endl;
            }

            outfile << "next(" << print_literal(aig, latch->lit) << ") := "
                    << print_literal(aig, latch->next) << ";" << std::endl;
        }

        outfile << "DEFINE" << std::endl;

        outfile << "--ands" << std::endl;
        for(unsigned i = 0; i < aig->num_ands; ++i)
        {
            aiger_and *a = aig->ands + i;

            outfile << print_literal(aig, a->lhs)  << " := " 
                    << print_literal(aig, a->rhs0) << " & "
                    << print_literal(aig, a->rhs1) << ";" 
                    << std::endl;
        }

        outfile << "--outputs" << std::endl;
        for(unsigned i = 0; i < aig->num_outputs; ++i)
        {
            aiger_symbol *output = aig->outputs + i;

            std::string name = output->name;
            if(is_controllable(name)) name.erase(0, CONTROLLABLE_PREFIX_LEN);

            outfile << name << " := " << print_literal(aig, output->lit) << ";" << std::endl;
        }
    }
}
