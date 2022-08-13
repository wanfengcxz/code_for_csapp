//
// Created by 晚风吹行舟 on 2022/8/11.
//

#ifndef reg_guard
#define reg_guard

#include <cstdint>

typedef struct {
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

extern reg_t reg;

void print_register();

#endif