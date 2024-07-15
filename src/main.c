#include<args.h>
#include<files.h>
#include<linker.h>

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
        //TODO: WRITE FILES

        return 0;
    }

    //TODO: COMPILE

    if(params->toAssemblyOnly)
    {
        //TODO: WRITE FILES

        return 0;
    }

    //TODO: ASSEMBLE

    if(params->compileOnly)
    {
        //TODO: WRITE FILES

        return 0;
    }

    while(VECTOR_SIZE(params->inputs) > 1)
    {
        if(!link_together(&params->inputs, 0, 1));
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

    if(!link_resolve(VECTOR_AT(params->inputs, 0)))
    {
        for(VECTOR_ITER(params->inputs, i))
        {
            free(VECTOR_EL(params->inputs, i).content);
        }
        ccparams_destroy(params);
        free(params);
        return 1;
    }

    //TODO: WRITE FINAL BINARY
}
