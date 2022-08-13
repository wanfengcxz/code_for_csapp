//
// Created by 晚风吹行舟 on 2022/8/11.
//
#ifndef inst_guard
#define inst_guard

#include <cstdint>
#include <cstdlib>

#define NUM_INSTR_TYPE 30

typedef enum {
    IMM, REG, MM_IMM, MM_REG, MM_IMM_REG, MM_REG1_REG2,
    MM_IMM_REG1_REG2, MM_REG2_S, MM_IMM_REG2_S,
    MM_REG1_REG2_S, MM_IMM_REG1_REG2_S,
    EMPTY
} od_type_t;

typedef struct {
    od_type_t type;

    int64_t imm;
    int64_t scale;
    uint64_t *reg1;
    uint64_t *reg2;
} od_t;

typedef enum OP {
    mov_reg_reg,
    add_reg_reg,
    mov_reg_mem,
    mov_mem_reg,
    push_reg,
    pop_reg,
    call,
    ret,
} op_type_t;

typedef struct {
    op_type_t op;

    od_t src;
    od_t dst;
    char code[100];
} inst_t;

// pointer pointing to function
typedef void (*handler_t)(uint64_t, uint64_t);

extern handler_t  handler_table[NUM_INSTR_TYPE];

void init_handler_table();

void instruction_cycle();

void mov_reg_reg_handler(uint64_t src, uint64_t dst);

void add_reg_reg_handler(uint64_t src, uint64_t dst);

void mov_reg_mem_handler(uint64_t src, uint64_t dst);

void mov_mem_reg_handler(uint64_t src, uint64_t dst);

void call_handler(uint64_t src, uint64_t dst);

void push_reg_handler(uint64_t src, uint64_t dst);

void pop_reg_handler(uint64_t src, uint64_t dst);

void ret_handler(uint64_t src, uint64_t dst);

#endif