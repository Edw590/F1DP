cmake_minimum_required(VERSION 3.21)
project(F1DPPatcher C)

# Open Watcom 2 toolchain created with instructions from:
# https://github.com/JetBrains/clion-custom-defined-compiler-examples/tree/master/CMake-OpenWatcom2.

# If the standard is changed here, change on Cppcheck too.
# If the build target (bt) is changed here, change on CMake options settings page too.
set(CMAKE_C_FLAGS "-bt=dos -l=dos4g -fpd -mf -oi -ol+ -oneatx -ot -wcd=119 -we -wo -wx -ze")
# For testing (no optimizations, so errors come up decently, as already happened an important crash not happening
# with optimizations, but happen without them, so disable them for debug and testing and only enable for releases (or
# not even that, doesn't really matter in such a small program).

# Example of Warning 119: "Warning! W119: Address of static function '[something]' has been taken". The program needs
# the functions to be static and it's needed to get their address for the game EXE. So ignore the warning.

add_executable(F1DPPatcher main.c Utils/General.h GameAddrs/CStdFuncs.h Utils/EXEPatchUtils.c Utils/EXEPatchUtils.h Utils/BlockAddrUtils.c Utils/GlobalEXEAddrs.h PatcherPatcher/PatcherPatcher.c PatcherPatcher/PatcherPatcher.h Utils/BlockAddrUtils.h Utils/IniUtils.c Utils/IniUtils.h CLibs/stdio.h CLibs/stdlib.c CLibs/stdlib.h CLibs/fcntl.c CLibs/fcntl.h CLibs/io.c CLibs/io.h CLibs/unistd.c CLibs/unistd.h CLibs/CLibs.txt CLibs/string.c CLibs/string.h SFall1Patches/SFall1Patches.c SFall1Patches/SFall1Patches.h CLibs/stdio.c CLibs/conio.c CLibs/conio.h CLibs/ctype.c CLibs/ctype.h Utils/GlobalVars.h SFall1Patches/Define.h GameAddrs/FalloutEngine.h SFall1Patches/Inventory.c SFall1Patches/Inventory.h SFall1Patches/Bugs.c SFall1Patches/Bugs.h SFall1Patches/dinput.c SFall1Patches/dinput.h SFall1Patches/SFall1Main.c SFall1Patches/SFall1Main.h SFall1Patches/PartyControl.c SFall1Patches/PartyControl.h SFall1Patches/LoadGameHook.c SFall1Patches/LoadGameHook.h SFall1Patches/timer.cNOTDONE SFall1Patches/timer.hNOTDONE CLibs/math.c CLibs/math.h SFall1Patches/AmmoMod.c SFall1Patches/AmmoMod.h SFall1Patches/Criticals.c SFall1Patches/Criticals.h SFall1Patches/AnimationsAtOnceLimit.c SFall1Patches/AnimationsAtOnceLimit.h SFall1Patches/MainMenu.c SFall1Patches/MainMenu.h SFall1Patches/Credits.c SFall1Patches/Credits.h SFall1Patches/version.h SFall1Patches/Quests.c SFall1Patches/Quests.h SFall1Patches/Sound.c SFall1Patches/Sound.h TeamXPatches/TeamXPatches.c TeamXPatches/TeamXPatches.h FixtPatches/FixtPatches.c FixtPatches/FixtPatches.h SFall1Patches/AI.c SFall1Patches/AI.h HighResPatches/HighResPatchMain.c HighResPatches/HighResPatchMain.h SFall1Patches/DebugMode.c SFall1Patches/DebugMode.h HighResPatches/HighResPatches.c HighResPatches/HighResPatches.h HighResPatches/fixes_other.c HighResPatches/fixes_other.h HighResPatches/fixes_dialog.c HighResPatches/fixes_dialog.h HighResPatches/F_Objects.h HighResPatches/fixes_win_general.c HighResPatches/fixes_win_general.h HighResPatches/fixes_maps.cNOTDONE HighResPatches/fixes_maps.h HighResPatches/F_Mapper.c HighResPatches/F_Mapper.h HighResPatches/F_Art.h)
