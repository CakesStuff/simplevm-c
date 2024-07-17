#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H
#include<core.h>
#include<linker.h>

typedef uint16_t Lit12Bit;
typedef uint16_t Lit10Bit;
typedef uint8_t Lit7Bit;
typedef uint8_t Nibble;

typedef enum {
    Zero, A, B, C, M, SP, PC, BP,
} Register;

typedef enum {
    OpInvalid               = 0x00,
    OpAdd                   = 0x01,
    OpSub                   = 0x02,
    OpAddImm                = 0x03,
    OpAddImmSigned          = 0x04,
    OpShiftLeft             = 0x05,
    OpShiftRightLogical     = 0x06,
    OpShiftRightArithmetic  = 0x07,
    OpLoadWord              = 0x08,
    OpStoreWord             = 0x09,
    OpJumpOffset            = 0x0a,
    OpTest                  = 0x0b,
    OpAddIf                 = 0x0c,
    OpStack                 = 0x0d,
    OpLoadStackOffset       = 0x0e,
    OpSystem                = 0x0f,
    OpSetAndSave            = 0x10,
    OpAddAndSave            = 0x11,
    OpLoadByte              = 0x12,
    OpStoreByte             = 0x13,

    OpMul                   = 0x15,

    //DIFFERENT FORMAT
    OpImm                   = 0xff,
} OpCode;

typedef enum {
    TestEqual           = 0,
    TestNotEqual        = 1,
    TestLessThan        = 2,
    TestLessEqual       = 3,
    TestGreaterThan     = 4,
    TestGreaterEqual    = 5,
    TestBothZero        = 6,
    TestEitherNonZero   = 7,
    TestBothNonZero     = 8,
} TestOp;

typedef enum {
    StackPop    = 0,
    StackPush   = 1,
    StackPeek   = 2,
    StackSwap   = 3,
    StackDup    = 4,
    StackRotate = 5,
    StackAdd    = 6,
    StackSub    = 7,
} StackOp;

/**
 * Formats an instruction from a given OpCode and three numbers.
 * 
 * \param code the OpCode
 * \param X optional x number, is the zero register if not provided
 * \param Y optional y number, is the zero register if not provided
 * \param Z optional z number, is the zero register if not provided
 * \returns The instruction as a uint16_t
 */
uint16_t make_ins0(OpCode code, Register X, Register Y, Nibble Z);

/**
 * Formats an instruction from a given 12 bit literal.
 * 
 * \param lit12bit the 12 bit literal
 * \returns The instruction.
 */
uint16_t make_ins1(Register R, Lit12Bit lit12bit);

uint16_t make_op_add(Register dst, Register op1, Register op2);
uint16_t make_op_sub(Register dst, Register num, Register sub);
uint16_t make_op_add_imm(Register dst, Lit7Bit num);
uint16_t make_op_add_imm_signed(Register dst, Lit7Bit num);
uint16_t make_op_shift_left(Register dst, Register src, Nibble amount);
uint16_t make_op_shift_right_logical(Register dst, Register src, Nibble amount);
uint16_t make_op_shift_right_arithmetic(Register dst, Register src, Nibble amount);
uint16_t make_op_load_word(Register dst, Register segment, Register addr);
uint16_t make_op_store_word(Register dst, Register segment, Register addr);
uint16_t make_op_jump_offset(Lit10Bit amount);
uint16_t make_op_test(Register one, Register two, TestOp to);
uint16_t make_op_add_if(Register dst, Register src, Nibble half);
uint16_t make_op_stack(Register reg, Register sp, StackOp so);
uint16_t make_op_load_stack_offset(Register dst, Register stack, Nibble half_off);
uint16_t make_op_system_fast(Nibble handler, Register arg);
uint16_t make_op_system_slow(Register handler, Nibble arg);
uint16_t make_op_set_and_save(Register to_set, Register num, Register to_save);
uint16_t make_op_add_and_save(Register to_add_to, Register num, Register to_save);
uint16_t make_op_load_byte(Register dst, Register segment, Register addr);
uint16_t make_op_store_byte(Register dst, Register segment, Register addr);
uint16_t make_op_mul(Register dst, Register op1, Register op2);
uint16_t make_op_imm(Register reg, Lit12Bit val);

/**
 * Parses a string of length str_len to a number.
 * Can be signed or unsigned, so check the value range.
 * 
 * \param str the string to parse
 * \param str_len the length of the string
 * \param out where to put the number
 * \returns True on success.
 */
bool parse_uint64_t(char* str, size_t str_len, uint64_t* out);

typedef enum {
    ASM_INS_MOV,
    ASM_INS_SHL,
    ASM_INS_SHR,
    ASM_INS_SHRS,
    ASM_INS_MUL,
    ASM_INS_ADD,
    ASM_INS_SUB,
    ASM_INS_LEA,
    //TODO: MORE INSTRUCTIONS
} AsmInstructionType;

extern char* AsmInstructionStrings[];

typedef enum {
    ASM_TOKEN_LABEL,
    ASM_TOKEN_LITERAL,
    ASM_TOKEN_OPEN_SQUARE,
    ASM_TOKEN_COLON,
    ASM_TOKEN_CLOSED_SQUARE,
    ASM_TOKEN_COMMA,
    ASM_TOKEN_INSTRUCTION,
    ASM_TOKEN_EXTERN,
    ASM_TOKEN_REGISTER,
    ASM_TOKEN_EQU,
    ASM_TOKEN_MINUS,
    ASM_TOKEN_PLUS,
    ASM_TOKEN_SHL,
    ASM_TOKEN_SHR_P,
    ASM_TOKEN_SHR_M,
    ASM_TOKEN_WORD,
    ASM_TOKEN_BYTE,
    ASM_TOKEN_DB,
    ASM_TOKEN_DW,
    ASM_TOKEN_NEWLINE,
    ASM_TOKEN_MUL,
} AsmTokenType;

typedef struct {
    AsmTokenType type;
    size_t line;
    union {
        uint8_t label_name[32];
        uint64_t literal_value;
        Register reg;
        AsmInstructionType instruction;
    };
} AsmToken;

DEFINE_VECTOR_OF(AsmToken) token_v;

/**
 * Tokenizes an assembly source file.
 * 
 * \param content the content buffer
 * \param size the size of the content buffer
 * \param filename the name of the assembly file
 * \returns A vector of tokens, is empty on failure.
 */
token_v asm_tokenize(uint8_t* content, size_t size, char* filename);

DEFINE_VECTOR_OF(LinkLabel) label_v;

/**
 * Collects all labels in the given file and validates them.
 * <label> equ <value> adds a label of type ABSVAL with value <value>,
 * <label>: adds a label of type ADDR with TBD value,
 * .<label>: adds a label with name <full_label>.<label>, type ADDR and TBD value.
 * global <label> sets the global flag of <label>.
 * 
 * \param tokens the vector of tokens in the file
 * \param filename the name of the file
 * \returns A vector of labels or NULL on failure.
 */
label_v* asm_first_pass(token_v* tokens, char* filename);

DEFINE_VECTOR_OF(LinkRelocation) rel_v;
DEFINE_VECTOR_OF(uint16_t) ins_v;

/**
 * Assembles a file and outputs an object file.
 * 
 * \param in the file to assemble
 * \returns True on success.
 */
bool asm_assemble(CCInput* in);

#endif /* _ASSEMBLER_H */