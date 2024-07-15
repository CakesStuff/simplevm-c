#include<files.h>

bool file_load(CCInput* in)
{
    if(!strcmp(in->name, "-"))return file_load_stdin(in);

    FILE* file;
    if(in->type != MODE_OBJ)
    {
        file = fopen(in->name, "r");
    }
    else
    {
        file = fopen(in->name, "rb");
    }
    
    if(file == NULL)
    {
        return false;
    }

    fseek(file, 0, SEEK_END);
    in->content_size = ftell(file);
    rewind(file);

    in->content = malloc(in->content_size);
    if(fread(in->content, in->content_size, 1, file) != 1)
    {
        free(in->content);
        return false;
    }
    else
    {
        return true;
    }
}

bool file_load_stdin(CCInput* in)
{
    VariableBuffer buf;
    VECTOR_INIT(buf);
    uint8_t c;
    while(read(0, &c, 1))
    {
        VECTOR_PUSH(buf, c);
    }
    in->content_size = VECTOR_SIZE(buf);
    in->content = malloc(in->content_size);
    memcpy(in->content, VECTOR_AT(buf, 0), in->content_size);
    VECTOR_FREE(buf);
    return true;
}
