//
// Created by 晚风吹行舟 on 2022/8/8.
//
#include <stdio.h>
#include "ass.h"

void judge_big_or_little_end(){
    reg.rax = 0x12345678abcd1234;
    printf("rax:0x%llx\n", reg.rax);
    printf("eax:0x%x\n", reg.eax);
    printf("ax:0x%x\n", reg.ax);
    printf("al:0x%x\n", reg.al);
    printf("ah:0x%x\n", reg.ah);
}

uint64_t decode_od(od_t od){
    uint64_t addr = MM_LEN + 0xff;
    switch (od.type) {
        case IMM:
            return od.imm;
        case REG:
            return (uint64_t)od.reg1; // reg1's address
//        default:
//            return mm[addr % MM_LEN];
    }
}


int main(){
    judge_big_or_little_end();
    return 1;
}