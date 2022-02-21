//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_PATCHPATCHER_H
#define FALLOUT1DOSPATCHES_PATCHPATCHER_H



#include <stdint.h>

#define SN_BASE 0x70563412 // 12 34 56 70 in little endian

void patchPatcher(uint32_t sn_base);



#endif //FALLOUT1DOSPATCHES_PATCHPATCHER_H
