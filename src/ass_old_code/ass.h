//
// Created by 晚风吹行舟 on 2022/8/8.
//
#include <cstdint>

// assembly simulator
typedef struct CPU_STRUCT {
    union {
        struct {
            uint8_t al;
            uint8_t ah;
        };
        uint16_t ax;
        uint32_t eax;
        uint64_t rax;
    };
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;

    uint64_t rip;
} reg_t;

typedef enum {
    IMM, REG, MM_IMM, MM_REG, MM_IMM_REG, MM_REG1_REG2,
    MM_IMM_REG1_REG2, MM_REG_S, MM_IMM_REG_S,
    MM_REG1_REG2_S, MM_IMM_REG1_REG2_S
} od_type_t;

typedef struct {
    od_type_t type;

    int64_t imm;
    int64_t scale;
    uint64_t *reg1;
    uint64_t *reg2;
} od_t;

typedef enum OP {
    MOV,
    PUSH,
    CALL
} op_type_t;

typedef struct {
    op_type_t type;

    od_t src;
    od_t dst;
}inst_t;

reg_t reg;

#define MM_LEN 1000;

uint8_t mm[1000];

#define INST_LEN 100

inst_t program[INST_LEN];

uint64_t decode_od(od_t od);




