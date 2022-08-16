//
// Created by 晚风吹行舟 on 2022/8/16.
//

// Memory Management Unit
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"

uint64_t va2pa(uint64_t vaddr, core_t *cr) {
    return vaddr & (0xffffffffffffffff >> (64 - MAX_PHYSICAL_BIT));
}