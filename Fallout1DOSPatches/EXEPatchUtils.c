//
// Created by DADi590 on 20/02/2022.
//


#include "Headers/EXEPatchUtils.h"
#include "Headers/AddressUtils.h"

// And right after, the Data section begins
#define CODE_SEC_EXE_IDA_END_ADDR 0xEAFFF

// Volatile to turn off optimizations for automatic addition if the bool value is constant. The SN_ constants must NEVER
// be optimized, so the patchPatcher() can do its job fully.
uint32_t getRealAddr(const uint32_t addr);

void writeMem32(const uint32_t addr, const uint32_t data) {
	uint32_t real_addr = getRealAddr(addr);

	*(uint32_t *) real_addr = data;
}
void writeMem16(const uint32_t addr, const uint16_t data) {
	uint32_t real_addr = getRealAddr(addr);

	*(uint16_t *) real_addr = data;
}
void writeMem8(const uint32_t addr, const uint8_t data) {
	uint32_t real_addr = getRealAddr(addr);

	*(uint8_t *) real_addr = data;
}

uint32_t readMem32(const uint32_t addr) {
	uint32_t real_addr = getRealAddr(addr);

	return *(uint32_t *) real_addr;
}
uint16_t readMem16(const uint32_t addr) {
	uint32_t real_addr = getRealAddr(addr);

	return *(uint16_t *) real_addr;
}
uint8_t readMem8(const uint32_t addr) {
	uint32_t real_addr = getRealAddr(addr);

	return *(uint8_t *) real_addr;
}


void memSet(const uint32_t addr, const uint8_t val, const int len) {
	int i = 0;
	uint32_t real_addr = getRealAddr(addr);

	for (i = 0; i < len; ++i) {
		*(uint8_t *) (real_addr + i) = val;
	}
}

void strCpy(const uint32_t addr, const char *const str, const int str_len) {
	int i = 0;
	uint32_t real_addr = getRealAddr(addr);

	for (i = 0; i < str_len; ++i) {
		*(uint8_t *) (real_addr + i) = str[i];
	}
}

void HookCall(const uint32_t addr, const void *const func) {
	uint32_t real_addr = getRealAddr(addr);

	writeMem32(real_addr + 1, (uint32_t) func - (real_addr + 5));
}

void MakeCall(const uint32_t addr, const void *const func, const bool jump) {
	uint32_t real_addr = getRealAddr(addr);

	writeMem8(jump ? 0xE9 : 0xE8, real_addr);
	HookCall(real_addr, func);
}

void BlockCall(const uint32_t addr) {
	uint32_t real_addr = getRealAddr(addr);

	memSet(real_addr, 0x90, 5);
}



uint32_t getRealAddr(const uint32_t addr) {
	if (addr <= CODE_SEC_EXE_IDA_END_ADDR) {
		return addr + SN_CODE_SEC_EXE_ADDR;
	} else {
		return addr + SN_DATA_SEC_EXE_ADDR;
	}
}
