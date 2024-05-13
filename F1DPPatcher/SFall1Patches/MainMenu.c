/*
 *    sfall
 *    Copyright (C) 2012  The sfall team, 2022 Edw590
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "MainMenu.h"
#include "SFall1Patches.h"
#include <stdint.h>

static uint32_t MainMenuYOffset = 0;
static uint32_t MainMenuTextOffset = 0;
static uint32_t OverrideColour = 0;


__declspec(naked) static void MainMenuButtonYHook(void) {
	__asm {
			xor     edi, edi
			xor     esi, esi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     ebp, [edi+MainMenuYOffset]
			pop     edi
			ret
	}
}

__declspec(naked) static void MainMenuTextYHook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			add     eax, [edi+MainMenuTextOffset]
			pop     edi

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[edi+D__text_to_buf]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void FontColour(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+OverrideColour], 0
			pop     edi
			je      skip
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     eax, [edi+OverrideColour]
			pop     edi
			ret

		skip:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			movzx   eax, byte ptr ds:[edi+0x2A6CE4]
			pop     edi
			or      eax, 0x6000000
			ret
	}
}

__declspec(naked) static void MainMenuTextHook(void) {
	__asm {
			// [Edw590: I've changed the implementation here - now it's here only to fix the color for the user chosen one]

			lea     esp, [esp-4] // Reserve space on the stack for the last function PUSH
			push    edi
			mov     edi, [(esp+4)+4]
			mov     [(esp+4)+0], edi
			mov     edi, [(esp+4)+8]
			mov     [(esp+4)+4], edi
			pop     edi
			mov     [esp+2*4], edi // "PUSH EDI"

			push    eax
			call    FontColour
			mov     [(esp+4)+4], eax
			pop     eax

			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_print_]
			call    edi
			pop     edi

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, ds:[edi+0x73390]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

void MainMenuInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MainMenuCreditsOffsetX", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x73332 + 1, 15 + (uint32_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MainMenuCreditsOffsetY", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x7333B + 1, 460 + (uint32_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MainMenuOffsetX", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x7345B + 1, 425 + (uint32_t) temp_int, true);
		*(uint32_t *) getRealBlockAddrData(&MainMenuTextOffset) = (uint32_t) temp_int;
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MainMenuOffsetY", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		*(uint32_t *) getRealBlockAddrData(&MainMenuYOffset) = (uint32_t) temp_int;
		*(uint32_t *) getRealBlockAddrData(&MainMenuTextOffset) += (uint32_t) temp_int * 640;
		writeMem8EXE(0x73424, 0x90, true);
		makeCallEXE(0x73424 + 1, &MainMenuButtonYHook, false);
	}

	if (0 != *(uint32_t *) getRealBlockAddrData(&MainMenuTextOffset)) {
		writeMem8EXE(0x73513, 0x90, true);
		makeCallEXE(0x73513 + 1, &MainMenuTextYHook, false);
	}

	makeCallEXE(0x7338B, &MainMenuTextHook, true);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MainMenuFontColour", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%X", getRealBlockAddrData(&OverrideColour));
	if (0 != *(uint32_t *) getRealBlockAddrData(&OverrideColour)) {
		makeCallEXE(0x7332C, &FontColour, false);
	}

	//hookCallEXE(0x7338B, &func_40B582); - Disabled because there's usage of VersionString on it, and that's already
	// taken care of by me. I'll just ignore this.
}
