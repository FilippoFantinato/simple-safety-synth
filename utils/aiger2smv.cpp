#include "aiger.h"

namespace Utils::Aiger
{
    std::string create_smv_literal(aiger *aig, AigerLit lit)
    {
        if(lit == 0) return "FALSE";
        if(lit == 1) return "TRUE";
        if(is_negated(lit))  return "!" + create_smv_literal(aig, normalize(lit));
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

    std::string create_smv_and(aiger *aig, AigerLit lhs, AigerLit rhs)
    {
        if(lhs == aiger_false || rhs == aiger_false) return "FALSE";
        if(lhs == aiger_true) return create_smv_literal(aig, rhs);
        if(rhs == aiger_true) return create_smv_literal(aig, lhs);

        return create_smv_literal(aig, lhs) + " & " + create_smv_literal(aig, rhs);
    }

    void write_smv(std::ostream& outfile, aiger *aig, bool submodule)
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
                outfile << create_smv_literal(aig, input->lit) << " : boolean;" << std::endl;
            }
        }
        
        for(unsigned i = 0; i < aig->num_latches; ++i)
        {
            aiger_symbol *latch = aig->latches + i;
            outfile << create_smv_literal(aig, latch->lit) << " : boolean;" << std::endl;
        }

        outfile << "ASSIGN" << std::endl;
        for(unsigned i = 0; i < aig->num_latches; ++i)
        {
            aiger_symbol *latch = aig->latches + i;

            if(latch->reset != latch->lit)
            {
                outfile << "init(" << create_smv_literal(aig, latch->lit) << ") := " 
                        << create_smv_literal(aig, latch->reset) << ";" << std::endl;
            }

            outfile << "next(" << create_smv_literal(aig, latch->lit) << ") := "
                    << create_smv_literal(aig, latch->next) << ";" << std::endl;
        }

        outfile << "DEFINE" << std::endl;

        outfile << "--ands" << std::endl;
        for(unsigned i = 0; i < aig->num_ands; ++i)
        {
            aiger_and *a = aig->ands + i;

            outfile << create_smv_literal(aig, a->lhs)  << " := " 
                    << create_smv_and(aig, a->rhs0, a->rhs1) << ";" 
                    << std::endl;
        }

        outfile << "--outputs" << std::endl;
        for(unsigned i = 0; i < aig->num_outputs; ++i)
        {
            aiger_symbol *output = aig->outputs + i;

            std::string name = output->name;
            if(is_controllable(name)) name.erase(0, CONTROLLABLE_PREFIX_LEN);

            outfile << name << " := " << create_smv_literal(aig, output->lit) << ";" << std::endl;
        }
    }
}
