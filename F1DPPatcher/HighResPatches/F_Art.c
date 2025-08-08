/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2025 Edw590

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

#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/GlobalEXEAddrs.h"

//____________________________________________________________________________
void __declspec(naked) F_GetFrmID() {
    __asm {
            push    ebp
            mov     ebp, esp
            push    ecx
            push    ebx
            lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
            push    [ebp+0x18]
            mov     ecx, [ebp+0x14]
            mov     ebx, [ebp+0x10]
            mov     edx, [ebp+0xC]
            mov     eax, [ebp+0x8]
            mov     [esp+1*4], edi // [Edw590: "PUSH EDI"]
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_art_id_]
            call    edi
            pop     edi
            mov     [ebp-0x4], eax
            mov     eax, [ebp-0x4]
            pop     ebx
            mov     esp, ebp
            pop     ebp
            ret
    }
}
