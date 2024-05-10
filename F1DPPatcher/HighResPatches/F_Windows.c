/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2022 Edw590

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//
// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-08-28.

#include "../CLibs/stdlib.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "F_Windows.h"

//__________________________________________________
uint8_t* FReallocateMemory(uint8_t * mem, uint32_t sizeBytes) {
	__asm {
			mov     edx, sizeBytes
			mov     eax, mem
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mem_realloc_]
			call    edi
			pop     edi
			mov     mem, eax
	}
	return mem;
}


//__________________________________
struct WinStruct* GetWinStruct(int32_t WinRef) {

	struct WinStruct* winStruct = NULL;
	__asm {
			mov     eax, WinRef
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_GNW_find_]
			call    edi
			pop     edi
			mov     winStruct, eax
	}
	return winStruct;
}
