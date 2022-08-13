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
#include "AI.h"
#include "SFall1Patches.h"

static int AI_Called_Freq_Div = 0;

static void __declspec(naked) ai_called_shot_hook(void) {
	__asm {
			xchg    edx, eax                             // eax = cap.called_freq
			cdq
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			div     dword ptr [edi+AI_Called_Freq_Div]
			pop     edi
			xchg    eax, edx
			xor     eax, eax
			inc     eax
			cmp     eax, edx
			jbe     end
			mov     edx, eax
		end:
			retn
	}
}

static void __declspec(naked) func_4012C7(void) {
	__asm {
			push    ecx
			cmp     edx, 9C40h
			jb      loc_4012D4
			xor     edx, edx
			jmp     loc_401308
		loc_4012D4:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edx, ds:[edi+D__objectTable][edx*4]
			pop     edi
			loc_4012DB:
			test    edx, edx
			jz      loc_401308
			mov     ecx, [edx]
			cmp     [ecx+28h], ebx
			jnz     loc_401301
			test    byte ptr [ecx+24h], 1
			jnz     loc_401301
			cmp     ecx, eax
			jz      loc_401301
			mov     ecx, [ecx+20h]
			and     ecx, 0F000000h
			sar     ecx, 18h
			sub     ecx, 3
			jz      loc_401306
		loc_401301:
			mov     edx, [edx+4]
			jmp     loc_4012DB
		loc_401306:
			mov     edx, [edx]
		loc_401308:
			xchg    eax, edx
			pop     ecx
			retn
	}
}

static void __declspec(naked) is_within_perception_hook(void) {
	__asm {
			sub     esp, 4
			xor     ebp, ebp
			test    byte ptr [ecx+0x44], 0x40
			jnz     loc_401353
			mov     edx, ebx
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_can_see_]
			call    edi
			pop     edi
			dec     eax
			jnz     loc_401353
			mov     edx, esp
			push    ecx
			push    ebx
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]

			lea     esp, [esp-4] // [DADi590] Reserve space for the push
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_blocking_at_]
			mov     [esp+4], edi
			pop     edi

			push    0x10
			mov     [edx], eax
			push    edx
			xchg    eax, ecx
			mov     ebx, [ebx+4]
			mov     edx, [eax+4]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_make_straight_path_func_]
			call    edi
			pop     edi
			pop     ebx
			pop     ecx
			cmp     [esp], ebx
			jnz     loc_401353
			lea     esi, [esi+esi*4]
			test    byte ptr [ebx+0x26], 2
			jz      loc_401384
			shr     esi, 1
			jmp     loc_401384
		loc_401353:
			shl     esi, 1
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1 // In battle?
			pop     edi
			jnz     loc_401385
			shr     esi, 1
			xor     eax, eax
			mov     edx, esp
			push    ecx
			push    ebx
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]

			lea     esp, [esp-4] // [DADi590] Reserve space for the push
			push    edi
			mov     edi, SN_CODE_SEC_BLOCK_ADDR
			lea     edi, [edi+func_4012C7]
			mov     [esp+4], edi
			pop     edi

			push    0x10
			mov     [edx], eax
			push    edx
			xchg    eax, ecx
			mov     ebx, [ebx+4]
			mov     edx, [eax+4]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_make_straight_path_func_]
			call    edi
			pop     edi
			pop     ebx
			pop     ecx
			cmp     [esp], ebp
			jnz     loc_401385
		loc_401384:
			inc     ebp
		loc_401385:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ebx, ds:[edi+D__obj_dude]
			pop     edi
			jnz     loc_401399
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_pc_sneak_working_]
			call    edi
			pop     edi
			dec     eax
			jnz     loc_401399
			shr     esi, 2
		loc_401399:
			inc     ebp
			cmp     edi, esi
			jbe     loc_4013A0
			xor     ebp, ebp
		loc_4013A0:
			xchg    eax, ebp
			add     esp, 4
			pop     ebp
			pop     edi
			pop     esi
			pop     ecx
			pop     ebx
			retn
	}
}

static void __declspec(naked) op_obj_can_hear_obj_hook(void) {
	__asm {
			mov     esi, eax                             // esi = source
			mov     ecx, [esi+0x44]
			or      byte ptr [esi+0x44], 0x40            // source.results & DAM_BLIND
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_within_perception_]
			call    edi
			pop     edi
			mov     [esi+0x44], ecx
			retn
	}
}

static void __declspec(naked) func_4013BD(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_within_perception_]
			call    edi
			pop     edi
			cmp     eax, 2
			jnz     locret_4013CB
			inc     edi
			xor     eax, eax
		locret_4013CB:
			retn
	}
}

static void __declspec(naked) func_4013CC(void) {
	__asm {
			push    edi
			push    ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_soundUpdate_]
			call    edi
			pop     edi
			mov     ecx, [esi+0x50]
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__curr_crit_list]
			pop     esi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ebp, ds:[edi+D__curr_crit_num]
			pop     edi
			inc     ebp
		loc_4013E4:
			dec     ebp
			jz      loc_40140A
			mov     ebx, [edi]
			add     edi, 4
			test    byte ptr [ebx+0x3C], 1
			jnz     loc_4013E4
			cmp     [ebx+0x50], ecx
			jnz     loc_4013E4
			mov     edx, esi
			mov     eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_within_perception_]
			call    edi
			pop     edi
			dec     eax
			jnz     loc_4013E4
			or      byte ptr [ebx+0x3C], 1
			jmp     loc_4013E4
		loc_40140A:
			pop     ecx
			pop     edi
			retn
	}
}

// Entire file updated to version 1.8

void AIInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AI_Called_Freq_Div", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*(uint32_t *) getRealBlockAddrData(&AI_Called_Freq_Div) = (uint32_t) temp_int;
	if (temp_int > 1) {
		makeCallEXE(0x258BE, getRealBlockAddrCode((void *) &ai_called_shot_hook), true);
	}

	// Additional checks for blindness and line of sight
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "CanSeeAndHearFix", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		makeCallEXE(0x264E2, getRealBlockAddrCode((void *) &is_within_perception_hook), true);
		hookCallEXE(0x4EFA8, getRealBlockAddrCode((void *) &op_obj_can_hear_obj_hook));
		hookCallEXE(0x4DC9E, getRealBlockAddrCode((void *) &func_4013BD));
		hookCallEXE(0x2095D, getRealBlockAddrCode((void *) &func_4013CC));
	}
}
