#include<args.h>

CCParams ccparams_create()
{
    CCParams params = {
        .linkIncrementally = false,
        .compileOnly = false,
        .preprocessOnly = false,
        .toAssemblyOnly = false,
        .output = NULL,
    };
    VECTOR_INIT(params.includePaths);
    VECTOR_INIT(params.inputs);
    return params;
}

void ccparams_destroy(CCParams* params)
{
    VECTOR_FREE(params->inputs);
    VECTOR_FREE(params->includePaths);
}

LanguageMode parse_language_mode(char* mode)
{
    char* end = strrchr(mode, '.');
    if(end == NULL)
    {
        return MODE_INVALID;
    }
    end++;
    if(!strcmp(end, "dc"))
    {
        return MODE_DC;
    }
    else if(!strcmp(end, "asm"))
    {
        return MODE_ASM;
    }
    else if(!strcmp(end, "o"))
    {
        return MODE_OBJ;
    }
    else
    {
        return MODE_INVALID;
    }
}

CCParams* compile_and_verify_argv(int argc, char* argv[])
{
    CCParams params = ccparams_create();
    LanguageMode languageMode = MODE_INVALID;
    bool optimize = true;

    if(argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
    {
        printf("Usage: %s [options] file...\n", argv[0]);
        printf("Options:\n");
        printf("    -E: preprocess only\n");
        printf("    -S: do not assemble\n");
        printf("    -c: do not link\n");
        printf("    -i: link incrementally\n");
        printf("    -O1: optimize\n");
        printf("    -O0: do not optimize\n");
        printf("    -I<path>: add path to include paths\n");
        printf("    -o<file>: compile to file\n");
        printf("    -x<mode>: sets the language mode for indeterminable files\n");
        printf("    -: add file from stdin\n");
        ccparams_destroy(&params);
        return NULL;
    }

    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-"))
        {
            if(languageMode == MODE_INVALID)
            {
                fprintf(stderr, "%s: error: -x is required when using standard input\n", argv[0]);
                ccparams_destroy(&params);
                return NULL;
            }
            else
            {
                CCInput in = {
                    .name = argv[i],
                    .type = languageMode,
                    .optimize = optimize,
                    .content = NULL,
                    .content_size = 0,
                };
                VECTOR_PUSH(params.inputs, in);
            }
        }
        else if(!strcmp(argv[i], "-E"))
        {
            params.preprocessOnly = true;
        }
        else if(!strcmp(argv[i], "-S"))
        {
            params.toAssemblyOnly = true;
        }
        else if(!strcmp(argv[i], "-c"))
        {
            params.compileOnly = true;
        }
        else if(!strcmp(argv[i], "-i"))
        {
            params.linkIncrementally = true;
        }
        else if(!strcmp(argv[i], "-O0"))
        {
            optimize = false;
        }
        else if(!strcmp(argv[i], "-O1"))
        {
            optimize = true;
        }
        else if(!strncmp(argv[i], "-o", 2))
        {
            params.output = argv[i] + 2;
        }
        else if(!strncmp(argv[i], "-x", 2))
        {
            LanguageMode mode = parse_language_mode(argv[i] + 2);
            if(mode == MODE_INVALID)
            {
                fprintf(stderr, "%s: error: language mode %s not recognized\n", argv[0], argv[i] + 2);
                ccparams_destroy(&params);
                return NULL;
            }
            else
            {
                languageMode = mode;
            }
        }
        else if(!strncmp(argv[i], "-I", 2))
        {
            VECTOR_PUSH(params.includePaths, argv[i] + 2);
        }
        else if(!strncmp(argv[i], "-", 1))
        {
            fprintf(stderr, "%s: error: option %s not recognized\n", argv[0], argv[i]);
            ccparams_destroy(&params);
            return NULL;
        }
        else
        {
            LanguageMode mode = parse_language_mode(argv[i]);
            if(mode == MODE_INVALID)
            {
                mode = languageMode;
                if(mode == MODE_INVALID)
                {
                    fprintf(stderr, "%s: error: language mode for file %s not recognized and -x was not used\n", argv[0], argv[i]);
                    ccparams_destroy(&params);
                    return NULL;
                }
            }
            CCInput in = {
                .name = argv[i],
                .optimize = optimize,
                .type = mode,
                .content = NULL,
                .content_size = 0,
            };
            VECTOR_PUSH(params.inputs, in);
        }
    }
    
    VECTOR_PUSH(params.includePaths, ".");

    if(VECTOR_SIZE(params.inputs) == 0)
    {
        fprintf(stderr, "%s: error: no input files\n", argv[0]);
        ccparams_destroy(&params);
        return NULL;
    }
    else if(params.preprocessOnly && params.output != NULL && VECTOR_SIZE(params.inputs) > 1)
    {
        fprintf(stderr, "%s: error: cannot use -o when preprocessing multiple input files\n", argv[0]);
        ccparams_destroy(&params);
        return NULL;
    }
    else if((params.toAssemblyOnly || params.compileOnly) && params.output != NULL && VECTOR_SIZE(params.inputs) > 1)
    {
        fprintf(stderr, "%s: error: cannot use -o when compiling multiple input files\n", argv[0]);
        ccparams_destroy(&params);
        return NULL;
    }

    for(VECTOR_ITER(params.includePaths, i))
    {
        DIR* dir = opendir(VECTOR_EL(params.includePaths, i));
        if(dir == NULL)
        {
            fprintf(stderr, "%s: error: could not open include path %s\n", argv[0], VECTOR_EL(params.includePaths, i));
            ccparams_destroy(&params);
            return NULL;
        }
        else
        {
            closedir(dir);
        }
    }
    for(VECTOR_ITER(params.inputs, i))
    {
        if(!strcmp(VECTOR_EL(params.inputs, i).name, "-"))continue;
        FILE* file = fopen(VECTOR_EL(params.inputs, i).name, "r");
        if(file == NULL)
        {
            fprintf(stderr, "%s: error: could not open file %s\n", argv[0], VECTOR_EL(params.inputs, i).name);
            ccparams_destroy(&params);
            return NULL;
        }
        else
        {
            fclose(file);
        }
    }

    CCParams* retval = malloc(sizeof(CCParams));
    *retval = params;
    return retval;
}
