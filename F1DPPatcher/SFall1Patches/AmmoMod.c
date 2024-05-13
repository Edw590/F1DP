/*
 *    sfall
 *    Copyright (C) 2008, 2009, 2010, 2013, 2014  The sfall team, 2022 Edw590
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
#include "../GameAddrs/CStdFuncs.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "AmmoMod.h"
#include "Define.h"
#include "SFall1Patches.h"

__declspec(naked) static void item_w_damage_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ebx, ds:[edi+D__obj_dude]
			pop     edi
			jne     skip
			mov     eax, PERK_bonus_hth_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			shl     eax, 1
		skip:
			add     eax, 1
			xchg    ebp, eax
			ret
	}
}

__declspec(naked) static void item_w_damage_hook1(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ebx, ds:[edi+D__obj_dude]
			pop     edi
			jne     skip
			push    eax
			mov     eax, PERK_bonus_hth_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			shl     eax, 1
			add     [esp+0x4+0x8], eax                   // min_dmg
			pop     eax
		skip:
			ret
	}
}

__declspec(naked) static void display_stats_hook(void) {
	__asm {
			mov     eax, PERK_bonus_hth_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			shl     eax, 1
			add     [esp+4*4], eax                       // min_dmg

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void display_stats_hook1(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			add     eax, 2
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    eax
			mov     eax, PERK_bonus_hth_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			shl     eax, 1
			add     eax, 1
			push    eax
			mov     eax, [esp+0x94+0x4 + 4]
			push    eax

			lea     esp, [esp-4] // [Edw590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF91F8]                         // '%s %d-%d'
			mov     [esp+4], edi // "PUSH"
			pop     edi

			lea     eax, [esp+0xC+0x4 + 4]
			push    eax
			mov     [esp+5*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			add     esp, 5*4
			pop     edi

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x65CC4]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void display_stats_hook2(void) {
	__asm {
			mov     eax, PERK_bonus_ranged_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			shl     eax, 1
			add     [esp+4*4], eax                       // min_dmg
			add     [esp+4*5], eax                       // max_dmg

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

void AmmoModInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BonusHtHDamageFix", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		// Bonus HtH Damage Perk fix
		makeCallEXE(0x6AF56, &item_w_damage_hook, false);
		hookCallEXE(0x6AFA8, &item_w_damage_hook1);
		hookCallEXE(0x65AA1, &display_stats_hook);
		makeCallEXE(0x65CA1, &display_stats_hook1, true);
	}

	// "Show changes min./max. damage to the weapon if the perk "Bonus damage at distance" is taken."
	hookCallEXE(0x65A75, &display_stats_hook2);
}
