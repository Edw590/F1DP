//
// Created by DADi590 on 20/02/2022.
//


#include "EXEPatchUtils.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../CFuncs/string.h"

// And right after, the Data section begins
#define CODE_SEC_EXE_IDA_END_ADDR 0xEAFFF

// All these functions suppose `addr` is an address value seen on IDA as the value if the EXE would load in its supposed
// address, 0x10000 (for the code section), and something else for the data section. This is important because they
// automatically they care of seeing if the address is supposed to be from the Data section or the Code section and it
// is corrected internally. So do NOT correct it externally, or it WILL be wrong inside the functions.

void * getRealEXEAddr(uint32_t addr);

void writeMem32EXE(uint32_t addr, uint32_t data) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	*real_addr = data;
}
void writeMem16EXE(uint32_t addr, uint16_t data) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	*(uint16_t *) real_addr = data;
}
void writeMem8EXE(uint32_t addr, uint8_t data) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	*(uint8_t *) real_addr = data;
}

uint32_t readMem32EXE(uint32_t addr) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	return *real_addr;
}
uint16_t readMem16EXE(uint32_t addr) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	return *(uint16_t *) real_addr;
}
uint8_t readMem8EXE(uint32_t addr) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	return *(uint8_t *) real_addr;
}


void memsetEXE(uint32_t addr, int c, size_t n) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	memset(real_addr, c, n);
}

void strcpyEXE(uint32_t addr, const char * s2) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	strcpy((char *) real_addr, s2);
}

// AFTER you test if the last line of the function is being removed (optimizations), and AFTER you test if this actually
// works, remove noreturn thing. This is just here as a reminder.
__declspec(noreturn) void strcpy_sEXE(uint32_t addr, const char * s2, int n) {
	uint32_t * real_addr = getRealEXEAddr(addr);

	strncpy((char *) real_addr, s2, (size_t) n);
	((char *) addr)[n] = '\0';
}

void HookCallEXE(uint32_t addr, const void * func) {
	uint32_t real_addr = (uint32_t) getRealEXEAddr(addr);

	writeMem32EXE(real_addr + 1, (uint32_t) func - (real_addr + 5));
}

void MakeCallEXE(uint32_t addr, const void * func, bool jump) {
	uint32_t real_addr = (uint32_t) getRealEXEAddr(addr);

	writeMem8EXE(jump ? 0xE9 : 0xE8, real_addr);
	HookCallEXE(real_addr, func);
}

void BlockCallJmpEXE(uint32_t addr) {
	uint32_t real_addr = (uint32_t) getRealEXEAddr(addr);

	// 5 is the length of the CALL near instruction with opcode E8 or the JMP near instruction with opcode E9.
	memsetEXE(real_addr, 0x90, 5);
}



void * getRealEXEAddr(uint32_t addr) {
	if (addr <= CODE_SEC_EXE_IDA_END_ADDR) {
		return (uint32_t *) (addr + SN_CODE_SEC_EXE_ADDR);
	} else {
		return (uint32_t *) (addr + SN_DATA_SEC_EXE_ADDR);
	}
}
