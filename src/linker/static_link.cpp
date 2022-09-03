//
// Created by 晚风吹行舟 on 2022/9/1.
//

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include "headers/linker.h"
#include "headers/common.h"

#define MAX_SYMBOL_MAP_LENGTH 64
#define MAX_SECTION_BUFFER_LENGTH 64

// internal mapping between source and destination symbol entries
typedef struct {
    elf_t *elf;   // src elf file
    st_entry_t *src;   // src symbol
    st_entry_t *dst;   // dst symbol: used for relocation - find the function referencing the undefined symbol
    // TODO:
    // relocation entry (referencing section, referenced symbol) converted to (referencing symbol, referenced symbol) entry
} smap_t;

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
                              smap_t *smap_table, int *smap_count);

static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate);

static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count);

static void merge_section(elf_t **src, int num_src, elf_t *dst, smap_t *smap_table, int *smap_count);

/* ------------------------------------ */
/* Exposed Interface for Static Linking */
/* ------------------------------------ */

void link_elf(elf_t **srcs, int num_srcs, elf_t *dst) {
    // reset the destination since it's a new file
    memset(dst, 0, sizeof(elf_t));

    // create the map table to connect the source elf files and destination elf file symbols
    int smap_count = 0;
    smap_t smap_table[MAX_SYMBOL_MAP_LENGTH];

    // update the smap table - symbol processing
    symbol_processing(srcs, num_srcs, dst,
                      (smap_t *) &smap_table, &smap_count);

    printf("----------------------\n");

    for (int i = 0; i < smap_count; ++i) {
        st_entry_t *ste = smap_table[i].src;
        debug_printf(DEBUG_LINKER, "%s\t", ste->st_name);
        debug_printf(DEBUG_LINKER, "%d\t", ste->bind);
        debug_printf(DEBUG_LINKER, "%d\t", ste->type);
        debug_printf(DEBUG_LINKER, "%s\t", ste->st_shndx);
        debug_printf(DEBUG_LINKER, "%d\t", ste->st_value);
        debug_printf(DEBUG_LINKER, "%d\n", ste->st_size);
    }

    // compute dst Section Header Table and write into buffer
    // UPDATE section headert table: compute runtime address of each section
    // UPDATE buffer: EOF file header: file line count, section header table line count, section header table
    // compute running address of each section: .text, .rodata, .data, .symtab
    // eof starting from 0x00400000
    compute_section_header(dst, smap_table, &smap_count);

    // malloc the dst.symt
    dst->symt_count = smap_count;
    dst->symt = (st_entry_t *) malloc(dst->symt_count * sizeof(st_entry_t));

    // to this point, the EOF file header and section header table is placed
    // merge the left sections and relocate the entries in .text and .data

    // merge the symbol content from ELF src into dst sections
    merge_section(srcs, num_srcs, dst, smap_table, &smap_count);
}

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
                              smap_t *smap_table, int *smap_count) {
    for (int i = 0; i < num_srcs; i++) {
        elf_t *elfp = srcs[i];
        for (int j = 0; j < elfp->symt_count; ++j) {
            st_entry_t *sym = &(elfp->symt[j]);
            if (sym->bind == STB_LOCAL) {
                // insert the static (local) symbol to new elf with confidence:
                // compiler would check if the symbol is redeclared in one *.c file
                assert(*smap_count < MAX_SYMBOL_MAP_LENGTH);
                // even if local symbol has the same name, just insert it into dst
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = elfp;
                // we have not created dst here
                (*smap_count)++;
            } else if (sym->bind == STB_GLOBAL) {
                // for other bind: STB_GLOBAL, etc. it's possible to have name conflict
                // check if this symbol has been cached in the map
                for (int k = 0; k < *smap_count; ++k) {
                    // check name conflict
                    // what if the cached symbol is STB_LOCAL?
                    st_entry_t *candidate = smap_table[k].src;
                    if (candidate->bind == STB_GLOBAL &&
                        (strcmp(candidate->st_name, sym->st_name) == 0)) {
                        // having name conflict, do simple symbol resolution
                        // pick one symbol from current sym and cached map[k]
                        simple_resolution(sym, elfp, &smap_table[k]);
                        goto NEXT_SYMBOL_PROCESS;
                    }
                }
                // not find any name conflict
                // cache current symbol sym to the map since there is no name conflict
                assert(*smap_count <= MAX_SYMBOL_MAP_LENGTH);
                // update map table
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = elfp;
                (*smap_count)++;
            }
            NEXT_SYMBOL_PROCESS:
            // do nothing
            ;
        }
    }
    // all the elf files have been processed
    // cleanup: check if there is any undefined symbols in the map table
    for (int i = 0; i < *smap_count; ++i) {
        st_entry_t *s = smap_table[i].src;

        // check no more SHN_UNDEF here
        assert(strcmp(s->st_shndx, "SHN_UNDEF") != 0);
        assert(s->type != STT_NOTYPE);

        // the remaining COMMON go to .bss
        if (strcmp(s->st_shndx, "COMMON") == 0) {
            char *bss = ".bss";
            for (int j = 0; j < MAX_CHAR_SECTION_NAME; ++j) {
                if (j < 4) {
                    s->st_shndx[j] = bss[j];
                } else {
                    s->st_shndx[j] = '\0';
                }
            }
            // .bss default is zero, so we don't need offset.
            s->st_value = 0;
        }
    }
}

static inline int symbol_precedence(st_entry_t *sym) {
    // use inline function to imporve efficiency in run-time by preprocessing
    /*  we do not consider weak because it's very rare
        and we do not consider local because it's not conflicting

            bind        type        shndx               prec
            --------------------------------------------------
            global      notype      undef               0 - undefined
            global      object      common              1 - tentative
            global      object      data,bss,rodata     2 - defined
            global      func        text                2 - defined
    */
    assert(sym->bind == STB_GLOBAL);

    // get precedence of the symbol
    if (strcmp(sym->st_shndx, "SHN_UNDEF") == 0 && sym->type == STT_NOTYPE) {
        // Undefined: symbols referenced but not assigned a storage address
        return 0;
    }
    if (strcmp(sym->st_shndx, "COMMON") == 0 && sym->type == STT_OBJECT) {
        // Tentative: section to be decided after symbol resolution
        return 1;
    }
    if ((strcmp(sym->st_shndx, ".text") == 0 && sym->type == STT_FUNC) ||
        (strcmp(sym->st_shndx, ".data") == 0 && sym->type == STT_OBJECT) ||
        (strcmp(sym->st_shndx, ".rodata") == 0 && sym->type == STT_OBJECT) ||
        (strcmp(sym->st_shndx, ".bss") == 0 && sym->type == STT_OBJECT)) {
        // Defined
        return 2;
    }

    debug_printf(DEBUG_LINKER, "symbol resolution: cannot determine the symbol \"%s\" precedence", sym->st_name);
    exit(0);
}

static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate) {
    // sym: symbol from current elf file
    // candidate: pointer to the internal map table slot: src -> dst

    // determines which symbol is the one to be kept with 3 rules
    // rule 1: multiple strong symbols with the same name are not allowed
    // rule 2: given a strong symbol and multiple weak symbols with the same name, choose the strong symbol
    // rule 3: given multiple weak symbols with the same name, choose any of the weak symbols
    int pre1 = symbol_precedence(sym);
    int pre2 = symbol_precedence(candidate->src);

    if (pre1 == 2 && pre2 == 2) {
        // rule 1
        debug_printf(DEBUG_LINKER,
                     "symbol resolution: strong symbol \"%s\" is redeclared\n", sym->st_name);
        exit(0);
    } else if (pre1 != 2 && pre2 != 2) {
        /* rule 3 - select higher precedence
                pre1    pre2
            ---------------------
                0       0
                0       1
                1       0
                1       1
         */
        // use the stronger one as best match
        if (pre1 > pre2) {
            // select sym as best match
            candidate->src = sym;
            candidate->elf = sym_elf;
        }
        return;
    } else if (pre1 == 2) {
        /* rule 2 - select current symbol
                pre1    pre2
            ---------------------
                2       0
                2       1
         */
        // select sym as best match
        candidate->src = sym;
        candidate->elf = sym_elf;
    }
}

static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count) {
    int count_data = 0, count_text = 0, count_rodata = 0;
    for (int i = 0; i < *smap_count; i++) {
        st_entry_t *sym = smap_table[i].src;

        if (strcmp(sym->st_shndx, ".text") == 0) {
            count_text += sym->st_size;
        } else if (strcmp(sym->st_shndx, ".data") == 0) {
            count_data += sym->st_size;
        } else if (strcmp(sym->st_shndx, ".rodata") == 0) {
            count_rodata += sym->st_size;
        }
    }

    // count the section: with .symtab
    dst->sht_count = (count_text != 0) + (count_rodata != 0) + (count_data != 0) + 1;
    // count the total lines
    // the target dst: line_count, sht_count, sht, .text, .rodata, .data, .symtab
    dst->line_count = 1 + 1 + dst->sht_count + *smap_count + count_data + count_rodata + count_text;

    // write to buffer
    sprintf(dst->buffer[0], "%lld", dst->line_count);
    sprintf(dst->buffer[1], "%lld", dst->sht_count);

    // compute the run-time virtual address of the sections
    uint64_t text_runtime_vaddr = 0x00400000;
    uint64_t rodata_runtime_vaddr = text_runtime_vaddr + count_text * MAX_INSTRUCTION_CHAR * sizeof(char);
    uint64_t data_runtime_vaddr = rodata_runtime_vaddr + count_rodata * sizeof(uint64_t);
    uint64_t symtab_runtime_vaddr = 0; // For EOF, .symtab is not loaded into run-time memory but still on disk

    assert(dst->sht == NULL);
    dst->sht = (sh_entry_t *) malloc(dst->sht_count * sizeof(sh_entry_t));

    // write in .text, .rodata, .data order
    // the start of the offset
    uint64_t section_offset = 1 + 1 + dst->sht_count;
    int sh_index = 0;
    sh_entry_t *sh = NULL;
    if (count_text > 0) {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);

        strcpy(sh->sh_name, ".text");
        sh->sh_addr = text_runtime_vaddr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_text;

        // write to buffer
        sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%llx,%lld,%lld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        sh_index++;
        section_offset += sh->sh_size;
    }

    // .rodata
    if (count_rodata > 0) {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);

        strcpy(sh->sh_name, ".rodata");
        sh->sh_addr = rodata_runtime_vaddr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_rodata;

        // write to buffer
        sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%llx,%lld,%lld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        sh_index++;
        section_offset += sh->sh_size;
    }

    // .data
    if (count_data > 0) {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);

        strcpy(sh->sh_name, ".data");
        sh->sh_addr = data_runtime_vaddr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_data;

        // write to buffer
        sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%llx,%lld,%lld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        sh_index++;
        section_offset += sh->sh_size;
    }

    // .symtab
    // get the pointer
    assert(sh_index < dst->sht_count);
    sh = &(dst->sht[sh_index]);

    strcpy(sh->sh_name, ".symtab");
    sh->sh_addr = 0;
    sh->sh_offset = section_offset;
    sh->sh_size = *smap_count;

    // write to buffer
    sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%llx,%lld,%lld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

    assert(sh_index + 1 == dst->sht_count);

    // print and check
    if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0) {
        printf("----------------------\n");
        printf("Destination ELF's SHT in Buffer:\n");
        for (int i = 0; i < 2 + dst->sht_count; ++i) {
            printf("%s\n", dst->buffer[i]);
        }
    }
}

// precondition: dst should know the section offset of each section
// merge the target section lines from ELF files and update dst symtab
static void merge_section(elf_t **src, int num_src, elf_t *dst, smap_t *smap_table, int *smap_count) {
    int line_written = 1 + 1 + dst->sht_count;
    int symt_written = 0;
    int sym_section_offset = 0;

    for (int section_index = 0; section_index < dst->sht_count; ++section_index) {
        // get the section by section id
        sh_entry_t *target_sh = &dst->sht[section_index];
        sym_section_offset = 0;
//        sym_section_offset = 0;
        debug_printf(DEBUG_LINKER, "merging section '%s'\n", target_sh->sh_name);

        // merge the sections
        // scan every input ELF file
        for (int i = 0; i < num_src; i++) {
            int src_section_index = -1;
            for (int j = 0; j < src[i]->sht_count; j++) {
                // check if this ELF srcs[i] contains the same section as target_sh
                if (strcmp(src[i]->sht[j].sh_name, target_sh->sh_name) == 0) {
                    src_section_index = j;
                }
            }

            if (src_section_index == -1) {
                continue;
            } else {
                // found the section in this ELF srcs[i]
                // check its symtab
                for (int j = 0; j < src[i]->symt_count; j++) {
                    st_entry_t *sym = &src[i]->symt[j];
                    for (int k = 0; k < *smap_count; k++) {
                        // scan the cached dst symbols to check
                        // if this symbol should be merged into this section

                        if (sym == smap_table[k].src) {
                            // exactly the cached symbol
                            debug_printf(DEBUG_LINKER, "\t\tsymbol '%s'\n", sym->st_name);

                            // this symbol should be merged into dst's section target_sh
                            // copy this symbol from srcs[i].buffer into dst.buffer
                            // srcs[i].buffer[sh_offset + st_value, sh_offset + st_value + st_size] inclusive
                            for (int t = 0; t < sym->st_size; t++) {
                                int dst_index = line_written + t;
                                int src_index = src[i]->sht[src_section_index].sh_offset +
                                                sym->st_value + t;

                                assert(dst_index < MAX_ELF_FILE_LENGTH);
                                assert(src_index < MAX_ELF_FILE_LENGTH);

                                strcpy(
                                        dst->buffer[dst_index],
                                        src[i]->buffer[src_index]);
                            }

                            // copy the symbol table entry from srcs[i].symt[j] to
                            // dst.symt[symt_written]
                            assert(symt_written < dst->symt_count);

                            // copy the entry
                            strcpy(sym->st_name, dst->symt[symt_written].st_name);
                            strcpy(sym->st_shndx, dst->symt[symt_written].st_shndx);
                            dst->symt[symt_written].bind = sym->bind;
                            dst->symt[symt_written].type = sym->type;
                            // MUST NOT BE A COMMON, so the section offset MUST NOT BE alignment
                            dst->symt[symt_written].st_value = sym_section_offset;
                            dst->symt[symt_written].st_size = sym->st_size;

                            // update the smap_table
                            // this will hep the relocation
                            smap_table[k].dst = &dst->symt[symt_written];

                            // udpate the counter
                            symt_written += 1;
                            line_written += sym->st_size;
                            sym_section_offset += sym->st_size;

                        }
                    }
                }
            }

        }

    }

}