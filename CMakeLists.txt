cmake_minimum_required(VERSION 3.19)
project(code_for_csapp)

set(CMAKE_CXX_STANDARD 14)
include_directories("src/")


# hardware
# # cpu
# # # mesi
add_executable(mesi src/hardware/cpu/mesi.cpp)

add_executable(hardware test_hardware.cpp src/hardware/cpu/isa.cpp src/hardware/memory/dram.cpp src/hardware/cpu/mmu.cpp src/common/convert.cpp src/common/print.cpp)

add_executable(linker test_elf.cpp src/linker/parse_elf.cpp src/common/print.cpp src/common/convert.cpp)