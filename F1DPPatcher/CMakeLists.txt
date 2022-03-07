cmake_minimum_required(VERSION 3.21)
project(F1DPPatcher C)

# NOTE: this file exists only so that CLion can check the code with Microsoft Visual C++ compiler. It's not used with
# Open Watcom at all. I only use CLion for the static analysis code check. I could be doing this in Sublime Text or
# VS Code or with Notepad.

# If the standard is changed here, change on Cppcheck too.
set(CMAKE_C_FLAGS "-std=c90 -Wall -pedantic-errors")
# These flags are specifically selected for the Microsoft Visual C++ Compiler (for GCC I'd add some more).

add_executable(F1DPPatcher main.c OtherHeaders/General.h GameAddrs/CStdFuncs.h Utils/EXEPatchUtils.c Utils/EXEPatchUtils.h Utils/BlockAddrUtils.c OtherHeaders/GlobalEXEAddrs.h PatcherPatcher.c OtherHeaders/PatcherPatcher.h Utils/BlockAddrUtils.h Utils/IniUtils.c Utils/IniUtils.h CLibs/stdio.h CLibs/stdlib.c CLibs/stdlib.h CLibs/fcntl.c CLibs/fcntl.h CLibs/io.c CLibs/io.h CLibs/unistd.c CLibs/unistd.h CLibs/CLibs.txt CLibs/string.c CLibs/string.h SFall1Patches/SFall1Patches.c SFall1Patches/SFall1Patches.h CLibs/stdio.c CLibs/conio.c CLibs/conio.h CLibs/ctype.c CLibs/ctype.h OtherHeaders/GlobalVars.h SFall1Patches/Define.h SFall1Patches/FalloutEngine.h SFall1Patches/Inventory.c SFall1Patches/Inventory.h SFall1Patches/Bugs.c SFall1Patches/Bugs.h SFall1Patches/dinput.c SFall1Patches/dinput.h SFall1Patches/sFall1Main.c SFall1Patches/sFall1Main.h SFall1Patches/PartyControl.c SFall1Patches/PartyControl.h SFall1Patches/LoadGameHook.c SFall1Patches/LoadGameHook.h SFall1Patches/timer.cNOTDONE SFall1Patches/timer.hNOTDONE CLibs/math.c CLibs/math.h SFall1Patches/AmmoMod.c SFall1Patches/AmmoMod.h SFall1Patches/Criticals.c SFall1Patches/Criticals.h SFall1Patches/AnimationsAtOnceLimit.c SFall1Patches/AnimationsAtOnceLimit.h SFall1Patches/MainMenu.c SFall1Patches/MainMenu.h)
