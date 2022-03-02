/*
 *    sfall
 *    Copyright (C) 2011  Timeslip, 2022 DADi590
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

#include "../CLibs/string.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "Define.h"
#include "FalloutEngine.h"
#include "Inventory.h"

static void __declspec(naked) make_loot_drop_button() {
	__asm {
		cmp     dword ptr [esp+0x4+0x4], 2
		jne     end
		xor     ebp, ebp
		push    edi
		mov     edi, SN_DATA_SEC_EXE_ADDR
		cmp     ds:[edi+D__gIsSteal], ebp                  // Âîðîâñòâî?
		pop     edi
		jne     end                                  // Äà
		push    edi
		mov     edi, SN_DATA_SEC_EXE_ADDR
		mov     eax, ds:[edi+D__inven_dude]
		pop     edi
		push    edi
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F_critter_body_type_
		call    edi
		pop     edi
		test    eax, eax                             // Ýòî Body_Type_Biped?
		jnz     end                                  // Íåò
		push    esi
		mov     esi, SN_DATA_SEC_EXE_ADDR
		mov     edi, ds:[esi+D__i_wid]                     // GNWID
		pop     esi
		xor     ecx, ecx                             // ID1
		xor     ebx, ebx                             // ID2
		push    edi
		push    ebx                                  // ID3
		mov     edx, 265                             // Index (USEGETN.FRM (Action menu use/get normal))
		mov     eax, ObjType_Intrface                // ObjType
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F_art_id_
		call    edi
		pop     edi
		xor     edx, edx
		mov     ecx, 0x19B6B4
		xor     ebx, ebx
		push    edi
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F_art_ptr_lock_data_
		call    edi
		pop     edi
		test    eax, eax
		jz      noLootButton
		xchg    esi, eax
		xor     ecx, ecx                             // ID1
		xor     ebx, ebx                             // ID2
		push    edi
		push    ebx                                  // ID3
		mov     edx, 264                             // Index (USEGETH.FRM (Action menu use/get highlighted))
		mov     eax, ObjType_Intrface                // ObjType
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F_art_id_
		call    edi
		pop     edi
		xor     edx, edx
		mov     ecx, 0x19B6B8
		xor     ebx, ebx
		push    edi
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F_art_ptr_lock_data_
		call    edi
		pop     edi
		test    eax, eax
		jz      noLootButton
		push    edi
		push    ebp                                  // ButType
		push    ebp
		push    eax                                  // PicDown
		push    esi                                  // PicUp
		dec     ebp
		push    ebp                                  // ButtUp
		push    65                                   // ButtDown
		push    ebp                                  // HovOff
		push    ebp                                  // HovOn
		mov     ecx, 40                              // Width
		push    ecx                                  // Height
		mov     edx, 354                             // Xpos
		mov     ebx, 154                             // Ypos
		mov     eax, edi                             // GNWID
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F_win_register_button_
		call    edi
		pop     edi
		inc     ebp
		noLootButton:
			mov     ebx, [esp+0x18+0x4]
			mov     eax, [ebx+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // Ýòî ObjType_Item?
			jnz     skip                                 // Íåò
			xchg    ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_get_type_
			call    edi
			pop     edi
			dec     eax                                  // item_type_container?
			jz      goodTarget                           // Äà
			jmp     end
		skip:
			dec     eax                                  // ObjType_Critter?
			jnz     end                                  // Íåò
			xchg    ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_critter_body_type_
			call    edi
			pop     edi
			test    eax, eax                             // Ýòî Body_Type_Biped?
			jnz     end                                  // Íåò
		goodTarget:
			xor     ecx, ecx                             // ID1
			xor     ebx, ebx                             // ID2
			push    edi
			push    ebx                                  // ID3
			mov     edx, 255                             // Index (DROPN.FRM (Action menu drop normal))
			mov     eax, ObjType_Intrface                // ObjType
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_art_id_
			call    edi
			pop     edi
			xor     edx, edx
			mov     ecx, 0x19B6BC
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_art_ptr_lock_data_
			call    edi
			pop     edi
			test    eax, eax
			jz      end
			xchg    esi, eax
			xor     ecx, ecx                             // ID1
			xor     ebx, ebx                             // ID2
			push    edi
			push    ebx                                  // ID3
			mov     edx, 254                             // Index (DROPH.FRM (Action menu drop highlighted))
			mov     eax, ObjType_Intrface                // ObjType
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_art_id_
			call    edi
			pop     edi
			xor     edx, edx
			mov     ecx, 0x19B6C0
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_art_ptr_lock_data_
			call    edi
			pop     edi
			test    eax, eax
			jz      end
			push    edi
			push    ebp                                  // ButType
			push    ebp
			push    eax                                  // PicDown
			push    esi                                  // PicUp
			dec     ebp
			push    ebp                                  // ButtUp
			push    68                                   // ButtDown
			push    ebp                                  // HovOff
			push    ebp                                  // HovOn
			mov     ecx, 40                              // Width
			push    ecx                                  // Height
			mov     edx, 140                             // Xpos
			mov     ebx, 154                             // Ypos
			xchg    edi, eax                             // GNWID
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_win_register_button_
			call    edi
			pop     edi
		end:
			cmp     dword ptr [esp+0x4+0x4], 3
			retn
	}
}

static char OverloadedLoot[48] = "";
static char OverloadedDrop[48] = "";
static void __declspec(naked) loot_drop_all(void) {
	__asm {
		xor     ebx, ebx
		cmp     eax, 'A'
		je      lootKey
		cmp     eax, 'a'
		je      lootKey
		cmp     eax, 'D'
		je      dropKey
		cmp     eax, 'd'
		je      dropKey
		cmp     eax, 0x148
		retn

		lootKey:
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__gIsSteal], ebx
			pop     edi
			jne     end
			mov     ecx, [esp+0x10C+0x20+0x4]
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_critter_body_type_
			call    edi
			pop     edi
			test    eax, eax                             // Ýòî Body_Type_Biped?
			jnz     end                                  // Íåò
			mov     edx, STAT_carry_amt
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_stat_level_
			call    edi
			pop     edi
			xchg    edx, eax
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_total_weight_
			call    edi
			pop     edi
			sub     edx, eax
			mov     eax, ebp
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_total_weight_
			call    edi
			pop     edi
			cmp     eax, edx
			jg      cantLoot
			mov     esi, 2501
			mov     edx, ecx
			mov     eax, ebp
			jmp     moveAll
		cantLoot:
			mov     edx, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edx+OverloadedLoot]
			jmp     printError
		dropKey:
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__gIsSteal], ebx
			pop     edi
			jne     end
			mov     esi, ebp
			mov     ecx, [esp+0x10C+0x20+0x4]
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_critter_body_type_
			call    edi
			pop     edi
			test    eax, eax                             // Ýòî Body_Type_Biped?
			jnz     end                                  // Íåò
			mov     eax, [esi+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // Ýòî ObjType_Item?
			jz      itsItem                              // Äà
			cmp     eax, ObjType_Critter
			jne     end                                  // Íåò
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_critter_body_type_
			call    edi
			pop     edi
			test    eax, eax                             // Ýòî Body_Type_Biped?
			jnz     end                                  // Íåò
		itsCritter:
			mov     edx, STAT_carry_amt
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_stat_level_
			call    edi
			pop     edi
			xchg    edx, eax                             // edx = ìàêñ. âåñ ãðóçà öåëè
			// todo sub     edx, WeightOnBody                    // Ó÷èòûâàåì âåñ îäåòîé íà öåëè áðîíè è îðóæèÿ
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_total_weight_              // eax = îáùèé âåñ ãðóçà öåëè
			call    edi
			pop     edi
			sub     edx, eax
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_total_weight_
			call    edi
			pop     edi
			xchg    edi, eax                             // edi = îáùèé âåñ ãðóçà èãðîêà
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_rhand]
			pop     edi
			test    eax, eax
			jz      noRight
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_weight_
			call    edi
			pop     edi
			sub     edi, eax
		noRight:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_lhand]
			pop     edi
			test    eax, eax
			jz      noLeft
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_weight_
			call    edi
			pop     edi
			sub     edi, eax
		noLeft:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_worn]
			pop     edi
			test    eax, eax
			jz      noArmor
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_weight_
			call    edi
			pop     edi
			sub     edi, eax
		noArmor:
			xchg    edi, eax                             // eax = îáùèé âåñ ãðóçà èãðîêà
			jmp     compareSizeWeight
		itsItem:
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_get_type_
			call    edi
			pop     edi
			dec     eax                                  // Ýòî ñóìêà/ðþêçàê?
			jnz     end                                  // Íåò
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_obj_top_environment_
			call    edi
			pop     edi
			test    eax, eax                             // Åñòü âëàäåëåö?
			jz      noOwner                              // Íåò
			xchg    esi, eax                             // esi = âëàäåëåö ñóìêè
			mov     eax, [esi+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			cmp     eax, ObjType_Critter                 // Ýòî ïåðñîíàæ?
			je      itsCritter                           // Íåò
		noOwner:
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_c_max_size_
			call    edi
			pop     edi
			xchg    edx, eax
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_c_curr_size_
			call    edi
			pop     edi
			sub     edx, eax
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_c_curr_size_
			call    edi
			pop     edi
		compareSizeWeight:
			cmp     eax, edx
			jg      cantDrop
			mov     esi, 2500
			mov     edx, ebp
			mov     eax, ecx
		moveAll:
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_gsound_red_butt_press_
			call    edi
			pop     edi
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_item_move_all_
			call    edi
			pop     edi
			xor     edx, edx
			dec     edx                                  // edx = -1
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__stack_offset], eax
			mov     ds:[edi+D__target_stack_offset], eax
			pop     edi
			inc     eax
			cmp     esi, 2500
			je      itsDrop
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__target_curr_stack], eax
			pop     edi
			inc     eax
			push    eax
			xchg    edx, eax                             // edx = mode, eax = -1
			xchg    esi, eax                             // eax = 2501, esi = -1
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_container_exit_
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__curr_stack]
			mov     eax, ds:[edi+D__stack_offset][eax*4]       // eax = inventory_offset
			pop     edi
			mov     edx, esi                             // -1 (visible_offset)
			pop     ebx                                  // 2 (mode)
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_display_inventory_
			call    edi
			pop     edi
			jmp     end
		itsDrop:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__curr_stack], eax
			pop     edi
			inc     eax
			push    eax
			xchg    edx, eax                             // edx = mode, eax = -1
			xchg    esi, eax                             // eax = 2500, esi = -1
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_container_exit_
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__target_curr_stack]
			mov     eax, ds:[edi+D__target_stack_offset][eax*4]// eax = inventory_offset
			pop     edi
			mov     edx, esi                             // -1 (visible_offset)
			pop     ecx                                  // 2 (mode)
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ebx, ds:[edi+D__target_pud]                // target_inventory
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_display_target_inventory_
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_wid]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_win_draw_
			call    edi
			pop     edi
			jmp     end
		cantDrop:
			mov     edx, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edx+OverloadedDrop]
		printError:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_gsound_red_butt_press_
			call    edi
			pop     edi
			push    edi
			xor     eax, eax
			push    eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     al, ds:[edi+0x2A9858]                    // color
			pop     edi
			push    eax
			xor     ebx, ebx
			push    ebx
			push    eax
			mov     ecx, 169
			push    117
			xor     eax, eax
			xchg    edx, eax
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, F_dialog_out_
			call    edi
			pop     edi
		end:
			popad
			pop     ebx                                  // Destroying the return address
			// [DADi590] And since EBX seems to be able to have trash on it... (And btw, I'll just put a JMP - cleaner)
			mov     ebx, SN_CODE_SEC_EXE_ADDR
			add     ebx, 0x6770E
			jmp     ebx
	}
}

void InventoryInit(struct FileInfo sfall1_ini_info) {
	(void) sfall1_ini_info;

	// "Take All" and "Put All" Buttons
	MakeCallEXE(0x6352A, getRealBlockAddrCode((void *) &make_loot_drop_button), false);
	MakeCallEXE(0x672C1, getRealBlockAddrCode((void *) &loot_drop_all), false);
	strcpy(OverloadedLoot, "Sorry, you cannot carry that much.");
	strcpy(OverloadedDrop, "Sorry, there is not enough space left.");
	// todo GetPrivateProfileString("sfall", "OverloadedLoot", "Sorry, you cannot carry that much.", OverloadedLoot, 48, translationIni);
	// todo GetPrivateProfileString("sfall", "OverloadedDrop", "Sorry, there is no space left.", OverloadedDrop, 48, translationIni);
}
