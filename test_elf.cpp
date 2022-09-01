//
// Created by 晚风吹行舟 on 2022/9/1.
//

#include "headers/linker.h"

int read_elf(const char *file_name, uint64_t buf_addr);

int main(){

    char buff[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];

    read_elf("../files/sum.elf.txt",(uint64_t)&buff);

    return 0;
}
