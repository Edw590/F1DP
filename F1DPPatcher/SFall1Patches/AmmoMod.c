// Copyright 2022 DADi590
//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

//
// Created by DADi590 on 05/03/2022.
//

#include "../CLibs/string.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/IniUtils.h"
#include "AmmoMod.h"
#include "Define.h"
#include "FalloutEngine.h"
#include "SFall1Patches.h"

static void __declspec(naked) item_w_damage_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ebx, ds:[edi+D__obj_dude]
			pop     edi
			jne     skip
			mov     eax, PERK_bonus_hth_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_perk_level_
			call    edi
			pop     edi
			shl     eax, 1
		skip:
			add     eax, 1
			xchg    ebp, eax
			retn
	}
}

static void __declspec(naked) item_w_damage_hook1(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_stat_level_
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
			add     edi, C_perk_level_
			call    edi
			pop     edi
			shl     eax, 1
			add     [esp+0x4+0x8], eax                   // min_dmg
			pop     eax
		skip:
			retn
	}
}

static void __declspec(naked) display_stats_hook(void) {
	__asm {
			mov     eax, PERK_bonus_hth_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_perk_level_
			call    edi
			pop     edi
			shl     eax, 1
			add     [esp+4*4], eax                       // min_dmg

			sub     esp, 4 // [DADi590] Reserve space for the return address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_sprintf_
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) display_stats_hook1(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_stat_level_
			call    edi
			pop     edi
			add     eax, 2
			sub     esp, 4 // [DADi590: reserve space to "PUSH EDI"]
			push    eax
			mov     eax, PERK_bonus_hth_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_perk_level_
			call    edi
			pop     edi
			shl     eax, 1
			add     eax, 1
			push    eax
			mov     eax, [esp+0x94+0x4 + 4]
			push    eax

			sub     esp, 4 // [DADi590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, 0xF91F8                         // '%s %d-%d'
			mov     [esp+4], edi // "PUSH"
			pop     edi

			lea     eax, [esp+0xC+0x4 + 4]
			push    eax
			mov     [esp+5*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_sprintf_
			call    edi
			add     esp, 4*5
			pop     edi

			sub     esp, 4 // [DADi590] Reserve space for the return address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, 0x65CC4
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) display_stats_hook2(void) {
	__asm {
			mov     eax, PERK_bonus_ranged_damage
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_perk_level_
			call    edi
			pop     edi
			shl     eax, 1
			add     [esp+4*4], eax                       // min_dmg
			add     [esp+4*5], eax                       // max_dmg

			sub     esp, 4 // [DADi590] Reserve space for the return address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_sprintf_
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

void AmmoModInit(void) {
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BonusHtHDamageFix", "1", prop_value, &sfall1_ini_info_G);
	if (0 != strcmp(prop_value, "0")) {
		// Bonus HtH Damage Perk fix
		MakeCallEXE(0x6AF56, getRealBlockAddrCode((void *) &item_w_damage_hook), false);
		HookCallEXE(0x6AFA8, getRealBlockAddrCode((void *) &item_w_damage_hook1));
		HookCallEXE(0x65AA1, getRealBlockAddrCode((void *) &display_stats_hook));
		MakeCallEXE(0x65CA1, getRealBlockAddrCode((void *) &display_stats_hook1), true);
	}

	// "Show changes min./max. damage to the weapon if the perk "Bonus damage at distance" is taken."
	HookCallEXE(0x65A75, getRealBlockAddrCode((void *) &display_stats_hook2));

}
