// Copyright (C) 2022 Edw590
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// This had no copyright notice on it. I'm keeping the same license as in the other files of the project on it though
// (I'm just licensing it only to myself because I can't put the original authors - no notice).
//
// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/IniUtils.h"
#include "SFall1Patches.h"
#include "Sound.h"
#include <stdint.h>

/*__declspec(naked) static void gsnd_build_weapon_sfx_name_hook(void) {
	__asm {
			push    edx
			inc     eax
			mov     edx, eax
			inc     edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_roll_random_]
			call    edi
			pop     edi
			pop     edx
			mov     esi, eax
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    eax
			mov     al, dl
			push    eax
			push    ebx
			mov     al, ch
			push    eax
			mov     al, cl
			push    eax

			lea     esp, [esp-4] // [Edw590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF347C]                   // 'W%c%c%1d%cXX%1d'
			mov     [esp+4], edi // [Edw590: "PUSH"]
			pop     edi

			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [esi+D__sfx_file_name]
			pop     esi
			push    edi
			mov     [esp+7*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			add     esp, 7*4
			pop     edi
			xchg    edi, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_strupr_]
			call    edi
			pop     edi
			push    edx
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]

			lea     esp, [esp-4] // [Edw590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF2FA4]                   // '.ACM'
			mov     [esp+4], edi // [Edw590: "PUSH"]
			pop     edi

			push    eax                                  // _sfx_file_name

			lea     esp, [esp-4] // [Edw590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF2C50]                   // 'sound\sfx\'
			mov     [esp+4], edi // [Edw590: "PUSH"]
			pop     edi

			lea     esp, [esp-4] // [Edw590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF3334]                   // '%s%s%s'
			mov     [esp+4], edi // [Edw590: "PUSH"]
			pop     edi

			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edx, [edi+D__str]
			pop     edi
			push    edx
			mov     [esp+5*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			mov     eax, esp
			xchg    edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_db_dir_entry_]
			call    edi
			pop     edi
			add     esp, 5*4
			pop     edi
			pop     edx
			inc     eax
			jnz     skip
			inc     eax
			xchg    esi, eax
		skip:
			pop     edi
			push    esi
			mov     al, dl
			push    eax
			push    edi
			ret
	}
}*/

// Entire file updated to version 1.8

void SoundInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Sound", "NumSoundBuffers", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int > 0) && (temp_int <= 4)) {
		writeMem8EXE(0x48CA7 + 2, (uint8_t) temp_int, true);
	}

	// No idea what this does, but the game crashes with it enabled after firing the pistol, so I've disabled it.
	//makeCallEXE(0x493DB, &gsnd_build_weapon_sfx_name_hook, false);
}
