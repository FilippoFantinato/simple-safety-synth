#include "aiger-utils.h"

namespace AigerUtils
{
    aiger* open_aiger(char const *filename)
    {
        FILE *input = fopen(filename, "r");
        aiger *aig = aiger_init();
        aiger_read_from_file(aig, input);
        fclose(input);
        return aig;
    }

    NegatedNormalized normalize(AigerLit lit)
    {
        return {lit & 1, lit & ~1};
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
        unsigned lit = AigerUtils::next_var_index(aig);
        aiger_add_and(aig, lit, lhs, rhs);
        return lit;
    }

    unsigned translate_lit(aiger *aig, unsigned offset, unsigned lit)
    {
        auto normalized = normalize(lit);
        switch (aiger_lit2tag(aig, normalized.second))
        {
        case 0: // Constant
            return normalized.second;

        case 1: // Input
            return normalized.first ? aiger_not(offset) : offset;

        case 3: // And
            return lit + offset;
        
        default:
            throw std::runtime_error("ERROR");
        }
    }

    aiger* merge_arena_strategy(aiger* arena, aiger *strategy)
    {
        aiger *aig = aiger_init();

        // inputs
        for(unsigned i = 0; i < arena->num_inputs; ++i)
        {
            aiger_symbol *symb = arena->inputs + i;
            if(!AigerUtils::is_controllable(symb->name))
                aiger_add_input(aig, symb->lit, symb->name);
        }

        // latches
        for(unsigned i = 0; i < arena->num_latches; ++i)
        {
            aiger_symbol *symb = arena->latches + i;
            aiger_add_latch(aig, symb->lit, symb->next, symb->name);
        }

        // outputs
        for(unsigned i = 0; i < arena->num_outputs; ++i)
        {
            aiger_symbol *symb = arena->outputs + i;
            aiger_add_output(aig, symb->lit, symb->name);
        }

        for(unsigned i = 0; i < arena->num_ands; ++i)
        {
            aiger_and *symb = arena->ands + i;
            aiger_add_and(aig, symb->lhs, symb->rhs0, symb->rhs1);
        }

        unsigned offset = next_var_index(arena);
        
        
        for(unsigned i = 0; i < strategy->num_ands; ++i)
        {
            aiger_and *symb = strategy->ands + i;
            unsigned lhs = translate_lit(strategy, offset, symb->lhs);
            unsigned rhs0 = translate_lit(strategy, offset, symb->rhs0);    
            unsigned rhs1 = translate_lit(strategy, offset, symb->rhs1);    
            aiger_add_and(aig, lhs, rhs0, rhs1);
        }

        for(unsigned i = 0; i < strategy->num_outputs; ++i)
        {
            aiger_symbol *symb = strategy->outputs + i;
            unsigned rhs0 = translate_lit(strategy, offset, symb->lit);    
            aiger_add_and(aig, symb->lit, rhs0, 1);
        }

        return aig;
    }
}