//
// Created by DADi590 on 21/02/2022.
//

#include "../GameAddrs/Funcs.h"
#include "../OtherHeaders/General.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "stdlib.h"
#include "../Utils/BlockAddrUtils.h"

__declspec(noreturn) void exit(int status) {
	__asm {
	// No need for pusha and popa --> exit() does not return. But I'll put them anyway.
		pusha

		mov     eax, dword ptr [status]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_exit_
		call    ecx

		popa
	}
}

void free(void * ptr) {
	// ptr is a reserved word for MASM...

	// Pointer correction
	void * pointer = getRealBlockAddrData(ptr);

	__asm {
		pusha

		mov     eax, dword ptr [pointer]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F__nfree_
		call    ecx

		popa
	}
}

void * malloc(size_t size) {
	void * ret_var = NULL;
	__asm {
		pusha

		mov     eax, dword ptr [size]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F__nmalloc_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}
