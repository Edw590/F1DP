//
// Created by DADi590 on 20/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_GAMESTDCFUNCS_H
#define FALLOUT1DOSPATCHES_GAMESTDCFUNCS_H



#include "General.h"
#include "GameAddrs.h"

#define CODE_SEG_ADDR_OFFSET 4
#define DATA_SEG_ADDR_OFFSET 8

#define printfBefore(block_address)                       \
	__asm {push    esi}                                   \
	__asm {push    edi}                                   \
	__asm {push    eax}                                   \
	__asm {mov     esi, [block_address]}                  \
	__asm {mov     esi, [esi + CODE_SEG_ADDR_OFFSET]}     \
	__asm {mov     edi, [block_address]}                  \
	__asm {mov     edi, [edi + DATA_SEG_ADDR_OFFSET]}
#define printfAfter(block_address)                        \
	__asm {pop     eax}                                   \
	__asm {pop     edi}                                   \
	__asm {pop     esi}

__declspec(naked) int printf(const char * const format, ...);
int open(const uint32_t code_seg_addr, const char * const pathname, const int flags);
int filelength(const uint32_t code_seg_addr, const int fd);
void * malloc(const uint32_t code_seg_addr, const size_t size);
int read(const uint32_t code_seg_addr, const int fd, const void * buf, const size_t count);
int close(const uint32_t code_seg_addr, const int fd);
void exit(const uint32_t code_seg_addr, const int status);



#endif //FALLOUT1DOSPATCHES_GAMESTDCFUNCS_H
