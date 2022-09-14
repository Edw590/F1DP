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

#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "F_File.h"
#include "F_Mapper.h"
#include "fixes_maps.h"


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
			mov     eax, [esp]
			mov     edx, [esp+4]
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
			mov     eax, ebx
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
			mov     eax, ebx
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
			mov     eax, ebx
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


//_______________________________________________
void __declspec(naked) get_floor_tile_light(void) {

	__asm {
			push    ebx
			push    ecx
			push    esi
			push    edi
			push    ebp

			push    ebp //global light
			mov     ebx, ebp
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
void CheckPcMovement() {
	static int32_t pcHexNum = 0;
	//RECT rcObj;
	struct OBJStruct* pcObj = *(*(struct OBJStruct ** *) getRealBlockAddrData(&lpObj_PC));

	if (!*(int *) getRealBlockAddrData(&FOG_OF_WAR)) {
		return;
	}

	if (pcObj->hexNum != pcHexNum) {
		pcHexNum = pcObj->hexNum;

		ReDrawViewWin();
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

// todo I absolutely have no idea how to fix all this and on F_Mapper. I think I copied and ported everything correctly!
//  If I didn't, no idea what I did wrong. Maybe more functions are missing? I only copied those who seemed to be
//  related to FOW, either by their name, or by their usage of variables with "fog" in their name.

void MapFixes(void) {

	SetMapGlobals();

	//hookCallEXE(0x44B36, &h_get_objects_at_pos); todo Problems

	//makeCallEXE(0x7B300, &h_draw_objs, true); todo Problems

	writeMem32EXE(0x7B5CB+2, (uint32_t) getRealBlockAddrData(&pCombatOutlineList));

	writeMem32EXE(0x7B5BB+2, (uint32_t) getRealBlockAddrData(&combatOutlineCount));
	writeMem32EXE(0x7B5D7+2, (uint32_t) getRealBlockAddrData(&combatOutlineCount));

	//FIX - ORIGINALLY PLAYER POSITION SET TO SCROLL POSITION FOR JUMP TO MAP
	//NOW PLAYER POSITION SET BEFORE SCROLL
	writeMem32EXE(0x74C08+2, (uint32_t) getRealEXEAddr(D__map_ent_tile));


	//writeMem8EXE(0x9E590, 0x90);
	//makeCallEXE(0x9E591, &get_next_hex_pos, true);// todo Problems

	makeCallEXE(0x9E47C, &get_hex_dist, true);


	makeCallEXE(0x3B8C2, &check_pc_movement, false);

	makeCallEXE(0x9F40C, &draw_floor_tiles, true);

	hookCallEXE(0x9FD5F, &get_floor_tile_light);
	writeMem16EXE(0x9FD64, 0x15EB);

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

	//hookCallEXE(0x74953, &fog_of_war_load); //todo Problems

	if (0 != *(int *) getRealBlockAddrData(&FOG_OF_WAR)) {
		writeMem8EXE(0x7DE34, 0xC3);
	}
}
