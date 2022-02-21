//
// Created by DADi590 on 20/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_ADDRESSUTILS_H
#define FALLOUT1DOSPATCHES_ADDRESSUTILS_H



// Special 32-bit numbers ("SN") that are not supposed to appear ANYwhere else in the entire code copied to the BIN
// file. They're weird enough for me to think they won't appear. (Too much coincidence?)
#define SN_MAIN_FUNCTION 0x78563412 // 12 34 56 78 in little endian
#define SN_CODE_SEC_EXE_ADDR 0x79563412 // 12 34 56 79 in little endian
#define SN_DATA_SEC_EXE_ADDR 0x77563412 // 12 34 56 77 in little endian
// ATTENTION: these are NOT all the ones that exist. Seach for "SN_" on the entire project files to see the others.



#endif //FALLOUT1DOSPATCHES_ADDRESSUTILS_H
