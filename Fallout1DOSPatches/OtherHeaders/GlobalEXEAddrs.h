//
// Created by DADi590 on 20/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_GLOBALEXEADDRS_H
#define FALLOUT1DOSPATCHES_GLOBALEXEADDRS_H



// Special 32-bit numbers ("SN") that are not supposed to appear ANYwhere else in the entire code copied to the BIN
// file. They're weird enough for me to think they won't appear. (Too much coincidence?)
#define SN_CODE_SEC_EXE_ADDR 0x79563412 // 12 34 56 79 in little endian
#define SN_DATA_SEC_EXE_ADDR 0x77563412 // 12 34 56 77 in little endian
// ATTENTION: these are NOT all the ones that exist. Seach for "SN_" on the entire project files to see the others. Most
// of them will be in the PatchPatcher.



#endif //FALLOUT1DOSPATCHES_GLOBALEXEADDRS_H
