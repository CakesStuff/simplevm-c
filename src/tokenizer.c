#include<tokenizer.h>

dc_token_v dc_tokenize(CCInput* in, char* prog)
{
    dc_token_v tokens;
    VECTOR_INIT(tokens);
    size_t index = 0;
    size_t line = 1;
    ssize_t line_tokens = 0;
    char* file = malloc(strlen(in->name) + 1);
    strcpy(file, (char*)in->name);
    
    #define PUSH_TYPE(t) DCToken token = {.type = t, .line = line, .file = malloc(strlen(file)+1)}; strcpy(token.file, file); VECTOR_PUSH(tokens, token)

    while(index < in->content_size)
    {
        if(in->content[index] == '\n')
        {
            line++;
            line_tokens = -1;
        }
        else if(in->content[index] == '#')
        {
            if(line_tokens == 0 && in->content_size - index > 6)
            {
                if(!strncmp("line", (char*)&in->content[index+1], 4) && in->content[index+5] == ' ')
                {
                    size_t num_start = index + 6;
                    size_t num_len = 0;
                    while(num_start + num_len < in->content_size && isdigit(in->content[num_start + num_len]))num_len++;
                    if(num_len == 0)
                    {
                        fprintf(stderr, "%s: error: invalid #line directive (%s|%s:%lu)\n", prog, in->name, file, line);
                        goto cleanup;
                    }
                    size_t num = 0;
                    for(size_t i = num_start; i < num_start + num_len; i++)
                    {
                        num *= 10;
                        num += in->content[i] - '0';
                    }
                    if(in->content[num_start + num_len] != '\n' && in->content_size != num_start + num_len)
                    {
                        fprintf(stderr, "%s: error: trailing tokens after #line directive (%s|%s:%lu)\n", prog, in->name, file, line);
                        goto cleanup;
                    }
                    line = num;
                    line_tokens = -1;
                    index += num_len + 7 - 1;
                }
                else if(!strncmp("file", (char*)&in->content[index+1], 4) && in->content[index+5] == ' ')
                {
                    size_t file_start = index + 6;
                    size_t file_len = 0;
                    while(file_start + file_len < in->content_size && in->content[file_start + file_len] != '\n')file_len++;
                    free(file);
                    file = malloc(file_len + 1);
                    sprintf(file, "%.*s", (int)file_len, &in->content[file_start]);
                    line_tokens = -1;
                    index += file_len + 7 - 1;
                }
                else {PUSH_TYPE(DC_TOKEN_HASH);}
            }
            else {PUSH_TYPE(DC_TOKEN_HASH);}
        }
        else if(isdigit(in->content[index]))
        {
            //TODO: TOKENIZE LABEL
        }
        else if(isdigit(in->content[index]))
        {
            //TODO: TOKENIZE LITERAL
        }
        else
        {
            fprintf(stderr, "%s: error: unknown token sequence (%s|%s:%lu)\n", prog, in->name, file, line);
            goto cleanup;
        }
        //TODO: TOKENIZE AND ADD REMAINING TOKENS
    
        line_tokens++;
        index++;
    }

    if(VECTOR_SIZE(tokens) == 0)
    {
        fprintf(stderr, "%s: warn: empty input file %s\n", prog, in->name);
    }

    for(VECTOR_ITER(tokens, i))
    {
        DCToken token = VECTOR_EL(tokens, i);
        printf("Token of type %d at line %lu in file %s|%s\n", token.type, token.line, in->name, token.file);
    }

    line++;

    PUSH_TYPE(DC_TOKEN_SEMICOLON);

    #undef PUSH_TYPE

    free(file);
    return tokens;

    cleanup:
    
    for(VECTOR_ITER(tokens, i))
    {
        free(VECTOR_EL(tokens, i).file);
    }
    VECTOR_FREE(tokens);
    VECTOR_INIT(tokens);
    free(file);
    return tokens;
}
