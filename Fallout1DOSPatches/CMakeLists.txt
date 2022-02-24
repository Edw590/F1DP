cmake_minimum_required(VERSION 3.21)
project(Fallout1DOSPatches C)

# NOTE: this file exists only so that CLion can check the code with Microsoft Visual C++ compiler. It's not used with
# Open Watcom at all. I only use CLion for the static analysis code check. I could be doing this in Sublime Text or
# VS Code or with Notepad.

# If the standard is changed here, change on Cppcheck too.
set(CMAKE_C_FLAGS "-std=c90 -Wall -pedantic-errors")
# These flags are specifically selected for the Microsoft Visual C++ Compiler (for GCC I'd add some more).

add_executable(Fallout1DOSPatches main.c OtherHeaders/General.h GameAddrs/Funcs.h SFall1Patches/MainMenu.c Utils/EXEPatchUtils.c Utils/EXEPatchUtils.h Utils/BlockAddrUtils.c OtherHeaders/GlobalEXEAddrs.h SFall1Patches/Criticals.c SFall1Patches/Criticals.h PatcherPatcher.c OtherHeaders/PatcherPatcher.h SFall1Patches/MainMenu.h Utils/BlockAddrUtils.h Utils/IniUtils.c Utils/IniUtils.h CFuncs/stdio.h CFuncs/stdlib.c CFuncs/stdlib.h CFuncs/fcntl.c CFuncs/fcntl.h CFuncs/io.c CFuncs/io.h CFuncs/unistd.c CFuncs/unistd.h CFuncs/CFuncs.c CFuncs/string.c CFuncs/string.h GameAddrs/Strings.h SFall1Patches/SFall1Patches.c SFall1Patches/SFall1Patches.h CFuncs/stdio.c)
