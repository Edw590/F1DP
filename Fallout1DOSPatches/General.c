//
// Created by DADi590 on 20/02/2022.
//

#include "Headers/General.h"

#define BLOCK_CODE_SEG_OFFSET 0x10 // The code segment begins in the 16th byte, not on the 0th
#define BLOCK_DATA_SEG_OFFSET 0x1000 + BLOCK_CODE_SEG_OFFSET // The data segment begins 0x1000 bytes after the code segment (for now at least)

uint32_t getRealAddrData(const uint32_t block_address, const void * const address) {
	return (uint32_t) address + block_address + BLOCK_DATA_SEG_OFFSET;
}

uint32_t getRealAddrCode(const uint32_t block_address, const void * const address) {
	return (uint32_t) address + block_address + BLOCK_CODE_SEG_OFFSET;
}
