// Copyright (C) 2022 DADi590
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
// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "Define.h"
#include "LoadGameHook.h"
#include "PartyControl.h"
#include "SFall1Main.h"
#include "SFall1Patches.h"
#include <stdbool.h>

uint32_t InLoop = 0;

static void __declspec(naked) ResetState(void) {
	__asm {
			pushad
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      end
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+DelayedExperience], eax
			pop     edi
			call    RestoreDudeState
		end:
			popad
			retn
	}
}

static void __declspec(naked) LoadGame_hook(void) {
	__asm {
			call ResetState

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_LoadSlot_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) gnw_main_hook(void) {
	__asm {
			call ResetState

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_main_menu_loop_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) gnw_main_hook1(void) {
	__asm {
			call ResetState

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_main_game_loop_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static char SaveFailMsg[128] = {0};
static uint32_t SaveInCombatFix = 0;
static void __declspec(naked) SaveGame_hook(void) {
	__asm {
			xor     esi, esi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1
			pop     edi
			jz      skip                                 // Not in battle
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], esi
			pop     edi
			jne     end
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+SaveInCombatFix], esi
			pop     edi
			je      skip
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+SaveInCombatFix], 2
			pop     edi
			je      end
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			mov     ebx, [eax+0x40]                      // curr_mp
			mov     edx, STAT_max_move_points
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			cmp     eax, ebx
			jne     restore
			mov     eax, PERK_bonus_move
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			shl     eax, 1
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     eax, ds:[edi+D__combat_free_move]
			pop     edi
			jne     restore
			popad
		skip:
			dec     esi
			retn

		restore:
			popad
		end:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     eax, [edi+SaveFailMsg]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_print_]
			call    edi
			pop     edi
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x6E966]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) setup_inventory_hook(void) {
	__asm {
			mov     esi, 6
			test    eax, eax
			jnz     notZero
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			or      [edi+InLoop], INVENTORY
			pop     edi
			retn

		notZero:
			dec     eax
			jnz     notOne
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			or      [edi+InLoop], INTFACEUSE
			pop     edi
			retn

		notOne:
			dec     eax
			jnz     notTwo
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			or      [edi+InLoop], INTFACELOOT
			pop     edi
			retn

		notTwo:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			or      [edi+InLoop], BARTER
			pop     edi
			retn
	}
}

void LoadGameHookInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	hookCallEXE(0x6EAEC, getRealBlockAddrCode((void *) &LoadGame_hook));
	hookCallEXE(0x6F562, getRealBlockAddrCode((void *) &LoadGame_hook));
	hookCallEXE(0x7291B, getRealBlockAddrCode((void *) &gnw_main_hook));
	hookCallEXE(0x7299F, getRealBlockAddrCode((void *) &gnw_main_hook1));

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "SaveInCombat", "Cannot save at this time.", SaveFailMsg,
					&translation_ini_info_G);
	makeCallEXE(0x6DC87, getRealBlockAddrCode((void *) &SaveGame_hook), false);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SaveInCombatFix", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*(uint32_t *) getRealBlockAddrData(&SaveInCombatFix) = (uint32_t) (temp_int <= 2 ? temp_int : 0);

	makeCallEXE(0x62AB5, getRealBlockAddrCode((void *) &setup_inventory_hook), false);// INVENTORY + INTFACEUSE + INTFACELOOT + BARTER
}
