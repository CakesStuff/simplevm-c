#include<args.h>
#include<files.h>
#include<assembler.h>
#include<linker.h>

//TODO: FIX LABEL SIZE ASSUMPTIONS

int main(int argc, char* argv[])
{
    CCParams* params = compile_and_verify_argv(argc, argv);
    if(!params)return 1;

    for(VECTOR_ITER(params->inputs, i))
    {
        if(!file_load(VECTOR_AT(params->inputs, i)))
        {
            fprintf(stderr, "%s: error: failed to load file \"%s\"\n", argv[0], VECTOR_EL(params->inputs, i).name);
            for(VECTOR_ITER(params->inputs, j))
            {
                if(j == i)break;
                free(VECTOR_EL(params->inputs, j).content);
            }
            ccparams_destroy(params);
            free(params);
            return 1;
        }
    }
    
    //TODO: PREPROCESS

    if(params->preprocessOnly)
    {
        for(VECTOR_ITER(params->inputs, i))
        {
            file_adjust_ending(VECTOR_AT(params->inputs, i));
            if(!file_write(VECTOR_AT(params->inputs, i), params->output ? params->output : VECTOR_EL(params->inputs, i).name))
            {
                fprintf(stderr, "%s: warn: could not write output \"%s\"\n", argv[0], params->output ? params->output : VECTOR_EL(params->inputs, i).name);
            }
            free(VECTOR_EL(params->inputs, i).name);
            free(VECTOR_EL(params->inputs, i).content);
        }

        ccparams_destroy(params);
        free(params);
        return 0;
    }

    //TODO: COMPILE

    if(params->toAssemblyOnly)
    {
        for(VECTOR_ITER(params->inputs, i))
        {
            file_adjust_ending(VECTOR_AT(params->inputs, i));
            if(!file_write(VECTOR_AT(params->inputs, i), params->output ? params->output : VECTOR_EL(params->inputs, i).name))
            {
                fprintf(stderr, "%s: warn: could not write output \"%s\"\n", argv[0], params->output ? params->output : VECTOR_EL(params->inputs, i).name);
            }
            free(VECTOR_EL(params->inputs, i).name);
            free(VECTOR_EL(params->inputs, i).content);
        }

        ccparams_destroy(params);
        free(params);
        return 0;
    }

    for(VECTOR_ITER(params->inputs, i))
    {
        if(VECTOR_EL(params->inputs, i).type != MODE_ASM)continue;
        if(!asm_assemble(VECTOR_AT(params->inputs, i)))
        {
            for(VECTOR_ITER(params->inputs, j))
            {
                free(VECTOR_EL(params->inputs, j).content);
            }
            ccparams_destroy(params);
            free(params);
            return 1;
        }
    }

    if(params->compileOnly)
    {
        for(VECTOR_ITER(params->inputs, i))
        {
            file_adjust_ending(VECTOR_AT(params->inputs, i));
            if(!file_write(VECTOR_AT(params->inputs, i), params->output ? params->output : VECTOR_EL(params->inputs, i).name))
            {
                fprintf(stderr, "%s: warn: could not write output \"%s\"\n", argv[0], params->output ? params->output : VECTOR_EL(params->inputs, i).name);
            }
            free(VECTOR_EL(params->inputs, i).name);
            free(VECTOR_EL(params->inputs, i).content);
        }

        ccparams_destroy(params);
        free(params);
        return 0;
    }

    while(VECTOR_SIZE(params->inputs) > 1)
    {
        if(!link_together(&params->inputs, 0, 1))
        {
            for(VECTOR_ITER(params->inputs, i))
            {
                free(VECTOR_EL(params->inputs, i).content);
            }
            ccparams_destroy(params);
            free(params);
            return 1;
        }
    }

    if(params->linkIncrementally)
    {
        if(!file_write(VECTOR_AT(params->inputs, 0), params->output ? params->output : "a.out"))
        {
            fprintf(stderr, "%s: error: could not write final output \"%s\"\n", argv[0], params->output ? params->output : "a.out");
        }

        free(VECTOR_EL(params->inputs, 0).content);
        ccparams_destroy(params);
        free(params);
        return 0;
    }

    //TODO: LINK ENTRY

    if(!link_resolve(VECTOR_AT(params->inputs, 0)))
    {
        free(VECTOR_EL(params->inputs, 0).content);
        ccparams_destroy(params);
        free(params);
        return 1;
    }

    if(!file_write(VECTOR_AT(params->inputs, 0), params->output ? params->output : "a.out"))
    {
        fprintf(stderr, "%s: error: could not write final output \"%s\"\n", argv[0], params->output ? params->output : "a.out");
    }

    free(VECTOR_EL(params->inputs, 0).content);
    ccparams_destroy(params);
    free(params);
    return 0;
}
