cmake_minimum_required(VERSION 3.22)
project(F1DPInstaller C)

# NOTE: this file exists only so that CLion can check the code with Microsoft Visual C++ compiler. It's not used with
# Open Watcom at all. I only use CLion for the static analysis code check. I could be doing this in Sublime Text or
# VS Code or with Notepad.

# If the standard is changed here, change on Cppcheck too.
set(CMAKE_C_FLAGS "-std=c90 -Wall -pedantic-errors")
# These flags are specifically selected for the Microsoft Visual C++ Compiler (for GCC I'd add some more).

add_executable(F1DPInstaller main.c md5.c md5.h)
