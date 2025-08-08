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

#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "F_Mapper.h"
#include "F_Objects.h"
#include "HighResPatches.h"
#include "F_Art.h"
#include "F_File.h"
#include "F_Windows.h"

void __declspec(naked) bitset() {
    __asm {
            mov     eax, [esp+0x4]
            push    esi
            mov     esi, ecx
            mov     [esi], eax
            shr     eax, 5
            inc     eax
            mov     [esi+8], eax
            shl     eax, 2
            push    edi
            call    malloc
            mov     ecx, [esi+8]
            shl     ecx, 2
            mov     edx, ecx
            mov     edi, eax
            shr     ecx, 2
            mov     [esi+4], edi
            xor     eax, eax
            rep stosd
            mov     ecx, edx
            and     ecx, 3
            rep stosb
            pop     edi
            mov     eax, esi
            pop     esi
            ret     4
    }
}

void __declspec(naked) set1() {
    __asm {
            mov     edx, [esp+0x4]
            push    esi
            mov     esi, [ecx+8]
            mov     eax, edx
            shr     eax, 5
            cmp     eax, esi
            pop     esi
            jnb     locret_10009E16
            mov     ecx, [ecx+4]
            lea     eax, [ecx+eax*4]
            mov     ecx, edx
            and     ecx, 1Fh
            mov     edx, 1
            shl     edx, cl
            or      [eax], edx

        locret_10009E16:
            ret     4
    }
}
void __declspec(naked) get() {
    __asm {
            mov     edx, [esp+0x4]
            mov     eax, edx
            push    esi
            mov     esi, [ecx+8]
            shr     eax, 5
            cmp     eax, esi
            jnb     loc_10009E49
            mov     esi, [ecx+4]
            mov     eax, [esi+eax*4]
            mov     ecx, edx
            and     ecx, 1Fh
            mov     edx, 1
            shl     edx, cl
            pop     esi
            and     eax, edx
            ret     4

            loc_10009E49:
            xor     eax, eax
            pop     esi
            ret     4
    }
}


int FOG_OF_WAR = false;
int32_t fogLight = 0; // Edw590: not 0x1000 as in the original code because the feature was disabled in F1DP
struct bitset *fogHexMapBits = NULL;
bool isRecordingObjFog = false;//needed to prevent obj discovery before true pcObj position set.

struct OBJNode* upperMapObjNodeArray[40000] = {0};
struct OBJStruct* pCombatOutlineList[500] = {0};
int combatOutlineCount = 0;

//step adjustment array ori 0-5 {evenHex y, oddHex y, x}
int32_t hexStep[6][3] = {
        {0,-1, -1}, {1, 0,-1},  {1, 1, 0},  {1, 0, 1},  {0,-1, 1}, {-1,-1,0}
};
int32_t hexStep2[6][3] = {
        {0,-200, -1}, {200, 0,-1},  {200, 200, 0},  {200, 0, 1},  {0,-200, 1}, {-200,-200,0}
};


//mark and display other wall objects that are 1 hex away from current wall object - to reduce blockiness
//_____________________________________________________________
void __declspec(naked) MarkVisibleWalls() {
    __asm {
            push    ecx
            push    ebx
            mov     ebx, [esp+8+0x8]
            push    ebp
            mov     ebp, [esp+0Ch+0x4]
            push    esi
            xor     esi, esi
            push    edi
            mov     [esp+14h-0x4], esi

        loc_1000C1F3:
            mov     eax, [ebx+4]
            push    1
            push    esi
            push    eax
            call    GetNextHexPos
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__objectTable]
            pop     edi
            mov     edi, [ecx+eax*4]
            add     esp, 0Ch
            test    edi, edi
            jz      loc_1000C2B1

        loc_1000C213:
            mov     esi, [edi]
            mov     eax, [esi+28h]
            mov     ecx, [ebx+28h]
            cmp     eax, ecx
            jg      loc_1000C2AD
            xor     edx, edx
            mov     dl, [esi+23h]
            and     edx, 0Fh
            cmp     eax, ecx
            jnz     loc_1000C2A2
            mov     eax, [esi+24h]
            test    eax, 40000000h
            jnz     loc_1000C2A2
            cmp     edx, 3
            jnz     loc_1000C250
            mov     edx, [esi+4]
            or      eax, 40000000h
            mov     [esi+24h], eax
            mov     eax, [ebp+4]
            push    edx
            push    eax
            jmp     loc_1000C28C

        loc_1000C250:
            cmp     edx, 2
            jnz     loc_1000C2A2
            mov     edx, [esi+64h]
            lea     ecx, [esp+14h+0x8]
            push    ecx
            push    edx
            mov     [esp+1Ch+0x8], 0
            call    F_GetPro
            mov     eax, [esp+1Ch+0x8]
            mov     ecx, [eax+20h]
            add     esp, 8
            test    ecx, ecx
            jnz     loc_1000C2A2
            mov     eax, [esi+24h]
            mov     ecx, [esi+4]
            or      eax, 40000000h
            mov     [esi+24h], eax
            mov     edx, [ebp+4]
            push    ecx
            push    edx

        loc_1000C28C:
            call    IsInLineOfSightBlocked
            add     esp, 8
            test    al, al
            jnz     loc_1000C2A2
            push    esi
            push    ebp
            call    MarkVisibleWalls
            add     esp, 8

        loc_1000C2A2:

            mov     edi, [edi+4]
            test    edi, edi
            jnz     loc_1000C213

        loc_1000C2AD:
            mov     esi, [esp+14h-0x4]

        loc_1000C2B1:
            inc     esi
            cmp     esi, 6
            mov     [esp+14h-0x4], esi
            jl      loc_1000C1F3
            pop     edi
            pop     esi
            pop     ebp
            pop     ebx
            pop     ecx
            ret

        // This function is here and not on F_Mapper.c because Watcom refused to create the complete function for some
        // reason. Meaning: when I went there with a disassembler, the function was cut off at the beginning. Wtf?
        // So now it's here directly. Seems to have worked. Never saw this before. Weird.
        GetNextHexPos:
            sub     esp, 8
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__grid_length]
            pop     edi
            mov     edx, [ecx]
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__grid_width]
            pop     edi
            mov     ecx, [esp+8+0x4]
            push    ebx
            push    ebp
            push    esi
            mov     esi, [eax]
            mov     [esp+14h-0x4], edx
            mov     eax, ecx
            cdq
            idiv    esi
            push    edi
            mov     edi, [esp+18h+0xC]
            test    edi, edi
            mov     [esp+18h-0x8], esi
            mov     esi, edx
            mov     edx, ecx
            jle     loc_1000A3A1
            mov     ecx, [esp+18h+0x8]
            lea     edi, [ecx+ecx*2]
            lea     ecx, [ecx+ecx*2]
            shl     ecx, 2
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+hexStep+8]
            mov     ebx, [edi+ecx]
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+hexStep2+8]
            mov     ebp, [edi+ecx]
            pop     edi
            lea     esp, [esp+0]

        loc_1000A360:
            mov     ecx, esi
            and     ecx, 1
            add     ecx, edi
            shl     ecx, 2
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+hexStep2]
            add     edx, [edi+ecx]
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+hexStep]
            add     eax, [edi+ecx]
            pop     edi
            add     esi, ebx
            add     edx, ebp
            test    esi, esi
            jl      loc_1000A3AB
            cmp     esi, [esp+18h-0x8]
            jge     loc_1000A3AB
            test    eax, eax
            jl      loc_1000A3AB
            cmp     eax, [esp+18h-0x4]
            jge     loc_1000A3AB
            mov     ecx, [esp+18h+0xC]
            dec     ecx
            test    ecx, ecx
            mov     [esp+18h+0x4], edx
            mov     [esp+18h+0xC], ecx
            jg      loc_1000A360
            mov     ecx, edx

        loc_1000A3A1:
            pop     edi
            pop     esi
            pop     ebp
            mov     eax, ecx
            pop     ebx
            add     esp, 8
            ret

        loc_1000A3AB:

            mov     eax, [esp+18h+0x4]
            pop     edi
            pop     esi
            pop     ebp
            pop     ebx
            add     esp, 8
            ret
    }
}


//________________________________________________
void __declspec(naked) GetHexDistance() {
    __asm {
            mov     eax, [esp+0x4]
            cmp     eax, 0FFFFFFFFh
            jnz     loc_1000A43F
            mov     eax, 270Fh
            ret

        loc_1000A43F:
            push    ebx
            mov     ebx, [esp+4+0x8]
            cmp     ebx, 0FFFFFFFFh
            jnz     loc_1000A450
            mov     eax, 270Fh
            pop     ebx
            ret

        loc_1000A450:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__grid_width]
            pop     edi
            cdq
            push    esi
            mov     esi, [ecx]
            idiv    esi
            push    edi
            mov     edi, edx
            mov     ecx, edx
            sar     ecx, 1
            add     ecx, eax
            mov     eax, ebx
            cdq
            idiv    esi
            mov     esi, edx
            sar     esi, 1
            add     esi, eax
            mov     eax, edx
            sub     eax, edi
            shl     eax, 1
            sub     eax, esi
            add     eax, ecx
            cdq
            mov     edi, eax
            xor     edi, edx
            mov     eax, esi
            sub     eax, ecx
            sub     edi, edx
            cdq
            xor     eax, edx
            sub     eax, edx
            cmp     eax, edi
            jge     loc_1000A492
            add     eax, edi
            sar     eax, 1

        loc_1000A492:
            pop     edi
            pop     esi
            pop     ebx
            ret
    }
}


//Check is object is visible to PC. returns 1=display normaly, 0=display but darken, -1=dont display.
//_______________________________
void __declspec(naked) IsVisibleByPC() {
    __asm {
            push    ecx
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+FOG_OF_WAR]
            pop     edi
            test    eax, eax
            jz      loc_1000C2ED
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__refresh_enabled]
            pop     edi
            cmp     dword ptr [eax], 0
            jz      loc_1000C2ED
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     al, [edi+isRecordingObjFog]
            pop     edi
            test    al, al
            jnz     loc_1000C2F4

        loc_1000C2ED:

            mov     eax, 1
            pop     ecx
            ret

        loc_1000C2F4:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__obj_dude]
            pop     edi
            push    ebp
            mov     ebp, [ecx]
            push    esi
            mov     esi, [esp+0Ch+0x4]
            cmp     esi, ebp
            mov     [esp+0Ch-0x4], ebp
            jnz     loc_1000C31A
            or      dword ptr [esi+24h], 40000000h
            pop     esi
            mov     eax, 1
            pop     ebp
            pop     ecx
            ret

        loc_1000C31A:
            xor     edx, edx
            mov     dl, [esi+23h]
            push    ebx
            push    edi
            and     edx, 0Fh
            mov     edi, edx
            cmp     edi, 1
            jz      loc_1000C401
            test    dword ptr [esi+24h], 40000000h
            jnz     loc_1000C3F6
            mov     eax, [esi+4]
            mov     ecx, [ebp+4]
            push    eax
            push    ecx
            call    IsInLineOfSightBlocked
            add     esp, 8
            test    al, al
            jz      loc_1000C3DB
            mov     ebx, [esi+4]
            mov     ebp, [ebp+4]
            push    ebx
            push    ebp
            call    GetHexDistance
            add     esp, 8
            cmp     eax, 2
            jge     loc_1000C36E
            cmp     edi, 3
            jz      loc_1000C3D7

        loc_1000C36E:
            push    ebx
            push    ebp
            call    GetHexDistance
            add     esp, 8
            cmp     eax, 2
            jge     loc_1000C3C0
            cmp     edi, 2
            jnz     loc_1000C3C0
            mov     eax, [esi+64h]
            lea     edx, [esp+14h-0x4]
            push    edx
            push    eax
            mov     [esp+1Ch-0x4], 0
            call    F_GetPro
            mov     ecx, [esp+1Ch-0x4]
            mov     eax, [ecx+20h]
            add     esp, 8
            test    eax, eax
            jnz     loc_1000C43A
            mov     eax, [esi+24h]
            pop     edi
            or      eax, 40000000h
            pop     ebx
            mov     [esi+24h], eax
            pop     esi
            mov     eax, 1
            pop     ebp
            pop     ecx
            ret

        loc_1000C3C0:

            cmp     edi, 3
            jz      loc_1000C3CF
            cmp     edi, 2
            jz      loc_1000C3CF
            cmp     edi, 5
            jnz     loc_1000C43A

        loc_1000C3CF:

            pop     edi
            pop     ebx
            pop     esi
            xor     eax, eax
            pop     ebp
            pop     ecx
            ret

        loc_1000C3D7:
            mov     ebp, [esp+14h-0x4]

        loc_1000C3DB:
            mov     ecx, [esi+24h]
            or      ecx, 40000000h
            cmp     edi, 3
            mov     [esi+24h], ecx
            jnz     loc_1000C3F6
            push    esi
            push    ebp
            call    MarkVisibleWalls
            add     esp, 8

        loc_1000C3F6:

            pop     edi
            pop     ebx
            pop     esi
            mov     eax, 1
            pop     ebp
            pop     ecx
            ret

        loc_1000C401:
            mov     ecx, [esi+24h]
            mov     eax, [esi+50h]
            or      ecx, 40000000h
            test    eax, eax
            mov     [esi+24h], ecx
            jz      loc_1000C3F6
            mov     eax, [esi+74h]
            test    eax, 0FFFFFFh
            jz      loc_1000C426
            test    eax, eax
            js      loc_1000C426
            test    al, 20h
            jz      loc_1000C3F6

        loc_1000C426:

            mov     edx, [esi+4]
            mov     eax, [ebp+4]
            push    edx
            push    eax
            call    IsInLineOfSightBlocked
            add     esp, 8
            test    al, al
            jz      loc_1000C3F6

        loc_1000C43A:

            pop     edi
            pop     ebx
            pop     esi
            or      eax, 0FFFFFFFFh
            pop     ebp
            pop     ecx
            ret
    }
}


//__________________________________________________________________________
void __declspec(naked) GetTileXY() {
    __asm {
            push    ebp
            mov     ebp, esp
            push    ebx
            push    esi
            push    edi
            push    esi
            push    edi
            mov     ecx, [ebp+0x14]
            mov     edi, [ebp+0x8]
            mov     esi, [ebp+0xC]
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_offy]
            pop     edi
            mov     ebx, [eax]
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_offx]
            pop     edi
            mov     edx, [eax]
            sub     esi, ebx
            sub     edi, edx
            sub     esi, 0Ch
            lea     edx, ds:0[edi*4]
            lea     eax, ds:0[esi*4]
            sub     edx, edi
            sub     edx, eax
            mov     [ecx], edx
            test    edx, edx
            jge     loc_1000B671
            inc     edx
            mov     ebx, 0C0h
            mov     eax, edx
            sar     edx, 1Fh
            idiv    ebx
            dec     eax
            jmp     loc_1000B67D

        loc_1000B671:
            mov     ebx, 0C0h
            mov     eax, edx
            sar     edx, 1Fh
            idiv    ebx

        loc_1000B67D:
            mov     [ecx], eax
            shl     esi, 2
            add     edi, esi
            mov     ebx, [ebp+0x18]
            mov     [ebx], edi
            test    edi, edi
            jge     loc_1000B6A2
            lea     edx, [edi+1]
            mov     eax, edx
            sar     edx, 1Fh
            shl     edx, 7
            sbb     eax, edx
            sar     eax, 7
            dec     eax
            jmp     loc_1000B6B1

        loc_1000B6A2:
            mov     edx, edi
            mov     eax, edi
            sar     edx, 1Fh
            shl     edx, 7
            sbb     eax, edx
            sar     eax, 7

        loc_1000B6B1:
            mov     ebx, [ebp+0x18]
            mov     [ebx], eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_x]
            pop     edi
            mov     eax, [eax]
            add     [ecx], eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_y]
            pop     edi
            mov     eax, [eax]
            mov     edi, [ebx]
            add     edi, eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_width]
            pop     edi
            mov     eax, [eax]
            mov     [ebx], edi
            dec     eax
            mov     ebx, [ecx]
            sub     eax, ebx
            mov     [ecx], eax
            pop     edi
            pop     esi
            pop     edi
            pop     esi
            pop     ebx
            pop     ebp
            ret
    }

}


//_________________________________________________________________________
void __declspec(naked) GetScrnXYTile() {
    __asm {
            push    ebp
            mov     ebp, esp
            push    ecx
            push    ebx
            push    esi
            push    edi
            mov     [ebp-0x4], 0
            push    esi
            push    edi
            mov     eax, [ebp+0x8]
            mov     esi, eax
            test    eax, eax
            jl      loc_1000B714
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     edx, [edi+D__square_size]
            pop     edi
            cmp     eax, [edx]
            jl      loc_1000B71E

        loc_1000B714:
            mov     eax, 0FFFFFFFFh
            jmp     loc_1000B7C2

        loc_1000B71E:
            mov     edx, eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__square_width]
            pop     edi
            mov     ecx, [ecx]
            sar     edx, 1Fh
            idiv    ecx
            dec     ecx
            mov     eax, esi
            sub     ecx, edx
            mov     edx, esi
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     esi, [edi+D__square_width]
            pop     edi
            mov     esi, [esi]
            sar     edx, 1Fh
            idiv    esi
            mov     edx, eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_offx]
            pop     edi
            mov     eax, [eax]
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ebx, [edi+D__square_x]
            pop     edi
            mov     ebx, [ebx]
            mov     edi, [ebp+0xC]
            mov     [edi], eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_offy]
            pop     edi
            mov     eax, [eax]
            sub     ecx, ebx
            mov     ebx, [ebp+0x10]
            mov     [ebx], eax
            mov     eax, ecx
            lea     esi, ds:0[ecx*4]
            sub     esi, ecx
            shl     esi, 4
            mov     ecx, [edi]
            add     ecx, esi
            mov     esi, eax
            mov     [edi], ecx
            shl     esi, 2
            mov     edi, [ebx]
            sub     esi, eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_y]
            pop     edi
            mov     eax, [eax]
            shl     esi, 2
            sub     edx, eax
            sub     edi, esi
            mov     eax, edx
            mov     [ebx], edi
            shl     edx, 5
            mov     esi, eax
            mov     edi, [ebp+0xC]
            mov     ecx, [edi]
            shl     esi, 2
            add     ecx, edx
            sub     esi, eax
            mov     [edi], ecx
            shl     esi, 3
            mov     edi, [ebx]
            add     edi, esi
            xor     eax, eax
            mov     [ebx], edi

        loc_1000B7C2:
            pop     edi
            pop     esi
            mov     [ebp-0x4], eax
            mov     eax, [ebp-0x4]
            pop     edi
            pop     esi
            pop     ebx
            mov     esp, ebp
            pop     ebp
            ret
    }
}


//______________________________________
void __declspec(naked) DrawObjects() {
    __asm {
            push    ebp
            mov     ebp, esp
            sub     esp, 1Ch
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__ambient_light]
            lea     ecx, [edi+D__grid_size]
            pop     edi
            push    ebx
            mov     ebx, [eax]
            push    esi
            xor     esi, esi
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     [edi+combatOutlineCount], esi
            pop     edi
            cmp     [ecx], esi
            push    edi
            mov     [ebp-0x10], ebx
            mov     [ebp-0xC], esi
            mov     [ebp-0x8], esi
            jle     loc_1000DE3C
            nop

        loc_1000DD00:
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     eax, [edi+C_light_get_tile_]
            pop     edi
            xor     edi, edi
            cmp     eax, edi
            mov     [ebp-0x4], edi
            jz      loc_1000DD23
            mov     edx, [ebp-0x8]
            mov     eax, [ebp+0xC]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_light_get_tile_]
            call    edi
            pop     edi
            mov     [ebp-0x4], eax
            mov     edi, [ebp-0x4]
            mov     [ebp-0x4], edi

        loc_1000DD23:
            cmp     edi, ebx
            jge     loc_1000DD2C
            mov     edi, ebx
            mov     [ebp-0x4], edi

        loc_1000DD2C:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     edx, [edi+D__objectTable]
            pop     edi
            mov     esi, [edx+esi*4]
            test    esi, esi
            jz      loc_1000DE22
            lea     ecx, [ecx+0]

        loc_1000DD40:
            mov     eax, [esi]
            mov     ecx, [eax+28h]
            mov     edx, [ebp+0xC]
            cmp     ecx, edx
            jg      loc_1000DE22
            jnz     loc_1000DDE3
            mov     ecx, [eax+24h]
            test    cl, 8
            jz      loc_1000DE14
            test    cl, 1
            jnz     loc_1000DDE3
            push    eax
            call    IsVisibleByPC
            add     esp, 4
            test    eax, eax
            jnz     loc_1000DDF0
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogLight]
            pop     edi
            cmp     edi, eax
            jl      loc_1000DDF5
            test    eax, eax
            jz      loc_1000DDF5
            mov     ecx, [esi]
            mov     [ebp-0x18], eax
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     eax, [edi+C_obj_render_object_]
            pop     edi
            test    eax, eax
            mov     [ebp-0x14], ecx
            jz      loc_1000DDA5
            mov     ebx, [ebp-0x18]
            mov     edx, [ebp+0x8]
            mov     eax, [ebp-0x14]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_obj_render_object_]
            call    edi
            pop     edi

        loc_1000DDA5:

            mov     ecx, [esi]
            mov     eax, [ecx+74h]
            test    eax, 0FFFFFFh
            jz      loc_1000DDE3
            test    eax, eax
            js      loc_1000DDE3
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     edx, [edi+FOG_OF_WAR]
            pop     edi
            test    edx, edx
            jz      loc_1000DDCA
            mov     edx, [ecx+50h]
            test    edx, edx
            jz      loc_1000DDCA
            test    al, 20h
            jnz     loc_1000DDE3

        loc_1000DDCA:

            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+combatOutlineCount]
            pop     edi
            cmp     eax, 1F4h
            jge     loc_1000DDE3
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+pCombatOutlineList]
            mov     [edi+eax*4], ecx
            pop     edi
            inc     eax
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     [edi+combatOutlineCount], eax
            pop     edi

        loc_1000DDE3:

            mov     esi, [esi+4]
            test    esi, esi
            jnz     loc_1000DD40
            jmp     loc_1000DE22

        loc_1000DDF0:
            cmp     eax, 1
            jnz     loc_1000DDA5

        loc_1000DDF5:

            mov     eax, [esi]
            mov     [ebp-0x14], eax
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     eax, [edi+C_obj_render_object_]
            pop     edi
            test    eax, eax
            jz      loc_1000DDA5
            mov     ebx, [ebp-0x4]
            mov     edx, [ebp+0x8]
            mov     eax, [ebp-0x14]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_obj_render_object_]
            call    edi
            pop     edi
            jmp     loc_1000DDA5

        loc_1000DE14:
            mov     eax, [ebp-0xC]
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+upperMapObjNodeArray]
            mov     [edi+eax*4], esi
            pop     edi
            inc     eax
            mov     [ebp-0xC], eax

        loc_1000DE22:

            mov     esi, [ebp-0x8]
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     edx, [edi+D__grid_size]
            pop     edi
            mov     eax, [edx]
            mov     ebx, [ebp-0x10]
            inc     esi
            cmp     esi, eax
            mov     [ebp-0x8], esi
            jl      loc_1000DD00

        loc_1000DE3C:
            mov     eax, [ebp+0x8]
            push    0
            push    eax
            // Edw590: call commented out. Not used in FOW and anyway this specific call was throwing errors
            //call    CheckAngledObjEdge
            mov     ecx, [ebp-0xC]
            xor     eax, eax
            add     esp, 8
            test    ecx, ecx
            mov     [ebp-0x14], eax
            jle     loc_1000DF67
            jmp     loc_1000DE60

        loc_1000DE5C:
            mov     ebx, [ebp-0x10]
            nop

        loc_1000DE60:
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+upperMapObjNodeArray]
            mov     esi, [edi+eax*4]
            pop     edi
            xor     edi, edi
            cmp     esi, edi
            jz      loc_1000DE75
            mov     ecx, [esi]
            mov     edx, [ecx+4]
            mov     [ebp-0x8], edx

        loc_1000DE75:
            push    esi
            mov     esi, SN_CODE_SEC_EXE_ADDR
            lea     esi, [esi+C_light_get_tile_]
            cmp     esi, edi
            pop     esi
            mov     [ebp-0x4], edi
            jz      loc_1000DE95
            mov     edx, [ebp-0x8]
            mov     eax, [ebp+0xC]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_light_get_tile_]
            call    edi
            pop     edi
            mov     [ebp-0x4], eax
            mov     edi, [ebp-0x4]
            mov     [ebp-0x4], edi

        loc_1000DE95:
            cmp     edi, ebx
            jge     loc_1000DE9E
            mov     edi, ebx
            mov     [ebp-0x4], edi

        loc_1000DE9E:
            test    esi, esi
            jz      loc_1000DF55

        loc_1000DEA6:
            mov     eax, [esi]
            mov     ecx, [eax+28h]
            mov     edx, [ebp+0xC]
            cmp     ecx, edx
            jg      loc_1000DF55
            jnz     loc_1000DF4A
            test    byte ptr [eax+24h], 1
            jnz     loc_1000DF4A
            push    eax
            call    IsVisibleByPC
            add     esp, 4
            test    eax, eax
            jnz     loc_1000DF7C
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogLight]
            pop     edi
            cmp     edi, eax
            jl      loc_1000DF81
            test    eax, eax
            jz      loc_1000DF81
            mov     ecx, [esi]
            mov     [ebp-0x1C], eax
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     eax, [edi+C_obj_render_object_]
            pop     edi
            test    eax, eax
            mov     [ebp-0x18], ecx
            jz      loc_1000DF0C
            mov     ebx, [ebp-0x1C]
            mov     edx, [ebp+0x8]
            mov     eax, [ebp-0x18]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_obj_render_object_]
            call    edi
            pop     edi

        loc_1000DF0C:

            mov     ecx, [esi]
            mov     eax, [ecx+74h]
            test    eax, 0FFFFFFh
            jz      loc_1000DF4A
            test    eax, eax
            js      loc_1000DF4A
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     edx, [edi+FOG_OF_WAR]
            pop     edi
            test    edx, edx
            jz      loc_1000DF31
            mov     edx, [ecx+50h]
            test    edx, edx
            jz      loc_1000DF31
            test    al, 20h
            jnz     loc_1000DF4A

        loc_1000DF31:

            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+combatOutlineCount]
            pop     edi
            cmp     eax, 1F4h
            jge     loc_1000DF4A
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+pCombatOutlineList]
            mov     [edi+eax*4], ecx
            pop     edi
            inc     eax
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     [edi+combatOutlineCount], eax
            pop     edi

        loc_1000DF4A:

            mov     esi, [esi+4]
            test    esi, esi
            jnz     loc_1000DEA6

        loc_1000DF55:

            mov     eax, [ebp-0x14]
            mov     ecx, [ebp-0xC]
            inc     eax
            cmp     eax, ecx
            mov     [ebp-0x14], eax
            jl      loc_1000DE5C

        loc_1000DF67:
            mov     edx, [ebp+0x8]
            push    1
            push    edx
            // Edw590: call commented out. Not used in FOW.
            //call    CheckAngledObjEdge
            add     esp, 8
            pop     edi
            pop     esi
            pop     ebx
            mov     esp, ebp
            pop     ebp
            ret

        loc_1000DF7C:
            cmp     eax, 1
            jnz     loc_1000DF0C

        loc_1000DF81:

            mov     eax, [esi]
            mov     [ebp-0x18], eax
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     eax, [edi+C_obj_render_object_]
            pop     edi
            test    eax, eax
            jz      loc_1000DF0C
            mov     ebx, [ebp-0x4]
            mov     edx, [ebp+0x8]
            mov     eax, [ebp-0x18]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_obj_render_object_]
            call    edi
            pop     edi
            jmp     loc_1000DF0C
    }
}


//Check if object is visible -for mouse selection.
//______________________________
void __declspec(naked) IsNotFogged() {
    __asm {
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+FOG_OF_WAR]
            pop     edi
            test    eax, eax
            jz      loc_1000C4C3
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__obj_dude]
            pop     edi
            mov     edx, [eax]
            mov     ecx, [esp+0x4]
            xor     eax, eax
            mov     al, [ecx+23h]
            and     eax, 0Fh
            cmp     eax, 1
            jnz     loc_1000C4A1
            mov     eax, [ecx+50h]
            test    eax, eax
            jz      loc_1000C4C3
            mov     eax, [ecx+74h]
            test    eax, 0FFFFFFh
            jz      loc_1000C48A
            test    eax, eax
            js      loc_1000C48A
            test    al, 20h
            jz      loc_1000C4C3

        loc_1000C48A:

            mov     ecx, [ecx+4]
            mov     edx, [edx+4]
            push    ecx
            push    edx
            call    IsInLineOfSightBlocked
            add     esp, 8
            test    al, al
            jz      loc_1000C4C3

        loc_1000C49E:
            xor     al, al
            ret

        loc_1000C4A1:
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     edx, [edi+fogLight]
            pop     edi
            test    edx, edx
            jnz     loc_1000C4BA
            cmp     eax, 3
            jz      loc_1000C4C3
            cmp     eax, 2
            jz      loc_1000C4C3
            cmp     eax, 5
            jz      loc_1000C4C3

        loc_1000C4BA:
            test    dword ptr [ecx+24h], 40000000h
            jz      loc_1000C49E

        loc_1000C4C3:

            mov     al, 1
            ret
    }
}


//find the object who's frm lies under mouse cursor.
//_________________________________________________________________________________________
void __declspec(naked) GetObjectsAtPos() {
    __asm {
            push    ebp
            mov     ebp, esp
            sub     esp, 18h
            mov     edx, [ebp+0x18]
            push    ebx
            xor     ebx, ebx
            push    esi
            mov     [edx], ebx
            push    edi
            push    esi
            mov     esi, SN_DATA_SEC_EXE_ADDR
            lea     edi, [esi+D__grid_size]
            pop     esi
            xor     ecx, ecx
            xor     eax, eax
            xor     esi, esi
            cmp     [edi], ebx
            mov     [ebp-0xC], ecx
            mov     [ebp-1], bl
            mov     [ebp-0x14], eax
            jle     loc_1000C5F7
            lea     ecx, [ecx+0]
        loc_1000C500:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__objectTable]
            pop     edi
            mov     edi, [ecx+eax*4]
            cmp     edi, ebx
            lea     edx, ds:8[esi*8]
            mov     [ebp-8], edx
            jz      loc_1000C5CC
            jmp     loc_1000C520

        loc_1000C520:
            cmp     [ebp-1], bl
            jnz     loc_1000C5CC
            mov     eax, [edi]
            mov     ecx, [eax+28h]
            mov     edx, [ebp+0x10]
            cmp     ecx, edx
            jg      loc_1000C5C0
            jnz     loc_1000C5B9
            mov     edx, [ebp+0x14]
            cmp     edx, 0FFFFFFFFh
            jz      loc_1000C54F
            xor     ecx, ecx
            mov     cl, [eax+23h]
            and     ecx, 0Fh
            cmp     ecx, edx
            jnz     loc_1000C5B9
        loc_1000C54F:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__obj_egg]
            pop     edi
            cmp     eax, [ecx]
            jz      loc_1000C5B9
            push    eax
            call    IsNotFogged
            add     esp, 4
            test    al, al
            jz      loc_1000C5B9
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_obj_intersects_with_]
            cmp     edi, ebx
            pop     edi
            mov     edx, [edi]
            mov     [ebp-0x18], edx
            jz      loc_1000C5B9
            mov     [ebp-0x10], ebx
            mov     ebx, [ebp+0xC]
            mov     edx, [ebp+0x8]
            mov     eax, [ebp-0x18]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_obj_intersects_with_]
            call    edi
            pop     edi
            mov     [ebp-0x10], eax
            mov     ebx, [ebp-0x10]
            test    ebx, ebx
            jz      loc_1000C5B9
            mov     eax, [ebp-0x8]
            mov     ecx, [ebp-0xC]
            push    eax
            push    ecx
            call    FReallocateMemory
            add     esp, 8
            test    eax, eax
            mov     [ebp-0xC], eax
            jz      loc_1000C5B9
            mov     edx, [edi]
            mov     [eax+esi*8+4], edx
            mov     [eax+esi*8], ebx
            mov     eax, [ebp-0x8]
            inc     esi
            add     eax, 8
            mov     [ebp-0x8], eax
        loc_1000C5B9:
            mov     edi, [edi+4]
            xor     ebx, ebx
            jmp     loc_1000C5C4

        loc_1000C5C0:
            mov     [ebp-0x1], 1
        loc_1000C5C4:
            cmp     edi, ebx
            jnz     loc_1000C520
        loc_1000C5CC:
            mov     eax, [ebp-0x14]
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__grid_size]
            pop     edi
            mov     edx, [ecx]
            inc     eax
            cmp     eax, edx
            mov     [ebp-0x1], bl
            mov     [ebp-0x14], eax
            jl      loc_1000C500
            mov     eax, [ebp+0x18]
            mov     edx, [ebp-0xC]
            mov     [eax], edx
            mov     eax, esi
            pop     edi
            pop     esi
            pop     ebx
            mov     esp, ebp
            pop     ebp
            ret

        loc_1000C5F7:
            pop     edi
            mov     eax, esi
            pop     esi
            mov     [edx], ecx
            pop     ebx
            mov     esp, ebp
            pop     ebp
            ret

    }
}

//copy a file to or from save game slot  -returns 0 pass, -1 fail.
//_______________________________________________
void __declspec(naked) F_CopySaveFile() {
    __asm {
            push    ebp
            mov     ebp, esp
            push    ecx
            mov     [ebp-0x4], 0
            mov     edx, [ebp+0xC]
            mov     eax, [ebp+0x8]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_copy_file_]
            call    edi
            pop     edi
            mov     [ebp-0x4], eax
            mov     eax, [ebp-0x4]
            mov     esp, ebp
            pop     ebp
            ret
    }
}


//____________________________________
void __declspec(naked) CheckHexTransparency() {
    __asm {
            mov     eax, [esp+0x4]
            test    eax, eax
            jl      loc_1000B959
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__grid_size]
            pop     edi
            mov     ecx, [ecx]
            cmp     eax, ecx
            jge     loc_1000B959
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     edx, [edi+D__objectTable]
            pop     edi
            push    ebx
            mov     ebx, [edx+eax*4]
            push    esi
            push    edi
            push    esi
            mov     esi, SN_DATA_SEC_BLOCK_ADDR
            mov     edi, [esi+fogHexMapBits]
            pop     esi
            test    edi, edi
            jz      loc_1000B914
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     edx, [edi+D__map_elevation]
            pop     edi
            mov     edx, [edx]
            imul    edx, ecx
            lea     esi, [edx+eax]
            push    esi
            mov     ecx, edi
            call    get
            test    eax, eax
            jnz     loc_1000B914
            push    esi
            mov     ecx, edi
            call    set1

        loc_1000B914:

            test    ebx, ebx
            jz      loc_1000B94D
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__map_elevation]
            pop     edi
            mov     esi, [eax]
            mov     edi, 0A0008000h

        loc_1000B924:
            mov     eax, [ebx]
            mov     ecx, [eax+28h]
            cmp     ecx, esi
            jg      loc_1000B94D
            jnz     loc_1000B946
            xor     ecx, ecx
            mov     cl, [eax+23h]
            and     ecx, 0Fh
            cmp     ecx, 3
            jz      loc_1000B941
            cmp     ecx, 2
            jnz     loc_1000B946

        loc_1000B941:
            test    [eax+24h], edi
            jz      loc_1000B953

        loc_1000B946:

            mov     ebx, [ebx+4]
            test    ebx, ebx
            jnz     loc_1000B924

        loc_1000B94D:

            pop     edi
            pop     esi
            xor     al, al
            pop     ebx
            ret

        loc_1000B953:
            pop     edi
            pop     esi
            mov     al, 1
            pop     ebx
            ret

        loc_1000B959:

            mov     al, 1
            ret
    }
}


//__________________
void __declspec(naked) ReDrawViewWin() {
    __asm {
            push    ebp
            mov     ebp, esp
            sub     esp, 8
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__display_win]
            pop     edi
            mov     eax, [eax]
            cmp     eax, 0FFFFFFFFh
            push    ebx
            jz      loc_1000D376
            push    eax
            call    GetWinStruct
            add     esp, 4
            test    eax, eax
            jz      loc_1000D376
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__refresh_enabled]
            pop     edi
            cmp     dword ptr [ecx], 0
            jz      loc_1000D376
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     edx, [edi+D__map_elevation]
            pop     edi
            mov     ecx, [edx]
            add     eax, 8
            mov     [ebp-0x4], eax
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__tile_refresh]
            pop     edi
            test    eax, eax
            mov     [ebp-0x8], ecx
            jz      loc_1000D376
            push    ebx
            mov     edx, [ebp-0x8]
            mov     eax, [ebp-0x4]
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ebx, [edi+D__tile_refresh]
            pop     edi
            call    dword ptr [ebx]
            pop     ebx

        loc_1000D376:

            pop     ebx
            mov     esp, ebp
            pop     ebp
            ret
    }
}

static const char percent_s_str[] = "%s";
static const char wb_str[] = "wb";
static const char rb_str[] = "rb";
static const char dot_fog_str[] = ".fog";

//________________________________________________________
void __declspec(naked) FogOfWarMap_CopyFiles() {
    __asm {
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+FOG_OF_WAR]
            pop     edi
            sub     esp, 8
            push    ebx
            xor     ebx, ebx
            cmp     eax, ebx
            jnz     loc_1000ABB6
            xor     al, al
            pop     ebx
            add     esp, 8
            ret

        loc_1000ABB6:
            push    esi
            push    edi
            mov     eax, 100h
            call    malloc
            mov     esi, eax
            mov     eax, [esp+14h+0x4]
            push    eax
            mov     eax, SN_DATA_SEC_BLOCK_ADDR
            lea     eax, [eax+percent_s_str]
            push    eax
            push    esi
            mov     eax, SN_CODE_SEC_EXE_ADDR
            lea     eax, [eax+F_sprintf_]
            call    eax
            mov     eax, esi
            mov     edx, 2Eh
            call    strchr
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, dword ptr [edi+dot_fog_str]
            pop     edi
            mov     [eax], ecx
            mov     dl, 0
            mov     [eax+4], dl
            mov     eax, 100h
            call    malloc
            mov     edi, eax
            mov     eax, [esp+20h+0x8]
            push    eax
            mov     eax, SN_DATA_SEC_BLOCK_ADDR
            lea     eax, [eax+percent_s_str]
            push    eax
            push    edi
            mov     eax, SN_CODE_SEC_EXE_ADDR
            lea     eax, [eax+F_sprintf_]
            call    eax
            mov     eax, edi
            mov     edx, 2Eh
            call    strchr
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, dword ptr [edi+dot_fog_str]
            pop     edi
            mov     [eax], ecx
            mov     edx, SN_DATA_SEC_BLOCK_ADDR
            lea     edx, [edx+rb_str]
            push    edx
            push    esi
            mov     byte ptr [eax+4], 0
            call    F_fopen
            push    eax
            mov     eax, esi
            call    free
            pop     eax
            mov     esi, eax
            add     esp, 32
            cmp     esi, ebx
            jz      loc_1000AD59
            mov     eax, SN_DATA_SEC_BLOCK_ADDR
            lea     eax, [eax+rb_str]
            push    eax
            push    edi
            call    F_fopen
            push    eax
            mov     eax, edi
            call    free
            pop     eax
            mov     edi, eax
            add     esp, 8
            cmp     edi, ebx
            jnz     loc_1000AC60
            push    esi
            call    F_fclose
            add     esp, 4
            pop     edi
            pop     esi
            xor     al, al
            pop     ebx
            add     esp, 8
            ret

        loc_1000AC60:
            lea     eax, [esp+14h-0x8]
            push    eax
            push    esi
            mov     [esp+1Ch-0x8], ebx
            mov     [esp+1Ch-0x4], ebx
            call    F_fread32
            mov     eax, [esp+1Ch-0x8]
            add     esp, 8
            cmp     eax, 464F474Fh
            jnz     loc_1000ACD6
            push    464F474Fh
            push    edi
            call    F_fwrite32
            lea     ecx, [esp+1Ch-0x8]
            push    ecx
            push    esi
            call    F_fread32
            mov     eax, [esp+24h-0x8]
            add     esp, 10h
            cmp     eax, 46574152h
            jnz     loc_1000AD4A
            push    46574152h
            push    edi
            call    F_fwrite32
            lea     edx, [esp+1Ch-0x8]
            push    edx
            push    esi
            call    F_fread32
            mov     eax, [esp+24h-0x8]
            add     esp, 10h
            cmp     eax, 1
            jnz     loc_1000AD4A
            push    1
            push    edi
            call    F_fwrite32
            add     esp, 8

        loc_1000ACD6:
            lea     eax, [esp+14h-0x8]
            push    eax
            push    esi
            call    F_fread32
            mov     ecx, [esp+1Ch-0x8]
            push    ecx
            push    edi
            call    F_fwrite32
            lea     edx, [esp+24h-0x4]
            push    edx
            push    esi
            call    F_fread32
            mov     eax, [esp+2Ch-0x4]
            push    eax
            push    edi
            call    F_fwrite32
            mov     eax, [esp+34h-0x4]
            add     esp, 20h
            cmp     eax, ebx
            jbe     loc_1000AD32
            lea     ecx, [ecx+0]

        loc_1000AD10:
            lea     ecx, [esp+14h-0x8]
            push    ecx
            push    esi
            call    F_fread32
            mov     edx, [esp+1Ch-0x8]
            push    edx
            push    edi
            call    F_fwrite32
            mov     eax, [esp+24h-0x4]
            add     esp, 10h
            inc     ebx
            cmp     ebx, eax
            jb      loc_1000AD10

        loc_1000AD32:
            push    esi
            call    F_fclose
            push    edi
            call    F_fclose
            add     esp, 8
            pop     edi
            pop     esi
            mov     al, 1
            pop     ebx
            add     esp, 8
            ret

        loc_1000AD4A:

            push    esi
            call    F_fclose
            push    edi
            call    F_fclose
            add     esp, 8

        loc_1000AD59:
            pop     edi
            pop     esi
            xor     al, al
            pop     ebx
            add     esp, 8
            ret
    }
}

static const char fog_str[] = "fog";

//______________________________________
void __declspec(naked) FogOfWarMap_DeleteTmps() {
    __asm {
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+FOG_OF_WAR]
            pop     edi
            test    eax, eax
            jnz     loc_1000AD7C
            xor     eax, eax
            ret

        loc_1000AD7C:
            mov     eax, [esp+0x4]
            // Edw590: ESI is not used inside FDeleteTmpSaveFiles and in fog_of_war_delete_tmps it's POPed
            mov     esi, SN_DATA_SEC_BLOCK_ADDR
            lea     esi, [esi+fog_str]
            push    esi
            push    eax
            call    FDeleteTmpSaveFiles
            add     esp, 8
            ret
    }
}

//__________________________________
void __declspec(naked) FogOfWarMap_Save() {
    __asm {
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+FOG_OF_WAR]
            pop     edi
            sub     esp, 100h
            test    eax, eax
            jnz     loc_1000ADA8
            xor     al, al
            add     esp, 100h
            ret

        loc_1000ADA8:
            mov     eax, [esp+100h+0x4]
            push    esi
            push    eax
            lea     ecx, [esp+108h-0x100]
            mov     eax, SN_DATA_SEC_BLOCK_ADDR
            lea     eax, [eax+percent_s_str]
            push    eax
            push    ecx
            mov     eax, SN_CODE_SEC_EXE_ADDR
            lea     eax, [eax+F_sprintf_]
            call    eax
            lea     eax, [esp+110h-0x100]
            mov     edx, 2Eh
            call    strchr
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, dword ptr [edi+dot_fog_str]
            pop     edi
            mov     [eax], ecx
            mov     dl, 0
            mov     [eax+4], dl
            lea     eax, [esp+110h-0x100]
            mov     esi, SN_DATA_SEC_BLOCK_ADDR
            lea     esi, [esi+wb_str]
            push    esi
            push    eax
            call    F_fopen
            mov     esi, eax
            add     esp, 20 // Edw590: modified by me because strchr is from F1DP (register-based) and not the EXE
            test    esi, esi
            jnz     loc_1000ADFF
            xor     al, al
            pop     esi
            add     esp, 100h
            ret

        loc_1000ADFF:
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogHexMapBits]
            pop     edi
            test    eax, eax
            jz      loc_1000AE74
            push    edi
            push    464F474Fh
            push    esi
            call    F_fwrite32
            push    46574152h
            push    esi
            call    F_fwrite32
            push    1
            push    esi
            call    F_fwrite32
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, [edi+fogHexMapBits]
            pop     edi
            mov     edx, [ecx]
            push    edx
            push    esi
            call    F_fwrite32
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogHexMapBits]
            pop     edi
            mov     ecx, [eax+8]
            push    ecx
            push    esi
            call    F_fwrite32
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogHexMapBits]
            pop     edi
            mov     ecx, [eax+8]
            add     esp, 28h
            xor     edi, edi
            test    ecx, ecx
            jbe     loc_1000AE73

        loc_1000AE56:
            mov     edx, [eax+4]
            mov     eax, [edx+edi*4]
            push    eax
            push    esi
            call    F_fwrite32
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogHexMapBits]
            pop     edi
            mov     ecx, [eax+8]
            add     esp, 8
            inc     edi
            cmp     edi, ecx
            jb      loc_1000AE56

        loc_1000AE73:
            pop     edi

        loc_1000AE74:
            push    esi
            call    F_fclose
            add     esp, 4
            mov     al, 1
            pop     esi
            add     esp, 100h
            ret
    }
}

static const char maps_percent_s_str[] = "maps\\%s";

//__________________________________
void __declspec(naked) FogOfWarMap_Load() {
    __asm {
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+FOG_OF_WAR]
            pop     edi
            sub     esp, 104h
            push    edi
            xor     edi, edi
            cmp     eax, edi
            jnz     loc_1000D11C
            xor     al, al
            pop     edi
            add     esp, 104h
            ret

        loc_1000D11C:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__grid_width]
            pop     edi
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     byte ptr [edi+isRecordingObjFog], 1
            pop     edi
            mov     ecx, [eax]
            neg     ecx
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+hexStep2]
            mov     [edi+4], ecx
            mov     edx, [eax]
            mov     [edi+12], edx
            mov     ecx, [eax]
            mov     [edi+24], ecx
            mov     edx, [eax]
            mov     [edi+28], edx
            mov     ecx, [eax]
            mov     [edi+36], ecx
            mov     edx, [eax]
            neg     edx
            mov     [edi+52], edx
            mov     ecx, [eax]
            neg     ecx
            mov     [edi+60], ecx
            mov     edx, [eax]
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogHexMapBits]
            pop     edi
            neg     edx
            cmp     eax, edi
            push    esi
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            lea     edi, [edi+hexStep2]
            mov     [edi+64], edx
            pop     edi
            jz      loc_1000D19A
            lea     esi, [eax+4]
            mov     eax, [esi]
            call    free
            mov     [esi], edi
            push    esi
            mov     esi, SN_DATA_SEC_BLOCK_ADDR
            mov     [esi+fogHexMapBits], edi
            pop     esi

        loc_1000D19A:
            mov     ecx, [esp+10Ch+0x4]
            push    ecx
            lea     edx, [esp+110h-0x100]
            mov     eax, SN_DATA_SEC_BLOCK_ADDR
            lea     eax, [eax+maps_percent_s_str]
            push    eax
            push    edx
            mov     eax, SN_CODE_SEC_EXE_ADDR
            lea     eax, [eax+F_sprintf_]
            call    eax
            lea     eax, [esp+118h-0x100]
            mov     edx, 2Eh
            call    strchr
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, dword ptr [edi+dot_fog_str]
            pop     edi
            mov     [eax], ecx
            mov     dl, 0
            mov     [eax+4], dl
            lea     eax, [esp+118h-0x100]
            mov     esi, SN_DATA_SEC_BLOCK_ADDR
            lea     esi, [esi+rb_str]
            push    esi
            push    eax
            call    F_fopen
            mov     esi, eax
            add     esp, 20
            cmp     esi, edi
            jnz     loc_1000D229
            mov     eax, 0Ch
            call    malloc
            cmp     eax, edi
            jz      loc_1000D217
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__grid_size]
            pop     edi
            mov     edx, [ecx]
            lea     edx, [edx+edx*2]
            push    edx
            mov     ecx, eax
            call    bitset
            pop     esi
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     [edi+fogHexMapBits], eax
            pop     edi
            xor     al, al
            pop     edi
            add     esp, 104h
            ret

        loc_1000D217:
            xor     eax, eax
            pop     esi
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     [edi+fogHexMapBits], eax
            pop     edi
            xor     al, al
            pop     edi
            add     esp, 104h
            ret

        loc_1000D229:
            push    esi
            mov     esi, SN_DATA_SEC_BLOCK_ADDR
            cmp     [esi+fogHexMapBits], edi
            pop     esi
            mov     [esp+10Ch-0x104], edi
            jnz     loc_1000D2FD
            lea     eax, [esp+10Ch-0x104]
            push    eax
            push    esi
            call    F_fread32
            mov     eax, [esp+114h-0x104]
            add     esp, 8
            cmp     eax, 464F474Fh
            jnz     loc_1000D296
            lea     ecx, [esp+10Ch-0x104]
            push    ecx
            push    esi
            call    F_fread32
            mov     eax, [esp+114h-0x104]
            add     esp, 8
            cmp     eax, 46574152h
            jz      loc_1000D27F

        loc_1000D26B:
            push    esi
            call    F_fclose
            add     esp, 4
            pop     esi
            xor     al, al
            pop     edi
            add     esp, 104h
            ret

        loc_1000D27F:
            lea     edx, [esp+10Ch-0x104]
            push    edx
            push    esi
            call    F_fread32
            mov     eax, [esp+114h-0x104]
            add     esp, 8
            cmp     eax, 1
            jnz     loc_1000D26B

        loc_1000D296:
            lea     eax, [esp+10Ch-0x104]
            push    eax
            push    esi
            call    F_fread32
            add     esp, 8
            mov     eax, 0Ch
            call    malloc
            cmp     eax, edi
            jz      loc_1000D2BD
            mov     ecx, [esp+10Ch-0x104]
            push    ecx
            mov     ecx, eax
            call    bitset
            jmp     loc_1000D2BF

        loc_1000D2BD:
            xor     eax, eax

        loc_1000D2BF:
            lea     edx, [esp+10Ch-0x104]
            push    edx
            push    esi
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     [edi+fogHexMapBits], eax
            pop     edi
            call    F_fread32
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogHexMapBits]
            pop     edi
            mov     ecx, [eax+8]
            add     esp, 8
            cmp     ecx, edi
            jbe     loc_1000D2FD
            mov     edi, edi

        loc_1000D2E0:
            mov     eax, [eax+4]
            lea     ecx, [eax+edi*4]
            push    ecx
            push    esi
            call    F_fread32
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogHexMapBits]
            pop     edi
            mov     ecx, [eax+8]
            add     esp, 8
            inc     edi
            cmp     edi, ecx
            jb      loc_1000D2E0

        loc_1000D2FD:

            push    esi
            call    F_fclose
            add     esp, 4
            pop     esi
            mov     al, 1
            pop     edi
            add     esp, 104h
            ret
    }
}

//_______________________________
void __declspec(naked) SetMapBorders() {
    __asm {
            // Edw590: function reduced by me as the rest is not used in FOW

            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     [edi+isRecordingObjFog], 0
            pop     edi
            ret
    }
}

//_____________________________________________________
void __declspec(naked) IsInLineOfSightBlocked() {
    __asm {
            sub     esp, 14h
            push    ebx
            mov     ebx, [esp+18h+0x4]
            push    ebp
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ebp, [edi+D__grid_size]
            pop     edi
            push    esi
            mov     esi, [esp+20h+0x8]
            xor     ecx, ecx
            cmp     esi, ecx
            push    edi
            push    esi
            mov     esi, SN_DATA_SEC_EXE_ADDR
            lea     edi, [esi+D__grid_width]
            pop     esi
            mov     [esp+24h-0x8], ecx
            mov     [esp+24h-0xC], ecx
            jl      loc_1000B9DF
            cmp     esi, [ebp+0]
            jge     loc_1000B9DF
            mov     ecx, [edi]
            mov     eax, esi
            cdq
            idiv    ecx
            xor     ecx, ecx
            mov     [esp+24h-0xC], eax
            mov     [esp+24h-0x8], edx

        loc_1000B9DF:

            cmp     ebx, ecx
            mov     [esp+24h-0x4], ecx
            mov     [esp+24h-0x10], ecx
            jl      loc_1000BA01
            cmp     ebx, [ebp+0]
            jge     loc_1000BA01
            mov     ecx, [edi]
            mov     eax, ebx
            cdq
            idiv    ecx
            mov     ecx, edx
            mov     [esp+24h-0x10], eax
            mov     [esp+24h-0x4], ecx

        loc_1000BA01:

            mov     edi, [edi]
            mov     eax, esi
            cdq
            idiv    edi
            mov     [esp+24h-0x14], edi
            mov     esi, edx
            sar     esi, 1
            add     esi, eax
            lea     ebp, [edx+edx]
            mov     eax, ebx
            cdq
            idiv    edi
            sub     ebp, esi
            mov     edi, edx
            sar     edi, 1
            add     edi, eax
            lea     eax, [edx+edx]
            mov     edx, [esp+24h-0x8]
            sub     eax, edi
            cmp     ecx, edx
            mov     [esp+24h+0x4], eax
            jnz     loc_1000BA72
            cmp     ebx, [esp+24h+0x8]
            jz      loc_1000C1C2

        loc_1000BA3D:
            mov     ecx, [esp+24h-0x10]
            cmp     ecx, [esp+24h-0xC]
            mov     eax, [esp+24h-0x14]
            jge     loc_1000BA4F
            add     ebx, eax
            jmp     loc_1000BA51

        loc_1000BA4F:
            sub     ebx, eax

        loc_1000BA51:
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BA3D
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BA72:
            mov     edx, [esp+24h-0xC]
            cmp     [esp+24h-0x10], edx
            jnz     loc_1000BAB5
            cmp     ebx, [esp+24h+0x8]
            jz      loc_1000C1C2

       loc_1000BA86:
            mov     eax, [esp+24h-0x4]
            cmp     eax, [esp+24h-0x8]
            jge     loc_1000BA93
            inc     ebx
            jmp     loc_1000BA94

        loc_1000BA93:
            dec     ebx

        loc_1000BA94:
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BA86
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BAB5:
            cmp     edi, esi
            jnz     loc_1000BB11
            cmp     ebx, [esp+24h+0x8]
            jz      loc_1000C1C2
            jmp     loc_1000BAC9

        loc_1000BAC5:
            mov     eax, [esp+24h+0x4]

        loc_1000BAC9:
            cmp     eax, ebp
            mov     ecx, [esp+24h-0x14]
            jle     loc_1000BADF
            test    bl, 1
            jz      loc_1000BAD9
            dec     ebx
            jmp     loc_1000BAF0

        loc_1000BAD9:
            lea     ebx, [ebx+ecx-1]
            jmp     loc_1000BAF0

        loc_1000BADF:
            test    bl, 1
            jz      loc_1000BAEF
            mov     edx, 1
            sub     edx, ecx
            add     ebx, edx
            jmp     loc_1000BAF0

        loc_1000BAEF:
            inc     ebx

        loc_1000BAF0:

            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BAC5
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BB11:
            cmp     eax, ebp
            jnz     loc_1000BC02
            cmp     ebx, [esp+24h+0x8]
            jz      loc_1000C1C2

        loc_1000BB23:
            cmp     edi, esi
            jge     loc_1000BB88
            test    bl, 1
            mov     ebp, [esp+24h-0x14]
            jz      loc_1000BB5D
            lea     eax, [ebx+ebp]
            push    eax
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jz      loc_1000BB54
            lea     ecx, [ebx+1]
            push    ecx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC

        loc_1000BB54:
            lea     ebx, [ebx+ebp+1]
            jmp     loc_1000BBDD

        loc_1000BB5D:
            lea     edx, [ebx+ebp]
            push    edx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jz      loc_1000BB82
            lea     eax, [ebx+ebp+1]
            push    eax
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC

        loc_1000BB82:
            lea     ebx, [ebx+ebp*2+1]
            jmp     loc_1000BBDD

        loc_1000BB88:
            mov     ecx, [esp+24h-0x14]
            mov     ebp, ebx
            sub     ebp, ecx
            push    ebp
            call    CheckHexTransparency
            add     esp, 4
            test    bl, 1
            jz      loc_1000BBC4
            test    al, al
            jz      loc_1000BBB4
            dec     ebp
            push    ebp
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC

        loc_1000BBB4:
            mov     ecx, [esp+24h-0x14]
            or      eax, 0FFFFFFFFh
            lea     edx, [ecx+ecx]
            sub     eax, edx
            add     ebx, eax
            jmp     loc_1000BBDD

        loc_1000BBC4:
            test    al, al
            jz      loc_1000BBDA
            dec     ebx
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC

        loc_1000BBDA:
            lea     ebx, [ebp-1]

        loc_1000BBDD:

            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BB23
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BC02:
            mov     ecx, eax
            sub     ecx, edi
            sub     ecx, ebp
            add     ecx, esi
            jnz     loc_1000BC62
            cmp     ebx, [esp+24h+0x8]
            jz      loc_1000C1C2
            jmp     loc_1000BC1C

        loc_1000BC18:
            mov     eax, [esp+24h+0x4]

        loc_1000BC1C:
            cmp     eax, ebp
            mov     ecx, [esp+24h-0x14]
            jge     loc_1000BC32
            test    bl, 1
            jz      loc_1000BC2C
            inc     ebx
            jmp     loc_1000BC41

        loc_1000BC2C:
            lea     ebx, [ebx+ecx+1]
            jmp     loc_1000BC41

        loc_1000BC32:
            test    bl, 1
            jz      loc_1000BC40
            or      edx, 0FFFFFFFFh
            sub     edx, ecx
            add     ebx, edx
            jmp     loc_1000BC41

        loc_1000BC40:
            dec     ebx

        loc_1000BC41:

            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BC18
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BC62:
            mov     eax, esi
            sub     eax, edi
            cdq
            mov     ecx, eax
            xor     ecx, edx
            sub     ecx, edx
            mov     edx, [esp+24h+0x4]
            mov     eax, ebp
            sub     eax, edx
            cdq
            xor     eax, edx
            sub     eax, edx
            cmp     ecx, eax
            jg      loc_1000BF3A
            mov     edx, ecx
            imul    eax, ecx
            imul    edx, ecx
            cmp     edi, esi
            mov     ecx, [esp+24h+0x4]
            mov     [esp+24h-0x10], edx
            mov     [esp+24h-0xC], eax
            jge     loc_1000BDEE
            cmp     ecx, ebp
            jge     loc_1000BD50
            mov     ecx, [esp+24h+0x8]
            mov     esi, eax
            sub     esi, edx
            cmp     ebx, ecx
            mov     byte ptr [esp+24h+0x4], 1
            jz      loc_1000C1C2
            mov     edi, [esp+24h-0x14]
            nop

        loc_1000BCC0:
            cmp     esi, eax
            jg      loc_1000BCF6
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            mov     al, byte ptr [esp+24h+0x4]
            test    al, al
            jz      loc_1000BD29
            test    bl, 1
            jz      loc_1000BD21
            mov     edx, [esp+24h-0x14]
            mov     eax, 1
            sub     eax, edx
            add     ebx, eax
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BD2E

        loc_1000BCF6:
            mov     cl, byte ptr [esp+24h+0x4]
            sub     esi, eax
            test    cl, cl
            jz      loc_1000BD18
            test    bl, 1
            jz      loc_1000BD0D
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BD3C

        loc_1000BD0D:
            lea     ebx, [ebx+edi+1]
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BD3C

        loc_1000BD18:
            add     ebx, edi
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BD3C

        loc_1000BD21:
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BD2E

        loc_1000BD29:
            mov     byte ptr [esp+24h+0x4], 1

        loc_1000BD2E:

            mov     eax, [esp+24h-0x10]
            mov     edi, [esp+24h-0x14]
            add     esi, eax
            mov     eax, [esp+24h-0xC]

        loc_1000BD3C:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BCC0
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BD50:
            jle     loc_1000BDEC
            mov     esi, eax
            mov     eax, [esp+24h+0x8]
            sub     esi, edx
            cmp     ebx, eax
            mov     byte ptr [esp+24h+0x4], 0
            jz      loc_1000C1C2
            mov     ebp, [esp+24h-0xC]
            mov     edi, [esp+24h-0x14]

        loc_1000BD73:
            cmp     esi, ebp
            jg      loc_1000BD9D
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            mov     al, byte ptr [esp+24h+0x4]
            test    al, al
            jnz     loc_1000BDD3
            test    bl, 1
            jz      loc_1000BDC8
            dec     ebx
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BDD8

        loc_1000BD9D:
            mov     al, byte ptr [esp+24h+0x4]
            sub     esi, ebp
            test    al, al
            jnz     loc_1000BDB0
            add     ebx, edi
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BDDC

        loc_1000BDB0:
            test    bl, 1
            jz      loc_1000BDBD
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BDDC

        loc_1000BDBD:
            lea     ebx, [ebx+edi+1]
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BDDC

        loc_1000BDC8:
            lea     ebx, [ebx+edi-1]
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BDD8

        loc_1000BDD3:
            mov     byte ptr [esp+24h+0x4], 0

        loc_1000BDD8:

            add     esi, [esp+24h-0x10]

        loc_1000BDDC:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BD73
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BDEC:
            cmp     edi, esi

        loc_1000BDEE:
            jle     loc_1000C1CC
            cmp     ecx, ebp
            jge     loc_1000BE9D
            mov     esi, eax
            mov     eax, [esp+24h+0x8]
            sub     esi, edx
            cmp     ebx, eax
            mov     byte ptr [esp+24h+0x4], 1
            jz      loc_1000C1C2
            mov     ebp, [esp+24h-0xC]
            mov     edi, [esp+24h-0x14]
            lea     esp, [esp+0]

        loc_1000BE20:
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     esi, ebp
            mov     al, byte ptr [esp+24h+0x4]
            jle     loc_1000BE63
            sub     esi, ebp
            test    al, al
            jz      loc_1000BE48
            sub     ebx, edi
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BE8D

        loc_1000BE48:
            test    bl, 1
            jz      loc_1000BE5B
            or      ecx, 0FFFFFFFFh
            sub     ecx, edi
            add     ebx, ecx
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BE8D

        loc_1000BE5B:
            dec     ebx
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BE8D

        loc_1000BE63:
            test    al, al
            jz      loc_1000BE84
            test    bl, 1
            jz      loc_1000BE7C
            mov     edx, 1
            sub     edx, edi
            add     ebx, edx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BE89

        loc_1000BE7C:
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BE89

        loc_1000BE84:
            mov     byte ptr [esp+24h+0x4], 1

        loc_1000BE89:

            add     esi, [esp+24h-0x10]

        loc_1000BE8D:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BE20
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BE9D:
            jle     loc_1000C1CC
            mov     esi, eax
            mov     eax, [esp+24h+0x8]
            sub     esi, edx
            cmp     ebx, eax
            mov     byte ptr [esp+24h+0x4], 0
            jz      loc_1000C1C2
            mov     ebp, [esp+24h-0xC]
            mov     edi, [esp+24h-0x14]

        loc_1000BEC0:
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     esi, ebp
            mov     al, byte ptr [esp+24h+0x4]
            jle     loc_1000BF05
            sub     esi, ebp
            test    al, al
            jnz     loc_1000BEFC
            test    bl, 1
            jz      loc_1000BEF4
            mov     eax, 0FFFFFFFFh
            sub     eax, edi
            add     ebx, eax
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BF2A

        loc_1000BEF4:
            dec     ebx
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BF2A

        loc_1000BEFC:
            sub     ebx, edi
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BF2A

        loc_1000BF05:
            test    al, al
            jnz     loc_1000BF21
            test    bl, 1
            jz      loc_1000BF16
            dec     ebx
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BF26

        loc_1000BF16:
            lea     ebx, [ebx+edi-1]
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BF26

        loc_1000BF21:
            mov     byte ptr [esp+24h+0x4], 0

        loc_1000BF26:

            add     esi, [esp+24h-0x10]

        loc_1000BF2A:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BEC0
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BF3A:
            mov     edx, eax
            imul    edx, ecx
            mov     ecx, eax
            imul    ecx, eax
            cmp     edi, esi
            mov     eax, [esp+24h+0x4]
            mov     [esp+24h-0x10], edx
            mov     [esp+24h-0xC], ecx
            jge     loc_1000C097
            cmp     eax, ebp
            jge     loc_1000BFFA
            mov     eax, [esp+24h+0x8]
            mov     esi, edx
            sub     esi, ecx
            cmp     ebx, eax
            mov     byte ptr [esp+24h+0x4], 1
            jz      loc_1000C1C2
            mov     edi, [esp+24h-0x14]
            mov     ebp, edx
            jmp     loc_1000BF80

        loc_1000BF80:

            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     esi, ebp
            mov     al, byte ptr [esp+24h+0x4]
            jle     loc_1000BFC3
            sub     esi, ebp
            test    al, al
            jz      loc_1000BFBC
            test    bl, 1
            jz      loc_1000BFB4
            mov     ecx, 1
            sub     ecx, edi
            add     ebx, ecx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BFEA

        loc_1000BFB4:
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BFEA

        loc_1000BFBC:
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000BFEA

        loc_1000BFC3:
            test    al, al
            jz      loc_1000BFDF
            test    bl, 1
            jz      loc_1000BFD4
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BFE6

        loc_1000BFD4:
            lea     ebx, [ebx+edi+1]
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000BFE6

        loc_1000BFDF:
            add     ebx, edi
            mov     byte ptr [esp+24h+0x4], 1

        loc_1000BFE6:

            add     esi, [esp+24h-0xC]

        loc_1000BFEA:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000BF80
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000BFFA:
            jle     loc_1000C095
            mov     eax, [esp+24h+0x8]
            mov     esi, edx
            sub     esi, ecx
            cmp     ebx, eax
            mov     byte ptr [esp+24h+0x4], 0
            jz      loc_1000C1C2
            mov     ebp, [esp+24h-0x10]
            mov     edi, [esp+24h-0x14]
            lea     ecx, [ecx+0]

        loc_1000C020:
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     esi, ebp
            mov     al, byte ptr [esp+24h+0x4]
            jle     loc_1000C05E
            sub     esi, ebp
            test    al, al
            jnz     loc_1000C057
            test    bl, 1
            jz      loc_1000C04C
            dec     ebx
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000C085

        loc_1000C04C:
            lea     ebx, [ebx+edi-1]
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000C085

        loc_1000C057:
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C085

        loc_1000C05E:
            test    al, al
            jz      loc_1000C07A
            test    bl, 1
            jz      loc_1000C06F
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C081

        loc_1000C06F:
            lea     ebx, [ebx+edi+1]
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C081

        loc_1000C07A:
            add     ebx, edi
            mov     byte ptr [esp+24h+0x4], 1

        loc_1000C081:

            add     esi, [esp+24h-0xC]

        loc_1000C085:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000C020
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000C095:
            cmp     edi, esi

        loc_1000C097:
            jle     loc_1000C1CC
            cmp     eax, ebp
            jle     loc_1000C135
            mov     eax, [esp+24h+0x8]
            mov     esi, edx
            sub     esi, ecx
            cmp     ebx, eax
            mov     byte ptr [esp+24h+0x4], 0
            jz      loc_1000C1C2
            mov     ebp, [esp+24h-0x10]
            mov     edi, [esp+24h-0x14]

        loc_1000C0C2:
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     esi, ebp
            mov     al, byte ptr [esp+24h+0x4]
            jle     loc_1000C100
            sub     esi, ebp
            test    al, al
            jnz     loc_1000C0F9
            test    bl, 1
            jz      loc_1000C0EE
            dec     ebx
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000C125

        loc_1000C0EE:
            lea     ebx, [ebx+edi-1]
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000C125

        loc_1000C0F9:
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C125

        loc_1000C100:
            test    al, al
            jz      loc_1000C10D
            sub     ebx, edi
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C121

        loc_1000C10D:
            test    bl, 1
            jz      loc_1000C11B
            or      edx, 0FFFFFFFFh
            sub     edx, edi
            add     ebx, edx
            jmp     loc_1000C11C

        loc_1000C11B:
            dec     ebx

        loc_1000C11C:
            mov     byte ptr [esp+24h+0x4], 1

        loc_1000C121:
            add     esi, [esp+24h-0xC]

        loc_1000C125:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000C0C2
            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000C135:
            jge     loc_1000C1CC
            mov     eax, [esp+24h+0x8]
            mov     esi, edx
            sub     esi, ecx
            cmp     ebx, eax
            mov     byte ptr [esp+24h+0x4], 1
            jz      loc_1000C1C2
            mov     ebp, [esp+24h-0x10]
            mov     edi, [esp+24h-0x14]

        loc_1000C158:
            push    ebx
            call    CheckHexTransparency
            add     esp, 4
            test    al, al
            jnz     loc_1000C1CC
            cmp     esi, ebp
            mov     al, byte ptr [esp+24h+0x4]
            jle     loc_1000C197
            sub     esi, ebp
            test    al, al
            jz      loc_1000C190
            test    bl, 1
            jz      loc_1000C188
            mov     eax, 1
            sub     eax, edi
            add     ebx, eax
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C1BC

        loc_1000C188:
            inc     ebx
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C1BC

        loc_1000C190:
            mov     byte ptr [esp+24h+0x4], 1
            jmp     loc_1000C1BC

        loc_1000C197:
            test    al, al
            jz      loc_1000C1A4
            sub     ebx, edi
            mov     byte ptr [esp+24h+0x4], 0
            jmp     loc_1000C1B8

        loc_1000C1A4:
            test    bl, 1
            jz      loc_1000C1B2
            or      ecx, 0FFFFFFFFh
            sub     ecx, edi
            add     ebx, ecx
            jmp     loc_1000C1B3

        loc_1000C1B2:
            dec     ebx

        loc_1000C1B3:
            mov     byte ptr [esp+24h+0x4], 1

        loc_1000C1B8:
            add     esi, [esp+24h-0xC]

        loc_1000C1BC:

            cmp     ebx, [esp+24h+0x8]
            jnz     loc_1000C158

        loc_1000C1C2:

            pop     edi
            pop     esi
            pop     ebp
            xor     al, al
            pop     ebx
            add     esp, 14h
            ret

        loc_1000C1CC:

            pop     edi
            pop     esi
            pop     ebp
            mov     al, 1
            pop     ebx
            add     esp, 14h
            ret
    }
}

//_____________________________________________________________
void __declspec(naked) GetFloorHexLight() {
    __asm {
            mov     ecx, [esp+0x4]
            test    ecx, ecx
            jl      loc_1000D689
            cmp     ecx, 3
            jge     loc_1000D689
            push    edi
            mov     edi, [esp+4+0x8]
            test    edi, edi
            jl      loc_1000D685
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__grid_size]
            pop     edi
            mov     eax, [eax]
            cmp     edi, eax
            jge     loc_1000D685
            imul    eax, ecx
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__tile_intensity]
            pop     edi
            add     eax, edi
            push    esi
            mov     esi, [ecx+eax*4]
            mov     ecx, [esp+8+0xC]
            cmp     esi, ecx
            jge     loc_1000D61A
            mov     esi, ecx

        loc_1000D61A:
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, [edi+FOG_OF_WAR]
            pop     edi
            test    ecx, ecx
            push    ebx
            jz      loc_1000D66E
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, [edi+fogLight]
            pop     edi
            cmp     esi, ecx
            jle     loc_1000D66E
            test    ecx, ecx
            jz      loc_1000D66E
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     edx, [edi+D__obj_dude]
            pop     edi
            mov     ecx, [edx]
            mov     ebx, [ecx+4]
            cmp     ebx, 0FFFFFFFFh
            jz      loc_1000D66E
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     ecx, [edi+fogHexMapBits]
            pop     edi
            test    ecx, ecx
            jz      loc_1000D66E
            push    eax
            call    get
            test    eax, eax
            jnz     loc_1000D66E
            push    edi
            push    ebx
            call    IsInLineOfSightBlocked
            add     esp, 8
            test    al, al
            jz      loc_1000D66E
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+fogLight]
            pop     edi
            pop     ebx
            pop     esi
            pop     edi
            ret

        loc_1000D66E:

            cmp     esi, 10000h
            jle     loc_1000D67F
            pop     ebx
            pop     esi
            mov     eax, 10000h
            pop     edi
            ret

        loc_1000D67F:
            pop     ebx
            mov     eax, esi
            pop     esi
            pop     edi
            ret

        loc_1000D685:

            xor     eax, eax
            pop     edi
            ret

        loc_1000D689:

            xor     eax, eax
            ret
    }
}


//_______________________________________________________
void __declspec(naked) CheckAngledTileEdge() {
    __asm {
            // Edw590: function reduced by me as the rest is not used in FOW

            mov     eax, [esp+0xC]
            push    0
            push    0
            push    0
            push    eax
            push    4
            call    F_GetFrmID
            add     esp, 14h
            ret
    }
}


//_________________________________________
void __declspec(naked) DrawFloorTiles() {
    __asm {
            push    ebp
            mov     ebp, esp
            sub     esp, 3Ch
            push    ebx
            push    esi
            mov     esi, [ebp+0x8]
            mov     edx, [esi+4]
            push    edi
            mov     edi, [ebp+0xC]
            lea     eax, [ebp-0x38]
            push    eax
            mov     eax, [esi]
            lea     ecx, [ebp-0x4]
            push    ecx
            push    edi
            push    edx
            xor     ebx, ebx
            push    eax
            mov     [ebp-0x4], ebx
            call    GetTileXY
            mov     eax, [esi+4]
            lea     ecx, [ebp-0x4]
            push    ecx
            mov     ecx, [esi+8]
            lea     edx, [ebp-0x34]
            push    edx
            push    edi
            push    eax
            push    ecx
            call    GetTileXY
            mov     ecx, [esi+0Ch]
            lea     edx, [ebp-0x4]
            push    edx
            mov     edx, [esi]
            lea     eax, [ebp-0x3C]
            push    eax
            push    edi
            push    ecx
            push    edx
            call    GetTileXY
            mov     edx, [esi+0Ch]
            lea     eax, [ebp-0x30]
            push    eax
            mov     eax, [esi+8]
            lea     ecx, [ebp-0x4]
            push    ecx
            push    edi
            push    edx
            push    eax
            call    GetTileXY
            mov     edx, [ebp-0x34]
            add     esp, 50h
            cmp     edx, ebx
            jge     loc_1000D708
            xor     edx, edx
            jmp     loc_1000D717

        loc_1000D708:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__square_width]
            pop     edi
            mov     eax, [ecx]
            cmp     edx, eax
            jl      loc_1000D71A
            lea     edx, [eax-1]

        loc_1000D717:
            mov     [ebp-0x34], edx

        loc_1000D71A:
            mov     eax, [ebp-0x38]
            cmp     eax, ebx
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     esi, [edi+D__square_length]
            pop     edi
            jge     loc_1000D72B
            xor     eax, eax
            jmp     loc_1000D734

        loc_1000D72B:
            mov     ecx, [esi]
            cmp     eax, ecx
            jl      loc_1000D737
            lea     eax, [ecx-1]

        loc_1000D734:
            mov     [ebp-0x38], eax

        loc_1000D737:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__square_width]
            pop     edi
            mov     ecx, [ecx]
            mov     edi, [ebp-0x3C]
            cmp     edi, ecx
            jle     loc_1000D74B
            mov     [ebp-0x3C], ecx
            mov     edi, ecx

        loc_1000D74B:
            mov     ecx, [esi]
            cmp     [ebp-0x30], ecx
            jle     loc_1000D755
            mov     [ebp-0x30], ecx

        loc_1000D755:
            mov     esi, [ebp-0x30]
            cmp     esi, eax
            jl      loc_1000D864
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__squares]
            pop     edi
            mov     ecx, [ecx]
            mov     [ebp-0x20], ecx
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__square_width]
            pop     edi
            mov     [ebp-0xC], ebx
            mov     [ebp-0x8], ebx
            mov     ecx, [ecx]
            imul    ecx, eax
            cmp     eax, esi
            mov     [ebp-0x1C], ecx
            mov     [ebp-0x14], eax
            jg      loc_1000D864
            lea     ebx, [ebx+0]

        loc_1000D790:
            cmp     edx, edi
            jg      loc_1000D849
            lea     edi, [ecx+edx]
            mov     ebx, edx
            lea     eax, ds:0[edi*4]
            mov     [ebp-0x18], ebx
            mov     [ebp-0x10], eax
            lea     ebx, [ebx+0]

        loc_1000D7B0:
            test    edi, edi
            jl      loc_1000D828
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     ecx, [edi+D__square_size]
            pop     edi
            cmp     edi, [ecx]
            jge     loc_1000D828
            mov     eax, [ebp+0xC]
            mov     ecx, [ebp-0x20]
            mov     ecx, [ecx+eax*4]
            mov     esi, [ebp-0x10]
            mov     esi, [esi+ecx]
            test    esi, 1000h
            jnz     loc_1000D828
            push    eax
            lea     edx, [ebp-0x8]
            push    edx
            lea     eax, [ebp-0xC]
            push    eax
            push    edi
            call    GetScrnXYTile
            mov     ecx, [ebp-0x14]
            and     esi, 0FFFh
            push    esi
            push    ecx
            push    ebx
            call    CheckAngledTileEdge
            mov     edx, [ebp-0x8]
            mov     [ebp-0x24], eax
            mov     eax, [ebp-0xC]
            mov     [ebp-0x28], eax
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     eax, [edi+C_floor_draw_]
            pop     edi
            add     esp, 1Ch
            test    eax, eax
            mov     [ebp-0x2C], edx
            jz      loc_1000D825
            mov     ecx, [ebp+0x8]
            mov     ebx, [ebp-0x2C]
            mov     edx, [ebp-0x28]
            mov     eax, [ebp-0x24]
            push    edi
            mov     edi, SN_CODE_SEC_EXE_ADDR
            lea     edi, [edi+C_floor_draw_]
            call    edi
            pop     edi
            mov     ebx, [ebp-0x18]

        loc_1000D825:
            mov     edx, [ebp-0x34]

        loc_1000D828:

            mov     esi, [ebp-0x10]
            mov     eax, [ebp-0x3C]
            inc     ebx
            add     esi, 4
            inc     edi
            cmp     ebx, eax
            mov     [ebp-0x18], ebx
            mov     [ebp-0x10], esi
            jle     loc_1000D7B0
            mov     ecx, [ebp-0x1C]
            mov     esi, [ebp-0x30]
            mov     edi, eax

        loc_1000D849:
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__square_width]
            pop     edi
            mov     ebx, [eax]
            mov     eax, [ebp-0x14]
            add     ecx, ebx
            inc     eax
            cmp     eax, esi
            mov     [ebp-0x1C], ecx
            mov     [ebp-0x14], eax
            jle     loc_1000D790

        loc_1000D864:

            pop     edi
            pop     esi
            pop     ebx
            mov     esp, ebp
            pop     ebp
            ret
    }
}


//__________________
void SetMapGlobals(void) {
    int temp_int = 0;
    char prop_value[MAX_PROP_VALUE_LEN];
    memset(prop_value, 0, MAX_PROP_VALUE_LEN);

    getPropValueIni(MAIN_INI_SPEC_SEC_HIGHRES_PATCH, "MAPS", "FOG_OF_WAR", "0", prop_value, &high_res_patch_ini_info_G);
    sscanf(prop_value, "%d", &temp_int);
    GET_BD_SYM(int, FOG_OF_WAR) = temp_int != 0;
}
