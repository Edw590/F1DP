//
// Created by DADi590 on 20/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_LOGGER_H
#define FALLOUT1DOSPATCHES_LOGGER_H



#include "AddressUtils.h"

const char logger_str[] = "[Log F1DP] %s\n";

// Same usage as printf. Look there.
#define logLnConsole(NUM_PARAMS)                   \
	__asm mov     ecx, offset logger_str           \
	__asm add     ecx, SN_DATA_SEC_BLOCK_ADDR      \
	__asm push    ecx                              \
	printf(NUM_PARAMS + 1);



#endif //FALLOUT1DOSPATCHES_LOGGER_H
