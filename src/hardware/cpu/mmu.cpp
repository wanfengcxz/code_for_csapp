//
// Created by 晚风吹行舟 on 2022/8/11.
//

#include "mmu.h"
#include "dram.h"

uint64_t va2pa(uint64_t vaddr){
    return vaddr % MM_LEN;
}
