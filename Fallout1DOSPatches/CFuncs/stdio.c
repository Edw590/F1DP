//
// Created by DADi590 on 24/02/2022.
//

#include "stdio.h"

uint32_t ret_addr = 0;

__declspec(naked) int printf(const char * format, ...) {
	(void) format; // To ignore the unused parameter warning
	__asm {
		// Pointer correction (only on the first argument though - aside from not knowing how many others, one could be
		// just an int and I can't add the address to an int)
		// Below, I'm using ESP and not just [format] because Watcom thought it would be a good idea to use EBP to get
		// to the parameter, even though I have no instruction here that messes with EBP to equal it to ESP. So I'm
		// doing it manually.
		mov     eax, dword ptr [esp+4]
		call    getRealBlockAddrData
		mov     dword ptr [esp+4], eax

		push    ecx

		mov     ecx, dword ptr [esp+4] // Return address
		lea     eax, dword ptr [ret_addr]
		add     eax, SN_DATA_SEC_BLOCK_ADDR
		mov     dword ptr [eax], ecx

		pop     ecx
		add     esp, 4 // Remove the return address from the stack (so the stack can be what it was before calling printf())

		// Now we call printf_() with the arguments passed to printf() and nothing else on the stack - it's like we had
		// jumped to printf_() and not called a wrapper at all.
		mov     eax, SN_CODE_SEC_EXE_ADDR
		add     eax, F_printf_
		call    eax

		sub     esp, 4 // Get space on the stack to put the return address back on
		push    ecx
		lea     ecx, dword ptr [ret_addr]
		add     ecx, SN_DATA_SEC_BLOCK_ADDR
		mov     ecx, dword ptr [ecx]
		mov     dword ptr [esp+4], ecx // Store the return address on the stack again

		lea     ecx, dword ptr [ret_addr]
		add     ecx, SN_DATA_SEC_BLOCK_ADDR
		mov     dword ptr [ecx], 0 // Empty ret_addr just in case I ever use it elsewhere and check its value(?)

		pop     ecx

		ret
	}
}
