//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_STDIO_H
#define FALLOUT1DOSPATCHES_STDIO_H



#include "../GameAddrs/Funcs.h"
#include "../OtherHeaders/General.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"

#define LOGGER_STR "[Log F1DP] %s"NL

__declspec(naked) int printf(const char * format, ...);



#endif //FALLOUT1DOSPATCHES_STDIO_H
