//
// Created by DADi590 on 21/02/2022.
//

#include "../GameAddrs/Funcs.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "string.h"
#include "../Utils/BlockAddrUtils.h"
#include <stdint.h>

void * memset(void *s, int c, size_t n) {
	void * ret_var = NULL;

	// Pointer correction
	s = getRealBlockAddrData(s);

	__asm {
		pusha

		mov     eax, dword ptr [s]
		mov     edx, dword ptr [c]
		mov     ebx, dword ptr [n]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_memset_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}

char * strcpy(char * s1, const char * s2) {
	char * ret_var = NULL;

	// Pointer correction
	s1 = getRealBlockAddrData(s1);
	s2 = getRealBlockAddrData(s2);

	__asm {
		pusha

		mov     eax, dword ptr [s1]
		mov     edx, dword ptr [s2]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_strcpy_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}

int strncmp(const char * s1, const char * s2, size_t n) {
	int ret_var = 0;

	// Pointer correction
	s1 = getRealBlockAddrData(s1);
	s2 = getRealBlockAddrData(s2);

	__asm {
		pusha

		mov     eax, dword ptr [s1]
		mov     edx, dword ptr [s2]
		mov     ebx, dword ptr [n]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_strncmp_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	return ret_var;
}

char * strncpy(char * s1, const char * s2, size_t n) {
	char * ret_var = NULL;

	// Pointer correction
	s1 = getRealBlockAddrData(s1);
	s2 = getRealBlockAddrData(s2);

	__asm {
		pusha

		mov     eax, dword ptr [s1]
		mov     edx, dword ptr [s2]
		mov     ebx, dword ptr [n]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_strncpy_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}

size_t strnlen(const char * s, size_t maxlen) {
	size_t len = 0;

	// I don't find the function on the EXE. I only find __far_strlen, but I don't like the "far" part there. That's
	// related to going from one segment to others. None of the other functions I'm using have that and they work. So I
	// guess I'll just implement it myself.
	// EDIT: since it's my implementation, might as well go right away for strnlen() instead of strlen().

	// Pointer correction
	s = getRealBlockAddrData(s);

	for (len = 0; len < maxlen ; ++len) {
		if ('\0' == s[len]) {
			break;
		}
	}

	return len;
}

char * strrchr(const char * s, int c) {
	char * ret_var = NULL;

	// Pointer correction
	s = getRealBlockAddrData(s);

	__asm {
		pusha

		mov     eax, dword ptr [s]
		mov     edx, dword ptr [c]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_strrchr_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}
