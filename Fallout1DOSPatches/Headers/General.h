//
// Created by DADi590 on 05/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_GENERAL_H
#define FALLOUT1DOSPATCHES_GENERAL_H



#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NULL ((void *)0)

#define getSegAddr(block_address, SEG_OFFSET) *((uint32_t *) (block_address + SEG_OFFSET))

uint32_t getRealAddrData(const uint32_t block_address, const void * const address);
uint32_t getRealAddrCode(const uint32_t block_address, const void * const address);



#endif //FALLOUT1DOSPATCHES_GENERAL_H
