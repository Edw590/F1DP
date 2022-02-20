cmake_minimum_required(VERSION 3.21)
project(Fallout1DOSPatches C)

# NOTE: this file exists only so that CLion can check the code with Microsoft Visual C++ compiler. It's not used with
# Open Watcom at all. I only use CLion for the static analysis code check. I could be doing this in Sublime Text or
# VS Code or with Notepad.

# If the standard is changed here, change on Cppcheck too.
set(CMAKE_C_FLAGS "-std=c90 -Wall -pedantic-errors")
# These flags are specifically selected for the Microsoft Visual C++ Compiler (for GCC I'd add some more).

add_executable(Fallout1DOSPatches main.c Headers/General.h Headers/GameAddrs.h GameStdCFuncs.c Headers/GameStdCFuncs.h General.c)
