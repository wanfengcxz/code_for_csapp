//
// Created by 晚风吹行舟 on 2022/8/5.
//

#include <cstdio>
#include <random>

#define DEBUG 1;

typedef enum {
    MODIFIED, // unique and dirty
    EXCLUSIVE, // unique and clean
    SHARED, // not unique and clean
    INVALID // not unique and dirty or miss
} state_t;

struct line_s {
    state_t state;
    int value;
};

const int NUM_PROCESSOR = 6;

line_s cache[NUM_PROCESSOR];

// the value of current variable in memory
int mem_value = 15213;

int check_state() {
    int m_count = 0;
    int e_count = 0;
    int s_count = 0;
    int i_count = 0;

    for (auto &i : cache) {
        if (i.state == MODIFIED)
            m_count++;
        else if (i.state == EXCLUSIVE)
            e_count++;
        else if (i.state == SHARED)
            s_count++;
        else
            i_count++;
    }

#ifdef DEBUG
    printf("M %d\t E %d\t S %d\t I %d\n", m_count, e_count, s_count, i_count);
#endif

    if ((m_count == 1 && i_count == (NUM_PROCESSOR - m_count)) ||
        (e_count == 1 && i_count == (NUM_PROCESSOR - e_count)) ||
        (s_count > 1 && i_count == (NUM_PROCESSOR - s_count)) ||
        (i_count == NUM_PROCESSOR)) {
        // current state is valid
        return 1;
    }

    // current state is invalid
    return 0;
}

/**
 * @param i index of processor
 * @param read_value
 * @return 1 or 0
 */
int read_cacheline(int i, int &read_value) {
    if (cache[i].state == MODIFIED) {
        // read hit
#ifdef DEBUG
        printf("[%d] read hit; dirty value %d\n", i, cache[i].value);
#endif
        read_value = cache[i].value;
        return 1;
    } else if (cache[i].state == EXCLUSIVE) {
        // read hit
#ifdef DEBUG
        printf("[%d] read hit; exclusive clean value %d\n", i, cache[i].value);
#endif
        read_value = cache[i].value;
        return 1;
    } else if (cache[i].state == SHARED) {
        // read hit
#ifdef DEBUG
        printf("[%d] read hit; shared clean value %d\n", i, cache[i].value);
#endif
        read_value = cache[i].value;
        return 1;
    } else {
        // cache[i].state == INVALID
        // read miss
        // bus broadcast read miss
        for (int j = 0; j < NUM_PROCESSOR; j++) {
            if (cache[j].state == MODIFIED) {
                // write back
                mem_value = cache[j].value;
                cache[j].state = SHARED;

                // update read miss cache
                // load data from cache j by bus, which faster than by memory
                cache[i].value = cache[j].value;
                cache[i].state = SHARED;

                read_value = cache[i].value;

#ifdef DEBUG
                printf("[%d] read miss; [%d] supplies dirty value %d; write back; s_count == 2\n", i, j, cache[i].value);
#endif

                return 1;
            } else if (cache[j].state == EXCLUSIVE) {
                // update state
                cache[j].state = SHARED;

                // update read miss cache
                cache[i].value = cache[j].value;
                cache[i].state = SHARED;

                read_value = cache[i].value;

#ifdef DEBUG
                printf("[%d] read miss; [%d] supplies clean value %d; s_count == 2\n", i, j, cache[i].value);
#endif

                return 1;
            } else if (cache[j].state == SHARED) {
                // update read miss cache
                cache[i].state = SHARED;
                cache[i].value = cache[j].value;

                read_value = cache[i].value;

#ifdef DEBUG
                printf("[%d] read miss; [%d] supplies clean value %d; s_count >= 3\n", i, j, cache[i].value);
#endif

                return 1;
            }
        }
        // all cache is invalid
        cache[i].state = EXCLUSIVE;
        // read memory
        cache[i].value = mem_value;

        read_value = cache[i].value;

#ifdef DEBUG
        printf("[%d] read miss; mem supplies clean value %d; e_count == 1\n", i, cache[i].value);
#endif

        return 1;
    }
}

/**
 * @param i index of processor
 * @param write_value
 * @return 0 or 1
 */
int write_cacheline(int i, int write_value) {
    if (cache[i].state == MODIFIED) {
        // write hit
        cache[i].value = write_value;

#ifdef DEBUG
        printf("[%d] write hit; update to value %d\n", i, cache[i].value);
#endif

        return 1;
    } else if (cache[i].state == EXCLUSIVE) {
        // write hit
        cache[i].state = MODIFIED;
        cache[i].value = write_value;

#ifdef DEBUG
        printf("[%d] write hit; update to value %d\n", i, cache[i].value);
#endif

        return 1;
    } else if (cache[i].state == SHARED) {
        // write hit
        cache[i].state = MODIFIED;
        cache[i].value = write_value;

        // bus broadcast write
        for (int j = 0; j < NUM_PROCESSOR; j++) {
            if (cache[j].state == SHARED) {
                // update state
                cache[j].state = INVALID;
            }
        }

#ifdef DEBUG
        printf("[%d] write hit; boardcast invalid; update to value %d\n", i, cache[i].value);
#endif

        return 1;
    } else {
        // cache[i].state = INVALID
        // write miss
        for (int j = 0; j < NUM_PROCESSOR; j++) {
            if (cache[j].state == MODIFIED) {
                // cache j write back
                mem_value = cache[j].value;
                cache[j].state = INVALID;

                // update write miss cache
                // load data from cache j by bus
                cache[i].value = cache[j].value;
                // write
                cache[i].value = write_value;
                cache[i].state = MODIFIED;

                return 1;
            } else if (cache[j].state == EXCLUSIVE) {
                // update state of cache j
                cache[j].state = INVALID;

                // update write miss cache
                // load data from process j by bus
                cache[i].value = cache[j].value;
                cache[i].state = MODIFIED;
                cache[i].value = write_value;

#ifdef DEBUG
                printf("[%d] write miss; boardcast invalid to E; update to value %d\n", i, cache[i].value);
#endif

                return 1;
            } else if (cache[j].state == SHARED) {
                // update state of cache j
                cache[j].state = INVALID;

                // update the rest SHARED [j+1, NUM_PROCESSOR-1]
                for (int k = j + 1; k < NUM_PROCESSOR; k++) {
                    if (cache[k].state == SHARED)
                        cache[k].state = INVALID;
                }

                // update write miss cache
                // load data from process j by bus
                cache[i].value = cache[j].value;
                cache[i].state = MODIFIED;
                cache[i].value = write_value;

#ifdef DEBUG
                printf("[%d] write miss; boardcast invalid to S; update to value %d\n", i, cache[i].value);
#endif

                return 1;
            }
        }
        // all cache is INVALID
        cache[i].state = MODIFIED;
        // load data from memory
        cache[i].value = mem_value;
        cache[i].value = write_value;

#ifdef DEBUG
        printf("[%d] write miss; all invalid; update to value %d\n", i, cache[i].value);
#endif

        return 1;
    }
}

/**
 * @param i the index of process
 * @return 0 or 1
 */
int evict_cacheline(int i) {
    if (cache[i].state == MODIFIED) {
        // write back
        mem_value = cache[i].value;
        cache[i].state = INVALID;
        cache[i].value = 0;

#ifdef DEBUG
        printf("[%d] evict; write back value %d\n", i, cache[i].value);
#endif

        return 1;
    } else if (cache[i].state == EXCLUSIVE) {
        cache[i].state = INVALID;

#ifdef DEBUG
        printf("[%d] evict\n", i);
#endif

        return 1;
    } else if (cache[i].state == SHARED) {
        cache[i].state = INVALID;
        cache[i].value = 0;

        // may left only one shared to be exclusive
        int s_count = 0;
        int last_s = -1;

        for (int j = 0; j < NUM_PROCESSOR; ++j) {
            if (cache[j].state == SHARED) {
                last_s = j;
                s_count++;
            }
        }

        if (s_count == 1) {
            cache[last_s].state = EXCLUSIVE;
        }

#ifdef DEBUG
        printf("[%d] evict\n", i);
#endif

        return 1;
    }

    // evict when cache line is Invalid
    // not related with target physical address
    return 0;
}

void print_cacheline() {
    char state_set[4] = {'M', 'E', 'S', 'I'};
    for (int i = 0; i < NUM_PROCESSOR; i++) {
        printf("\t[%d]      state %c        value %d\n", i, state_set[cache[i].state], cache[i].value);
    }
    printf("\t                          memory value %d\n", mem_value);
}

void init(){
    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;
    }
}

int main() {
    srand(12345);

    init();
    print_cacheline();

    int do_print = 0;
    int read_value;
    for (int i = 0; i < 100; i++) {
        int core_i = rand() % NUM_PROCESSOR;
        int instruction_i = rand() % 2;

        if (instruction_i == 0) {
            do_print = read_cacheline(core_i, read_value);
        } else if (instruction_i == 1) {
            do_print = write_cacheline(core_i, rand());
        }
//        else if (instruction_i == 2) {
//            do_print = evict_cacheline(core_i);
//        }

        if (do_print) {
            print_cacheline();
        }

        if (check_state() == 0) {
            printf("failed\n");
            return 0;
        }

    }

    printf("pass\n");
    return 0;
}


