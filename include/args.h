#ifndef _ARGS_H
#define _ARGS_H
#include<core.h>

DEFINE_VECTOR_OF(char*) string_v;

typedef enum {
    MODE_INVALID,
    MODE_DC,
    MODE_ASM,
    MODE_OBJ,
    MODE_BIN,
} LanguageMode;

typedef struct {
    char* name;
    LanguageMode type;
    bool optimize;
    uint8_t* content;
    size_t content_size;
} CCInput;

DEFINE_VECTOR_OF(CCInput) inp_v;

typedef struct {
    bool linkIncrementally, compileOnly, toAssemblyOnly, preprocessOnly;
    string_v includePaths;
    char* output;
    inp_v inputs;
    char* entry;
} CCParams;

/**
 * Compiles argv to CCParams for use in compilation and so on.
 * Options:
 *     -E: preprocess only
 *     -S: do not assemble
 *     -c: do not link
 *     -i: link incrementally
 *     -O1: optimize
 *     -O0: do not optimize
 *     -I<path>: add path to include paths
 *     -o<file>: compile to file
 *     -e<label>: use <label> as entry point, default is main
 *     -x<mode>: sets the language mode for indeterminable files
 *     -: add file from stdin
 * \param argc argument count
 * \param argv argument strings
 * \returns A CCParams handle or NULL on failure.
 */
CCParams* compile_and_verify_argv(int argc, char* argv[]);

/**
 * Destroys a ccparams object without freeing the handle.
 * 
 * \param params the CCParams handle
 */
void ccparams_destroy(CCParams* params);

#endif /* _ARGS_H */