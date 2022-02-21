//
// Created by DADi590 on 20/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_EXEPATCHUTILS_H
#define FALLOUT1DOSPATCHES_EXEPATCHUTILS_H



#include <stdint.h>
#include <stdbool.h>

void writeMem32(const uint32_t addr, const uint32_t data);
void writeMem16(const uint32_t addr, const uint16_t data);
void writeMem8(const uint32_t addr, const uint8_t data);
uint32_t readMem32(const uint32_t addr);
uint16_t readMem16(const uint32_t addr);
uint8_t readMem8(const uint32_t addr);

void memSet(const uint32_t addr, const uint8_t val, const int len);

void strCpy(const uint32_t addr, const char *const str, const int str_len);

void HookCall(const uint32_t addr, const void *const func);

void MakeCall(const uint32_t addr, const void *const func, const bool jump);

void BlockCall(const uint32_t addr);



#endif //FALLOUT1DOSPATCHES_EXEPATCHUTILS_H
