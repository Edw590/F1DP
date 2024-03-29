/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2022 DADi590

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
// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-08-28.

#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/EXEPatchUtils.h"
#include <stdint.h>

//____________________________________________________________________________
uint32_t F_GetFrmID(uint32_t objType, uint32_t lstNum, uint32_t id2, uint32_t id1, uint32_t id3) {
	uint32_t FID = 0;
	uint32_t F_LOAD_FRM = (uint32_t) getRealEXEAddr(C_art_id_);
	__asm {
			push    id3
			mov     ecx, id1
			mov     ebx, id2
			MOV     edx, lstNum
			MOV     eax, objType
			call    F_LOAD_FRM
			mov     FID, eax
	}
	return FID;
}
