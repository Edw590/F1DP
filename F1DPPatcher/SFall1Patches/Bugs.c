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

#include "../GameAddrs/CStdFuncs.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "Bugs.h"
#include "Define.h"
#include "PartyControl.h"
#include "SFall1Main.h"

uint32_t WeightOnBody = 0;

static const uint32_t QWordToDWord[4] = {
		0x5E39B, 0x5E3E3, 0x5E4AB, 0x5E526,
};

__declspec(naked) static void determine_to_hit_func_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]               // Perception
			call    edi
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			cmp     edi, ds:[esi+D__obj_dude]
			pop     esi
			jne     end
			mov     ecx, PERK_sharpshooter
			xchg    ecx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			shl     eax, 1
			add     eax, ecx
		end:
			retn
	}
}

__declspec(naked) static void perform_withdrawal_start_hook(void) {
	__asm {
			test    eax, eax
			jnz     end
			retn

		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_print_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void pipboy_hook(void) {
	__asm {
			cmp     ebx, 0x20E                           // BACK button?
			je      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     byte ptr ds:[edi+D__holo_flag], 0
			pop     edi
			jne     end
			xor     ebx, ebx                             // No person - no problem (c) :-p
		end:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__crnt_func]
			pop     edi
			retn
	}
}

__declspec(naked) static void PipAlarm_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__crnt_func], eax
			pop     edi
			mov     eax, 0x400
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_PipStatus_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     eax, [edi+0xFB910]                  // 'iisxxxx1'
			pop     edi
			retn
	}
}

__declspec(naked) static void scr_save_hook(void) {
	__asm {
			mov     ecx, 16
			cmp     [esp+0xDC+4], ecx                    // number_of_scripts
			jg      skip
			mov     ecx, [esp+0xDC+4]
			cmp     ecx, 0
			jg      skip
			xor     eax, eax
			retn

		skip:
			sub     [esp+0xDC+4], ecx                    // number_of_scripts
			push    dword ptr [ebp+0xD00]                // num
			mov     [ebp+0xD00], ecx                     // num
			xor     ecx, ecx
			xchg    [ebp+0xD04], ecx                     // NextBlock
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_scr_write_ScriptNode_]
			call    edi
			pop     edi
			xchg    [ebp+0xD04], ecx                     // NextBlock
			pop     dword ptr [ebp+0xD00]                // num
			retn
	}
}

__declspec(naked) static void item_d_check_addict_hook(void) {
	__asm {
			xor     edx, edx
			inc     edx
			inc     edx                                  // type = addiction
			inc     eax
			test    eax, eax                             // There is drug_pid?
			jz      skip                                 // No
			dec     eax
			xchg    ebx, eax                             // ebx = drug_pid
			mov     eax, esi                             // eax = source
			call    queue_find_first_
		loopQueue:
			test    eax, eax                             // Is there anything on the list?
			jz      end                                  // No
			cmp     ebx, [eax+0x4]                       // drug_pid == queue_addict.drug_pid?
			je      end                                  // There is a specific relationship
			mov     eax, esi                             // eax = source
			call    queue_find_next_
			jmp     loopQueue
		skip:
			xchg    ecx, eax                             // eax = _obj_dude
			call    queue_find_first_
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x6CA8D]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void queue_clear_type_hook(void) {
	__asm {
			mov     ebx, [esi]

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mem_free_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void invenWieldFunc_hook(void) {
	__asm {
			pushad
			mov     edi, ecx
			mov     edx, esi
			xor     ebx, ebx
			inc     ebx
			push    ebx
			mov     cl, [edi+0x27]
			and     cl, 0x3
			xchg    edx, eax                             // eax = source, edx = item
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_remove_mult_]
			call    edi
			pop     edi
			xchg    ebx, eax
			mov     eax, esi
			test    cl, 0x2                              // Right hand?
			jz      leftHand                             // No
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_right_hand_]
			call    edi
			pop     edi
			jmp     removeFlag
		leftHand:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_left_hand_]
			call    edi
			pop     edi
		removeFlag:
			test    eax, eax
			jz      noWeapon
			and     byte ptr [eax+0x27], 0xFC            // Resetting the weapon flag in hand
		noWeapon:
			or      [edi+0x27], cl                       // Set the weapon flag in hand
			inc     ebx
			pop     ebx
			jz      skip
			xchg    esi, eax
			mov     edx, edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_add_force_]
			call    edi
			pop     edi
		skip:
			popad

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void inven_item_wearing(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     esi, ds:[edi+D__inven_dude]
			pop     edi
			xchg    ebx, eax                             // ebx = source
			mov     eax, [esi+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // Is it ObjType_Item?
			jnz     skip                                 // No
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			dec     eax                                  // Bag backpack?
			jnz     skip                                 // No
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_top_environment_]
			call    edi
			pop     edi
			test    eax, eax                             // Is there an owner?
			jz      skip                                 // No
			mov     ecx, [eax+0x20]
			and     ecx, 0xF000000
			sar     ecx, 0x18
			cmp     ecx, ObjType_Critter                 // Is it a character?
			jne     skip                                 // No
			cmp     eax, ebx                             // bag owner == source?
			je      end                                  // Yes
		skip:
			xchg    ebx, eax
			cmp     eax, esi
		end:
			retn
	}
}

__declspec(naked) static void inven_right_hand_hook(void) {
	__asm {
			call    inven_item_wearing
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x6538D]
			pop     edi
			jne     end
			xchg    edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x653A2]
			pop     edi
		end:
			jmp     esi
	}
}

__declspec(naked) static void inven_left_hand_hook(void) {
	__asm {
			call    inven_item_wearing
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x653CD]
			pop     edi
			jne     end
			xchg    edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x653E2]
			pop     edi
		end:
			jmp     esi
	}
}

__declspec(naked) static void inven_worn_hook(void) {
	__asm {
			call    inven_item_wearing
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x6540D]
			pop     edi
			jne     end
			xchg    edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x65422]
			pop     edi
		end:
			jmp     esi
	}
}

__declspec(naked) static void loot_container_hook(void) {
	__asm {
			mov     eax, [esp+0x110+0x4]
			test    eax, eax
			jz      noArmor
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
		noArmor:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+WeightOnBody], eax
			pop     edi
			mov     eax, [esp+0x114+0x4]
			test    eax, eax
			jz      noLeftWeapon
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
		noLeftWeapon:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			add     [edi+WeightOnBody], eax
			pop     edi
			mov     eax, [esp+0x118+0x4]
			test    eax, eax
			jz      noRightWeapon
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
		noRightWeapon:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			add     [edi+WeightOnBody], eax
			pop     edi
			xor     eax, eax
			inc     eax
			inc     eax
			retn
	}
}

__declspec(naked) static void inven_pickup_hook(void) {
	__asm {
			test    eax, eax
			jz      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_wid]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_GNW_find_]
			call    edi
			pop     edi
			mov     ecx, [eax+0x8+0x4]                   // ecx = _i_wid.rect.y
			mov     eax, [eax+0x8+0x0]                   // eax = _i_wid.rect.x
			add     eax, 44                              // x_start
			mov     ebx, 64
			add     ebx, eax                             // x_end
			xor     edx, edx
		next:
			push    eax
			push    edx
			push    ecx
			push    ebx
			imul    edx, edx, 48
			add     edx, 35
			add     edx, ecx                             // y_start
			mov     ecx, edx
			add     ecx, 48                              // y_end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mouse_click_in_]
			call    edi
			pop     edi
			pop     ebx
			pop     ecx
			pop     edx
			test    eax, eax
			pop     eax
			jnz     found
			inc     edx
			cmp     edx, 6
			jb      next
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x64C46]
			mov     [esp+4], edi
			pop     edi
			retn

		found:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     ebx, [edi+0x64BCA]
			pop     edi
			add     edx, [esp+0x3C]                      // inventory_offset
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__pud]
			pop     edi
			mov     ecx, [eax]                           // itemsCount
			jecxz   skip
			dec     ecx
			cmp     edx, ecx
			ja      skip
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     ebx, [edi+0x64B6C]
			pop     edi
		skip:
			jmp     ebx
	}
}

__declspec(naked) static void drop_ammo_into_weapon_hook(void) {
	__asm {
			push    ecx
			mov     esi, ecx
			dec     esi
			test    esi, esi                             // One box of ammo?
			jz      skip                                 // Yes
			xor     esi, esi
			// Extra check for from_slot, but let it be
			cmp     edi, 1006                            // Arms?
			jge     skip                                 // Yes
			lea     edx, [eax+0x2C]                      // Inventory
			mov     ecx, [edx]                           // itemsCount
			jcxz    skip                                 // inventory is empty (another extra check, but let it be)
			mov     edx, [edx+8]                         // FirstItem
		nextItem:
			cmp     ebp, [edx]                           // Our weapons?
			je      foundItem                            // Yes
			add     edx, 8                               // To the next
			loop    nextItem
			jmp     skip                                 // Our weapons are not in inventory
		foundItem:
			cmp     dword ptr [edx+4], 1                 // Weapon in a single copy?
			jg      skip                                 // No
			lea     edx, [eax+0x2C]                      // Inventory
			mov     edx, [edx]                           // itemsCount
			sub     edx, ecx                             // edx=weapon slot serial number
			lea     ecx, [edi-1000]                      // from_slot
			add     ecx, [esp+0x3C+4+0x24+8]             // ecx=serial number of the slot with cartridges
			cmp     edx, ecx                             // Weapon after ammo?
			jg      skip                                 // Yes
			inc     esi                                  // No, you need to change from_slot
		skip:
			pop     ecx
			mov     edx, ebp
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_remove_mult_]
			call    edi
			pop     edi
			test    eax, eax                             // Have you removed the weapon from your inventory?
			jnz     end                                  // No
			sub     [esp+0x24+4], esi                    // Yes, we correct from_slot
		end:
			retn
	}
}

__declspec(naked) static void PipStatus_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_AddHotLines_]
			call    edi
			pop     edi
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__hot_line_count], eax
			pop     edi
			retn
	}
}

//checks if an attacked object is a critter before attempting dodge animation
__declspec(naked) static void action_melee_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edx, [edi+0x112EC]
			pop     edi
			mov     ebx, [eax+0x20]                      // pobj.fid
			and     ebx, 0x0F000000
			sar     ebx, 0x18
			cmp     ebx, ObjType_Critter                 // check if object FID type flag is set to critter
			jne     end                                  // if object not a critter skip dodge animation
			test    byte ptr [eax+0x44], 0x3             // (DAM_KNOCKED_OUT or DAM_KNOCKED_DOWN)?
			jnz     end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edx, [edi+0x1130E]
			pop     edi
		end:
			jmp     edx
	}
}

__declspec(naked) static void action_ranged_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edx, [edi+0x119F1]
			pop     edi
			mov     ebx, [eax+0x20]                      // pobj.fid
			and     ebx, 0x0F000000
			sar     ebx, 0x18
			cmp     ebx, ObjType_Critter                 // check if object FID type flag is set to critter
			jne     end                                  // if object not a critter skip dodge animation
			test    byte ptr [eax+0x44], 0x3             // (DAM_KNOCKED_OUT or DAM_KNOCKED_DOWN)?
			jnz     end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edx, [edi+0x11A56]
			pop     edi
		end:
			jmp     edx
	}
}


static uint32_t XPWithSwiftLearner = 0;
__declspec(naked) static void stat_pc_add_experience_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+XPWithSwiftLearner], esi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__Experience_]
			pop     edi
			retn
	}
}

__declspec(naked) static void combat_give_exps_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_pc_add_experience_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     ebx, [edi+XPWithSwiftLearner]
			pop     edi
			retn
	}
}

__declspec(naked) static void loot_container_hook1(void) {
	__asm {
			xchg    edi, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_pc_add_experience_]
			call    edi
			pop     edi
			cmp     edi, 1
			jne     skip
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]

			lea     esp, [esp-4] // [DADi590] Reserve space for the push
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, [edi+XPWithSwiftLearner]
			mov     [esp+4], edi
			pop     edi

			mov     ebx, [esp+0xF4]
			push    ebx
			lea     eax, [esp+0x8]
			push    eax
			mov     [esp+5*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			add     esp, 0xC
			pop     edi
			mov     eax, esp
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_print_]
			call    edi
			pop     edi
		skip:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x6782A]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void set_new_results_hook(void) {
	__asm {
			test    ah, 0x1                              // DAM_KNOCKED_OUT?
			jz      end                                  // No
			mov     eax, esi
			xor     edx, edx
			inc     edx                                  // type = blackout
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_queue_remove_this_]      // Remove the disconnect from the queue (if there is a disconnect)
			call    edi
			pop     edi
			ret

		end:
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x22821]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void critter_wake_clear_hook(void) {
	__asm {
			test    dl, 0x80                             // DAM_DEAD?
			jnz     end                                  // This is a corpse
			and     dl, 0xFE                             // Reset DAM_KNOCKED_OUT
			or      dl, 0x2                              // Set DAM_KNOCKED_DOWN
			mov     [esi+0x44], dl
		end:
			xor     eax, eax
			inc     eax
			pop     esi
			pop     ecx
			pop     ebx
			retn
	}
}

__declspec(naked) static void obj_load_func_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edx, [edi+0x7ACA7]
			pop     edi
			test    byte ptr [eax+0x25], 0x4             // Temp_
			jnz     end
			mov     edi, [eax+0x64]
			shr     edi, 0x18
			dec     edi                                  // ObjType_Critter?
			jnz     end                                  // No
			test    byte ptr [eax+0x44], 0x2             // DAM_KNOCKED_DOWN?
			jz      clear                                // No
			pushad
			xor     ecx, ecx
			inc     ecx
			xor     ebx, ebx
			xor     edx, edx
			xchg    edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_queue_add_]
			call    edi
			pop     edi
			popad
		clear:
			and     word ptr [eax+0x44], 0x7FFD          // not (DAM_LOSE_TURN or DAM_KNOCKED_DOWN)
		end:
			jmp     edx
	}
}

__declspec(naked) static void partyMemberPrepLoad_hook(void) {
	__asm {
			test    byte ptr [ecx+0x44], 0x2             // DAM_KNOCKED_DOWN
			jz      skip
			mov     eax, ecx
			mov     edx, [ecx+0x1C]
			xor     ebx, ebx
			dec     ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_dude_stand_]
			call    edi
			pop     edi
		skip:
			and     word ptr [ecx+0x44], 0x7FFD          // not (DAM_LOSE_TURN or DAM_KNOCKED_DOWN)
			xor     edx, edx
			mov     ebx, [ecx+0x2C]
			retn
	}
}

__declspec(naked) static void combat_ctd_init_hook(void) {
	__asm {
			mov     [esi+0x24], eax                      // ctd.targetTile
			mov     eax, [ebx+0x54]                      // pobj.who_hit_me
			inc     eax
			jnz     end
			mov     [ebx+0x54], eax                      // pobj.who_hit_me
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x20E01]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void obj_save_hook(void) {
	__asm {
			inc     eax
			jz      end
			dec     eax
			mov     edx, [esp+0x1C]                      // combat_data
			mov     eax, [eax+0x68]                      // pobj.who_hit_me.cid
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1 // In battle?
			pop     edi
			jz      clear                                // No
			cmp     dword ptr [edx], 0                   // In battle?
			jne     skip                                 // Yes
		clear:
			xor     eax, eax
			dec     eax
		skip:
			mov     [edx+0x18], eax                      // combat_data.who_hit_me
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x7B1D2]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void explode_critter_kill(void) {
	__asm {
			push    edx
			push    ecx
			push    ebx
			push    eax
			mov     edx, [eax+0x54]                      // pobj.who_hit_me
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
		skip:
			mov     eax, [eax+0x78]                      // pobj.sid
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_scr_set_objs_]
			call    edi
			pop     edi
			pop     eax                                  // pobj.sid
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
		end:
			pop      eax
			pop      ebx
			pop      ecx
			pop      edx

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_kill_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

__declspec(naked) static void action_use_an_item_on_object_hook(void) {
	__asm {
			test    ebp, ebp
			jz      end
			pop     eax                                  // Destroying the return address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     eax, [edi+0x11EA4]
			pop     edi
			push    eax
		end:
			retn
	}
}

__declspec(naked) static void use_inventory_on_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1
			pop     edi
			jz      use                                  // Not in battle
			cmp     dword ptr [eax+0x40], 2              // curr_mp
			jl      end                                  // Not enough action points
			push    eax
			call    FakeCombatFix3                       // analogue of calling action_use_an_item_on_object_
			cmp     eax, -1
			pop     eax
			je      end
			push    ecx
			mov     ecx, [eax+0x40]                      // curr_mp
			dec     ecx
			dec     ecx
			test    ecx, ecx
			jge     skip
			xor     ecx, ecx
		skip:
			mov     [eax+0x40], ecx                      // curr_mp
			xchg    ecx, eax
			pop     ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_update_move_points_]
			call    edi
			pop     edi
		use:
			call    FakeCombatFix3                       // analogue of calling action_use_an_item_on_object_
		end:
			retn
	}
}

__declspec(naked) static void compute_damage_hook(void) {
	__asm {
			test    byte ptr [esi+0x14], 0x80            // ctd.flagsSource & DAM_DEAD?
			jz      skipSource                           // No
			mov     ecx, [esi]                           // ctd.source
			jecxz   skipSource
			mov     ecx, [ecx+0x58]                      // source.curr_hp
			cmp     ecx, [esi+0x10]
			jle     skipSource
			mov     [esi+0x10], ecx                      // ctd.amountSource
		skipSource:
			test    byte ptr [esi+0x30], 0x80            // ctd.flagsTarget & DAM_DEAD?
			jz      skipTarget                           // No
			mov     ecx, [esi+0x20]                      // ctd.target
			jecxz   skipTarget
			mov     ecx, [ecx+0x58]                      // target.curr_hp
			cmp     ecx, [esi+0x2C]
			jle     skipTarget
			mov     [esi+0x2C], ecx                      // ctd.amountTarget
		skipTarget:
			pop     ebp
			pop     edi
			pop     esi
			pop     ecx
			retn
	}
}

__declspec(naked) static void op_obj_can_see_obj_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     ebx, [edi+0x4DCD5]
			pop     edi
			mov     edx, [esp+4]                         // source
			xchg    edx, eax                             // eax=source, edx=target
			mov     ecx, [eax+0x28]                      // source.elev
			cmp     ecx, [edx+0x28]                      // target.elev
			jne     end
			cmp     dword ptr [eax+0x4], -1              // source.tile_num
			je      end
			cmp     dword ptr [edx+0x4], -1              // target.tile_num
			je      end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     ebx, [edi+0x4DC9E]
			pop     edi
		end:
			jmp     ebx
	}
}

__declspec(naked) static void op_obj_can_hear_obj_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x4EFB6]
			pop     edi
			mov     edx, [esp]                           // target
			test    edx, edx
			jz      end
			mov     eax, [esp+4]                         // source
			test    eax, eax
			jz      end
			mov     ecx, [eax+0x28]                      // source.elev
			cmp     ecx, [edx+0x28]                      // target.elev
			jne     end
			cmp     dword ptr [eax+0x4], -1              // source.tile_num
			je      end
			cmp     dword ptr [edx+0x4], -1              // target.tile_num
			je      end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x4EFA8]
			pop     edi
		end:
			jmp     esi
	}
}

__declspec(naked) static void switch_hand_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__inven_dude]
			pop     edi
			push    eax
			mov     [edi], ebp
			inc     ecx
			jz      skip
			xor     ebx, ebx
			inc     ebx
			mov     edx, ebp
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_remove_mult_]
			call    edi
			pop     edi
		skip:
			pop     edx
			mov     eax, ebp
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			dec     eax                                  // item_type_container?
			jnz     end                                  // No
			mov     [ebp+0x7C], edx                      // iobj.owner = _inven_dude
		end:
			pop     ebp
			pop     edi
			pop     esi
			retn
	}
}

__declspec(naked) static void combat_display_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x22B51]
			pop     edi
			je      end                                  // Is it ObjType_Critter
			cmp     dword ptr [ecx+0x78], -1             // Does the target have a script?
			jne     end                                  // Yes
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     esi, [edi+0x22C01]
			pop     edi
		end:
			jmp     ebx
	}
}

__declspec(naked) static void inven_action_cursor_hook(void) {
	__asm {
			cmp     dword ptr [esp+0x40+0x4], item_type_container
			jne     end
			mov     eax, [esp+0x1C+0x4]
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_top_environment_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     eax, ds:[edi+D__stack]
			pop     edi
			je      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     eax, ds:[edi+D__target_stack]
			pop     edi
		end:
			retn
	}
}

__declspec(naked) static void exec_script_proc_hook(void) {
	__asm {
			mov     eax, [esp+0x10+0x4]
			mov     eax, [eax+0x58]
			test    eax, eax
			ja      end
			inc     eax
		end:
			retn
	}
}

__declspec(naked) static void gdActivateBarter_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_talk_to_pressed_barter_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__dialogue_state], ecx
			pop     edi
			jne     skip
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__dialogue_switch_mode], esi
			pop     edi
			je      end
		skip:
			push    ecx
			push    esi
			push    edi
			push    ebp
			sub     esp, 0x14

			lea     esp, [esp-4] // [DADi590] Reserve space for the push
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x410C3]
			pop     edi
		end:
			retn
	}
}

__declspec(naked) static void op_negate_hook(void) {
	__asm {
			mov     edx, ecx
			cmp     ax, VAR_TYPE_FLOAT
			mov     eax, [esi+0x1C]
			jne     end
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_popLongStack_]
			call    edi
			pop     edi
			push    eax
			fld     dword ptr [esp]                      // Loading from memory to the top of the stack ST(0) a real number (float)
			fchs                                         // Sign change
			fstp    dword ptr [esp]                      // Loading from the top of the stack ST(0) into the memory of a real number (float)
			pop     ebx
			mov     edx, ecx
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_pushLongStack_]
			call    edi
			pop     edi
			mov     ebx, VAR_TYPE_FLOAT
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [DADi590] Reserve space for the push
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x5EA70]
			pop     edi
		end:
			retn
	}
}

// Note to update this to v1.8: almost all game addresses match on 1.7.20 and 1.8, so hopefully the functions match too,
// or at least don't differ too much. There are just a few addresses different, which almost certainly indicate
// different functions. Also.... there are various more fixes to include here. Will take time.

void BugsInit(void) {
	int i = 0;

	// Applying sharpshooter patch.
	hookCallEXE(0x2200D, &determine_to_hit_func_hook);
	writeMem8EXE(0x22044, 0xEB);

	// Applying withdrawal perk description crash fix.
	hookCallEXE(0x6C8B7, &perform_withdrawal_start_hook);

	// Fixing clickability bugs in pipboy
	makeCallEXE(0x86BAE, &pipboy_hook, false);
	makeCallEXE(0x88E5C, &PipAlarm_hook, false);

	// Fix "Too Many Items Bug"
	hookCallEXE(0x93AE6, &scr_save_hook);
	hookCallEXE(0x93B3D, &scr_save_hook);

	// Correction of drug addiction treatment
	makeCallEXE(0x6CA30, &item_d_check_addict_hook, true);

	// Fixed a crash (which is not here) when using stimpacks on a victim and then exiting the map
	hookCallEXE(0x90AAB, &queue_clear_type_hook);

	// Fix "Unlimited Ammo bug"
	hookCallEXE(0x65F0B, &invenWieldFunc_hook);

	// Fixed display of negative values in the skill window ("S")
	writeMem8EXE(0x99987, 0x7F);                // jg

	// Fixed return of equipped armor and weapons in hands
	makeCallEXE(0x65386, &inven_right_hand_hook, true);
	makeCallEXE(0x653C6, &inven_left_hand_hook, true);
	makeCallEXE(0x65406, &inven_worn_hook, true);

	// Correction of the error of not taking into account the weight of dressed things
	makeCallEXE(0x66F62, &loot_container_hook, false);

	// Text width 64, not 80
	writeMem8EXE(0x6855C+1, 64);
	writeMem8EXE(0x68704+1, 64);

	// Fix bug in player inventory related to IFACE_BAR_MODE=1 from f1_res.ini
	makeCallEXE(0x64B35, &inven_pickup_hook, true);

	// Fix for using only one pack of ammo when weapon is in front of ammo
	hookCallEXE(0x6943B, &drop_ammo_into_weapon_hook);

	// Applying black skilldex patch.
	hookCallEXE(0x876D0, &PipStatus_hook);

	// Applying Dodgy Door Fix.
	makeCallEXE(0x112E6, &action_melee_hook, true);
	makeCallEXE(0x11A50, &action_ranged_hook, true);

	// When displaying the number of experience points received, take into account the perk 'Diligent student'
	makeCallEXE(0x9CC88, &stat_pc_add_experience_hook, false);
	hookCallEXE(0x20223, &combat_give_exps_hook);
	makeCallEXE(0x677FC, &loot_container_hook1, true);

	// "NPC turns into a container" fix
	makeCallEXE(0x227E9, &set_new_results_hook, false);
	makeCallEXE(0x28D80, &critter_wake_clear_hook, true);
	makeCallEXE(0x7ACA1, &obj_load_func_hook, true);
	makeCallEXE(0x8561C, &partyMemberPrepLoad_hook, false);

	// Fix explosives bugs
	makeCallEXE(0x20DF5, &combat_ctd_init_hook, true);
	makeCallEXE(0x7B1C3, &obj_save_hook, true);
	hookCallEXE(0x12E44, &explode_critter_kill);
	hookCallEXE(0x12E67, &explode_critter_kill);

	// Fixed a bug where action points were lost when using chemistry from the hand in battle
	makeCallEXE(0x11E91, &action_use_an_item_on_object_hook, false);
	hookCallEXE(0x65320, &use_inventory_on_hook);

	// Fixed display of received damage on instant death
	makeCallEXE(0x22471, &compute_damage_hook, true);

	// [DADi590]: This below was commented out, but on release 1.7.17 it wasn't, so here it is enabled.
	// Applying imported procedure patch (http://teamx.ru/site_arc/smf/index.php-topic=398.0.htm)
	writeMem32EXE(0x5F181, 0x1C24A489);         // Fixing Time Stack Issues
	writeMem8EXE(0x61A95, 0xEB);                // Disabling warnings
	writeMem8EXE(0x61D39, 0xEB);                // Disabling warnings
	writeMem8EXE(0x3A21B+1, 0x00);              // prevent crash on re-export

	// Fix op_obj_can_see_obj_ and op_obj_can_hear_obj_
	makeCallEXE(0x4DC5B, &op_obj_can_see_obj_hook, true);
	makeCallEXE(0x4EF7F, &op_obj_can_hear_obj_hook, true);

	// Fix disappearing items in the inventory when trying to place them in an enclosed bag if the player is overloaded
	hookCallEXE(0x693AE, getRealEXEAddr(C_item_add_force_));
	// Restoring a bag to a player after placing it in a hand
	makeCallEXE(0x64F87, &switch_hand_hook, true);

	// Fixed displaying a message in the monitor window when hitting a random scripted target if it is not a character
	makeCallEXE(0x22B4B, &combat_display_hook, true);

	// Fixed a potential crash when trying to open a bag/backpack in the trade/trade window
	makeCallEXE(0x66630, &inven_action_cursor_hook, false);

	// Fixed bug of dead characters freezing when using kill_critter_type
	writeMem32EXE(0x4EB1E+1, 0x0);
	writeMem32EXE(0x4E9CD, 0x30BE0075);

	// Correction of using a fixed position to call the start script procedure in the absence of a standard handler
	makeCallEXE(0x929FB, &exec_script_proc_hook, false);

	// Hide unused windows when calling gdialog_mod_barter
	hookCallEXE(0x40B88, &gdActivateBarter_hook);

	// Fix for negate operator not working on float values
	makeCallEXE(0x5EA53, &op_negate_hook, false);

	// Fix incorrect int-to-float conversion, replace "fild qword ptr [esp]" to "fild dword ptr [esp]"
	for (i = 0; i < ((int) sizeof(QWordToDWord) / 4); ++i) {
		*(uint16_t *) (((uint32_t *) getRealBlockAddrData(QWordToDWord))[i]) = 0x04DB;
	}

	// Fix handling of mood parameter in start_gdialog function
	writeMem32EXE(0x4E025, 0x75FFF883);
}
