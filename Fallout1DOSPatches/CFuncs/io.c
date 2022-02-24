//
// Created by DADi590 on 21/02/2022.
//

#include "../GameAddrs/Funcs.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "io.h"

long filelength(int handle) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, dword ptr [handle]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_filelength_
		call    ecx
		mov     dword ptr [ret_var], eax

		popa
	}

	return ret_var;
}
