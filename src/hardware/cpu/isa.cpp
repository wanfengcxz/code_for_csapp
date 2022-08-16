//
// Created by 晚风吹行舟 on 2022/8/11.
//
#include <cstdio>

#include "headers/cpu.h"
#include "headers/common.h"
#include "headers/memory.h"


/*======================================*/
/*      instruction set architecture    */
/*======================================*/

// data structures
typedef enum INST_OPERATOR {
    INST_MOV,
    INST_PUSH,
    INST_POP,
    INST_LEAVE,
    INST_CALL,
    INST_RET,
    INST_ADD,
    INST_SUB,
    INST_CMP,
    INST_JNE,
    INST_JMP,
} op_t;

typedef enum OPERAND_TYPE {
    EMPTY,
    IMM,
    REG,
    MM_IMM,
    MM_REG1,
    MM_IMM_REG1,
    MM_REG1_REG2,
    MM_IMM_REG1_REG2,
    MM_REG2_SCALE,
    MM_IMM_REG2_SCALE,
    MM_REG1_REG2_SCALE,
    MM_IMM_REG1_REG2_SCALE,

} od_type_t;

typedef struct OPERAND_STRUCT {
    od_type_t type;

    int64_t imm;
    int64_t scale;      // scale number to register 2
    uint64_t reg1;     // main register address
    uint64_t reg2;     // register 2 address
} od_t;


typedef struct INST_STRUCT {
    op_t op;

    od_t src;
    od_t dst;
} inst_t;

/*======================================*/
/*      parse assembly instruction      */
/*======================================*/

// functions to map the string assembly code to inst_t instance
static void parse_instruction(const char *str, inst_t *inst, core_t *cr);

static void parse_operand(const char *str, od_t *od, core_t *cr);

static uint64_t decode_operand(od_t *od);

// interpret the operand
static uint64_t decode_operand(od_t *od) {
    if (od->type == IMM)
        return *((uint64_t *) &(od->imm));
    else if (od->type == REG)
        return od->reg1;
    else if (od->type == EMPTY)
        return 0x0;
    else {

        // access memory: return the virtual address
        uint64_t vaddr = 0;

        if (od->type == MM_IMM)
            vaddr = od->imm;
        else if (od->type == MM_REG1)
            vaddr = *((uint64_t *) od->reg1);
        else if (od->type == MM_IMM_REG1)
            vaddr = od->imm + (*((uint64_t *) od->reg1));
        else if (od->type == MM_REG1_REG2)
            vaddr = (*((uint64_t *) od->reg1)) + (*((uint64_t *) od->reg2));
        else if (od->type == MM_IMM_REG1_REG2)
            vaddr = od->imm + (*((uint64_t *) od->reg1)) + (*((uint64_t *) od->reg2));
        else if (od->type == MM_REG2_SCALE)
            vaddr = (*((uint64_t *) od->reg2)) * od->scale;
        else if (od->type == MM_IMM_REG2_SCALE)
            vaddr = od->imm + (*((uint64_t *) od->reg2)) * od->scale;
        else if (od->type == MM_REG1_REG2_SCALE)
            vaddr = (*((uint64_t *) od->reg1)) + (*((uint64_t *) od->reg2)) * od->scale;
        else if (od->type == MM_IMM_REG1_REG2_SCALE)
            vaddr = od->imm + (*((uint64_t *) od->reg1)) + (*((uint64_t *) od->reg2)) * od->scale;

        return vaddr;
    }

    // empty
    return 0;
}

static void parse_instruction(const char *str, inst_t *inst, core_t *cr) {

}


static void parse_operand(const char *str, od_t *od, core_t *cr) {
    // str: assembly code string, e.g. mov $rsp,$rbp
    // od: pointer to the address to store the parsed operand
    // cr: active core processor

    od->type = EMPTY;
    od->imm = 0;
    od->scale = 0;
    od->reg1 = 0;
    od->reg2 = 0;

    int str_len = strlen(str);
    if (str_len == 0){
        // empty code string
        return ;
    }

    if(str[0] == '$'){
        // imm
        od->type = IMM;
        od->imm = string2uint_range(str, 1, -1);
    }
//    else if (str[0] == '%'){
//
//        // register
//    } else {
//        // memory access
//
//    }

}

/*======================================*/
/*      instruction handlers            */
/*======================================*/

// insturction (sub)set
// In this simulator, the instructions have been decoded and fetched
// so there will be no page fault during fetching
// otherwise the instructions must handle the page fault (swap in from disk) first
// and then re-fetch the instruction and do decoding
// and finally re-run the instruction

static void mov_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void push_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void pop_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void leave_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void call_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void ret_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void add_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void sub_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void cmp_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void jne_handler(od_t *src_od, od_t *dst_od, core_t *cr);

static void jmp_handler(od_t *src_od, od_t *dst_od, core_t *cr);

// handler table storing the handlers to different instruction types
typedef void (*handler_t)(od_t *, od_t *, core_t *);

#define NUM_INSTR_TYPE 11
// look-up table of pointers to function
static handler_t handler_table[NUM_INSTR_TYPE] = {
        &mov_handler,               // 0
        &push_handler,              // 1
        &pop_handler,               // 2
        &leave_handler,             // 3
        &call_handler,              // 4
        &ret_handler,               // 5
        &add_handler,               // 6
        &sub_handler,               // 7
        &cmp_handler,               // 8
        &jne_handler,               // 9
        &jmp_handler,               // 10
};

// reset the condition flags
// inline to reduce cost
static inline void reset_cflags(core_t *cr) {
    cr->CF = 0;
    cr->OF = 0;
    cr->SF = 0;
    cr->ZF = 0;
}

// update the rip register to the next instruction sequentially
static inline void next_rip(core_t *cr) {
    // we are handling the fixed-length of assembly string here
    // but their size can be variable as true X86 instructions
    // that's because the operands' sizes follow the specific encoding rule
    // the risc-v is a fixed length ISA
    cr->rip = cr->rip + sizeof(char) * MAX_INSTRUCTION_CHAR;
}

// instructions handlers

static void mov_handler(od_t *src_od, od_t *dst_od, core_t *cr) {
    // src is an address except that src_od->type is imm
    // dst is an address except that dst_od->type is imm
    uint64_t src = decode_operand(src_od);
    uint64_t dst = decode_operand(src_od);

    if (src_od->type == REG && dst_od->type == REG) {
        *(uint64_t *) dst = *(uint64_t *) src;
        next_rip(cr);
        reset_cflags(cr);
        return;
    } else if (src_od->type == REG && dst_od->type >= MM_IMM) {
        write64bits_dram(va2pa(dst, cr), *(uint64_t *) src, cr);
        next_rip(cr);
        reset_cflags(cr);
        return;
    } else if (src_od->type == IMM && dst_od->type == REG) {
        *(uint64_t *) dst = src;
        next_rip(cr);
        reset_cflags(cr);
        return;
    } else if (src_od->type >= MM_IMM && dst_od->type == REG) {
        *(uint64_t *) dst = read64bits_dram(va2pa(dst, cr), cr);
        next_rip(cr);
        reset_cflags(cr);
        return;
    } else if (src_od->type == IMM && dst_od->type >= MM_IMM) {
        // TODO: ???? imm to mm
    }
}

static void push_handler(od_t *src_od, od_t *dst_od, core_t *cr) {
    // src is an address except that src_od'type is imm
    uint64_t src = decode_operand(src_od);

    if (src_od->type == REG) {
        (cr->reg).rsp -= 0x8;
        write64bits_dram(
                va2pa((cr->reg).rsp, cr),
                *(uint64_t *) src,
                cr
        );
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
}

static void pop_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

    uint64_t src = decode_operand(src_od);

    if (src_od->type == REG) {
        *(uint64_t *) src = read64bits_dram(
                va2pa((cr->reg).rsp, cr),
                cr
        );
        (cr->reg).rsp += 0x8;
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
}

static void leave_handler(od_t *src_od, od_t *dst_od, core_t *cr) {
}

static void call_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

    uint64_t src = decode_operand(src_od);

    (cr->reg).rsp -= 8;
    write64bits_dram(
            va2pa((cr->reg).rsp, cr),
            cr->rip + sizeof(char) * MAX_INSTRUCTION_CHAR,
            cr
    );
    cr->rip = src;
    reset_cflags(cr);
}

static void ret_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

    uint64_t ret_addr = read64bits_dram(
            va2pa((cr->reg).rsp, cr),
            cr
    );
    (cr->reg).rsp = (cr->reg).rsp + 8;
    // jump to return address
    cr->rip = ret_addr;
    reset_cflags(cr);
}


static void add_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

    uint64_t src = decode_operand(src_od);
    uint64_t dst = decode_operand(src_od);

    if (src_od->type == REG && dst_od->type == REG) {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        uint64_t val = *(uint64_t *) dst + *(uint64_t *) src;

        // set condition flags

        // update registers
        *(uint64_t *) dst = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        next_rip(cr);
        return;
    }

}


static void sub_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

}


static void cmp_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

}


static void jne_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

}


static void jmp_handler(od_t *src_od, od_t *dst_od, core_t *cr) {

}

void instruction_cycle(core_t *cr) {

    // FETCH: get the instruction string by rip register
    const char *inst_str = (const char *) (cr->rip);
    debug_printf(DEBUG_INSTRUCTION_CYCLE, "%lx    %s\n", cr->rip, inst_str);

    // DECODE: decode the run-time instruction operands
    inst_t inst;
    parse_instruction(inst_str, &inst, cr);

    // EXECUTE: get the function pointer or handler by the operator
    handler_t handler = handler_table[inst.op];
    // update CPU and memory according the instruction
    handler(&inst.src, &inst.dst, cr);

}

void print_register(core_t *cr) {

    if ((DEBUG_VERBOSE_SET & DEBUG_REGISTERS) == 0x0) {
        return;
    }

    reg_t reg = cr->reg;

    printf("rax = %16llx\trbx = %16llx\trcx = %16llx\trdx = %16llx\n",
           reg.rax, reg.rbx, reg.rcx, reg.rdx);
    printf("rsi = %16llx\trdi = %16llx\trbp = %16llx\trsp = %16llx\n",
           reg.rsi, reg.rdi, reg.rbp, reg.rsp);
    printf("rip = %16llx\n", cr->rip);
    printf("CF = %u\tZF = %u\tSF = %u\tOF = %u\n",
           cr->CF, cr->ZF, cr->SF, cr->OF);
}


void print_stack(core_t *cr) {
    if ((DEBUG_VERBOSE_SET & DEBUG_PRINT_STACK) == 0x0) {
        return;
    }

    int n = 10;
    uint64_t *high = (uint64_t *) &pm[va2pa((cr->reg).rsp, cr)];
    high = &high[n];
    uint64_t va = (cr->reg).rsp + n * 8;

    for (int i = 0; i < 2 * n; ++i) {
        uint64_t *ptr = (uint64_t *) (high - i);
        printf("0x%16lx : %16lx", va, (uint64_t) *ptr);

        if (i == n) {
            printf(" <== rsp");
        }
        printf("\n");
        va -= 8;
    }
}


