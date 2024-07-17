#include<assembler.h>

uint16_t make_ins0(OpCode code, Register X, Register Y, Nibble Z)
{
    return ((X & 0b111) << 12) | ((Y & 0b111) << 9) | ((code & 0b11111) << 4) | (Z & 0b1111);
}

uint16_t make_ins1(Register R, Lit12Bit lit12bit)
{
    return 0b1000000000000000 | ((R & 0b111) << 12) | (lit12bit & 0xfff);
}

uint16_t make_op_add(Register dst, Register op1, Register op2)
{
    return make_ins0(OpAdd, op1, op2, dst);
}

uint16_t make_op_sub(Register dst, Register num, Register sub)
{
    return make_ins0(OpSub, num, sub, dst);
}

uint16_t make_op_add_imm(Register dst, Lit7Bit num)
{
    return make_ins0(OpAddImm, dst, (num & 0b1110000) >> 4, num & 0b1111);
}

uint16_t make_op_add_imm_signed(Register dst, Lit7Bit num)
{
    return make_ins0(OpAddImmSigned, dst, (num & 0b1110000) >> 4, num & 0b1111);
}

uint16_t make_op_shift_left(Register dst, Register src, Nibble amount)
{
    return make_ins0(OpShiftLeft, src, dst, amount);
}

uint16_t make_op_shift_right_logical(Register dst, Register src, Nibble amount)
{
    return make_ins0(OpShiftRightLogical, src, dst, amount);
}

uint16_t make_op_shift_right_arithmetic(Register dst, Register src, Nibble amount)
{
    return make_ins0(OpShiftRightArithmetic, src, dst, amount);
}

uint16_t make_op_load_word(Register dst, Register segment, Register addr)
{
    return make_ins0(OpLoadWord, dst, addr, segment);
}

uint16_t make_op_store_word(Register dst, Register segment, Register addr)
{
    return make_ins0(OpStoreWord, dst, addr, segment);
}

uint16_t make_op_jump_offset(Lit10Bit amount)
{
    return make_ins0(OpJumpOffset, (amount & 0b1110000000) >> 7, (amount & 0b1110000) >> 4, amount & 0b1111);
}

uint16_t make_op_test(Register one, Register two, TestOp to)
{
    return make_ins0(OpTest, one, two, to);
}

uint16_t make_op_add_if(Register dst, Register src, Nibble half)
{
    return make_ins0(OpAddIf, dst, src, half);
}

uint16_t make_op_stack(Register reg, Register sp, StackOp so)
{
    return make_ins0(OpStack, reg, sp, so);
}

uint16_t make_op_load_stack_offset(Register dst, Register stack, Nibble half_off)
{
    return make_ins0(OpLoadStackOffset, dst, stack, half_off);
}

uint16_t make_op_system_fast(Nibble handler, Register arg)
{
    return make_ins0(OpSystem, Zero, arg, handler);
}

uint16_t make_op_system_slow(Register handler, Nibble arg)
{
    return make_ins0(OpSystem, handler, 0, arg);
}

uint16_t make_op_set_and_save(Register to_set, Register num, Register to_save)
{
    return make_ins0(OpSetAndSave, num, to_save, to_set);
}

uint16_t make_op_add_and_save(Register to_add_to, Register num, Register to_save)
{
    return make_ins0(OpAddAndSave, num, to_save, to_add_to);
}

uint16_t make_op_load_byte(Register dst, Register segment, Register addr)
{
    return make_ins0(OpLoadByte, dst, addr, segment);
}

uint16_t make_op_store_byte(Register dst, Register segment, Register addr)
{
    return make_ins0(OpStoreByte, dst, addr, segment);
}

uint16_t make_op_mul(Register dst, Register op1, Register op2)
{
    return make_ins0(OpMul, op1, op2, dst);
}

uint16_t make_op_imm(Register reg, Lit12Bit val)
{
    return make_ins1(reg, val);
}

bool parse_uint64_t(char* str, size_t str_len, uint64_t* out)
{
    if(str_len < 1)return false;
    if(str[0] == '\'' && str_len == 3 && str[2] == '\'')
    {
        *out = str[1];
        return true;
    }
    size_t value_len = 0;
    uint64_t value = 0;
    int factor = 10;
    if(str[value_len] == '0')
    {
        factor = 8;
        value_len++;
        if(value_len < str_len && str[value_len] == 'x')
        {
            factor = 16;
            value_len++;
            if(value_len == str_len)return false;
        }
        else if(value_len < str_len && str[value_len] == 'b')
        {
            factor = 2;
            value_len++;
            if(value_len == str_len)return false;
        }
    }
    if(value_len == str_len && factor != 8)return false;
    while(value_len < str_len && isxdigit(str[value_len]))
    {
        if(factor == 16 && !isdigit(str[value_len]))
        {
            value *= factor;
            value += tolower(str[value_len]) - 'a' + 10;
        }
        else if(!isdigit(str[value_len]))
        {
            return false;
        }
        else if(str[value_len] - '0' >= factor)
        {
            return false;
        }
        else
        {
            value *= factor;
            value += str[value_len] - '0';
        }
        value_len++;
    }
    if(value_len != str_len)
    {
        return false;
    }
    *out = value;
    return true;
}

char* AsmInstructionStrings[] = {
    [ASM_INS_MOV] = "mov",
    [ASM_INS_SHL] = "shl",
    [ASM_INS_SHR] = "shr",
    [ASM_INS_SHRS] = "shrs",
    [ASM_INS_MUL] = "mul",
    [ASM_INS_ADD] = "add",
    [ASM_INS_SUB] = "sub",
    [ASM_INS_LEA] = "lea",
};

token_v asm_tokenize(uint8_t* content, size_t size, char* filename)
{
    token_v tokens;
    VECTOR_INIT(tokens);
    size_t index = 0;
    size_t line = 1;

    #define PUSH_TYPE(t) AsmToken token = {.type = t, .line = line}; VECTOR_PUSH(tokens, token)

    while(index < size)
    {
        if(content[index] == '\n')
        {
            PUSH_TYPE(ASM_TOKEN_NEWLINE);
            line++;
        }
        else if(content[index] == ';')
        {
            while(index < size && content[index] != '\n')index++;
            index--;
        }
        else if(isspace(content[index]));
        else if(content[index] == '*') {PUSH_TYPE(ASM_TOKEN_MUL);}
        else if(content[index] == '[') {PUSH_TYPE(ASM_TOKEN_OPEN_SQUARE);}
        else if(content[index] == ']') {PUSH_TYPE(ASM_TOKEN_CLOSED_SQUARE);}
        else if(content[index] == '+')
            if((size - index == 1) || content[index+1] != '>') {PUSH_TYPE(ASM_TOKEN_PLUS);}
            else {index++;PUSH_TYPE(ASM_TOKEN_SHR_P);}
        else if(content[index] == '-')
            if((size - index == 1) || content[index+1] != '>') {PUSH_TYPE(ASM_TOKEN_MINUS);}
            else {index++;PUSH_TYPE(ASM_TOKEN_SHR_M);}
        else if(content[index] == ':') {PUSH_TYPE(ASM_TOKEN_COLON);}
        else if(content[index] == ',') {PUSH_TYPE(ASM_TOKEN_COMMA);}
        else if(content[index] == '<')
            if((size - index > 1) && content[index+1] == '<') {PUSH_TYPE(ASM_TOKEN_SHL);}
            else
            {
                fprintf(stderr, "asm: error: invalid token '<' in %s:%lu\n", filename, line);
                VECTOR_FREE(tokens);
                VECTOR_INIT(tokens);
                return tokens;
            }
        else if(isalpha(content[index]))
        {
            size_t len = 0;
            while(index + len < size && isalnum(content[index + len]))len++;
            size_t i = 0;
            for(i = 0; i < ARRAYSIZE(AsmInstructionStrings); i++)
            {
                if(strlen(AsmInstructionStrings[i]) == len && !strncmp((char*)&content[index], AsmInstructionStrings[i], len))
                {
                    AsmToken token = {
                        .line = line,
                        .type = ASM_TOKEN_INSTRUCTION,
                        .instruction = i,
                    };
                    VECTOR_PUSH(tokens, token);
                    break;
                }
            }
            if(len >= 2 && i == ARRAYSIZE(AsmInstructionStrings))
            {
                if(len >= 3)
                {
                    if(len >= 4)
                    {
                        if(len >= 6)
                        {
                            if(!strncmp((char*)&content[index], "extern", len)) {PUSH_TYPE(ASM_TOKEN_EXTERN);}
                            else
                            {
                                if(len > 31)
                                {
                                    fprintf(stderr, "asm: error: label exceeds length in %s:%lu\n", filename, line);
                                    VECTOR_FREE(tokens);
                                    VECTOR_INIT(tokens);
                                    return tokens;
                                }
                                AsmToken token = {
                                    .line = line,
                                    .type = ASM_TOKEN_LABEL,
                                };
                                sprintf((char*)token.label_name, "%.*s", (int)len, (char*)&content[index]);
                                VECTOR_PUSH(tokens, token);
                            }
                        }
                        else if(!strncmp((char*)&content[index], "WORD", len)) {PUSH_TYPE(ASM_TOKEN_WORD);}
                        else if(!strncmp((char*)&content[index], "BYTE", len)) {PUSH_TYPE(ASM_TOKEN_BYTE);}
                        else if(!strncmp((char*)&content[index], "Zero", len))
                        {
                            AsmToken token = {
                                .line = line,
                                .reg = Zero,
                                .type = ASM_TOKEN_REGISTER,
                            };
                            VECTOR_PUSH(tokens, token);
                        }
                        else
                        {
                            AsmToken token = {
                                .line = line,
                                .type = ASM_TOKEN_LABEL,
                            };
                            sprintf((char*)token.label_name, "%.*s", (int)len, (char*)&content[index]);
                            VECTOR_PUSH(tokens, token);
                        }
                    }
                    else if(!strncmp((char*)&content[index], "equ", 3)) {PUSH_TYPE(ASM_TOKEN_EQU);}
                    else
                    {
                        AsmToken token = {
                            .line = line,
                            .type = ASM_TOKEN_LABEL,
                        };
                        sprintf((char*)token.label_name, "%.*s", (int)len, (char*)&content[index]);
                        VECTOR_PUSH(tokens, token);
                    }
                }
                else if(!strncmp((char*)&content[index], "SP", 2))
                {
                    AsmToken token = {
                        .line = line,
                        .type = ASM_TOKEN_REGISTER,
                        .reg = SP,
                    };
                    VECTOR_PUSH(tokens, token);
                }
                else if(!strncmp((char*)&content[index], "BP", 2))
                {
                    AsmToken token = {
                        .line = line,
                        .type = ASM_TOKEN_REGISTER,
                        .reg = BP,
                    };
                    VECTOR_PUSH(tokens, token);
                }
                else if(!strncmp((char*)&content[index], "PC", 2))
                {
                    AsmToken token = {
                        .line = line,
                        .type = ASM_TOKEN_REGISTER,
                        .reg = PC,
                    };
                    VECTOR_PUSH(tokens, token);
                }
                else if(!strncmp((char*)&content[index], "db", 2)) {PUSH_TYPE(ASM_TOKEN_DB);}
                else if(!strncmp((char*)&content[index], "dw", 2)) {PUSH_TYPE(ASM_TOKEN_DW);}
                else
                {
                    AsmToken token = {
                        .line = line,
                        .type = ASM_TOKEN_LABEL,
                    };
                    sprintf((char*)token.label_name, "%.*s", (int)len, (char*)&content[index]);
                    VECTOR_PUSH(tokens, token);
                }
            }
            else if(i == ARRAYSIZE(AsmInstructionStrings))
            {
                switch(content[index])
                {
                    case 'A':
                    case 'B':
                    case 'C':
                    case 'M':
                    {
                        AsmToken token = {
                            .line = line,
                            .type = ASM_TOKEN_REGISTER,
                            .reg = content[index] == 'A' ? A : content[index] == 'B' ? B : content[index] == 'C' ? C : M,
                        };
                        VECTOR_PUSH(tokens, token);
                    }
                        break;
                    default:
                    {
                        AsmToken token = {
                            .line = line,
                            .type = ASM_TOKEN_LABEL,
                        };
                        sprintf((char*)token.label_name, "%.*s", (int)len, (char*)&content[index]);
                        VECTOR_PUSH(tokens, token);
                    }
                        break;
                }
            }
            index += len - 1;
        }
        else if(content[index] == '.')
        {
            if(size - index < 2)
            {
                fprintf(stderr, "asm: warn: trailing dot at the end of %s:%lu\n", filename, line);
                VECTOR_FREE(tokens);
                VECTOR_INIT(tokens);
                return tokens;
            }
            size_t len = 1;
            while(index + len < size && isalnum(content[index + len]))len++;
            if(len == 1)
            {
                fprintf(stderr, "asm: error: . is invalid except if used to mark local labels (%s:%lu)\n", filename, line);
                VECTOR_FREE(tokens);
                VECTOR_INIT(tokens);
                return tokens;
            }
            if(len > 31)
            {
                fprintf(stderr, "asm: error: label exceeds length in %s:%lu\n", filename, line);
                VECTOR_FREE(tokens);
                VECTOR_INIT(tokens);
                return tokens;
            }
            AsmToken token = {
                .line = line,
                .type = ASM_TOKEN_LABEL,
            };
            sprintf((char*)token.label_name, "%.*s", (int)len, (char*)&content[index]);
            VECTOR_PUSH(tokens, token);
            index += len - 1;
        }
        else
        {
            size_t len = 0;
            while(index + len < size && (isalnum(content[index + len]) || content[index + len] == 'x' || content[index + len] == 'b'))len++;
            uint64_t value;
            if(!parse_uint64_t((char*)&content[index], len, &value))
            {
                fprintf(stderr, "asm: error: failed to parse literal in %s:%lu\n", filename, line);
                VECTOR_FREE(tokens);
                VECTOR_INIT(tokens);
                return tokens;
            }
            index += len - 1;
            AsmToken token = {
                .line = line,
                .literal_value = value,
                .type = ASM_TOKEN_LITERAL,
            };
            VECTOR_PUSH(tokens, token);
        }
        index++;
    }

    if(VECTOR_SIZE(tokens) == 0)
    {
        fprintf(stderr, "asm: warn: empty input file %s\n", filename);
    }

    PUSH_TYPE(ASM_TOKEN_NEWLINE);
    line++;

    #undef PUSH_TYPE

    return tokens;
}

label_v* asm_first_pass(token_v* tokens, char* filename)
{
    label_v labels;
    VECTOR_INIT(labels);
    LinkLabel* current = NULL;

    for(size_t i = 0; i < VECTOR_SIZE(*tokens);)
    {
        switch(VECTOR_EL(*tokens, i).type)
        {
            case ASM_TOKEN_EXTERN:
            {
                AsmToken first = VECTOR_EL(*tokens, i);
                VECTOR_CUT(*tokens, i);
                if(VECTOR_SIZE(*tokens) == i)
                {
                    fprintf(stderr, "asm: error: extern keyword is missing a label in %s:%lu\n",
                        filename, first.line);
                    VECTOR_FREE(labels);
                    return NULL;
                }
                AsmToken label = VECTOR_EL(*tokens, i);
                VECTOR_CUT(*tokens, i);
                if(label.type != ASM_TOKEN_LABEL)
                {
                    fprintf(stderr, "asm: error: extern keyword is missing a label in %s:%lu\n",
                        filename, first.line);
                    VECTOR_FREE(labels);
                    return NULL;
                }
                if(i != VECTOR_SIZE(*tokens))
                {
                    AsmToken newline = VECTOR_EL(*tokens, i);
                    VECTOR_CUT(*tokens, i);
                    if(newline.type != ASM_TOKEN_NEWLINE)
                    {
                        fprintf(stderr, "asm: error: extra tokens in %s:%lu\n", filename, first.line);
                        VECTOR_FREE(labels);
                        return NULL;
                    }
                }

                if(label.label_name[0] == '.')
                {
                    fprintf(stderr, "asm: error: extern can not be used with local labels (%s:%lu)\n", filename, label.line);
                    VECTOR_FREE(labels);
                    return NULL;
                }

                size_t j;
                for(j = 0; j < VECTOR_SIZE(labels); j++)
                {
                    if(!strcmp((char*)label.label_name, (char*)VECTOR_EL(labels, j).name))
                    {
                        fprintf(stderr, "asm: error: cannot redefine label %s in %s:%lu\n", label.label_name, filename, first.line);
                        VECTOR_FREE(labels);
                        return NULL;
                    }
                }

                LinkLabel newLabel = {
                    .type = LABEL_UNDEFINED,
                };
                strcpy((char*)newLabel.name, (char*)label.label_name);
                VECTOR_PUSH(labels, newLabel);
            }
                break;
            case ASM_TOKEN_LABEL:
            {
                AsmToken label = VECTOR_EL(*tokens, i);
                i++;
                if(i == VECTOR_SIZE(*tokens) || VECTOR_EL(*tokens, i).type == ASM_TOKEN_NEWLINE)
                {
                    uint8_t name[32];
                    if(label.label_name[0] == '.')
                    {
                        if(current == NULL)
                        {
                            fprintf(stderr, "asm: error: local label in global scope in %s:%lu\n", filename, label.line);
                            VECTOR_FREE(labels);
                            return NULL;
                        }
                        else if(strlen((char*)label.label_name) + strlen((char*)current->name) > 31)
                        {
                            fprintf(stderr, "asm: error: label %s%s exceeds label size in %s:%lu\n",
                                current->name, label.label_name, filename, label.line);
                            VECTOR_FREE(labels);
                            return NULL;
                        }
                        else
                        {
                            sprintf((char*)name, "%s%s", current->name, label.label_name);
                        }
                    }
                    else
                    {
                        strcpy((char*)name, (char*)label.label_name);
                    }

                    fprintf(stderr, "asm: warn: label without ':' in %s:%lu\n", filename, label.line);

                    size_t j;
                    for(j = 0; j < VECTOR_SIZE(labels); j++)
                    {
                        if(!strcmp((char*)name, (char*)VECTOR_EL(labels, j).name))
                        {
                            fprintf(stderr, "asm: error: cannot redefine label %s (%s:%lu)\n", name, filename, label.line);
                            VECTOR_FREE(labels);
                            return NULL;
                        }
                    }

                    LinkLabel newLabel = {
                        .type = LABEL_ADDRESS,
                    };
                    strcpy((char*)newLabel.name, (char*)name);
                    VECTOR_PUSH(labels, newLabel);
                    if(label.label_name[0] != '.')current = VECTOR_AT(labels, j);
                    
                    i++;
                }
                else
                {
                    AsmToken separator = VECTOR_EL(*tokens, i);
                    i++;
                    if(separator.type == ASM_TOKEN_COLON)
                    {
                        if(i != VECTOR_SIZE(*tokens) && VECTOR_EL(*tokens, i).type != ASM_TOKEN_NEWLINE)
                        {
                            fprintf(stderr, "asm: error: syntax error during label declaration in %s:%lu\n", filename, label.line);
                            VECTOR_FREE(labels);
                            return NULL;
                        }
                        uint8_t name[32];
                        if(label.label_name[0] == '.')
                        {
                            if(current == NULL)
                            {
                                fprintf(stderr, "asm: error: local label in global scope in %s:%lu\n", filename, label.line);
                                VECTOR_FREE(labels);
                                return NULL;
                            }
                            else if(strlen((char*)label.label_name) + strlen((char*)current->name) > 31)
                            {
                                fprintf(stderr, "asm: error: label %s%s exceeds label size in %s:%lu\n",
                                    current->name, label.label_name, filename, label.line);
                                VECTOR_FREE(labels);
                                return NULL;
                            }
                            else
                            {
                                sprintf((char*)name, "%s%s", current->name, label.label_name);
                            }
                        }
                        else
                        {
                            strcpy((char*)name, (char*)label.label_name);
                        }

                        size_t j;
                        for(j = 0; j < VECTOR_SIZE(labels); j++)
                        {
                            if(!strcmp((char*)name, (char*)VECTOR_EL(labels, j).name))
                            {
                                fprintf(stderr, "asm: error: cannot redefine label %s (%s:%lu)\n", name, filename, label.line);
                                VECTOR_FREE(labels);
                                return NULL;
                            }
                        }

                        LinkLabel newLabel = {
                            .type = LABEL_ADDRESS,
                        };
                        strcpy((char*)newLabel.name, (char*)name);
                        VECTOR_PUSH(labels, newLabel);
                        if(label.label_name[0] != '.')current = VECTOR_AT(labels, j);
                        
                        i++;
                    }
                    else if(separator.type == ASM_TOKEN_EQU)
                    {
                        i -= 2;
                        VECTOR_CUT(*tokens, i);
                        VECTOR_CUT(*tokens, i);
                        AsmToken value = VECTOR_EL(*tokens, i);
                        VECTOR_CUT(*tokens, i);
                        size_t number;
                        if(value.type == ASM_TOKEN_MINUS)
                        {
                            value = VECTOR_EL(*tokens, i);
                            VECTOR_CUT(*tokens, i);
                            if(value.type != ASM_TOKEN_LITERAL)
                            {
                                fprintf(stderr, "asm: error: trailing tokens during label declaration in %s:%lu\n", filename, label.line);
                                VECTOR_FREE(labels);
                                return NULL;
                            }
                            number = -value.literal_value;
                        }
                        else if(value.type == ASM_TOKEN_LITERAL)
                        {
                            number = value.literal_value;
                        }
                        else
                        {
                            fprintf(stderr, "asm: error: syntax error during lable declaration in %s:%lu\n", filename, label.line);
                            VECTOR_FREE(labels);
                            return NULL;
                        }
                        if(i != VECTOR_SIZE(*tokens) && VECTOR_EL(*tokens, i).type != ASM_TOKEN_NEWLINE)
                        {
                            fprintf(stderr, "asm: error: trailing tokens during label declaration in %s:%lu\n", filename, label.line);
                            VECTOR_FREE(labels);
                            return NULL;
                        }
                        VECTOR_CUT(*tokens, i);

                        uint8_t name[32];
                        if(label.label_name[0] == '.')
                        {
                            if(current == NULL)
                            {
                                fprintf(stderr, "asm: error: local label in global scope in %s:%lu\n", filename, label.line);
                                VECTOR_FREE(labels);
                                return NULL;
                            }
                            else if(strlen((char*)label.label_name) + strlen((char*)current->name) > 31)
                            {
                                fprintf(stderr, "asm: error: label %s%s exceeds label size in %s:%lu\n",
                                    current->name, label.label_name, filename, label.line);
                                VECTOR_FREE(labels);
                                return NULL;
                            }
                            else
                            {
                                sprintf((char*)name, "%s%s", current->name, label.label_name);
                            }
                        }
                        else
                        {
                            strcpy((char*)name, (char*)label.label_name);
                        }

                        size_t j;
                        for(j = 0; j < VECTOR_SIZE(labels); j++)
                        {
                            if(!strcmp((char*)name, (char*)VECTOR_EL(labels, j).name))
                            {
                                fprintf(stderr, "asm: error: cannot redefine label %s (%s:%lu)\n", name, filename, label.line);
                                VECTOR_FREE(labels);
                                return NULL;
                            }
                        }

                        LinkLabel newLabel = {
                            .type = LABEL_ABSVAL,
                            .value = number,
                        };
                        strcpy((char*)newLabel.name, (char*)name);
                        VECTOR_PUSH(labels, newLabel);
                        if(label.label_name[0] != '.')current = VECTOR_AT(labels, j);
                    }
                    else
                    {
                        fprintf(stderr, "asm: error: syntax error during label declaration in %s:%lu\n", filename, label.line);
                        VECTOR_FREE(labels);
                        return NULL;
                    }
                }
            }
                break;
            default:
                while(VECTOR_EL(*tokens, i++).type != ASM_TOKEN_NEWLINE);
                break;
        }
    }

    label_v* retval = malloc(sizeof(label_v));
    *retval = labels;
    return retval;
}

bool asm_assemble(CCInput* in)
{
    token_v tokens = asm_tokenize(in->content, in->content_size, in->name);
    if(tokens.size == 0)
    {
        VECTOR_FREE(tokens);
        return false;
    }
    label_v* labels = asm_first_pass(&tokens, in->name);
    if(labels == NULL)
    {
        VECTOR_FREE(tokens);
        return false;
    }
    rel_v relocations;
    VECTOR_INIT(relocations);
    ins_v instructions;
    VECTOR_INIT(instructions);
    LinkLabel* current = NULL;
    
    for(VECTOR_ITER(tokens, i))
    {
        switch(VECTOR_EL(tokens, i).type)
        {
            case ASM_TOKEN_DB:
            {
                bool first = false;
                uint16_t prev = 0;

                while(true)
                {
                    i++;
                    AsmToken value = VECTOR_EL(tokens, i);
                    uint16_t number;
                    if(value.type == ASM_TOKEN_MINUS)
                    {
                        i++;
                        value = VECTOR_EL(tokens, i);
                        if(value.type != ASM_TOKEN_LITERAL)
                        {
                            fprintf(stderr, "asm: error: expected literal during db in %s:%lu\n", in->name, value.line);
                            goto cleanup;
                        }
                        if(value.literal_value > -INT8_MIN)
                        {
                            fprintf(stderr, "asm: warn: value overflow during db in %s:%lu\n", in->name, value.line);
                        }
                        number = -value.literal_value;
                    }
                    else if(value.type == ASM_TOKEN_LITERAL)
                    {
                        if(value.literal_value > UINT8_MAX)
                        {
                            fprintf(stderr, "asm: warn: value overflow during db in %s:%lu\n", in->name, value.line);
                        }
                        number = value.literal_value;
                    }
                    else
                    {
                        fprintf(stderr, "asm: error: unexpected token during db in %s:%lu\n", in->name, value.line);
                        goto cleanup;
                    }

                    if(first)
                    {
                        VECTOR_PUSH(instructions, (number << 8) | prev);
                        first = false;
                    }
                    else
                    {
                        first = true;
                        prev = number;
                    }
                    i++;

                    AsmToken after = VECTOR_EL(tokens, i);
                    if(after.type == ASM_TOKEN_NEWLINE)break;
                    else if(after.type == ASM_TOKEN_COMMA);
                    else
                    {
                        fprintf(stderr, "asm: error: unexpected token during db in %s:%lu\n", in->name, value.line);
                        goto cleanup;
                    }
                }
            
                if(first)
                {
                    VECTOR_PUSH(instructions, prev);
                }
            }
                break;
            case ASM_TOKEN_DW:
            {
                while(true)
                {
                    i++;
                    AsmToken value = VECTOR_EL(tokens, i);
                    uint16_t number;
                    if(value.type == ASM_TOKEN_MINUS)
                    {
                        i++;
                        value = VECTOR_EL(tokens, i);
                        if(value.type != ASM_TOKEN_LITERAL)
                        {
                            fprintf(stderr, "asm: error: expected literal during dw in %s:%lu\n", in->name, value.line);
                            goto cleanup;
                        }
                        if(value.literal_value > -INT16_MIN)
                        {
                            fprintf(stderr, "asm: warn: value overflow during dw in %s:%lu\n", in->name, value.line);
                        }
                        number = -value.literal_value;
                    }
                    else if(value.type == ASM_TOKEN_LITERAL)
                    {
                        if(value.literal_value > UINT16_MAX)
                        {
                            fprintf(stderr, "asm: warn: value overflow during dw in %s:%lu\n", in->name, value.line);
                        }
                        number = value.literal_value;
                    }
                    else
                    {
                        fprintf(stderr, "asm: error: unexpected token during dw in %s:%lu\n", in->name, value.line);
                        goto cleanup;
                    }

                    VECTOR_PUSH(instructions, number);
                    i++;

                    AsmToken after = VECTOR_EL(tokens, i);
                    if(after.type == ASM_TOKEN_NEWLINE)break;
                    else if(after.type == ASM_TOKEN_COMMA);
                    else
                    {
                        fprintf(stderr, "asm: error: unexpected token during dw in %s:%lu\n", in->name, value.line);
                        goto cleanup;
                    }
                }
            }
                break;
            case ASM_TOKEN_INSTRUCTION:
            {
                AsmToken instruction = VECTOR_EL(tokens, i);
                i++;
                switch(instruction.instruction)
                {
                    #if 1
                    #define REG_NUM(op, insf, bound) \
                        AsmToken dst = VECTOR_EL(tokens, i); \
                        i++; \
                        if(dst.type != ASM_TOKEN_REGISTER) \
                        { \
                            fprintf(stderr, "asm: error: "#op" expects a register and a number separated by ',' (%s:%lu)\n", in->name, dst.line); \
                            goto cleanup; \
                        } \
                        AsmToken comma = VECTOR_EL(tokens, i); \
                        i++; \
                        if(comma.type != ASM_TOKEN_COMMA) \
                        { \
                            fprintf(stderr, "asm: error: "#op" expects a register and a number separated by ',' (%s:%lu)\n", in->name, dst.line); \
                            goto cleanup; \
                        } \
                        AsmToken num = VECTOR_EL(tokens, i); \
                        i++; \
                        if(num.type != ASM_TOKEN_LITERAL) \
                        { \
                            fprintf(stderr, "asm: error: "#op" expects a register and a number separated by ',' (%s:%lu)\n", in->name, dst.line); \
                            goto cleanup; \
                        } \
                        AsmToken newLine = VECTOR_EL(tokens, i); \
                        if(newLine.type != ASM_TOKEN_NEWLINE) \
                        { \
                            fprintf(stderr, "asm: error: trailing tokens during "#op" in (%s:%lu)\n", in->name, dst.line); \
                            goto cleanup; \
                        } \
                        if(num.literal_value > bound) \
                        { \
                            fprintf(stderr, "asm: error: %lu out of bounds [0-%d] during "#op" in %s:%lu\n", num.literal_value, bound, in->name, dst.line); \
                            goto cleanup; \
                        } \
                        VECTOR_PUSH(instructions, insf(dst.reg, dst.reg, num.literal_value))
                    #endif

                    case ASM_INS_SHL:
                    {
                        REG_NUM(shl, make_op_shift_left, 15);
                    }
                        break;
                    case ASM_INS_SHR:
                    {
                        REG_NUM(shr, make_op_shift_right_logical, 15);
                    }
                        break;
                    case ASM_INS_SHRS:
                    {
                        REG_NUM(shr, make_op_shift_right_arithmetic, 15);
                    }
                        break;
                    case ASM_INS_MOV:
                    //TODO:
                        break;
                    case ASM_INS_LEA:
                    {
                        AsmToken dst = VECTOR_EL(tokens, i);
                        i++;
                        if(dst.type != ASM_TOKEN_REGISTER)
                        {
                            fprintf(stderr, "asm: error: lea expects a register and a label (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken comma = VECTOR_EL(tokens, i);
                        i++;
                        if(comma.type != ASM_TOKEN_COMMA)
                        {
                            fprintf(stderr, "asm: error: lea expects a register and a label (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken label = VECTOR_EL(tokens, i);
                        i++;
                        if(label.type != ASM_TOKEN_LABEL)
                        {
                            fprintf(stderr, "asm: error: lea expects a register and a label (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken newLine = VECTOR_EL(tokens, i);
                        if(newLine.type != ASM_TOKEN_NEWLINE)
                        {
                            fprintf(stderr, "asm: error: trailing tokens after lea (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        
                        uint8_t name[32];
                        if(label.label_name[0] == '.' && (strlen((char*)current->name) + strlen((char*)label.label_name) < 32))
                        {
                            sprintf((char*)name, "%s%s", current->name, label.label_name);
                        }
                        else
                        {
                            strcpy((char*)name, (char*)label.label_name);
                        }

                        size_t j;
                        for(j = 0; j < VECTOR_SIZE(*labels); j++)
                        {
                            if(!strcmp((char*)VECTOR_EL(*labels, j).name, (char*)name))
                            {
                                LinkRelocation rel = {
                                    .label_id = j,
                                    .offset = VECTOR_SIZE(instructions) * 2,
                                    .type = RELOCATION_UPPER12_LOWER12,
                                };
                                VECTOR_PUSH(relocations, rel);
                                VECTOR_PUSH(instructions, make_op_imm(dst.reg, 0));
                                VECTOR_PUSH(instructions, make_op_shift_left(dst.reg, dst.reg, 4));
                                rel = (LinkRelocation){
                                    .label_id = j,
                                    .offset = VECTOR_SIZE(instructions) * 2,
                                    .type = RELOCATION_LOWER4_LOWER4,
                                };
                                VECTOR_PUSH(relocations, rel);
                                VECTOR_PUSH(instructions, make_op_add_imm(dst.reg, 0));
                            }
                        }
                    }
                        break;
                    case ASM_INS_MUL:
                    {
                        AsmToken dst = VECTOR_EL(tokens, i);
                        i++;
                        if(dst.type != ASM_TOKEN_REGISTER)
                        {
                            fprintf(stderr, "asm: error: mul expects two registers separated by ',' (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken comma = VECTOR_EL(tokens, i);
                        i++;
                        if(comma.type != ASM_TOKEN_COMMA)
                        {
                            fprintf(stderr, "asm: error: mul expects two registers separated by ',' (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken mul = VECTOR_EL(tokens, i);
                        i++;
                        if(mul.type != ASM_TOKEN_REGISTER)
                        {
                            fprintf(stderr, "asm: error: mul expects two registers separated by ',' (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken newLine = VECTOR_EL(tokens, i);
                        if(newLine.type != ASM_TOKEN_NEWLINE)
                        {
                            fprintf(stderr, "asm: error: trailing tokens during mul in %s:%lu\n", in->name, dst.line);
                            goto cleanup;
                        }
                        VECTOR_PUSH(instructions, make_op_mul(dst.reg, dst.reg, mul.reg));
                    }
                        break;
                    case ASM_INS_ADD:
                    {
                        AsmToken dst = VECTOR_EL(tokens, i);
                        i++;
                        if(dst.type != ASM_TOKEN_REGISTER)
                        {
                            fprintf(stderr, "asm: error: add expects a register and an operand (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken comma = VECTOR_EL(tokens, i);
                        i++;
                        if(comma.type != ASM_TOKEN_COMMA)
                        {
                            fprintf(stderr, "asm: error: add expects a register and an operand (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken op = VECTOR_EL(tokens, i);
                        i++;
                        if(op.type != ASM_TOKEN_LITERAL && op.type != ASM_TOKEN_REGISTER)
                        {
                            fprintf(stderr, "asm: error: add expects a register and an operand (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken newLine = VECTOR_EL(tokens, i);
                        if(newLine.type != ASM_TOKEN_NEWLINE)
                        {
                            fprintf(stderr, "asm: error: trailing tokens after add in %s:%lu\n", in->name, dst.line);
                            goto cleanup;
                        }
                        
                        if(op.type == ASM_TOKEN_LITERAL)
                        {
                            if(op.literal_value >= 0b10000000)
                            {
                                fprintf(stderr, "asm: error: %lu is out of bounds [0-%d[ for add (%s:%lu)\n",
                                    op.literal_value, 0b10000000, in->name, dst.line);
                                goto cleanup;
                            }
                            VECTOR_PUSH(instructions, make_op_add_imm(dst.reg, op.literal_value));
                        }
                        else
                        {
                            VECTOR_PUSH(instructions, make_op_add(dst.reg, dst.reg, op.reg));
                        }
                    }
                        break;
                    case ASM_INS_SUB:
                    {
                        AsmToken dst = VECTOR_EL(tokens, i);
                        i++;
                        if(dst.type != ASM_TOKEN_REGISTER)
                        {
                            fprintf(stderr, "asm: error: sub expects a register and an operand (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken comma = VECTOR_EL(tokens, i);
                        i++;
                        if(comma.type != ASM_TOKEN_COMMA)
                        {
                            fprintf(stderr, "asm: error: sub expects a register and an operand (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken op = VECTOR_EL(tokens, i);
                        i++;
                        if(op.type != ASM_TOKEN_LITERAL && op.type != ASM_TOKEN_REGISTER)
                        {
                            fprintf(stderr, "asm: error: sub expects a register and an operand (%s:%lu)\n", in->name, dst.line);
                            goto cleanup;
                        }
                        AsmToken newLine = VECTOR_EL(tokens, i);
                        if(newLine.type != ASM_TOKEN_NEWLINE)
                        {
                            fprintf(stderr, "asm: error: trailing tokens after sub in %s:%lu\n", in->name, dst.line);
                            goto cleanup;
                        }
                        
                        if(op.type == ASM_TOKEN_LITERAL)
                        {
                            if(op.literal_value > 0b1000000)
                            {
                                fprintf(stderr, "asm: error: %lu is out of bounds [0-%d] for sub (%s:%lu)\n",
                                    op.literal_value, 0b1000000, in->name, dst.line);
                                goto cleanup;
                            }
                            VECTOR_PUSH(instructions, make_op_add_imm_signed(dst.reg, -op.literal_value));
                        }
                        else
                        {
                            VECTOR_PUSH(instructions, make_op_sub(dst.reg, dst.reg, op.reg));
                        }
                    }
                        break;

                    #undef REG_NUM
                }
            }
                break;
            case ASM_TOKEN_LABEL:
            {
                AsmToken label = VECTOR_EL(tokens, i);
                i++;
                uint8_t name[32];
                if(label.label_name[0] == '.' && (strlen((char*)current->name) + strlen((char*)label.label_name)) < 32)
                {
                    sprintf((char*)name, "%s%s", current->name, label.label_name);
                }
                else
                {
                    strcpy((char*)name, (char*)label.label_name);
                }
                
                for(VECTOR_ITER(*labels, j))
                {
                    if(!strcmp((char*)VECTOR_EL(*labels, j).name, (char*)name))
                    {
                        VECTOR_EL(*labels, j).value = VECTOR_SIZE(instructions) * 2;
                        if(label.label_name[0] != '.')current = VECTOR_AT(*labels, i);
                        break;
                    }
                }
            }
                break;
            case ASM_TOKEN_NEWLINE:
                break;
            default:
                fprintf(stderr, "asm: error: invalid token sequence in %s:%lu\n", in->name, VECTOR_EL(tokens, i).line);
                goto cleanup;
        }
    }

    VECTOR_FREE(tokens);
    LinkFile* file = malloc(sizeof(LinkFile));
    file->label_count = VECTOR_SIZE(*labels);
    file->labels = VECTOR_AT(*labels, 0);
    if(file->label_count == 0)free(file->labels);
    free(labels);
    file->relocation_count = VECTOR_SIZE(relocations);
    file->relocations = VECTOR_AT(relocations, 0);
    if(file->relocation_count == 0)free(file->relocations);
    file->size = VECTOR_SIZE(instructions) * 2;
    file->content = (uint8_t*)VECTOR_AT(instructions, 0);
    if(file->size == 0)free(file->content);
    free(in->content);

    in->content_size = file->size + file->label_count * sizeof(LinkLabel) + file->relocation_count * sizeof(LinkRelocation)
                                            + sizeof(file->label_count) + sizeof(file->relocation_count) + sizeof(file->size);
    in->content = malloc(in->content_size);
    link_to_buffer(file, in->content);
    link_file_destroy(file);
    in->type = MODE_OBJ;
    return true;

cleanup:
    VECTOR_FREE(tokens);
    VECTOR_FREE(*labels);
    free(labels);
    VECTOR_FREE(relocations);
    VECTOR_FREE(instructions);
    return false;
}
