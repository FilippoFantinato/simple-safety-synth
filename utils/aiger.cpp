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

    AigerLit create_and(aiger *aig, AigerLit lhs, AigerLit rhs)
    {
        if(lhs == aiger_false || rhs == aiger_false) return aiger_false; 
        if(lhs == aiger_true) return rhs;
        if(rhs == aiger_true) return lhs;
        
        assert(lhs > aiger_true && rhs > aiger_true);
        AigerLit lit = Utils::Aiger::next_var_index(aig);
        aiger_add_and(aig, lit, lhs, rhs);
        return lit;
    }

    AigerLit translate_lit(aiger *aig, unsigned offset, AigerLit lit)
    {
        bool negated = is_negated(lit);
        AigerLit normalized = normalize(lit);
        switch (aiger_lit2tag(aig, normalized))
        {
        case 0: { // Constant
            return lit;
        }

        case 1: { // Input 
            AigerLit translated_lit = atoi(aiger_is_input(aig, normalized)->name);
            return negated ? aiger_not(translated_lit) : translated_lit;
        }

        case 3: { // And
            return lit + offset;
        }
        
        default:
            throw std::runtime_error("RUNTIME ERROR: Invalid aiger type");
        }
    }

    aiger* invert_arena(aiger *arena)
    {
        aiger *aig = aiger_init();
        
        for(unsigned i = 0; i < arena->num_inputs; ++i)
        {
            aiger_symbol *input = arena->inputs + i;
            
            std::string name(input->name);
            if(is_controllable(name))
            {
                name.erase(0, CONTROLLABLE_PREFIX_LEN);
                aiger_add_input(aig, input->lit, name.c_str());
            }
            else
            {
                name = std::string(CONTROLLABLE_PREFIX) + input->name;
                aiger_add_input(aig, input->lit, name.c_str());
            }
        }

        for(unsigned i = 0; i < arena->num_outputs; ++i)
        {
            aiger_symbol *output = arena->outputs + i;
            aiger_add_output(aig, output->lit, output->name);
        }

        for(unsigned i = 0; i < arena->num_latches; ++i)
        {
            aiger_symbol *latch = arena->latches + i;
            aiger_add_latch(aig, latch->lit, latch->next, latch->name);
            aiger_add_reset(aig, latch->lit, latch->reset);
        }

        for(unsigned i = 0; i < arena->num_ands; ++i)
        {
            aiger_and *a = arena->ands + i;
            aiger_add_and(aig, a->lhs, a->rhs0, a->rhs1);
        }
        
        return aig;
    }

    aiger* merge_arena_strategy(aiger *arena, aiger *strategy)
    {
        aiger *aig = aiger_init();
        std::unordered_map<AigerLit, char*> lit2name;

        // inputs
        for(unsigned i = 0; i < arena->num_inputs; ++i)
        {
            aiger_symbol *input = arena->inputs + i;
            if(is_controllable(input->name))
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

        // outputs TODO: fix names for multiple outputs
        AigerLit formula = aiger_not(arena->outputs->lit);
        for(unsigned i = 1; i < arena->num_outputs; ++i)
        {
            aiger_symbol *output = arena->outputs + i;
            formula = create_and(aig, formula, aiger_not(output->lit));
        }
        aiger_add_output(aig, formula, OUTPUT_FORMULA);
        // for(unsigned i = 0; i < arena->num_outputs; ++i)
        // {
        //     aiger_symbol *output = arena->outputs + i;
        //     aiger_add_output(aig, aiger_not(output->lit), OUTPUT_FORMULA);
        // }

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
}
