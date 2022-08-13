//
// Created by 晚风吹行舟 on 2022/8/12.
//
#include <cstdio>

#include "register.h"

reg_t reg;

void print_register(){
    printf("rax = %16llx\trbx = %16llx\trcx = %16llx\trdx = %16llx\n",
           reg.rax, reg.rbx, reg.rcx, reg.rdx);
    printf("rsi = %16llx\trdi = %16llx\trbp = %16llx\trsp = %16llx\n",
           reg.rsi, reg.rdi, reg.rbp, reg.rsp);

    printf("rip = %16llx\n", reg.rip);
}
