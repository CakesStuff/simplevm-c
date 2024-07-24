#ifndef _TOKENIZER_H
#define _TOKENIZER_H
#include<core.h>
#include<linker.h>

typedef enum {
    DC_TOKEN_LABEL,
    DC_TOKEN_LITERAL,
    DC_TOKEN_HASH,
    DC_TOKEN_SEMICOLON,
} DCTokenType;

typedef struct {
    DCTokenType type;
    size_t line;
    char* file;
    union {
        uint8_t label_name[sizeof(((LinkLabel*)0)->name)];
        uint64_t literal_value;
    };
} DCToken;

DEFINE_VECTOR_OF(DCToken) dc_token_v;

/**
 * Tokenizes a dc file for either preprocessing or compiling.
 * 
 * \param input the file to tokenize.
 * \param prog program part for error printing
 * \returns A vector of tokens that is empty on failure.
 */
dc_token_v dc_tokenize(CCInput* in, char* prog);

#endif /* _TOKENIZER_H */