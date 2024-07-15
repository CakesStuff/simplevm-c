#include<args.h>
#include<files.h>

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
                free(VECTOR_AT(params->inputs, j)->content);
            }
            ccparams_destroy(params);
            free(params);
            return 1;
        }
    }
    
    //TODO: PREPROCESS

    //TODO: COMPILE

    //TODO: ASSEMBLE

    //TODO: LINK
}
