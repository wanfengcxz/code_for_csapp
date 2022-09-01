//
// Created by 晚风吹行舟 on 2022/9/1.
//

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "headers/linker.h"
#include "headers/common.h"


int read_elf(const char *file_name, uint64_t buf_addr) {

    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        debug_printf(DEBUG_LINKER, "unable to open file %s\n", file_name);
        exit(1);
    }

    // read text file line by line
    char line[MAX_ELF_FILE_WIDTH];
    int line_counter = 0;

    while (fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL) {
        int len = strlen(line);
        if ((len == 0) ||
            (len >= 1 && (line[0] == '\n' || line[0] == '\r')) ||
            (len >= 2 && (line[0] == '/' && line[1] == '/'))) {
            continue;
        }

        // check if is empty or white line
        int iswhite = 1;
        for (int i = 0; i < len; ++i) {
            iswhite = iswhite && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r');
        }
        if (iswhite == 1) {
            continue;
        }
        if (line_counter < MAX_ELF_FILE_LENGTH) {
            // store this line to buffer[line_counter]
            uint64_t addr = buf_addr + line_counter * MAX_ELF_FILE_WIDTH * sizeof(char);
            char *line_buf = (char *) addr;

            int i = 0;
            while (i < len && i < MAX_ELF_FILE_WIDTH) {
                if ((line[i] == '\n') ||
                    (line[i] == '\r') ||
                    ((i + 1 < len) && (i + 1 < MAX_ELF_FILE_WIDTH) && line[i] == '/' && line[i + 1] == '/')) {
                    break;
                }
                line_buf[i] = line[i];
                i++;
            }
            line_buf[i] = '\0';
            line_counter++;
            printf("%\s\n", line_buf);

        } else {
            debug_printf(DEBUG_LINKER, "elf file %s is too long ", file_name);
            debug_printf(DEBUG_LINKER, "(>%d)\n", MAX_ELF_FILE_LENGTH);
            fclose(fp);
            exit(1);
        }

    }
    fclose(fp);
    assert(string2uint((char *)buf_addr) == line_counter);
    return line_counter;
}