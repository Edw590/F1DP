//
// Created by DADi590 on 21/02/2022.
//

#include "../GameAddrs/Funcs.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"

int open(const char * path, int oflag, int mode) {
	int ret_var = 0;

	// Pointer correction
	path = getRealBlockAddrData(path);

	__asm {
		pusha

		push    dword ptr [mode]
		push    dword ptr [oflag]
		push    dword ptr [path]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_open_
		call    ecx
		add     esp, 0Ch
		mov     dword ptr [ret_var], eax

		popa
	}

	return ret_var;
}
