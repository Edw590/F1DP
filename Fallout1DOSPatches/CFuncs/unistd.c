//
// Created by DADi590 on 21/02/2022.
//

#include <sys/types.h>
#include "../Utils/BlockAddrUtils.h"
#include "../GameAddrs/Funcs.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "unistd.h"

int close(int fildes) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, dword ptr [fildes]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_close_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	return ret_var;
}

ssize_t read(int fildes, void * buf, size_t nbyte) {
	int ret_var = 0;

	// Pointer correction
	buf = getRealBlockAddrData(buf);

	__asm {
		pusha

		mov     eax, dword ptr [fildes]
		mov     edx, dword ptr [buf]
		mov     ebx, dword ptr [nbyte]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_read_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	return ret_var;
}
