cmake_minimum_required(VERSION 3.22)
project(F1DPInstaller C)

# Open Watcom 2 toolchain created with instructions from:
# https://github.com/JetBrains/clion-custom-defined-compiler-examples/tree/master/CMake-OpenWatcom2.

# If the standard is changed here, change on Cppcheck too.
# If the build target (bt) is changed here, change on CMake options settings page too.
set(CMAKE_C_FLAGS "-bt=dos -l=dos4g -fpd -mf -oi -ol+ -oneatx -ot -wcd=119 -we -wo -wx -za99")
# For testing (no optimizations, so errors come up decently, as already happened an important crash not happening
# with optimizations, but happen without them, so disable them for debug and testing and only enable for releases (or
# not even that, doesn't really matter in such a small program).

# Example of Warning 119: "Warning! W119: Address of static function '[something]' has been taken". The program needs
# the functions to be static and it's needed to get their address for the game EXE. So ignore the warning.

add_executable(F1DPInstaller main.c md5.c md5.h)
