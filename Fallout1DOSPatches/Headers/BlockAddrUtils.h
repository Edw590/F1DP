//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_BLOCKADDRUTILS_H
#define FALLOUT1DOSPATCHES_BLOCKADDRUTILS_H



#include <stdint.h>

uint32_t getRealBlockAddrData(const void * const addr);
uint32_t getRealBlockAddrCode(const void * const addr);



#endif //FALLOUT1DOSPATCHES_BLOCKADDRUTILS_H
