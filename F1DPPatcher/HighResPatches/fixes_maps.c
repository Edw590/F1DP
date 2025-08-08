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

#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "F_Mapper.h"
#include "F_File.h"
#include "fixes_maps.h"

static int32_t pcHexNum = 0;

extern int32_t hexStep[6][3];
extern int32_t hexStep2[6][3];

//___________________________________________
void __declspec(naked) get_next_hex_pos(void) {

    __asm {
            push    ecx
            push    esi
            push    edi
            push    ebp

            push    ebx
            push    edx
            push    eax
            call    GetNextHexPos
            add     esp, 0xC

            pop     ebp
            pop     edi
            pop     esi
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


//_______________________________________
void __declspec(naked) get_hex_dist(void) {

    __asm {
            push    ebx
            push    ecx
            push    esi
            push    edi
            push    ebp

            push    edx
            push    eax
            call    GetHexDistance
            add     esp, 0x8


            pop     ebp
            pop     edi
            pop     esi
            pop     ecx
            pop     ebx
            ret
    }
}


//________________________________________________
void __declspec(naked) fog_of_war_copy_files(void) {
    __asm {
            push    ebx
            push    ecx
            push    esi
            push    edi
            push    ebp

            push    edx//pFromPath
            push    eax//pToPath
            call    F_CopySaveFile
            cmp     eax, -1
            jne     saveFog
            add     esp, 0x08
            ret

        saveFog:
            call    FogOfWarMap_CopyFiles
            add     esp, 0x08
            mov     eax, 0

            pop     ebp
            pop     edi
            pop     esi
            pop     ecx
            pop     ebx
            ret
    }
}


//_________________________________________________
void __declspec(naked) fog_of_war_delete_tmps(void) {
    __asm {
            push    ebx
            push    ecx
            push    esi

            push    eax

            push    edx// extension
            push    eax//savePath
            call    FDeleteTmpSaveFiles
            add     esp, 0x8

            pop     ebx

            push    eax

            push    ebx//mapName (+ extension)
            call    FogOfWarMap_DeleteTmps
            add     esp, 0x4

            pop     eax

            pop     esi
            pop     ecx
            pop     ebx
            ret
    }
}


//__________________________________________
void __declspec(naked) fog_of_war_save(void) {
    __asm {
            push    ebx
            push    ecx
            push    edx
            push    esi
            push    edi
            push    ebp

            push    eax

            push    edx// "rb"
            push    eax//maps\(mapName).ext
            call    F_fopen
            add     esp, 0x8

            pop     ebx

            cmp     eax, 0
            je      endFunc

            push    eax

            push    ebx//mapName (+ extension)
            call    FogOfWarMap_Save
            add     esp, 0x4

            pop     eax

        endFunc:
            pop     ebp
            pop     edi
            pop     esi
            pop     edx
            pop     ecx
            pop     ebx
            ret
    }
}


//__________________________________________
void __declspec(naked) fog_of_war_load(void) {
    __asm {
            push    ebx
            push    ecx
            push    edx
            push    esi
            push    edi
            push    ebp

            push    eax//FileStream
            call    F_fclose
            add     esp, 0x4

            push    eax

            push    ebx//mapName (+ extension)
            call    FogOfWarMap_Load
            add     esp, 0x4

            pop     eax

            pop     ebp
            pop     edi
            pop     esi
            pop     edx
            pop     ecx
            pop     ebx
            ret
    }
}


//__________________________________________
void __declspec(naked) set_map_borders(void) {

    __asm {
            push    ebx
            push    ecx
            push    esi
            push    edi
            push    ebp

            push    edx// "rb"
            push    eax//maps\(mapName).ext
            call    F_fopen
            add     esp, 0x8
            cmp     eax, 0
            je      endFunc

            push eax

            push ebx//mapName (+ extension)
            call SetMapBorders
            add esp, 0x4

            pop eax

            endFunc :
            pop ebp
            pop edi
            pop esi
            pop ecx
            pop ebx
            ret
    }
}


//_______________________________________________
void __declspec(naked) get_floor_tile_light(void) {

    __asm {
            push    ebx
            push    ecx
            push    esi
            push    edi
            push    ebp

            push    ebp //global light
            push    edx //hexNum
            push    eax //elevation
            call    GetFloorHexLight
            add     esp, 0xC

            pop     ebp
            pop     edi
            pop     esi
            pop     ecx
            pop     ebx
            ret
    }
}


//___________________________________________
void __declspec(naked) draw_floor_tiles(void) {

    __asm {
            push    ebx
            push    ecx
            push    esi
            push    edi
            push    ebp

            push    edx
            push    eax
            call    DrawFloorTiles
            add     esp, 0x8

            pop     ebp
            pop     edi
            pop     esi
            pop     ecx
            pop     ebx
            ret
    }
}


//____________________
void __declspec(naked) CheckPcMovement() {
    __asm {
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            mov     eax, [edi+FOG_OF_WAR]
            pop     edi
            test    eax, eax
            jz      locret_10016355
            push    edi
            mov     edi, SN_DATA_SEC_EXE_ADDR
            lea     eax, [edi+D__obj_dude]
            pop     edi
            mov     eax, [eax]
            mov     eax, [eax+4]
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            cmp     eax, [edi+pcHexNum]
            pop     edi
            jz      locret_10016355
            push    edi
            mov     edi, SN_DATA_SEC_BLOCK_ADDR
            cmp     [edi+pcHexNum], eax
            pop     edi
            jmp     ReDrawViewWin

        locret_10016355:

            ret
    }
}


//________________________________________
void __declspec(naked) check_pc_movement() {

    __asm {
            push    eax
            push    ebx
            push    edx

            call    CheckPcMovement

            pop     edx
            pop     ebx
            pop     eax

            xor     ebp, ebp
            cmp     eax, 0x5
            ret
    }
}


//__________________________________
void __declspec(naked) h_draw_objs() {

    __asm {
            push    ebx
            push    ecx
            push    esi
            push    edi
            push    ebp

            push    edx
            push    eax
            call    DrawObjects
            add     esp, 0x8

            pop     ebp
            pop     edi
            pop     esi
            pop     ecx
            pop     ebx
            ret
    }
}


//___________________________________________
void __declspec(naked) h_get_objects_at_pos() {

    __asm {
            push    esi
            push    edi
            push    ebp

            push    dword ptr ss : [esp + 0x10]
            push    ecx
            push    ebx
            push    edx
            push    eax
            call    GetObjectsAtPos
            add     esp, 0x14

            pop     ebp
            pop     edi
            pop     esi
            ret     0x4
    }
}

// NOTE: All related to Fog of War was ported through IDA with the help of the source code from GitHub to guide what
// should be copied and what shouldn't. I tried first porting the C++ to C from GitHub, but it never seemed to work, the
// I think 3 times I tried. So I went on plan A again with Assembly and it finally worked, this time with help of the
// source. Maybe was a calling convention problem, or maybe the GitHub code is not working.

void MapFixes(void) {

    SetMapGlobals();

    hookCallEXE(0x44B36, &h_get_objects_at_pos);

    makeCallEXE(0x7B300, &h_draw_objs, true);

    writeMem32EXE(0x7B5CB + 2, (uint32_t) getRealBlockAddrData(&pCombatOutlineList), true);

    writeMem32EXE(0x7B5BB + 2, (uint32_t) getRealBlockAddrData(&combatOutlineCount), true);
    writeMem32EXE(0x7B5D7 + 2, (uint32_t) getRealBlockAddrData(&combatOutlineCount), true);

    //FIX - ORIGINALLY PLAYER POSITION SET TO SCROLL POSITION FOR JUMP TO MAP
    //NOW PLAYER POSITION SET BEFORE SCROLL
    writeMem32EXE(0x74C08 + 2, (uint32_t) getRealEXEAddr(D__map_ent_tile), true);

    hookCallEXE(0x7493F, &set_map_borders);


    writeMem8EXE(0x9E590, 0x90, true);
    makeCallEXE(0x9E591, &get_next_hex_pos, true);

    makeCallEXE(0x9E47C, &get_hex_dist, true);


    makeCallEXE(0x3B8C2, &check_pc_movement, false);

    makeCallEXE(0x9F40C, &draw_floor_tiles, true);

    hookCallEXE(0x9FD5F, &get_floor_tile_light);
    writeMem16EXE(0x9FD64, 0x15EB, true);

    hookCallEXE(0x757D4, &fog_of_war_save);

    hookCallEXE(0x6DC55, &fog_of_war_delete_tmps);
    hookCallEXE(0x6DC6B, &fog_of_war_delete_tmps);
    hookCallEXE(0x71841, &fog_of_war_delete_tmps);
    hookCallEXE(0x71AEC, &fog_of_war_delete_tmps);
    hookCallEXE(0x71F12, &fog_of_war_delete_tmps);

    //fog files move to save slot
    hookCallEXE(0x71981, &fog_of_war_copy_files);

    //fog files move from save slot
    hookCallEXE(0x71B9B, &fog_of_war_copy_files);

    hookCallEXE(0x74953, &fog_of_war_load);

    if (GET_BD_SYM(int, FOG_OF_WAR) != 0) {
        writeMem8EXE(0x7DE34, 0xC3, true);
    }
}
