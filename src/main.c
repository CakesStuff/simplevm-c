#include<args.h>
#include<files.h>
#include<tokenizer.h>
#include<assembler.h>
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
    
    dc_token_v tokens = dc_tokenize(VECTOR_AT(params->inputs, 0), argv[0]);
    return VECTOR_SIZE(tokens) == 0;

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

    /**
     * Entry Point sets up SP to point to the end of the program for maximum stack,
     * and calls <entry> or main then halts.
     */
    CCInput entry = {
        .name = "internal_entry_start",
        .type = MODE_ASM,
        .optimize = true,
    };
    entry.content = malloc(1000);
    sprintf((char*)entry.content,
        "extern compiler_generated_program_end\nextern %s\nmov SP, compiler_generated_program_end\nlea A, %s\npush PC\nmov PC, A\nhlt\n",
        params->entry ? params->entry : "main", params->entry ? params->entry : "main");
    entry.content_size = strlen((char*)entry.content);
    VECTOR_PUSH(params->inputs, VECTOR_EL(params->inputs, 0));
    asm_assemble(&entry);
    VECTOR_EL(params->inputs, 0) = entry;
    CCInput end = {
        .name = "internal_program_end",
        .type = MODE_ASM,
        .optimize = true,
    };
    end.content = malloc(100);
    sprintf((char*)end.content, "compiler_generated_program_end:\n");
    end.content_size = strlen((char*)end.content);
    asm_assemble(&end);
    VECTOR_PUSH(params->inputs, end);

    link_together(&params->inputs, 0, 1);
    link_together(&params->inputs, 0, 1);

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
