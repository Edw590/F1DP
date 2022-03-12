/*
 *    sfall
 *    Copyright (C) 2013  The sfall team, 2022 DADi590
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

// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "Define.h"
#include "FalloutEngine.h"
#include "PartyControl.h"
#include "SFall1Main.h"
#include "SFall1Patches.h"
#include <stddef.h>

uint32_t IsControllingNPC = 0;
uint32_t HiddenArmor = 0;

// todo All the code is supposedly ready. But the DOS version can't handle it. Too many global data declared and an
// error appears: "Exit to error: CPU_SetSegGeneral: Stack segment beyond limits". So I think it's too many declarations
// and/or too big declarations. Not sure what to do about this. If it's this the problem, then it means I'll run out of
// memory... Not good. I thought it could handle it normally.
// And I think I can't change how much space the stack segment has (which is the data segment anyway) without messing
// with the fixups table, and for that I need to move the EXE contents all down --> not cool. If it's only this feature
// that goes away, doesn't seem bad. It's not an important feature anyway IMO.

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
static uint32_t real_itemButtonItems[(6 * 4) * 2] = {0};
static uint32_t real_drug_gvar[6] = {0};

static void __declspec(naked) PartyControl_CanUseWeapon(void) {
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
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    eax                                  // ID3=Direction code
			mov     eax, ObjType_Critter
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
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
			retn
	}
}

static bool __stdcall IsInPidList(uint32_t const *npc) {
	size_t i = 0;
	uint32_t pid = npc[0x64u / 4u] & 0xFFFFFFu;

	for (i = 0; i < (sizeof(Chars) / 2); ++i) {
		if (0 == ((uint16_t *) getRealBlockAddrData(Chars))[i]) {
			break;
		}
	}
	if (1 == i) { // Then size is 0 (checked the 1st index which was 0, incremented i and left)
		return true;
	}


	for (i = 0; i < (sizeof(Chars) / 2); ++i) {
		if (pid == ((uint16_t *) getRealBlockAddrData(Chars))[i]) {
			return true;
		}
	}

	return false;
}

// save "real" dude state
static void __declspec(naked) SaveDudeState(void) {
	__asm {
			push    edi
			push    esi
			push    edx
			push    ecx
			push    ebx
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
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_name_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_pc_set_name_]
			call    edi
			pop     edi
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
			mov     ecx, (6*4)*2
			rep     movsd
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     esi, [edi+D__perk_lev]
			pop     edi
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
			rep     stosd
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
			test    eax, eax                             // Åñòü çàâèñèìîñòü?
			jz      noAddict                             // Íåò
			xor     eax, eax
			inc     eax
		noAddict:
			mov     [edi], eax
			add     edx, 4
			add     edi, 4
			loop    loopDrug
			test    eax, eax                             // Åñòü çàâèñèìîñòü ê àëêîãîëþ (ïèâî)?
			jnz     skipBooze                            // Äà
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
			xor     ecx, ecx                             // Ëåâàÿ ðóêà
			mov     eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_right_hand_]
			call    edi
			pop     edi
			test    eax, eax                             // Åñòü âåùü â ïðàâîé ðóêå?
			jz      setActiveHand                        // Íåò
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_weapon
			pop     eax
			jne     setActiveHand                        // Íåò
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_anim_code_]
			call    edi
			pop     edi
			cmp     eax, edx                             // Àíèìàöèÿ îäèíàêîâàÿ?
			jne     setActiveHand                        // Íåò
			inc     ecx                                  // Ïðàâàÿ ðóêà
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
			test    eax, eax                             // Åñòü âåùü â ïðàâîé ðóêå?
			jz      noRightHand                          // Íåò
			push    eax
			call    PartyControl_CanUseWeapon
			test    eax, eax
			pop     eax
			jnz     noRightHand
			and     byte ptr [eax+0x27], 0xFD            // Ñáðàñûâàåì ôëàã âåùè â ïðàâîé ðóêå
		noRightHand:
			xchg    ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_left_hand_]
			call    edi
			pop     edi
			test    eax, eax                             // Åñòü âåùü â ëåâîé ðóêå?
			jz      noLeftHand                           // Íåò
			push    eax
			call    PartyControl_CanUseWeapon
			test    eax, eax
			pop     eax
			jnz     noLeftHand
			and     byte ptr [eax+0x27], 0xFE            // Ñáðàñûâàåì ôëàã âåùè â ëåâîé ðóêå
		noLeftHand:
			pop     ebx
			pop     ecx
			pop     edx
			pop     esi
			pop     edi
			retn
	}
}

// restore dude state
static void __declspec(naked) RestoreDudeState(void) {
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
			mov     ecx, (6*4)*2
			rep     movsd
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
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
			jz      end
			mov     ecx, 10
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_queue_add_]
			call    edi
			pop     edi
		end:
			popad
			retn
	}
}

static void __declspec(naked) CombatWrapper_v2(void) {
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
			je      skipControl                          // Ýòî ïåðâûé õîä
			mov     eax, [eax+0x4]                       // tile_num
			add     edx, 2
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

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_turn_]
			mov     [esp+4], edi
			pop     edi
			retn

		npcControl:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+IsControllingNPC], eax          // if game was loaded during turn, PartyControlReset()
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
			mov     edx, 2
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
			cmp     [edi+IsControllingNPC], eax          // if game was loaded during turn, PartyControlReset()
			pop     edi
			je      skipRestore                          // was called and already restored state
			call    RestoreDudeState
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_redraw_]
			call    edi
			pop     edi
		skipRestore:
			test    ecx, ecx                             // Íîðìàëüíîå çàâåðøåíèå õîäà?
			popad
			jz      end                                  // Äà
			// âûõîä/çàãðóçêà/ïîáåã/ñìåðòü
			test    byte ptr [eax+0x44], 0x80            // DAM_DEAD
			jnz     end
			xor     eax, eax
			dec     eax
			retn
		end:
			xor     eax, eax
			retn
	}
}

// hack to exit from this function safely when you load game during NPC turn
static void __declspec(naked) combat_add_noncoms_hook(void) {
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
			retn
	}
}

static void __declspec(naked) stat_pc_min_exp_hook(void) {
	__asm {
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      end
			dec     eax
			retn
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_pc_min_exp_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) inven_pickup_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			test    eax, eax                             // Ýòî item_type_armor?
			jnz     end                                  // Íåò
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], eax
			pop     edi
			je      end
			dec     eax
		end:
			retn
	}
}

static void __declspec(naked) handle_inventory_hook(void) {
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
			cmp     [edi+IsControllingNPC], eax
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
			inc     eax                                  // Óäàëèëè?
			jnz     nextArmor                            // Äà
			// Íå ñìîãëè óäàëèòü, ïîýòîìó ñíèìåì áðîíþ ñ ó÷¸òîì óìåíüøåíèÿ ÊÁ
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
		nextArmor:
			mov     eax, edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_worn_]
			call    edi
			pop     edi
			test    eax, eax
			jz      noArmor
			and     byte ptr [eax+0x27], 0xFB            // Ñáðàñûâàåì ôëàã îäåòîé áðîíè
			jmp     nextArmor
		noArmor:
			xchg    ebx, eax                             // eax = armor
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+HiddenArmor], eax
			pop     edi
		end:
			retn
	}
}

static void __declspec(naked) handle_inventory_hook1(void) {
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
			or      byte ptr [edx+0x27], 4               // Óñòàíàâëèâàåì ôëàã îäåòîé áðîíè
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

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_worn_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) switch_hand_hook(void) {
	__asm {
			call    PartyControl_CanUseWeapon
			dec     eax
			jz      end
			pop     esi                                  // Óíè÷òîæàåì àäðåñ âîçâðàòà
			pop     ebp
			pop     edi
			pop     esi
			retn
		end:
			mov     esi, ebx
			cmp     [edx], eax
			retn
	}
}

static void __declspec(naked) combat_input_hook(void) {
	__asm {
			xor     ebx, ebx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+IsControllingNPC], ebx
			pop     edi
			je      end
			cmp     eax, 0xD                             // Enter (çàâåðøåíèå áîÿ)?
			jne     end                                  // Íåò
			mov     eax, 0x20                            // Space (îêîí÷àíèå õîäà)
		end:
			mov     ebx, eax
			cmp     eax, 0x20                            // Space (îêîí÷àíèå õîäà)?
			retn
	}
}

static void __declspec(naked) action_skill_use_hook(void) {
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
			pop     eax                                  // Óíè÷òîæàåì àäðåñ âîçâðàòà
			xor     eax, eax
			dec     eax
		end:
			retn
	}
}

static void __declspec(naked) action_use_skill_on_hook(void) {
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
			retn
	}
}

void PartyControlInit(void) {
	uint32_t temp_uint32 = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	//printf("\0\0\0\0\0\0\0\0\0\0"); // "Reason" on DllMain2()

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ControlCombat", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*(uint32_t *) getRealBlockAddrData(&Mode) = temp_uint32;
	if ((1 == temp_uint32) || (2 == temp_uint32)) {
		char pidbuf[512];
		pidbuf[511] = 0;

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
						if (0 == ((uint16_t *) getRealBlockAddrData(Chars))[i]) {
							sscanf(prop_value, "%hu", &temp_uint16);
							((uint16_t *) getRealBlockAddrData(Chars))[i] = temp_uint16;
						}
					}
				}
				ptr = comma + 1;
			}
			if (strlen(ptr) > 0) {
				size_t i = 0;
				uint16_t temp_uint16 = '\0';
				for (i = 0; i < (sizeof(Chars) / 2); ++i) {
					if (0 == ((uint16_t *) getRealBlockAddrData(Chars))[i]) {
						sscanf(prop_value, "%hu", &temp_uint16);
						((uint16_t *) getRealBlockAddrData(Chars))[i] = temp_uint16;
					}
				}
			}
		}
		HookCallEXE(0x20351, getRealBlockAddrCode((void *) &combat_add_noncoms_hook));
		HookCallEXE(0x20D67, getRealBlockAddrCode((void *) &CombatWrapper_v2));
		HookCallEXE(0x2ED6F, getRealBlockAddrCode((void *) &stat_pc_min_exp_hook));// PrintLevelWin_
		HookCallEXE(0x339C3, getRealBlockAddrCode((void *) &stat_pc_min_exp_hook));// Save_as_ASCII_
		HookCallEXE(0x64D7D, getRealBlockAddrCode((void *) &inven_pickup_hook));
		HookCallEXE(0x62813, getRealBlockAddrCode((void *) &handle_inventory_hook));
		HookCallEXE(0x62A2B, getRealBlockAddrCode((void *) &handle_inventory_hook1));
		MakeCallEXE(0x64E93, getRealBlockAddrCode((void *) &switch_hand_hook), false);
		writeMem32EXE(0x655DF+1, 152);               // Text width 152, not 80
		MakeCallEXE(0x20833, getRealBlockAddrCode((void *) &combat_input_hook), false);
		MakeCallEXE(0x1234C, getRealBlockAddrCode((void *) &action_skill_use_hook), false);
		HookCallEXE(0x12603, getRealBlockAddrCode((void *) &action_use_skill_on_hook));
	}
}

void __stdcall PartyControlReset(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     dword ptr [edi+IsControllingNPC], 0
			pop     edi
			je      end
			call    RestoreDudeState
		end:
	}
}
