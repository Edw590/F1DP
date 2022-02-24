//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_BLOCKADDRUTILS_H
#define FALLOUT1DOSPATCHES_BLOCKADDRUTILS_H



#include <stdint.h>

#define SN_CODE_SEC_BLOCK_ADDR 0x76563412 // 12 34 56 75 in little endian
#define SN_DATA_SEC_BLOCK_ADDR 0x75563412 // 12 34 56 74 in little endian

void * getRealBlockAddrCode(const void * addr);
void * getRealBlockAddrData(const void * addr);



#endif //FALLOUT1DOSPATCHES_BLOCKADDRUTILS_H
