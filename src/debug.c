#include<debug.h>

void debug_print_ccparams(CCParams* params)
{
    printf("Set to %s with %lu inputs, %lu include paths and output \"%s\"\n",
        params->preprocessOnly ?
            "preprocess only"
            : params->toAssemblyOnly ?
                "compile to assembly only"
                : params->compileOnly ?
                    "compile only"
                    : "compile fully",
        VECTOR_SIZE(params->inputs),
        VECTOR_SIZE(params->includePaths),
        params->output == NULL ? "no output" : params->output);
    for(VECTOR_ITER(params->inputs, i))
    {
        printf("    input %lu with mode \"%s\", name \"%s\" and optimization %s\n",
            i,
            VECTOR_EL(params->inputs, i).type == MODE_DC ?
                "dc"
                : VECTOR_EL(params->inputs, i).type == MODE_ASM ?
                    "asm"
                    : "o",
            VECTOR_EL(params->inputs, i).name,
            VECTOR_EL(params->inputs, i).optimize ? "on" : "off");
    }
    for(VECTOR_ITER(params->includePaths, i))
    {
        printf("    include path \"%s\"\n", VECTOR_EL(params->includePaths, i));
    }
}
