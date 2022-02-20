cmake_minimum_required(VERSION 3.21)
project(Fallout1DOSPatches C)

# If the standard is changed here, change on Cppcheck too.
set(CMAKE_C_FLAGS "-std=c90 -Wall -pedantic-errors")
# These flags are specifically selected for the Microsoft Visual C++ Compiler (for GCC I'd add some more).

add_executable(Fallout1DOSPatches main.c General.h GameAddrs.h GameStandardCFuncs.h)
