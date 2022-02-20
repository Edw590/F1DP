//
// Created by DADi590 on 19/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_GAMESTANDARDCFUNCS_H
#define FALLOUT1DOSPATCHES_GAMESTANDARDCFUNCS_H

// Naming: M == Macro; N == Naked

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
/**
 * <p>ALWAYS call printfBefore() before calling this, and printfAfter() after calling this --> ALWAYS</p>
 */
__declspec(naked) int printf(const char * const format, ...) {
	// This is the implementation of printf_() on the Fallout 1 DOS EXE.
	// If I want a C printf function, I must not call printf() on the EXE inside the C function --> the C function must
	// have the printf() code already, so that the stack is not modified more than it was supposed.
	// The stack is modified when one calls the printf() in assembly. Here, if I call a C function that calls printf(),
	// I already messed up the stack. So this function is the printf() implementation to mimic someone calling printf()
	// with Assembly.
	__asm {
		push    ebx
		push    edx
		sub     esp, 4
		mov     eax, [format] // This is just for the warnings/errors on the compiler to go away with unused parameter.
		lea     eax, [esp + 0x14]
		mov     ebx, esp
		mov     edx, [esp + 0x10]
		mov     [esp], eax
		lea     eax, [edi + 0x13C02A]
		lea     ecx, [esi + 0xDF9C3] // __fprtf_()
		call    ecx
		add     esp, 4
		pop     edx
		pop     ebx
		retn
	}
}

int open(const uint32_t code_seg_addr, const char * const pathname, const int flags) {
	int ret_var = 0;
	__asm {
		pusha

		push    0
		push    [flags]
		mov     eax, [pathname]
		push    eax
		mov     ecx, [code_seg_addr]
		add     ecx, F_open_
		call    ecx
		add     esp, 0Ch
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

int filelength(const uint32_t code_seg_addr, const int fd) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, [fd]
		mov     ecx, [code_seg_addr]
		add     ecx, F_filelength_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

void * malloc(const uint32_t code_seg_addr, const size_t size) {
	void * ret_var = NULL;
	__asm {
		pusha

		mov     eax, [size]
		mov     ecx, [code_seg_addr]
		add     ecx, F__nmalloc_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

int read(const uint32_t code_seg_addr, const int fd, const void * buf, const size_t count) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, [fd]
		mov     ebx, [count]
		mov     edx, [buf]
		mov     ecx, [code_seg_addr]
		add     ecx, F_read_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

int close(const uint32_t code_seg_addr, const int fd) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, [fd]
		mov     ecx, [code_seg_addr]
		add     ecx, F_close_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

void exit(const uint32_t code_seg_addr, const int status) {
	__asm {
		// No need for pusha and popa --> exit() does not return
		mov     eax, [status]
		mov     ecx, [code_seg_addr]
		add     ecx, F_exit_
		call    ecx
	}
}

#endif //FALLOUT1DOSPATCHES_GAMESTANDARDCFUNCS_H
