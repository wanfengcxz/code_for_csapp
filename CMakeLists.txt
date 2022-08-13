cmake_minimum_required(VERSION 3.19)
project(code_for_csapp)

set(CMAKE_CXX_STANDARD 14)
include_directories("src/headers/")
include_directories("src/disk/")

# tests
### algorithm
##### bit operation
add_executable(test_bit_op src/tests/test_bit_op.cpp src/algorithm/bit_op.cpp)
### common
add_executable(common src/tests/test_common.cpp src/common/common.cpp)


# hardware
# # cpu
# # # mesi
add_executable(mesi src/hardware/cpu/mesi.cpp)

add_executable(main main.cpp src/hardware/memory/inst.cpp src/hardware/cpu/mmu.cpp src/hardware/memory/dram.cpp src/disk/code.cpp src/hardware/cpu/register.cpp)

# ass_old_code
# assembly simulator

# 废弃
add_executable(ass src/ass_old_code/ass.cpp)