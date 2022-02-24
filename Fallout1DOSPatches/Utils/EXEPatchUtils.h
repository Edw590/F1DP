//
// Created by DADi590 on 20/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_EXEPATCHUTILS_H
#define FALLOUT1DOSPATCHES_EXEPATCHUTILS_H



#include <stdint.h>
#include <stdbool.h>

void writeMem32EXE(uint32_t addr, uint32_t data);
void writeMem16EXE(uint32_t addr, uint16_t data);
void writeMem8EXE(uint32_t addr, uint8_t data);
uint32_t readMem32EXE(uint32_t addr);
uint16_t readMem16EXE(uint32_t addr);
uint8_t readMem8EXE(uint32_t addr);

void memsetEXE(uint32_t addr, int c, size_t n);

void strcpyEXE(uint32_t addr, const char * s2);
__declspec(noreturn) void strcpy_sEXE(uint32_t addr, const char * s2, int n);

void HookCallEXE(uint32_t addr, const void * func);

void MakeCallEXE(uint32_t addr, const void * func, bool jump);

void BlockCallJmpEXE(uint32_t addr);



#endif //FALLOUT1DOSPATCHES_EXEPATCHUTILS_H
