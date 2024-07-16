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
        fclose(file);
        return false;
    }
    else
    {
        fclose(file);
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

void file_adjust_ending(CCInput* in)
{
    char* ending = strrchr(in->name, '.');
    size_t len;
    if(ending)
    {
        len = ending - in->name;
    }
    else
    {
        len = strlen(in->name);
    }
    char* new_name = malloc(len + 4 /* MAX ENDING SIZE (WITH DOT) */ + 1);
    sprintf(new_name, "%.*s.%s", len, in->name, 
        in->type == MODE_DC ? "pdc" : in->type == MODE_ASM ? "asm" : in->type == MODE_OBJ ? "o" : "inv");
    in->name = new_name;
}

bool file_write(CCInput* in, char* path)
{
    FILE* file = fopen(path, "wb");
    if(!file)
    {
        return false;
    }
    if(fwrite(in->content, in->content_size, 1, file) != 1)
    {
        fclose(file);
        return false;
    }
    fclose(file);
    return true;
}
