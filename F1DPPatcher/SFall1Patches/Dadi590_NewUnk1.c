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

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "Dadi590_NewUnk1.h"
#include "FalloutEngine.h"
#include "SFall1Patches.h"

static uint32_t AICalledFreqDiv = 0;

static void __declspec(naked) ai_called_shot_hook(void) {
	__asm {
			xchg    eax, edx
			cdq
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			div     dword ptr [edi+AICalledFreqDiv]
			pop     edi
			xchg    eax, edx
			xor     eax, eax
			inc     eax
			cmp     eax, edx
			jbe     end
			mov     edx, eax
		end:
			ret
	}
}

static void __declspec(naked) is_within_perception_hook(void) {
	__asm {
			test    eax, eax
			jz      end
			xor     eax, eax
			inc     eax
			xchg    eax, edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			mov     ebp, eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1
			pop     edi
			jz      dadi590_newUnk1
			shl     eax, 1
		dadi590_newUnk1:
			xchg    eax, esi
			test    byte ptr [ecx+0x44], 0x40
			jnz     dadi590_newUnk2
			mov     edx, ebx
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_can_see_]
			call    edi
			pop     edi
			dec     eax
			jnz     dadi590_newUnk2
			push    ecx
			push    ebx
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]

			lea     esp, [esp-4] // [DADi590] Reserve space for the push
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_sight_blocking_at_]
			mov     [esp+4], edi
			pop     edi

			push    eax
			xchg    eax, ecx
			mov     ebx, [ebx+4]
			mov     edx, [eax+4]
			mov     [esp+2*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_make_path_func_]
			call    edi
			pop     edi
			pop     ebx
			pop     ecx
			test    eax, eax
			jz      dadi590_newUnk2
			lea     esi, [ebp+ebp*4]
			test    byte ptr [ebx+0x26], 2
			jz      dadi590_newUnk2
			shr     esi, 1
		dadi590_newUnk2:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ebx, ds:[edi+D__obj_dude]
			pop     edi
			jnz     dadi590_newUnk3
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_pc_sneak_working_]
			call    edi
			pop     edi
			dec     eax
			jnz     dadi590_newUnk3
			shr     esi, 2
		dadi590_newUnk3:
			mov     edx, ecx
			xchg    eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_dist_]
			call    edi
			pop     edi
			xor     edi, edi
			xchg    eax, edi
			cmp     edi, esi
			jg      end
			inc     eax
		end:
			pop     ebp
			pop     edi
			pop     esi
			pop     ecx
			pop     ebx
			ret
	}
}

static void __declspec(naked) is_within_perception_call_hook(void) {
	__asm {
			mov     esi, eax
			mov     ecx, [esi+0x44]
			or      byte ptr [esi+0x44], 0x40
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_within_perception_]
			call    edi
			pop     edi
			mov     [esi+0x44], ecx
			ret
	}
}

void Dadi590_NewUnk1Init(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AI_Called_Freq_Div", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*(uint32_t *) getRealBlockAddrData(&AICalledFreqDiv) = (uint32_t) temp_int;
	if (temp_int > 1) {
		MakeCallEXE(0x258BE, getRealBlockAddrCode((void *) &ai_called_shot_hook), true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "CanSeeAndHearFix", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		MakeCallEXE(0x264CD, getRealBlockAddrCode((void *) &is_within_perception_hook), true);
		HookCallEXE(0x4EFA8, getRealBlockAddrCode((void *) &is_within_perception_call_hook));
	}
}
