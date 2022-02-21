//
// Created by DADi590 on 20/02/2022.
//

#include "Headers/BlockAddrUtils.h"

#define SN_CODE_SEC_BLOCK_ADDR 0x76563412 // 12 34 56 75 in little endian
#define SN_DATA_SEC_BLOCK_ADDR 0x75563412 // 12 34 56 74 in little endian

uint32_t getRealBlockAddrCode(const void * const addr) {
	return (uint32_t) addr + SN_CODE_SEC_BLOCK_ADDR;
}

uint32_t getRealBlockAddrData(const void * const addr) {
	return (uint32_t) addr + SN_DATA_SEC_BLOCK_ADDR;
}
