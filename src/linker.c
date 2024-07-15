#include<linker.h>

LinkFile* link_from_buffer(uint8_t* content, size_t size)
{
    #define LINK_LOAD_ATTR(x, t) do { if(size - index < sizeof(x)) { if(lf.labels)free(lf.labels); if(lf.relocations)free(lf.relocations); return NULL; } x = *(t*)(content + index); index += sizeof(x); } while(0)

    size_t index = 0;
    LinkFile lf = {
        .labels = NULL,
        .relocations = NULL,
    };
    LINK_LOAD_ATTR(lf.label_count, uint64_t);
    lf.labels = malloc(sizeof(LinkLabel) * lf.label_count);
    for(uint64_t i = 0; i < lf.label_count; i++)
    {
        LINK_LOAD_ATTR(lf.labels[i], LinkLabel);
    }
    LINK_LOAD_ATTR(lf.relocation_count, uint64_t);
    lf.relocations = malloc(sizeof(LinkRelocation) * lf.relocation_count);
    for(uint64_t i = 0; i < lf.relocation_count; i++)
    {
        LINK_LOAD_ATTR(lf.relocations[i], LinkRelocation);
    }
    LINK_LOAD_ATTR(lf.size, uint64_t);
    if(size - index < lf.size)
    {
        free(lf.labels);
        free(lf.relocations);
        return NULL;
    }
    lf.content = malloc(lf.size);
    memcpy(lf.content, content + index, lf.size);

    LinkFile* retval = malloc(sizeof(LinkFile));
    *retval = lf;
    return retval;
    
    #undef LINK_LOAD_ATTR
}

void link_to_buffer(LinkFile* lf, uint8_t* content)
{
    size_t index = 0;
    *(uint64_t*)(content + index) = lf->label_count;
    index += sizeof(lf->label_count);
    for(uint64_t i = 0; i < lf->label_count; i++)
    {
        *(LinkLabel*)(content + index) = lf->labels[i];
        index += sizeof(LinkLabel);
    }
    *(uint64_t*)(content + index) = lf->relocation_count;
    index += sizeof(lf->relocation_count);
    for(uint64_t i = 0; i < lf->relocation_count; i++)
    {
        *(LinkRelocation*)(content + index) = lf->relocations[i];
        index += sizeof(LinkRelocation);
    }
    *(uint64_t*)(content + index) = lf->size;
    index += sizeof(lf->size);
    memcpy(content + index, lf->content, lf->size);
}

void link_file_destroy(LinkFile* lf)
{
    free(lf->labels);
    free(lf->relocations);
    free(lf->content);
    free(lf);
}

bool link_together(inp_v* ins, size_t file_1, size_t file_2)
{
    LinkFile* lf_1 = link_from_buffer(VECTOR_EL(*ins, file_1).content, VECTOR_EL(*ins, file_1).content_size);
    if(lf_1 == NULL)
    {
        fprintf(stderr, "link: error: failed to parse object file \"%s\"\n", VECTOR_EL(*ins, file_1).name);
        link_file_destroy(lf_1);
        return false;
    }
    LinkFile* lf_2 = link_from_buffer(VECTOR_EL(*ins, file_2).content, VECTOR_EL(*ins, file_2).content_size);
    if(lf_2 == NULL)
    {
        fprintf(stderr, "link: error: failed to parse object file \"%s\"\n", VECTOR_EL(*ins, file_2).name);
        link_file_destroy(lf_1);
        link_file_destroy(lf_2);
        return false;
    }

    if(lf_1->size + lf_2->size < lf_1->size || lf_1->size + lf_2->size < lf_2->size)
    {
        fprintf(stderr, "link: error: files \"%s\" and \"%s\" are too large together\n", VECTOR_EL(*ins, file_1).name, VECTOR_EL(*ins, file_2).name);
        link_file_destroy(lf_1);
        link_file_destroy(lf_2);
        return false;
    }

    free(VECTOR_EL(*ins, file_1).content);
    free(VECTOR_EL(*ins, file_2).content);

    for(uint64_t i = 0; i < lf_2->label_count; i++)
    {
        if(lf_2->labels[i].type == LABEL_ADDRESS)
        {
            lf_2->labels[i].value += lf_1->size;
        }
    }
    for(uint64_t i = 0; i < lf_2->relocation_count; i++)
    {
        lf_2->relocations[i].offset += lf_1->size;
    }
    LinkFile lf;
    lf.labels = malloc(sizeof(LinkLabel) * (lf_1->label_count + lf_2->label_count));
    lf.label_count = lf_1->label_count;
    for(uint64_t i = 0; i < lf_1->label_count; i++)
    {
        lf.labels[i] = lf_1->labels[i];
    }
    for(uint64_t i = 0; i < lf_2->label_count; i++)
    {
        uint64_t j = 0;
        for(j = 0; j < lf_1->label_count; j++)
        {
            if(!strcmp(lf.labels[j].name, lf_2->labels[i].name))
            {
                if(lf.labels[j].type != lf_2->labels[i].type)
                {
                    fprintf(stderr, "link: error: conflicting label \"%s\" with types %hhu and %hhu in files \"%s\" and \"%s\"\n",
                        lf.labels[j].name, lf.labels[j].type, lf_2->labels[i].type, VECTOR_EL(*ins, file_1).name, VECTOR_EL(*ins, file_2).name);
                    free(lf.labels);
                    link_file_destroy(lf_1);
                    link_file_destroy(lf_2);
                    return false;
                }

                if(lf.labels[j].type == LABEL_UNDEFINED && lf_2->labels[i].type == LABEL_UNDEFINED)
                {
                    for(uint64_t n = 0; n < lf_2->relocation_count; n++)
                    {
                        if(lf_2->relocations[n].label_id == i)
                            lf_2->relocations[n].label_id = j;
                    }
                    break;
                }
                else if(lf.labels[j].type != LABEL_UNDEFINED && lf_2->labels[i].type != LABEL_UNDEFINED)
                {
                    if(lf.labels[j].value == lf_2->labels[i].value)
                    {
                        for(uint64_t n = 0; n < lf_2->relocation_count; n++)
                        {
                            if(lf_2->relocations[n].label_id == i)
                                lf_2->relocations[n].label_id = j;
                        }
                        break;
                    }
                    else
                    {
                        fprintf(stderr, "link: error: conflicting label \"%s\" with values %lu and %lu in files \"%s\" and \"%s\"\n",
                            lf.labels[j].name, lf.labels[j].value, lf_2->labels[i].value, VECTOR_EL(*ins, file_1).name, VECTOR_EL(*ins, file_2).name);
                        free(lf.labels);
                        link_file_destroy(lf_1);
                        link_file_destroy(lf_2);
                        return false;
                    }
                }
                else if(lf.labels[j].type == LABEL_UNDEFINED)
                {
                    lf.labels[j] = lf_2->labels[i];
                    for(uint64_t n = 0; n < lf_2->relocation_count; n++)
                    {
                        if(lf_2->relocations[n].label_id == i)
                            lf_2->relocations[n].label_id = j;
                    }
                    break;
                }
                else
                {
                    for(uint64_t n = 0; n < lf_2->relocation_count; n++)
                    {
                        if(lf_2->relocations[n].label_id == i)
                            lf_2->relocations[n].label_id = j;
                    }
                    break;
                }
            }
            else
            {
                lf.labels[lf.label_count] = lf_2->labels[i];
                for(uint64_t n = 0; n < lf_2->relocation_count; n++)
                {
                    if(lf_2->relocations[n].label_id == i)
                        lf_2->relocations[n].label_id = lf.label_count;
                }
                lf.label_count++;
                break;
            }
        }
    }
    lf.relocation_count = lf_1->relocation_count + lf_2->relocation_count;
    lf.relocations = malloc(sizeof(LinkRelocation) * lf.relocation_count);
    for(uint64_t i = 0; i < lf_1->relocation_count; i++)
    {
        lf.relocations[i] = lf_1->relocations[i];
    }
    for(uint64_t i = 0; i < lf_2->relocation_count; i++)
    {
        lf.relocations[lf_1->relocation_count + i] = lf_2->relocations[i];
    }
    lf.size = lf_1->size + lf_2->size;
    memcpy(lf.content, lf_1->content, lf_1->size);
    memcpy(lf.content + lf_1->size, lf_2->content, lf_2->size);
    link_file_destroy(lf_1);
    link_file_destroy(lf_2);

    char* new_name = malloc(strlen(VECTOR_EL(*ins, file_1).name) + 1 + strlen(VECTOR_EL(*ins, file_2).name) + 1);
    sprintf(new_name, "%s+%s", VECTOR_EL(*ins, file_1).name, VECTOR_EL(*ins, file_2).name);
    if(file_1 > file_2)
    {
        VECTOR_REMOVE(*ins, file_1);
        file_1 = file_2;
    }
    else
    {
        VECTOR_REMOVE(*ins, file_2);
    }
    VECTOR_EL(*ins, file_1).name = new_name;
    VECTOR_EL(*ins, file_1).content_size = lf.size + lf.label_count * sizeof(LinkLabel) + lf.relocation_count * sizeof(LinkRelocation)
                                            + sizeof(lf.label_count) + sizeof(lf.relocation_count) + sizeof(lf.size);
    VECTOR_EL(*ins, file_1).content = malloc(VECTOR_EL(*ins, file_1).content_size);
    link_to_buffer(&lf, VECTOR_EL(*ins, file_1).content);
    LinkFile* lfh = malloc(sizeof(LinkFile));
    *lfh = lf;
    link_file_destroy(lfh);

    return true;
}

bool link_resolve(CCInput* in)
{
    LinkFile* lf = link_from_buffer(in->content, in->content_size);
    if(lf == NULL)
    {
        fprintf(stderr, "link: error: failed to parse object file \"%s\"\n", in->name);
        link_file_destroy(lf);
        return false;
    }
    for(uint64_t i = 0; i < lf->relocation_count; i++)
    {
        if(lf->relocations[i].label_id >= lf->label_count)
        {
            fprintf(stderr, "link: error: relocation uses label_id %lu out of bounds [0; %lu]\n", lf->relocations[i].label_id, lf->label_count);
            link_file_destroy(lf);
            return false;
        }
        switch(lf->relocations[i].type)
        {
            case RELOCATION_INVALID:
            default:
                fprintf(stderr, "link: error: relocation of invalid type %hhu found\n", lf->relocations[i].type);
                link_file_destroy(lf);
                return false;
            case RELOCATION_UPPER12_LOWER12:
                *(uint16_t*)(lf->content + lf->relocations[i].offset) &= 0b1111000000000000;
                *(uint16_t*)(lf->content + lf->relocations[i].offset) |= ((uint16_t)lf->labels[lf->relocations[i].label_id].value) >> 4;
                break;
            case RELOCATION_LOWER4_LOWER4:
                *(uint16_t*)(lf->content + lf->relocations[i].offset) &= 0b1111111111110000;
                *(uint16_t*)(lf->content + lf->relocations[i].offset) |= lf->labels[lf->relocations[i].label_id].value & 0b0000000000001111;
                break;
        }
    }
    in->content_size = lf->size;
    free(in->content);
    in->content = malloc(in->content_size);
    memcpy(in->content, lf->content, in->content_size);
    link_file_destroy(lf);
    return true;
}
