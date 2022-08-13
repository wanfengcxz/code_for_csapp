//
// Created by 晚风吹行舟 on 2022/8/11.
//
#include <cstdio>

#include "instruction.h"
#include "register.h"
#include "mmu.h"
#include "dram.h"

handler_t handler_table[NUM_INSTR_TYPE];

static uint64_t decode_od(od_t od) {
    if (od.type == IMM)
        return *((uint64_t *) &od.imm);
    else if (od.type == REG)
        return (uint64_t) od.reg1;
    else {
        // mm
        uint64_t vaddr = 0;

        if (od.type == MM_IMM)
            vaddr = od.imm;
        else if (od.type == MM_REG)
            vaddr = *(od.reg1);
        else if (od.type == MM_IMM_REG)
            vaddr = od.imm + *od.reg1;
        else if (od.type == MM_REG1_REG2)
            vaddr = *(od.reg1) + *(od.reg2);
        else if (od.type == MM_IMM_REG1_REG2)
            vaddr = od.imm + *(od.reg1) + *(od.reg2);
        else if (od.type == MM_REG2_S)
            vaddr = (*(od.reg2) * od.scale);
        else if (od.type == MM_IMM_REG2_S)
            vaddr = od.imm + (*(od.reg2) * od.scale);
        else if (od.type == MM_REG1_REG2_S)
            vaddr = *(od.reg1) + (*(od.reg2) * od.scale);
        else if (od.type == MM_IMM_REG1_REG2_S)
            vaddr = od.imm + *(od.reg1) + (*(od.reg2) * od.scale);

        return vaddr;
    }

}

void instruction_cycle() {
    inst_t *instr_p = (inst_t *) reg.rip;

    uint64_t src = decode_od(instr_p->src);
    uint64_t dst = decode_od(instr_p->dst);

    handler_t handler = handler_table[instr_p->op];

    handler(src, dst);
    printf("    %s\n", instr_p->code);
}

void init_handler_table() {
    handler_table[mov_reg_reg] = &mov_reg_reg_handler;
    handler_table[add_reg_reg] = &add_reg_reg_handler;
    handler_table[mov_mem_reg] = &mov_mem_reg_handler;
    handler_table[mov_reg_mem] = &mov_reg_mem_handler;
    handler_table[push_reg] = &push_reg_handler;
    handler_table[pop_reg] = &pop_reg_handler;
    handler_table[call] = &call_handler;
    handler_table[ret] = &ret_handler;
}

void mov_reg_reg_handler(uint64_t src, uint64_t dst) {
    *(uint64_t *) dst = *(uint64_t *) src;
    reg.rip = reg.rip + sizeof(inst_t);
}

void add_reg_reg_handler(uint64_t src, uint64_t dst) {
    *(uint64_t *) dst = *(uint64_t *) src + *(uint64_t *) dst;
    reg.rip = reg.rip + sizeof(inst_t);
}

void mov_reg_mem_handler(uint64_t src, uint64_t dst) {
    write64bits_dram(va2pa(dst), *(uint64_t *) src);
    reg.rip = reg.rip + sizeof(inst_t);
}

void mov_mem_reg_handler(uint64_t src, uint64_t dst) {
    // src: virtual memory address
    // dst: reg
    *(uint64_t *) dst = read64bits_dram(
            va2pa(src)
    );

    reg.rip = reg.rip + sizeof(inst_t);
}

void call_handler(uint64_t src, uint64_t dst) {
    // src: imm address of called function
    reg.rsp = reg.rsp - 8;
    write64bits_dram(va2pa(reg.rsp), reg.rip + sizeof(inst_t));

    reg.rip = src;
}

void push_reg_handler(uint64_t src, uint64_t dst) {
    // src: reg
    // dst: empty
    reg.rsp = reg.rsp - 8;
    write64bits_dram(va2pa(reg.rsp), *(uint64_t *) src);

    reg.rip = reg.rip + sizeof(inst_t);
}

void pop_reg_handler(uint64_t src, uint64_t dst) {
    *(uint64_t *) src = read64bits_dram(
            va2pa(reg.rsp)
    );
    reg.rsp = reg.rsp + 8;

    reg.rip = reg.rip + sizeof(inst_t);
}

void ret_handler(uint64_t src, uint64_t dst) {
    reg.rip = read64bits_dram(
            va2pa(reg.rsp)
    );
    reg.rsp = reg.rsp + 8;
}