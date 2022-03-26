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

static void __declspec(naked) is_within_perception_hook(void) {
	__asm {
			test    eax, eax                             // Is there a purpose?
			jz      end                                  // No
			xor     eax, eax
			inc     eax                                  // STAT_pe
			xchg    eax, edx                             // eax = source, edx = STAT_pe
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			mov     ebp, eax                             // ebp = perception source
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1 // In battle?
			pop     edi
			jz      notCombat                            // No
			shl     eax, 1                               // eax = perception * 2
		notCombat:
			xchg    esi, eax                             // esi = visibility (at this stage it is audibility)
			test    byte ptr [ecx+0x44], 0x40            // source.results & DAM_BLIND?
			jnz     cantSee                              // Yes, blind
			mov     edx, ebx
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_can_see_]
			call    edi
			pop     edi
			dec     eax                                  // In front of face?
			jnz     cantSee                              // No
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
			xchg    ecx, eax                             // eax = source, ecx = *sad_rotation_ptr (0)
			mov     ebx, [ebx+0x4]                       // ebx = target.tile_num
			mov     edx, [eax+0x4]                       // edx = source.tile_num
			mov     [esp+2*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_make_path_func_]         // Line of sight test
			call    edi
			pop     edi
			pop     ebx
			pop     ecx
			test    eax, eax                             // Are there barriers?
			jz      cantSee                              // Yes
			lea     esi, [ebp+ebp*4]                     // esi = perception * 5
			test    byte ptr [ebx+0x26], 2               // target.flags3 & TransGlass_?
			jz      cantSee                              // No
			shr     esi, 1                               // esi = visibility / 2
		cantSee:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ebx, ds:[edi+D__obj_dude]            // Goal == YY?
			pop     edi
			jnz     checkDistance                        // No
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_pc_sneak_working_]
			call    edi
			pop     edi
			dec     eax                                  // Does stealth work?
			jnz     checkDistance                        // No
			shr     esi, 2                               // esi = visibility / 4
		checkDistance:
			mov     edx, ecx
			xchg    eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_dist_]
			call    edi
			pop     edi
			xor     edi, edi
			xchg    edi, eax                             // edi = distance
			cmp     edi, esi                             // Is the distance greater than visibility?
			jg      end                                  // Yes
			inc     eax
		end:
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
		makeCallEXE(0x264CD, getRealBlockAddrCode((void *) &is_within_perception_hook), true);
		hookCallEXE(0x4EFA8, getRealBlockAddrCode((void *) &op_obj_can_hear_obj_hook));
	}
}
