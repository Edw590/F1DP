//
// Created by DADi590 on 20/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_GAMESTDCFUNCS_H
#define FALLOUT1DOSPATCHES_GAMESTDCFUNCS_H



#include "GameAddrs.h"
#include "AddressUtils.h"
#include <stdint.h>
#include <stddef.h>

/**
 * To use this macro, get all the arguments you want to send to printf in a variable of themselves. After that, push
 * ECX (required because it will be changed here), and start pushing all arguments in the opposite order you'd give them
 * to the C printf() function. Also, tell the macro how many parameters you're passing to printf so the macro can clean
 * up the stack for you.
 *
 * Example of use:
 *  {
 *  	uint32_t str1_addr = getRealBlockAddrData("Example 1");
 *  	uint32_t str2_addr = getRealBlockAddrData("Example 2");
 *  	__asm {
 *  		push    ecx
 *  		push    dword ptr [str1_addr]
 *  		push    dword ptr [str2_addr]
 *  	}
 *  	printf(1);
 *  }
 */
#define printf(NUM_PARAMS)                      \
    __asm mov     ecx, SN_CODE_SEC_EXE_ADDR     \
	__asm add     ecx, F_printf_                \
	__asm call    ecx                           \
	__asm add     esp, NUM_PARAMS * 4           \
	__asm pop     ecx

int open(const char *const pathname, const int flags);
int filelength(const int fd);
void * malloc(const size_t size);
int read(const int fd, const void *buf, const size_t count);
int close(const int fd);
void exit(const int status);



#endif //FALLOUT1DOSPATCHES_GAMESTDCFUNCS_H
