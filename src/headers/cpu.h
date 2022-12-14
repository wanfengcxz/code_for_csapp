//
// Created by 晚风吹行舟 on 2022/8/11.
//

#ifndef CPU_GUARD
#define CPU_GUARD

#include <cstdint>
#include <cstdlib>

/*======================================*/
/*               registers              */
/*======================================*/

// registers in a core of cpu
// these registers only accessible to the core itself.

typedef struct REGISTER_STRUCT{
    // function return value
    union {
        uint16_t ax;
        uint32_t eax;
        uint64_t rax;
        struct {
            uint8_t al;
            uint8_t ah;
        };
    };

    // callee saved
    // stack bottom pointer
    union {
        uint16_t bx;
        uint32_t ebx;
        uint64_t rbx;
        struct {
            uint8_t bl;
            uint8_t bh;
        };
    };

    // 1st argument
    union {
        uint64_t rdi;
        uint32_t edi;
        uint16_t di;
        struct {
            uint8_t dil;
            uint8_t dih;
        };
    };
    // 2nd argument
    union {
        uint64_t rsi;
        uint32_t esi;
        uint16_t si;
        struct {
            uint8_t sil;
            uint8_t sih;
        };
    };
    // 3th argument
    union {
        uint64_t rdx;
        uint32_t edx;
        uint16_t dx;
        struct {
            uint8_t dl;
            uint8_t dh;
        };
    };
    // 4th argument
    union {
        uint64_t rcx;
        uint32_t ecx;
        uint16_t cx;
        struct {
            uint8_t cl;
            uint8_t ch;
        };
    };

    // callee saved frame pointer
    union {
        uint64_t rbp;
        uint32_t ebp;
        uint16_t bp;
        struct {
            uint8_t bpl;
            uint8_t bph;
        };
    };

    // stack top pointer
    union {
        uint64_t rsp;
        uint32_t esp;
        uint16_t sp;
        struct {
            uint8_t spl;
            uint8_t sph;
        };
    };

    // 5th argument
    union {
        uint64_t r8;
        uint32_t r8d;
        uint16_t r8w;
        uint8_t r8b;
    };
    // 6th argument
    union {
        uint64_t r9;
        uint32_t r9d;
        uint16_t r9w;
        uint8_t r9b;
    };

    // caller saved
    union
    {
        uint64_t r10;
        uint32_t r10d;
        uint16_t r10w;
        uint8_t  r10b;
    };
    // caller saved
    union
    {
        uint64_t r11;
        uint32_t r11d;
        uint16_t r11w;
        uint8_t  r11b;
    };

    // callee saved
    union
    {
        uint64_t r12;
        uint32_t r12d;
        uint16_t r12w;
        uint8_t  r12b;
    };
    // callee saved
    union
    {
        uint64_t r13;
        uint32_t r13d;
        uint16_t r13w;
        uint8_t  r13b;
    };
    // callee saved
    union
    {
        uint64_t r14;
        uint32_t r14d;
        uint16_t r14w;
        uint8_t  r14b;
    };
    // callee saved
    union
    {
        uint64_t r15;
        uint32_t r15d;
        uint16_t r15w;
        uint8_t  r15b;
    };

} reg_t;

// cf of zf sf in total 64bit
typedef struct CPU_FLAGS_STRUCT{
    union {
        uint64_t __cpu_flag_value;
        struct {
            // carry flag: detect overflow for unsigned operations
            uint16_t CF;
            // overflow flag: detect overflow for signed operations
            uint16_t OF;
            // zero flag: result is zero
            uint16_t ZF;
            //sign flag: result is negative: highest bit
            uint16_t SF;
        };
    };
} cpu_flag_t;

/*======================================*/
/*               sram cache             */
/*======================================*/

/*
The components of a SRAM cache address:
Note: SRAM cache address is the translated physical address

|   TAG_LEN     |   INDEX_LEN   |   OFFSET_LEN  |
+---------------+---------------+---------------+
|   tag         |   index       |   offset      |
+---------------+---------------+---------------+
|   ADDR_LEN                                    |
*/

/*======================================*/
/*               cpu core               */
/*======================================*/

typedef struct CORE_STRUCT{

    // program counter or instruction pointer
    union {
        uint32_t eip;
        uint64_t rip;
    };

    // condition code flags of most recent (latest) operation
    // condition codes will only be set by the following integer arithmetic instructions

    /* integer arithmetic instructions
        inc     increment 1
        dec     decrement 1
        neg     negate  eg: mov $0x123,rcx; neg rcx; -> rcx:-0x123
        not     reverse(~)
        ----------------------------
        add     add
        sub     subtract
        imul    multiply
        xor     exclusive or
        or      or
        and     and
        ----------------------------
        sal     left shift
        shl     left shift (same as sal)
        sar     arithmetic right shift
        shr     logical right shift
    */

    /* comparison and test instructions
       cmp     compare
       test    test
   */

    cpu_flag_t flags;

    // registers
    reg_t reg;
    uint64_t pdbr;  // page directory base register

} core_t;

// define cpu core array to support core level parallelism
#define NUM_CORES 1
extern core_t cores[NUM_CORES];

// active core for current task
extern uint64_t ACTIVE_CORE;

#define MAX_INSTRUCTION_CHAR 64
#define NUM_INSTRUCTION_TYPE 14

// CPU's instruction cycle: execution of instructions
void instruction_cycle(core_t *cr);

/*--------------------------------------*/
// place the functions here because they requires the core_t type

/*--------------------------------------*/
// mmu functions

// translate the virtual address to physical address in MMU
// each MMU is owned by each core
uint64_t va2pa(uint64_t vaddr, core_t *cr);

#endif