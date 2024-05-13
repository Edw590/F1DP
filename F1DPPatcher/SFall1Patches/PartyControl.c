/*
 *    sfall
 *    Copyright (C) 2013  The sfall team, 2022 Edw590
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
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "Define.h"
#include "PartyControl.h"
#include "SFall1Main.h"
#include "SFall1Patches.h"
#include <stddef.h>

uint32_t IsControllingNPC = 0;
uint32_t HiddenArmor = 0;
uint32_t DelayedExperience = 0;

static uint32_t Mode = 0;
static uint16_t Chars[50] = {0};

static char real_pc_name[32] = {0};
static uint32_t real_last_level = 0;
static uint32_t real_Level = 0;
static uint32_t real_Experience = 0;
static uint32_t real_perk_lev[PERK_count] = {0};
static uint32_t real_free_perk = 0;
static uint32_t real_unspent_skill_points = 0;
static uint32_t real_map_elevation = 0;
static uint32_t real_sneak_working = 0;
static uint32_t real_sneak_queue_time = 0;
static uint32_t real_dude = 0;
static uint32_t real_hand = 0;
static uint32_t real_tag_skill[4] = {0};
static uint32_t real_trait = 0;
static uint32_t real_trait2 = 0;
static uint32_t real_itemButtonItems[6 * 2] = {0};
static uint32_t real_drug_gvar[6] = {0};
static uint32_t real_bbox_sneak = 0;
//static uint32_t party_PERK_bonus_awareness = 0; - [Edw590: not used, so commented out]

static uint8_t *NameBox = NULL;

__declspec(naked) static void PartyControl_CanUseWeapon(void) {
	__asm {
			push    edi
			push    esi
			push    edx
			push    ecx
			push    ebx
			mov     edi, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_weapon
			jne     canUse                               // No
			dec     eax
			dec     eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax          // Character control?
			pop     edi
			jne     end                                  // No
			inc     eax                                  // hit_right_weapon_primary
			xchg    ecx, eax
			mov     eax, edi                             // eax=item
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_anim_code_]
			call    edi
			pop     edi
			xchg    ecx, eax                             // ecx=ID1=Weapon code
			xchg    edx, eax
			xchg    edi, eax                             // eax=item
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_anim_weap_]
			call    edi
			pop     edi
			xchg    ebx, eax                             // ebx=ID2=Animation code
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     esi, ds:[edi+D__inven_dude]
			pop     edi
			mov     edx, [esi+0x20]                       // fid
			and     edx, 0xFFF                           // edx=Index
			mov     eax, [esi+0x1C]                       // cur_rot
			inc     eax
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    eax                                  // ID3=Direction code
			mov     eax, ObjType_Critter
			mov     [esp+1*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_exists_]
			call    edi
			pop     edi
			test    eax, eax
			jz      end
		canUse:
			xor     eax, eax
			inc     eax
		end:
			pop     ebx
			pop     ecx
			pop     edx
			pop     esi
			pop     edi
			ret
	}
}

static bool __stdcall IsInPidList(uint32_t const *npc) {
	int i = 0;
	uint32_t pid = npc[0x64u / 4u] & 0xFFFFFFu;

	// Here, suppose the usage of the array is measured until the first 0 word is found on it.

	for (i = 0; i < ((int) sizeof(Chars) / 2); ++i) {
		if (0 == GET_BD_ARR(uint16_t *, Chars)[i]) {
			break;
		}
	}
	if (1 == i) { // Then size is 0 (checked the 1st index which was 0, incremented i and left)
		return true;
	}


	for (i = 0; i < ((int) sizeof(Chars) / 2); ++i) {
		if (pid == GET_BD_ARR(uint16_t *, Chars)[i]) {
			return true;
		}
	}

	return false;
}

// save "real" dude state
__declspec(naked) void SaveDudeState(void) {
	__asm {
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     esi, [edi+D__pc_name]
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			lea     edi, [esi+real_pc_name]
			pop     esi
			mov     ecx, 32/4
			rep     movsd
			mov     eax, ebx
			push    ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_name_]
			call    edi
			pop     edi
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_pc_set_name_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__bbox+12+8]
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+real_bbox_sneak], eax
			mov     eax, [edi+NameBox]
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__bbox+12+8], eax
			pop     edi
			mov     ebx, 2730
			xor     edx, edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_memset_]
			call    edi
			pop     edi
			xchg    edi, eax                             // edi = Buffer
			pop     edx                                  // edx = DisplayText

			lea     esp, [esp-4] // [Edw590] Reserve space for the push
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+D__curr_font_num]
			mov     [esp+4], edi
			pop     edi

			mov     eax, 103
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_text_font_]
			call    edi
			pop     edi
			mov     esi, 21
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			call    ds:[edi+D__text_height]
			pop     edi
			sub     esi, eax
			shr     esi, 1
			inc     esi                                  // esi = y
			mov     ecx, 130                             // ecx = ToWidth
			imul    esi, ecx                             // esi = ToWidth * y
			lea     esi, [esi+67]                        // esi = ToWidth * y + 67
			lea     ebx, [ecx-4]
			mov     eax, edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			call    ds:[edi+D__text_width]
			pop     edi
			cmp     eax, ebx
			jbe     goodWidth
			xchg    ebx, eax
		goodWidth:
			mov     ebx, eax                             // ebx = TxtWidth
			shr     eax, 1                               // TxtWidth/2
			sub     esi, eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			movzx   eax, byte ptr ds:[edi+D__BlueColor]
			pop     edi
			push    eax                                  // ColorIndex
			lea     eax, [edi+esi]                       // eax = Buffer
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			call    ds:[edi+D__text_to_buf]
			pop     edi
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_text_font_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			movzx   eax, byte ptr ds:[edi+D__GreenColor]
			pop     edi
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    eax                                  // Color
			push    19
			mov     edx, 129
			push    edx
			inc     edx
			xor     ecx, ecx
			mov     ebx, 3
			xchg    edi, eax                             // toSurface
			mov     [esp+3*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_draw_box_]
			call    edi
			pop     edi
			pop     ebx
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__last_level]
			mov     [esi+real_last_level], eax
			mov     eax, ds:[edi+D__Level_]
			mov     [esi+real_Level], eax
			mov     eax, ds:[edi+D__Experience_]
			mov     [esi+real_Experience], eax
			movzx   eax, byte ptr ds:[edi+D__free_perk]
			mov     [esi+real_free_perk], eax
			mov     eax, ds:[edi+D__curr_pc_stat]
			mov     [esi+real_unspent_skill_points], eax
			mov     eax, ds:[edi+D__map_elevation]
			mov     [esi+real_map_elevation], eax
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			pop     esi
			push    eax
			mov     edx, 10
			call    queue_find_first_
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     edx, [edi+_tmpQNode]
			pop     edi
			test    edx, edx
			jz      noSneak
			mov     edx, [edx]
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__fallout_game_time]
			pop     edi
			sub     edx, eax
			pop     eax
			push    eax
			push    edx
			mov     edx, 10
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_queue_remove_this_]
			call    edi
			pop     edi
			pop     edx
		noSneak:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+real_sneak_queue_time], edx
			pop     edi
			pop     eax
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     [esi+real_dude], eax
			mov     eax, [ebx+0x64]
			mov     ds:[edi+D__inven_pid], eax
			mov     ds:[edi+D__obj_dude], ebx
			mov     ds:[edi+D__inven_dude], ebx
			mov     ds:[edi+D__stack], ebx
			mov     eax, ds:[edi+D__itemCurrentItem]
			mov     [esi+real_hand], eax
			mov     eax, ds:[edi+D__sneak_working]
			mov     [esi+real_sneak_working], eax
			lea     edx, [esi+real_trait2]
			lea     eax, [esi+real_trait]
			pop     edi
			pop     esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_trait_get_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     esi, [edi+D__itemButtonItems]
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			lea     edi, [esi+real_itemButtonItems]
			pop     esi
			mov     ecx, 6*2
			rep     movsd
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     esi, [edi+D__perk_lev]
			pop     edi
			mov     edx, [esi]                           // PERK_bonus_awareness
			push    esi
			push    esi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			lea     edi, [esi+real_perk_lev]
			pop     esi
			mov     ecx, PERK_count
			push    ecx
			rep     movsd
			xchg    ecx, eax
			pop     ecx
			pop     edi
			mov     esi, edi
			rep     stosd
			mov  [esi], edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__last_level], eax
			mov     ds:[edi+D__Level_], eax
			mov     ds:[edi+D__free_perk], al
			mov     ds:[edi+D__curr_pc_stat], eax
			mov     ds:[edi+D__sneak_working], eax
			mov     esi, ds:[edi+D__game_global_vars]
			pop     edi
			add     esi, 189*4                           // esi->NUKA_COLA_ADDICT
			push    esi
			push    esi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			lea     edi, [esi+real_drug_gvar]
			pop     esi
			mov     ecx, 6
			push    ecx
			rep     movsd
			pop     ecx
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edx, [edi+D__drug_pid]
			pop     edi
			mov     esi, ebx                             // _obj_dude
		loopDrug:
			mov     eax, [edx]                           // eax = drug_pid
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_d_check_addict_]
			call    edi
			pop     edi
			test    eax, eax                             // Is there an addiction?
			jz      noAddict                             // No
			xor     eax, eax
			inc     eax
		noAddict:
			mov     [edi], eax
			add     edx, 4
			add     edi, 4
			loop    loopDrug
			test    eax, eax                             // Are you addicted to alcohol (beer)?
			jnz     skipBooze                            // Yes
			mov     eax, [edx]                           // PID_BOOZE
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_d_check_addict_]
			call    edi
			pop     edi
			mov     [edi-4], eax
		skipBooze:
			mov     ecx, ebx                              // eax = _obj_dude
			xor     eax, eax
			dec     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_d_check_addict_]
			call    edi
			pop     edi
			mov     edx, 4
			test    eax, eax
			mov     eax, edx
			jz      unsetAddict
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_pc_flag_on_]
			call    edi
			pop     edi
			jmp     setAddict
		unsetAddict:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_pc_flag_off_]
			call    edi
			pop     edi
		setAddict:
			push    edx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     eax, [edi+real_tag_skill]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_skill_get_tags_]
			call    edi
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [esi+D__tag_skill]
			pop     esi
			pop     ecx
			xor     eax, eax
			dec     eax
			rep     stosd
			mov     edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_trait_set_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__Experience_], eax
			pop     edi
			// get active hand by weapon anim code
			mov     edx, [ebx+0x20]                      // fid
			and     edx, 0x0F000
			sar     edx, 0xC                             // edx = current weapon anim code as seen in hands
			xor     ecx, ecx                             // Left hand
			mov     eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_right_hand_]
			call    edi
			pop     edi
			test    eax, eax                             // Is there an item in your right hand?
			jz      checkAnim                            // No
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_weapon
			pop     eax
			jne     setActiveHand                        // No
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_anim_code_]
			call    edi
			pop     edi
		checkAnim:
			cmp     eax, edx                             // Is the animation the same?
			jne     setActiveHand                        // No
			inc     ecx                                  // Right hand
		setActiveHand:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__itemCurrentItem], ecx
			pop     edi
			mov     eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_right_hand_]
			call    edi
			pop     edi
			test    eax, eax                             // Is there an item in your right hand?
			jz      noRightHand                          // No
			push    eax
			call    PartyControl_CanUseWeapon
			test    eax, eax
			pop     eax
			jnz     noRightHand
			and     byte ptr [eax+0x27], 0xFD            // Reset the item flag in the right hand
		noRightHand:
			xchg    ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_left_hand_]
			call    edi
			pop     edi
			test    eax, eax                             // Have an item in your left hand?
			jz      noLeftHand                           // No
			push    eax
			call    PartyControl_CanUseWeapon
			test    eax, eax
			pop     eax
			jnz     noLeftHand
			and     byte ptr [eax+0x27], 0xFE            // Reset the item flag in the left hand
		noLeftHand:
			popad
			ret
	}
}

// restore dude state
__declspec(naked) void RestoreDudeState(void) {
	__asm {
			pushad
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			lea     eax, [esi+real_pc_name]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_pc_set_name_]
			call    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, [esi+real_last_level]
			mov     ds:[edi+D__last_level], eax
			mov     eax, [esi+real_Level]
			mov     ds:[edi+D__Level_], eax
			mov     eax, [esi+real_map_elevation]
			mov     ds:[edi+D__map_elevation], eax
			mov     eax, [esi+real_Experience]
			mov     ds:[edi+D__Experience_], eax
			pop     edi
			pop     esi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     esi, [edi+real_drug_gvar]
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__game_global_vars]
			pop     esi
			add     edi, 189*4                           // esi->NUKA_COLA_ADDICT
			mov     ecx, 6
			rep     movsd
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     esi, [edi+real_perk_lev]
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [esi+D__perk_lev]
			pop     esi
			mov     ecx, PERK_count
			rep     movsd
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     esi, [edi+real_itemButtonItems]
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [esi+D__itemButtonItems]
			pop     esi
			mov     ecx, 6*2
			rep     movsd
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, [esi+real_free_perk]
			mov     ds:[edi+D__free_perk], al
			mov     eax, [esi+real_unspent_skill_points]
			mov     ds:[edi+D__curr_pc_stat], eax
			mov     edx, 4
			lea     eax, [esi+real_tag_skill]
			pop     edi
			pop     esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_skill_set_tags_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     edx, [edi+real_trait2]
			mov     eax, [edi+real_trait]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_trait_set_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     ecx, [edi+real_dude]
			pop     edi
			push    ecx
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__obj_dude], ecx
			mov     ds:[edi+D__inven_dude], ecx
			mov     ds:[edi+D__stack], ecx
			mov     eax, [ecx+0x64]
			mov     ds:[edi+D__inven_pid], eax
			mov     eax, [esi+real_hand]
			mov     ds:[edi+D__itemCurrentItem], eax
			mov     eax, [esi+real_sneak_working]
			mov     ds:[edi+D__sneak_working], eax
			mov     eax, [esi+real_bbox_sneak]
			mov     ds:[edi+D__bbox+12+8], eax
			xor     eax, eax
			mov     [esi+IsControllingNPC], eax
			pop     edi
			pop     esi
			dec     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_d_check_addict_]
			call    edi
			pop     edi
			test    eax, eax
			mov     eax, 4
			jz      unsetAddict
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_pc_flag_on_]
			call    edi
			pop     edi
			jmp     skip
		unsetAddict:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_pc_flag_off_]
			call    edi
			pop     edi
		skip:
			pop     edx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     eax, [edi+real_sneak_queue_time]
			pop     edi
			test    eax, eax
			jz      noSneak
			mov     ecx, 10
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_queue_add_]
			call    edi
			pop     edi
		noSneak:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     eax, [edi+DelayedExperience]
			pop     edi
			test    eax, eax
			jz      end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_pc_add_experience_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+DelayedExperience], eax
			pop     edi
		end:
			popad
			ret
	}
}

__declspec(naked) static void CombatWrapper_v2(void) {
	__asm {
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     eax, ds:[edi+D__obj_dude]
			pop     edi
			jne     skip
			xor     edx, edx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+_combatNumTurns], edx
			pop     edi
			je      skipControl                          // This is the first move
			mov     eax, [eax+0x4]                       // tile_num
			add     edx, 3
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_tile_scroll_to_]
			call    edi
			pop     edi
			jmp     skipControl
		skip:
			push    eax
			call    IsInPidList
			and     eax, 0xFF
			test    eax, eax
			jz      skipControl
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+Mode], eax                      // control all critters?
			pop     edi
			je      npcControl
			popad
			pushad
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_isPartyMember_]
			call    edi
			pop     edi
			test    eax, eax
			jnz     npcControl
		skipControl:
			popad

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_turn_]
			mov     [esp+4], edi
			pop     edi
			ret

		npcControl:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+IsControllingNPC], eax          // if game was loaded during turn, PartyControlExit()
			pop     edi
			popad
			pushad
			xchg    ebx, eax                             // ebx = npc
			call    SaveDudeState
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_redraw_]
			call    edi
			pop     edi
			mov     eax, [ebx+0x4]                       // tile_num
			mov     edx, 3
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_tile_scroll_to_]
			call    edi
			pop     edi
			xchg    ebx, eax                             // eax = npc
			xor     edx, edx
			push    edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_turn_]
			call    edi
			pop     edi
			pop     ecx
			xchg    ecx, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax          // if game was loaded during turn, PartyControlExit()
			pop     edi
			je      skipRestore                          // was called and already restored state
			call    RestoreDudeState
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_redraw_]
			call    edi
			pop     edi
		skipRestore:
			test    ecx, ecx                             // Normal end of turn?
			popad
			jz      end                                  // Yes
			// exit/load/escape/death
			test    byte ptr [eax+0x44], 0x80            // DAM_DEAD
			jnz     end
			xor     eax, eax
			dec     eax
			ret

		end:
			xor     eax, eax
			ret
	}
}

// hack to exit from this function safely when you load game during NPC turn
__declspec(naked) static void combat_add_noncoms_hook(void) {
	__asm {
			call    CombatWrapper_v2
			inc     eax
			jnz     end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__list_com], eax
			pop     edi
			mov     ecx, ebp
		end:
			ret
	}
}

__declspec(naked) static void stat_pc_min_exp_hook(void) {
	__asm {
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      end
			dec     eax
			ret

		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_pc_min_exp_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void inven_pickup_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			test    eax, eax                             // Is it item_type_armor?
			jnz     end                                  // No
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      end
			dec     eax
		end:
			ret
	}
}

__declspec(naked) static void handle_inventory_hook(void) {
	__asm {
			xor     ebx, ebx
			mov     edx, eax                             // edx = _inven_dude
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_worn_]
			call    edi
			pop     edi
			test    eax, eax
			jz      end
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], ebx
			pop     edi
			je      end
			push    eax
			push    edx
			inc     ebx
			xchg    edx, eax                             // eax = source, edx = armore
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_remove_mult_]
			call    edi
			pop     edi
			pop     edx                                  // edx = source
			pop     ebx                                  // ebx = armor
			inc     eax                                  // Removed?
			jnz     skip                                 // Yes
			// We couldn’t remove it, so we will remove the armor, taking into account the decrease in KB
			push    edx
			push    eax
			xchg    ebx, eax                             // ebx = newarmor, eax = oldarmor
			xchg    edx, eax                             // edx = oldarmor, eax = source
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_adjust_ac_]
			call    edi
			pop     edi
			pop     ebx
			pop     edx
		skip:
			xchg    ebx, eax                             // eax = armor
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+HiddenArmor], eax
			pop     edi
		end:
			ret
	}
}

__declspec(naked) static void handle_inventory_hook1(void) {
	__asm {
			xor     edx, edx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], edx
			pop     edi
			je      end
			push    eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     edx, [edi+HiddenArmor]
			pop     edi
			test    edx, edx
			jz      skip
			or      byte ptr [edx+0x27], 4               // Set the armor flag
			xor     ebx, ebx
			inc     ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_add_force_]
			call    edi
			pop     edi
			xor     edx, edx
		skip:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+HiddenArmor], edx
			pop     edi
			pop     eax
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_worn_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void switch_hand_hook(void) {
	__asm {
			call    PartyControl_CanUseWeapon
			dec     eax
			jz      end
			pop     esi                                  // Destroying the return address
			pop     ebp
			pop     edi
			pop     esi
			ret

		end:
			mov     esi, ebx
			cmp     [edx], eax
			ret
	}
}

__declspec(naked) static void combat_input_hook(void) {
	__asm {
			xor     ebx, ebx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], ebx
			pop     edi
			je      end
			cmp     eax, 0xD                             // Enter (end of battle)?
			jne     end                                  // No
			mov     eax, 0x20                            // Space (end of turn)
		end:
			mov     ebx, eax
			cmp     eax, 0x20                            // Space (end of turn)?
			ret
	}
}

__declspec(naked) static void action_skill_use_hook(void) {
	__asm {
			cmp     eax, SKILL_SNEAK
			jne     skip
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      end
		skip:
			pop     eax                                  // Destroying the return address
			xor     eax, eax
			dec     eax
		end:
			ret
	}
}

__declspec(naked) static void action_use_skill_on_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			jne     end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_pc_flag_toggle_]
			call    edi
			pop     edi
		end:
			ret
	}
}

__declspec(naked) static void damage_object_hook(void) {
	__asm {
			push    ecx
			xor     ecx, ecx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], ecx
			pop     edi
			je      skip
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     edx, ds:[edi+D__obj_dude]
			pop     edi
			jne     skip
			mov     ecx, edx
			call    RestoreDudeState
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_redraw_]
			call    edi
			pop     edi
			pop     eax
		skip:
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_scr_set_objs_]
			call    edi
			pop     edi
			pop     eax
			mov     edx, destroy_p_proc
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_exec_script_proc_]
			call    edi
			pop     edi
			jecxz   end
			inc     ebx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+IsControllingNPC], ebx
			pop     edi
			mov     ebx, ecx
			call    SaveDudeState
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_redraw_]
			call    edi
			pop     edi
		end:
			pop     ecx
			pop     ebx                                  // Destroying the return address

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x228F6]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void op_give_exp_points_hook(void) {
	__asm {
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      skip
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			add     [edi+DelayedExperience], ecx
			pop     edi
			ret

		skip:
			xchg    ecx, eax

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_pc_add_experience_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void adjust_fid_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edx, ds:[edi+D__i_worn]
			pop     edi
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      skip
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     eax, [edi+HiddenArmor]
			pop     edi
			test    eax, eax
			jz      skip
			xchg    edx, eax
		skip:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x650E5]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void refresh_box_bar_win_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			jne     end

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_is_pc_flag_]
			mov     [esp+4], edi
			pop     edi
			ret

		end:
			inc     eax
			ret
	}
}

void PartyControlInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ControlCombat", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, Mode) = (uint32_t) temp_int;
	if ((1 == temp_int) || (2 == temp_int)) {
		char pidbuf[512];
		pidbuf[511] = 0;
		NameBox = malloc(2730 * sizeof(*NameBox));

		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ControlCombatPIDList", "", pidbuf, &sfall1_ini_info_G);
		if (0 != strcmp(pidbuf, "")) {
			char *ptr = pidbuf;
			char *comma = NULL;

			while (true) {
				comma = strchr(ptr, ',');
				if (NULL == comma) {
					break;
				}
				*comma = 0;
				if (strlen(ptr) > 0) {
					size_t i = 0;
					uint16_t temp_uint16 = '\0';
					for (i = 0; i < (sizeof(Chars) / 2); ++i) {
						if (0 == GET_BD_ARR(uint16_t *, Chars)[i]) {
							sscanf(prop_value, "%hu", &temp_uint16);
							GET_BD_ARR(uint16_t *, Chars)[i] = temp_uint16;
						}
					}
				}
				ptr = comma + 1;
			}
			if (strlen(ptr) > 0) {
				size_t i = 0;
				uint16_t temp_uint16 = '\0';
				for (i = 0; i < (sizeof(Chars) / 2); ++i) {
					if (0 == GET_BD_ARR(uint16_t *, Chars)[i]) {
						sscanf(prop_value, "%hu", &temp_uint16);
						GET_BD_ARR(uint16_t *, Chars)[i] = temp_uint16;
					}
				}
			}
		}
		hookCallEXE(0x20351, &combat_add_noncoms_hook);
		hookCallEXE(0x20D67, &CombatWrapper_v2);
		hookCallEXE(0x2ED6F, &stat_pc_min_exp_hook);// PrintLevelWin_
		hookCallEXE(0x339C3, &stat_pc_min_exp_hook);// Save_as_ASCII_
		hookCallEXE(0x64D7D, &inven_pickup_hook);
		hookCallEXE(0x62813, &handle_inventory_hook);
		hookCallEXE(0x62A2B, &handle_inventory_hook1);
		makeCallEXE(0x64E93, &switch_hand_hook, false);
		writeMem32EXE(0x655DF + 1, 152, true);               // Text width 152, not 80
		makeCallEXE(0x20833, &combat_input_hook, false);
		makeCallEXE(0x1234C, &action_skill_use_hook, false);
		hookCallEXE(0x12603, &action_use_skill_on_hook);
		hookCallEXE(0x228E4, &damage_object_hook);
		hookCallEXE(0x4BA12, &op_give_exp_points_hook);
		makeCallEXE(0x650DF, &adjust_fid_hook, true);
		hookCallEXE(0x56D32, &refresh_box_bar_win_hook);
	}
}

void PartyControlExit(void) {
	free(NameBox);
	GET_BD_SYM(uint8_t *, NameBox) = NULL;
}
