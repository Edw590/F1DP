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
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/ctype.h"
#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "Bugs.h"
#include "Define.h"
#include "FalloutEngine.h"
#include "Inventory.h"
#include "LoadGameHook.h"
#include "PartyControl.h"
#include "SFall1Patches.h"
#include "SFall1Main.h"

// Specifically initialized to true
static bool UseScrollWheel = true;

static char ReloadWeaponKey = '\0';

static uint32_t StackEmptyWeapons = 0;

static void __declspec(naked) ReloadActiveHand(void) {
	__asm {
			// esi=-1 unless reloading the inactive hand or shifting the inactive hand
			push    ebx
			push    ecx
			push    edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__itemCurrentItem]
			pop     edi
			imul    ebx, eax, 24
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			xor     ecx, ecx
		reloadItem:
			push    eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edx, ds:[edi+D__itemButtonItems][ebx]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_try_reload_]
			call    edi
			pop     edi
			test    eax, eax
			pop     eax
			jnz     endReloadItem
			inc     ecx
			jmp     reloadItem
		endReloadItem:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     dword ptr ds:[edi+D__itemButtonItems + 0x10][ebx], 5// mode
			pop     edi
			jne     skip_toggle_item_state
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_toggle_item_state_]
			call    edi
			pop     edi
		skip_toggle_item_state:
			test    ecx, ecx
			jnz     useActiveHand
			xchg    esi, ebx
		useActiveHand:
			push    ebx
			xor     esi, esi
			dec     esi
			xor     eax, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_update_items_]
			call    edi
			pop     edi
			pop     eax
			cmp     eax, esi
			je      end
			xor     ebx, ebx
			inc     ebx
			inc     ebx
			xor     ecx, ecx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edx, ds:[edi+D__itemButtonItems][eax]
			pop     edi

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x563D2]
			mov     [esp+4], edi
			pop     edi
			retn

		end:
			pop     edx
			pop     ecx
			pop     ebx
			retn
	}
}

static bool wasReloadWeaponKeyPressed(char c) {
	// I was checking if the wanted key was in the BIOS keyboard buffer. And that works. But not in this case, as it
	// seems the game first removes the key from the buffer before ReloadWeaponHotKey() is called. So my second thought
	// was to go check the key where it is still in the buffer and put that in a global variable which would be checked
	// in ReloadWeaponHotKey().
	// Until I realized the key is on EBX on the call to the mentioned function. It's not a key code - it's the
	// character. So a character must now be checked and not a key code - less options, but it's the easiest without
	// having to go to some place where the key is still in the buffer, check that and put in a global variable.

	return (toupper(c) == toupper(*(char *) getRealBlockAddrData(&ReloadWeaponKey)));
}

static void __declspec(naked) ReloadWeaponHotKey(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_gmouse_is_scrolling_]
			call    edi
			pop     edi
			test    eax, eax
			jnz     end
			pushad
			xchg    ebx, eax
			call    wasReloadWeaponKeyPressed // EAX is the parameter
			// [DADi590] If it's a bool, Watcom may/will xor AL with AL for false (it's either 0 or 1, so 8 bits is more than enough)
			test    al, al
			jnz     ourKey
			popad
			retn

		ourKey:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__intfaceEnabled], ebx
			pop     edi
			je      endReload
			xor     esi, esi
			dec     esi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__interfaceWindow], esi
			pop     edi
			je      endReload
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edx, ds:[edi+D__itemCurrentItem]
			pop     edi
			imul    eax, edx, 24
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__itemButtonItems + 0x5][eax], bl // itsWeapon
			pop     edi
			jne     itsWeapon                            // Äà
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_use_item_]
			call    edi
			pop     edi
			jmp     endReload
		itsWeapon:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1
			pop     edi
			jnz     inCombat
			call    ReloadActiveHand
			jmp     endReload
		inCombat:
			//xor     ebx, ebx                             // is_secondary [DADi590: already commented out]
			add     edx, hit_left_weapon_reload          // edx = 6/7 - reloading weapons in the left / right hand
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_mp_cost_]
			call    edi
			pop     edi
			xchg    ecx, eax
			pop     eax                                  // _obj_dude
			mov     edx, [eax+0x40]                      // curr_mp
			cmp     ecx, edx
			jg      endReload
			push    eax
			call    ReloadActiveHand
			test    eax, eax
			pop     eax                                  // _obj_dude
			jnz     endReload
			sub     edx, ecx
			mov     [eax+0x40], edx                      // curr_mp
			xchg    edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_update_move_points_]
			call    edi
			pop     edi
		endReload:
			popad
			inc     eax
		end:
			retn
	}
}

static void __declspec(naked) AutoReloadWeapon(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_scr_exec_map_update_scripts_]
			call    edi
			pop     edi
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     dword ptr ds:[edi+D__game_user_wants_to_quit], 0
			pop     edi
			jnz     end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_is_dead_]                // An extra check wouldn't hurt
			call    edi
			pop     edi
			test    eax, eax
			jnz     end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__intfaceEnabled], eax
			pop     edi
			je      end
			xor     esi, esi
			dec     esi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__interfaceWindow], esi
			pop     edi
			je      end
			inc     eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ebx, ds:[edi+D__itemCurrentItem]
			pop     edi
			push    ebx
			sub     eax, ebx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__itemCurrentItem], eax           // Setting the inactive hand
			pop     edi
			imul    ebx, eax, 24
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			xor     ecx, ecx
		reloadOffhand:
			push    eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edx, ds:[edi+D__itemButtonItems][ebx]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_try_reload_]
			call    edi
			pop     edi
			test    eax, eax
			pop     eax
			jnz     endReloadOffhand
			inc     ecx
			jmp     reloadOffhand
		endReloadOffhand:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     dword ptr ds:[edi+D__itemButtonItems + 0x10][ebx], 5// mode
			pop     edi
			jne     skip_toggle_item_state
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_toggle_item_state_]
			call    edi
			pop     edi
		skip_toggle_item_state:
			test    ecx, ecx
			jnz     useOffhand
			xchg    ebx, esi
		useOffhand:
			xchg    esi, ebx                             // esi=-1 if not reloaded or displaced inactive hand
			pop     eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__itemCurrentItem], eax           // Restoring the active hand
			pop     edi
			call    ReloadActiveHand
		end:
			popad
			retn
	}
}

static void __declspec(naked) SetDefaultAmmo(void) {
	__asm {
			push    eax
			push    ebx
			push    edx
			xchg    edx, eax
			mov     ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_weapon
			jne     end                                  // No
			cmp     dword ptr [ebx+0x3C], 0              // Is there any ammo in the weapon?
			jne     end                                  // Yes
			sub     esp, 4
			mov     edx, esp
			mov     eax, [ebx+0x64]                      // eax = pid weapons
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_proto_ptr_]
			call    edi
			pop     edi
			mov     edx, [esp]
			mov     eax, [edx+0x5C]                      // eax = default ammo prototype id
			mov     [ebx+0x40], eax                      // prototype of used cartridges
			add     esp, 4
		end:
			pop     edx
			pop     ebx
			pop     eax
			retn
	}
}

static void __declspec(naked) inven_action_cursor_hook(void) {
	__asm {
			mov     edx, [esp+0x1C]
			call    SetDefaultAmmo
			cmp     dword ptr [esp+0x18], 0

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x66B73]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) fontHeight(void) {
	__asm {
			push    ebx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ebx, ds:[edi+D__curr_font_num]
			pop     edi
			mov     eax, 101
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_text_font_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			call    ds:[edi+D__text_height]
			pop     edi
			xchg    ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_text_font_]
			call    edi
			pop     edi
			xchg    ebx, eax
			pop     ebx
			retn
	}
}

static char MsgBuf[32];
static void __declspec(naked) printFreeMaxWeight(void) {
	__asm {
			// ebx = source, ecx = ToWidth, edi = posOffset, esi = extraWeight
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__curr_font_num]
			pop     edi
			push    eax
			mov     eax, 101
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_text_font_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_wid]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_get_buf_]                   // eax=ToSurface
			call    edi
			pop     edi
			add     edi, eax                             // ToSurface+posOffset (Ypos*ToWidth+Xpos)
			mov     eax, [ebx+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // This ObjType_Item?
			jz      itsItem                              // Yes
			dec     eax                                  // This ObjType_Critter?
			jnz     noWeight                             // No
			mov     eax, ebx
			mov     edx, STAT_carry_amt
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]                     // eax = ìàêñ. âåñ ãðóçà
			call    edi
			pop     edi
			xchg    ebx, eax                             // ebx = Max. cargo weight, eax = source
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]              // eax = total weight of the cargo
			call    edi
			pop     edi
			jmp     printFree
		itsItem:
			mov     eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			dec     eax                                  // item_type_container?
			jnz     noWeight                             // No
			mov     eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_top_environment_]
			call    edi
			pop     edi
			test    eax, eax                             // Is there an owner?
			jz      noOwner                              // No
			mov     edx, [eax+0x20]
			and     edx, 0xF000000
			sar     edx, 0x18
			cmp     edx, ObjType_Critter                 // Is it a character?
			jne     noOwner                              // No
			push    eax
			mov     edx, STAT_carry_amt
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]                     // eax = Max. character's weight
			call    edi
			pop     edi
			xchg    ebx, eax                             // ebx = Max. character's weight, eax = source
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_max_size_]
			call    edi
			pop     edi
			xchg    edx, eax                             // edx = Max. bag cargo size
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_curr_size_]
			call    edi
			pop     edi
			sub     edx, eax                             // edx = free size
			pop     eax
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]             // eax = total weight of the cargo
			call    edi
			pop     edi
			sub     ebx, esi
			sub     ebx, eax                             // eax = free weight
			push    ebx

			lea     esp, [esp-4] // [DADi590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF91D4]                         // '%d/%d'
			mov     [esp+4], edi // [DADi590: "PUSH"]
			pop     edi

			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     esi, [edi+MsgBuf]
			pop     edi
			push    esi
			mov     [esp+4*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			add     esp, 4*4
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			movzx   eax, byte ptr ds:[edi+D__GreenColor]
			pop     edi
			cmp     ebx, 0                               // Weight overload?
			jl      Red                                  // Yes
			cmp     edx, 0                               // Size overload?
			jge     noRed                                // No
		Red:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     al, ds:[edi+D__RedColor]
			pop     edi
		noRed:
			lea     esp, [esp-4] // [DADi590: reserve space for "PUSH EDI" on 'print']
			push    eax
			push    esi
			xor     edx, edx
		nextChar:
			movzx   eax, byte ptr [esi]
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			call    ds:[edi+D__text_char_width]
			pop     edi
			inc     eax
			add     edx, eax
			inc     esi
			cmp     byte ptr [esi-1], '/'
			jne     nextChar
			sub     edi, edx
			pop     edx                                  // DisplayText
			jmp     print
		noOwner:
			mov     eax, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_max_size_]                // eax = Max. cargo size
			call    edi
			pop     edi
			xchg    ebx, eax                             // ebx = Max. cargo size, eax = source
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_curr_size_]
			call    edi
			pop     edi
			xor     esi, esi
		printFree:
			sub     ebx, esi
			sub     ebx, eax                             // ebx = free weight/size
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    ebx

			lea     esp, [esp-4] // [DADi590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF91C8]                         // '%d'
			mov     [esp+4], edi // [DADi590: "PUSH"]
			pop     edi

			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edi+MsgBuf]
			pop     edi
			push    edx
			mov     [esp+3*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			add     esp, 3*4
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			movzx   eax, byte ptr ds:[edi+D__GreenColor]
			pop     edi
			cmp     ebx, 0
			jge     skipColor
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     al, ds:[edi+D__RedColor]
			pop     edi
		skipColor:
			lea     esp, [esp-4] // [DADi590: reserve space for "PUSH EDI" on 'print']
			push    eax
			mov     eax, edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			call    ds:[edi+D__text_width]
			pop     edi
			shr     eax, 1
			inc     eax
			sub     edi, eax
		print:
			xchg    edi, eax                             // ToSurface+posOffset (Ypos*ToWidth+Xpos)
			mov     ebx, 64                              // TxtWidth

			// DADi590:
			// Seems that _text_to_buf has one of its parameters passed on the stack, and also that 'print' is jumped to
			// from 2 locations... That makes it more dificult. So I saw 2 choices: go see where the function is used
			// and check if I can use some register and leave garbage in it (I can - EDI and ESI, because after each of
			// the 3 CALLs to this function, there's POPA, so I could use any of those 2, as the other 4 are being used
			// as parameters); or, instead of hoping no usage of this function will be changed, I can just go make a
			// PUSH on all the locations, as long as the stack is not used anywhere after that PUSH. And that's what
			// I've done. Seems safer to me. If this function is ever used unmodified to something else and there's no
			// POPA or at least POP ESI or something at the end, the program will be doomed until someone figures out
			// why it's not working.
			mov     [esp+8], edi // [DADi590: "PUSH EDI"]

			mov     edi, SN_DATA_SEC_EXE_ADDR
			call    ds:[edi+D__text_to_buf]
			pop     edi
		noWeight:
			pop     eax

			// [DADi590: so, since I need to POP the used register... Let's do something that just came to my mind...]
			//jmp     C_text_font_
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_text_font_]
			mov     [esp+4], edi
			pop     edi
			retn
			// This is a jump! I just realized that it is after being using it for a while in this file. Perfect way
			// of making a jump to an absolute location still keeping all register values!
	}
}

static void __declspec(naked) display_inventory_hook(void) {
	__asm {
			call    fontHeight
			inc     eax
			inc     eax
			add     [esp+0x8], eax                       // height = height + text_height + 1

			// DADi590:
			// This mess below moves the stack 4 bytes down so that I can "PUSH" EDI. I can't, supposedly, because I'd
			// mess the stack and all PUSHes that were made before the jump to this function. So I saw 2 options: either
			// change where the jump to here is made and I put it to before all the PUSHes are done so that I can PUSH
			// EDI before the others, or I try something interesting like moving all stack values down 4 bytes and that
			// would be like I had PUSHed a value before all these PUSHes!
			lea     esp, [esp-4] // Reserve space on the stack for the last function PUSH
			push    edi
			mov     edi, [(esp+4)+4]
			mov     [(esp+4)+0], edi
			mov     edi, [(esp+4)+8]
			mov     [(esp+4)+4], edi
			mov     edi, [(esp+4)+12]
			mov     [(esp+4)+8], edi
			mov     edi, [(esp+4)+16]
			mov     [(esp+4)+12], edi
			mov     edi, [(esp+4)+20]
			mov     [(esp+4)+16], edi
			mov     edi, [(esp+4)+24]
			mov     [(esp+4)+20], edi
			pop     edi
			// After all the contents have been moved down, we can put EDI in the original place of the first function
			// PUSH.
			mov     [esp+6*4], edi

			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_buf_to_buf_]
			call    edi
			add     esp, 0x18
			pop     edi
			mov     eax, [esp+0x4]
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_ptr_unlock_]
			call    edi
			pop     edi
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__curr_stack]
			mov     ebx, ds:[edi+D__stack][eax*4]
			pop     edi
			mov     ecx, 537
			mov     edi, 324*537+44+32                   // Xpos=44, Ypos=324, max text width/2=32
			xor     esi, esi
			call    printFreeMaxWeight
			popad

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x63CB8]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) display_target_inventory_hook(void) {
	__asm {
			call    fontHeight
			inc     eax
			inc     eax
			add     [esp+0x8], eax                       // height = height + text_height + 1

			// For an explanation, see display_inventory_hook()
			lea     esp, [esp-4] // Reserve space on the stack for the last function PUSH
			push    edi
			mov     edi, [(esp+4)+4]
			mov     [(esp+4)+0], edi
			mov     edi, [(esp+4)+8]
			mov     [(esp+4)+4], edi
			mov     edi, [(esp+4)+12]
			mov     [(esp+4)+8], edi
			mov     edi, [(esp+4)+16]
			mov     [(esp+4)+12], edi
			mov     edi, [(esp+4)+20]
			mov     [(esp+4)+16], edi
			mov     edi, [(esp+4)+24]
			mov     [(esp+4)+20], edi
			pop     edi
			mov     [esp+6*4], edi

			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_buf_to_buf_]
			call    edi
			add     esp, 0x18
			pop     edi
			mov     eax, [esp]
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_ptr_unlock_]
			call    edi
			pop     edi
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__target_curr_stack]
			mov     ebx, ds:[edi+D__target_stack][eax*4]
			pop     edi
			mov     ecx, 537
			mov     edi, 324*537+426+32                  // Xpos=426, Ypos=324, max text width/2=32
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			sub     esi, [edi+WeightOnBody]              // We take into account the weight of armor and weapons worn on the target
			pop     edi
			call    printFreeMaxWeight
			popad

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x63FA4]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) display_table_inventories_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_get_buf_]
			call    edi
			pop     edi
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__btable]
			pop     esi
			mov     [esp+0x6C+4], edi
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__ptable]
			pop     esi
			mov     [esp+0x74+4], edi
			retn
	}
}

static void __declspec(naked) display_table_inventories_hook1(void) {
	__asm {
			add     dword ptr [esp+8], 20
			sub     dword ptr [esp+16], 20*480

			// For an explanation, see display_inventory_hook()
			lea     esp, [esp-4] // Reserve space on the stack for the last function PUSH
			push    edi
			mov     edi, [(esp+4)+4]
			mov     [(esp+4)+0], edi
			mov     edi, [(esp+4)+8]
			mov     [(esp+4)+4], edi
			mov     edi, [(esp+4)+12]
			mov     [(esp+4)+8], edi
			mov     edi, [(esp+4)+16]
			mov     [(esp+4)+12], edi
			mov     edi, [(esp+4)+20]
			mov     [(esp+4)+16], edi
			mov     edi, [(esp+4)+24]
			mov     [(esp+4)+20], edi
			pop     edi
			mov     [esp+6*4], edi

			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_buf_to_buf_]
			call    edi
			add     esp, 0x18
			pop     edi
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__btable]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]              // eax = the weight of things of the target in the barter window
			call    edi
			pop     edi
			xchg    esi, eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__curr_stack]
			mov     ebx, ds:[edi+D__stack][eax*4]
			pop     edi
			mov     ecx, 480
			mov     edi, 10*480+169+32                   // Xpos=169, Ypos=10, max text width/2=32
			call    printFreeMaxWeight
			popad

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x68487]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

// Draw a section of the window
static void __declspec(naked) display_table_inventories_hook2(void) {
	__asm {
			mov     dword ptr [edx+4], 4                 // WinRect.y_start = 4
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_draw_rect_]
			call    edi
			pop     edi
	}
}

static void __declspec(naked) barter_inventory_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_draw_]
			call    edi
			pop     edi
			xor     ecx, ecx
			dec     ecx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ebx, ds:[edi+D__btable]
			mov     edx, ds:[edi+D__ptable]
			mov     eax, ds:[edi+D__barter_back_win]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_table_inventories_]
			call    edi
			pop     edi
	}
}

static void __declspec(naked) barter_inventory_hook1(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_container_exit_]
			call    edi
			pop     edi
			xor     ecx, ecx
			dec     ecx
			mov     ebx, ebp                             // target
			mov     eax, edi                             // owner
			mov     edx, [esp+0x2C+0x4]                  // source
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_table_inventories_]
			call    edi
			pop     edi
	}
}

static void __declspec(naked) inven_pickup_hook2(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_wid]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_GNW_find_]
			call    edi
			pop     edi
			mov     ebx, [eax+0x8+0x0]                   // ebx = _i_wid.rect.x
			mov     ecx, [eax+0x8+0x4]                   // ecx = _i_wid.rect.y
			mov     eax, 176
			add     eax, ebx                             // x_start
			add     ebx, 176+60                          // x_end
			mov     edx, 37
			add     edx, ecx                             // y_start
			add     ecx, 37+100                          // y_end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mouse_click_in_]
			call    edi
			pop     edi
			test    eax, eax
			jz      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     edx, ds:[edi+D__curr_stack]
			pop     edi
			test    edx, edx
			jnz     end
			cmp     edi, 1006                            // Arms?
			jae     skip                                 // Yes
			mov     eax, [esp+0x18+0x4]                  // item
			mov     edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_drug
			jne     skip
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__stack]
			pop     edi
			push    eax
			push    edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_d_take_drug_]
			call    edi
			pop     edi
			pop     edx
			pop     ebx
			dec     eax
			jnz     notUsed
			inc     eax
			xchg    ebx, eax
			push    edx
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_remove_mult_]
			call    edi
			pop     edi
			inc     eax
			pop     eax
			mov     ebx, [eax+0x28]
			mov     edx, [eax+0x4]
			pop     eax
			jz      notUsed
			xor     ecx, ecx
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_connect_]
			call    edi
			pop     edi
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_destroy_]
			call    edi
			pop     edi
		notUsed:
			xor     eax, eax
			inc     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_intface_update_hit_points_]
			call    edi
			pop     edi
		skip:
			xor     eax, eax
		end:
			retn
	}
}

static void __declspec(naked) display_stats_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ecx, ds:[edi+D__curr_stack]
			mov     eax, ds:[edi+D__stack][ecx*4]
			pop     edi
			push    eax

			// A JECXZ was here, which seems to be a short jump always. But with the SN helper code, from here to
			// itsCritter seems to have gone above 128 bytes of instructions. So now must be a near jump.
			// It also seems JECXZ doesn't touch on the flags, so to replicate that behaviour, PUSHF and POPF help.
			pushf
			cmp     ecx, 0
			jz      itsCritter
			popf

			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_max_size_]
			call    edi
			pop     edi
			pop     edx
			xchg    edx, eax                             // eax=source, edx=Max. bag size
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_curr_size_]
			call    edi
			pop     edi
			sub     edx, eax                             // edx=free bag size
			pop     eax
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_top_environment_]
			call    edi
			pop     edi
			push    eax
			mov     edx, STAT_carry_amt
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]                     // Max. cargo
			call    edi
			pop     edi
			pop     edx
			xchg    edx, eax                             // edx=Max. cargo, eax=source
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]
			call    edi
			pop     edi
			sub     edx, eax                             // edx=free weight
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     eax, [edi+HiddenArmor]
			pop     edi
			test    eax, eax                             // Does he have armor?
			jz      noArmor                              // No
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
			sub     edx, eax
		noArmor:
			mov     edi, edx
			jmp     print
		itsCritter:
			popf
			mov     edx, STAT_carry_amt
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]                    // Max. cargo
			call    edi
			pop     edi
			pop     edi
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    eax
			xchg    edi, eax                             // eax=source, edi=Max. cargo
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]
			call    edi
			pop     edi
			xchg    edx, eax                             // edx=the weight of things
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     eax, [edi+HiddenArmor]
			pop     edi
			test    eax, eax                             // Does he have armor?
			jz      noHiddenArmor                        // No
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
			add     edx, eax
		noHiddenArmor:
			sub     edi, edx
		print:
			push    edx
			mov     edx, [esp+0x94+0x4 + 4]
			push    edx

			lea     esp, [esp-4] // [DADi590] Reserve space for the "PUSH"
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [edi+0xF9210]                         // '%s %d/%d'
			mov     [esp+4], edi // [DADi590: "PUSH"]
			pop     edi

			lea     eax, [esp+0xC+0x4 + 4]
			push    eax
			mov     [esp+5*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			add     esp, 5*4
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			movzx   eax, byte ptr ds:[edi+D__GreenColor]
			pop     edi
			cmp     edi, 0
			jge     noRed
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     al, ds:[edi+D__RedColor]
			pop     edi
		noRed:
			mov     ecx, 499
			mov     ebx, 120

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x65D55]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) make_loot_drop_button(void) {
	__asm {
			cmp     dword ptr [esp+0x4+0x4], 2
			jne     end
			xor     ebp, ebp
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__gIsSteal], ebp                  // Theft?
			pop     edi
			jne     end                                  // Yes
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__inven_dude]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_body_type_]
			call    edi
			pop     edi
			test    eax, eax                             // This Body_Type_Biped?
			jnz     end                                  // No
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__i_wid]                     // GNWID
			pop     esi
			xor     ecx, ecx                             // ID1
			xor     ebx, ebx                             // ID2
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    ebx                                  // ID3
			mov     edx, 265                             // Index (USEGETN.FRM (Action menu use/get normal))
			mov     eax, ObjType_Intrface                // ObjType
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			xor     edx, edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     ecx, [edi+0x19B6B4]
			pop     edi
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_ptr_lock_data_]
			call    edi
			pop     edi
			test    eax, eax
			jz      noLootButton
			xchg    esi, eax
			xor     ecx, ecx                             // ID1
			xor     ebx, ebx                             // ID2
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    ebx                                  // ID3
			mov     edx, 264                             // Index (USEGETH.FRM (Action menu use/get highlighted))
			mov     eax, ObjType_Intrface                // ObjType
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			xor     edx, edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     ecx, [edi+0x19B6B8]
			pop     edi
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_ptr_lock_data_]
			call    edi
			pop     edi
			test    eax, eax
			jz      noLootButton
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
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
			mov     [esp+9*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_register_button_]
			call    edi
			pop     edi
			inc     ebp
		noLootButton:
			mov     ebx, [esp+0x18+0x4]
			mov     eax, [ebx+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // This ObjType_Item?
			jnz     skip                                 // No
			xchg    ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			dec     eax                                  // item_type_container?
			jz      goodTarget                           // Yes
			jmp     end
		skip:
			dec     eax                                  // ObjType_Critter?
			jnz     end                                  // No
			xchg    ebx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_body_type_]
			call    edi
			pop     edi
			test    eax, eax                             // This Body_Type_Biped?
			jnz     end                                  // No
		goodTarget:
			xor     ecx, ecx                             // ID1
			xor     ebx, ebx                             // ID2
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    ebx                                  // ID3
			mov     edx, 255                             // Index (DROPN.FRM (Action menu drop normal))
			mov     eax, ObjType_Intrface                // ObjType
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			xor     edx, edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     ecx, [edi+0x19B6BC]
			pop     edi
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_ptr_lock_data_]
			call    edi
			pop     edi
			test    eax, eax
			jz      end
			xchg    esi, eax
			xor     ecx, ecx                             // ID1
			xor     ebx, ebx                             // ID2
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    ebx                                  // ID3
			mov     edx, 254                             // Index (DROPH.FRM (Action menu drop highlighted))
			mov     eax, ObjType_Intrface                // ObjType
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			xor     edx, edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     ecx, [edi+0x19B6C0]
			pop     edi
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_ptr_lock_data_]
			call    edi
			pop     edi
			test    eax, eax
			jz      end
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
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
			mov     [esp+9*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_register_button_]
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
			lea     edi, [edi+C_critter_body_type_]
			call    edi
			pop     edi
			test    eax, eax                             // This Body_Type_Biped?
			jnz     end                                  // No
			mov     edx, STAT_carry_amt
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			xchg    edx, eax
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]
			call    edi
			pop     edi
			sub     edx, eax
			mov     eax, ebp
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]
			call    edi
			pop     edi
			cmp     eax, edx
			jg      cantLoot
			mov     esi, 2501
			mov     edx, ecx
			mov     eax, ebp
			jmp     moveAll
		cantLoot:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edi+OverloadedLoot]
			pop     edi
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
			lea     edi, [edi+C_critter_body_type_]
			call    edi
			pop     edi
			test    eax, eax                             // This Body_Type_Biped?
			jnz     end                                  // No
			mov     eax, [esi+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // This ObjType_Item?
			jz      itsItem                              // Yes
			cmp     eax, ObjType_Critter
			jne     end                                  // No
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_body_type_]
			call    edi
			pop     edi
			test    eax, eax                             // This Body_Type_Biped?
			jnz     end                                  // No
		itsCritter:
			mov     edx, STAT_carry_amt
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			xchg    edx, eax                             // edx = Max. target cargo weight
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			sub     edx, [edi+WeightOnBody]              // We take into account the weight of armor and weapons worn on the target
			pop     edi
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]              // eax = total weight of target cargo
			call    edi
			pop     edi
			sub     edx, eax
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]
			call    edi
			pop     edi
			xchg    edi, eax                             // edi = the total weight of the player's cargo
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_rhand]
			pop     edi
			test    eax, eax
			jz      noRight
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
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
			lea     edi, [edi+C_item_weight_]
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
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
			sub     edi, eax
		noArmor:
			xchg    edi, eax                             // eax = the total weight of the player's cargo
			jmp     compareSizeWeight
		itsItem:
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			dec     eax                                  // Is it a bag/backpack?
			jnz     end                                  // No
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_top_environment_]
			call    edi
			pop     edi
			test    eax, eax                             // Is there an owner?
			jz      noOwner                              // No
			xchg    esi, eax                             // esi = bag owner
			mov     eax, [esi+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			cmp     eax, ObjType_Critter                 // Is it a character?
			je      itsCritter                           // No
		noOwner:
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_max_size_]
			call    edi
			pop     edi
			xchg    edx, eax
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_curr_size_]
			call    edi
			pop     edi
			sub     edx, eax
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_curr_size_]
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
			lea     edi, [edi+C_gsound_red_butt_press_]
			call    edi
			pop     edi
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_move_all_]
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
			lea     edi, [edi+C_container_exit_]
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
			lea     edi, [edi+C_display_inventory_]
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
			lea     edi, [edi+C_container_exit_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__target_curr_stack]
			mov     eax, ds:[edi+D__target_stack_offset][eax*4] // eax = inventory_offset
			pop     edi
			mov     edx, esi                             // -1 (visible_offset)
			pop     ecx                                  // 2 (mode)
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ebx, ds:[edi+D__target_pud]                // target_inventory
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_target_inventory_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_wid]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_win_draw_]
			call    edi
			pop     edi
			jmp     end
		cantDrop:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edi+OverloadedDrop]
			pop     edi
		printError:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_gsound_red_butt_press_]
			call    edi
			pop     edi
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
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
			mov     [esp+5*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_dialog_out_]
			call    edi
			pop     edi
		end:
			popad
			pop     ebx                                  // Destroying the return address

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x6770E]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static char SuperStimMsg[128] = "";
static void __declspec(naked) protinst_use_item_on_hook(void) {
	__asm {
			mov     edx, [ebx+0x64]                      // edx = item pid
			cmp     edx, PID_SUPER_STIMPAK
			jne     end
			mov     ecx, [edi+0x64]                      // ecx = target pid
			shr     ecx, 0x18
			cmp     ecx, ObjType_Critter
			jne     end
			push    edx
			push    eax
			mov     eax, edi                             // eax = target
			mov     edx, STAT_max_hit_points
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			cmp     eax, [edi+0x58]                      // max_hp == curr_hp?
			pop     eax
			pop     edx
			jne     end                                  // No
			pop     eax                                  // Destroying the return address
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     eax, [edi+SuperStimMsg]
			pop     edi

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x8B38D]
			mov     [esp+4], edi
			pop     edi
			retn

		end:
			mov     ecx, ebx                             // ecx = item
			retn
	}
}

static void __declspec(naked) loot_container_hook(void) {
	__asm {
			cmp     esi, 0x150                           // source_down
			je      scroll
			cmp     esi, 0x148                           // source_up
			jne     end
		scroll:
			push    edx
			push    ecx
			push    ebx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_wid]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_GNW_find_]
			call    edi
			pop     edi
			mov     ebx, [eax+0x8+0x0]                   // ebx = _i_wid.rect.x
			mov     ecx, [eax+0x8+0x4]                   // ecx = _i_wid.rect.y
			mov     eax, 422
			add     eax, ebx                             // x_start
			add     ebx, 422+64                          // x_end
			mov     edx, 35
			add     edx, ecx                             // y_start
			add     ecx, 35+6*48                         // y_end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mouse_click_in_]
			call    edi
			pop     edi
			pop     ebx
			pop     ecx
			pop     edx
			test    eax, eax
			jz      end
			cmp     esi, 0x150                           // source_down
			je      targetDown
			mov     esi, 0x18D                           // target_up
			jmp     end
		targetDown:
			mov     esi, 0x191                           // target_down
		end:
			mov     eax, esi
			cmp     eax, 0x151
			retn
	}
}

static void __declspec(naked) barter_inventory_hook2(void) {
	__asm {
			push    edx
			push    ecx
			push    ebx
			xchg    esi, eax
			cmp     esi, 0x150                           // source_down
			je      scroll
			cmp     esi, 0x148                           // source_up
			jne     end
		scroll:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__i_wid]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_GNW_find_]
			call    edi
			pop     edi
			mov     ebx, [eax+0x8+0x0]                   // ebx = _i_wid.rect.x
			mov     ecx, [eax+0x8+0x4]                   // ecx = _i_wid.rect.y
			push    ebx
			push    ecx
			mov     eax, 395
			add     eax, ebx                             // x_start
			add     ebx, 395+64                          // x_end
			mov     edx, 35
			add     edx, ecx                             // y_start
			add     ecx, 35+3*48                         // y_end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mouse_click_in_]
			call    edi
			pop     edi
			pop     ecx
			pop     ebx
			test    eax, eax
			jz      notTargetScroll
			cmp     esi, 0x150                           // source_down
			je      targetDown
			mov     esi, 0x18D                           // target_up
			jmp     end
		targetDown:
			mov     esi, 0x191                           // target_down
			jmp     end
		notTargetScroll:
			push    ebx
			push    ecx
			mov     eax, 250
			add     eax, ebx                             // x_start
			add     ebx, 250+64                          // x_end
			mov     edx, 20
			add     edx, ecx                             // y_start
			add     ecx, 20+3*48                         // y_end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mouse_click_in_]
			call    edi
			pop     edi
			pop     ecx
			pop     ebx
			test    eax, eax
			jz      notTargetBarter
			cmp     esi, 0x150                           // source_down
			je      barterTargetDown
			mov     esi, 0x184                           // target_barter_up
			jmp     end
		barterTargetDown:
			mov     esi, 0x176                           // target_barter_down
			jmp     end
		notTargetBarter:
			mov     eax, 165
			add     eax, ebx                             // x_start
			add     ebx, 165+64                          // x_end
			mov     edx, 20
			add     edx, ecx                             // y_start
			add     ecx, 20+3*48                         // y_end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_mouse_click_in_]
			call    edi
			pop     edi
			test    eax, eax
			jz      end
			cmp     esi, 0x150                           // source_down
			je      barterSourceDown
			mov     esi, 0x149                           // source_barter_up
			jmp     end
		barterSourceDown:
			mov     esi, 0x151                           // source_barter_down
		end:
			pop     ebx
			pop     ecx
			pop     edx
			mov     eax, esi
			cmp     eax, 0x151
			retn
	}
}

static void __declspec(naked) handle_inventory_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_container_exit_]
			call    edi
			pop     edi

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_stats_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

static void __declspec(naked) item_add_check(void) {
	__asm {
			push    edi
			push    esi
			push    edx
			push    ecx
			push    ebx
			mov     esi, eax
			mov     edi, edx
			cmp     ebx, 1
			jl      minus1
			mov     eax, [esi+0x20]                      // eax = pobj.fid
			and     eax, 0xF000000
			sar     eax, 0x18
			cmp     eax, ObjType_Critter
			jne     notCritter
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_body_type_]
			call    edi
			pop     edi
			test    eax, eax                             // Body_Type_Biped?
			jz      itsCritter
			mov     eax, -5
			jmp     end
		itsCritter:
			xchg    ecx, eax
			mov     eax, edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
			imul    eax, ebx                             // eax = item weight * quantity
			add     ecx, eax
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_total_weight_]
			call    edi
			pop     edi
			add     ecx, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			test    [edi+InLoop], 0x30000                // INTFACELOOT + BARTER
			pop     edi
			jz      noExtraWeightTarget
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     esi, ds:[edi+D__target_stack]
			pop     edi
			jne     noExtraWeightTarget
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			add     ecx, [edi+WeightOnBody]
			pop     edi
		noExtraWeightTarget:
			mov     edx, STAT_carry_amt
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			cmp     ecx, eax                             // total weight <= max weight?
			jle     force                                // Yes
			mov     eax, -6
			jmp     end
		notCritter:
			test    eax, eax                             // ObjType_Item?
			jnz     force                                // No (looks dumb - minus1 would be better instead of force)
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_container
			jne     notContainer
			xchg    edx, eax                             // eax = item
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_size_]
			call    edi
			pop     edi
			xchg    edx, eax                             // edx = item size
			imul    edx, ebx                             // edx = item size * quantity
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_curr_size_]
			call    edi
			pop     edi
			add     edx, eax
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_c_max_size_]
			call    edi
			pop     edi
			cmp     edx, eax                             // overall size <= max. size?
			jle     ownerContainer                       // Yes
			xor     eax, eax
			dec     eax
			dec     eax                                  // eax = -2 (no space in the container)
			jmp     end
		ownerContainer:
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_top_environment_]
			call    edi
			pop     edi
			test    eax, eax                             // Is there an owner?
			jz      force                                // No
			xchg    esi, eax                             // esi = container owner
			mov     eax, [esi+0x20]                      // eax = pobj.fid
			and     eax, 0xF000000
			sar     eax, 0x18
			cmp     eax, ObjType_Critter
			jne     force
			dec     eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     esi, ds:[edi+D__stack]                     // Does the bag belong to the player?
			pop     edi
			jne     itsCritter                           // No
			xchg    ecx, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			test    [edi+InLoop], 0x1000                       // INVENTORY
			pop     edi
			jz      notFound
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__curr_stack], ecx                // Drag an item in hand or armor into a bag in the main backpack?
			pop     edi
			jne     notFound                             // No
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     edi, ds:[edi+D__i_rhand]
			pop     edi
			je      found
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     edi, ds:[edi+D__i_lhand]
			pop     edi
			je      found
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     edi, ds:[edi+D__i_worn]
			pop     edi
			jne     notFound
		found:
			mov     eax, edi                             // Found a thing
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
			imul    eax, ebx                             // eax = item weight * quantity
			sub     ecx, eax                             // We adjust the weight
		notFound:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     eax, [edi+HiddenArmor]
			pop     edi
			test    eax, eax
			jz      noHiddenArmor
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_weight_]
			call    edi
			pop     edi
		noHiddenArmor:
			add     eax, ecx
			jmp     itsCritter
		notContainer:
			cmp     eax, item_type_misc_item
			jne     minus1
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_m_cell_pid_]
			call    edi
			pop     edi
			cmp     eax, [edi+0x64]
			je      force
		minus1:
			xor     eax, eax
			dec     eax
			jmp     end
		force:
			xor     eax, eax
		end:
			pop     ebx
			pop     ecx
			pop     edx
			pop     esi
			pop     edi
			retn
	}
}

static void __declspec(naked) item_add_mult(void) {
	__asm {
			mov     ecx, eax
			call    item_add_check
			test    eax, eax                             // May I add?
			jnz     end                                  // No
			xchg    ecx, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+StackEmptyWeapons], ecx
			pop     edi
			je      skip
			call    SetDefaultAmmo
		skip:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_add_force_]
			call    edi
			pop     edi
		end:
			pop     ecx
			retn
	}
}

static void __declspec(naked) drop_into_container_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     eax, ds:[edi+D__stack]                     // container == main backpack?
			pop     edi
			je      end                                  // Yes, moving armor/item from hand with open pouch or from open pouch to main backpack
			push    eax                                  // eax = source
			call    item_add_check
			cmp     eax, -2                              // Not enough space in your bag/backpack?
			pop     eax
			jne     end                                  // No
			inc     eax
			retn
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_add_force_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) item_add_force_call(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     dword ptr ds:[edi+D__curr_stack], 0
			pop     edi
			je      force
			// Was there an unsuccessful attempt to place in a bag or load a weapon?
			call    drop_into_container_hook
		force:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_add_force_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) move_table_source(void) {
	__asm {
			xchg    ecx, eax                             // ecx = source, eax = count
			xchg    ebx, eax                             // ebx = count, eax = item
			xchg    edx, eax                             // edx = item, eax = target
			mov     ebp, eax                             // ebp = target
			call    item_add_check
			cmp     eax, -2                              // Íå õâàòèò ìåñòà â ñóìêå/ðþêçàêå?
			je      end                                  // Äà
			xchg    ebp, eax                             // eax = target
			xchg    edx, eax                             // edx = target, eax = item
			xchg    ebx, eax                             // ebx = item, eax = count
			xchg    ecx, eax                             // ecx = count, eax = source

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_move_force_]
			mov     [esp+4], edi
			pop     edi
			retn
		end:
			inc     eax
			retn
	}
}

static void __declspec(naked) move_table_target(void) {
	__asm {
			mov     edx, ds:[D__target_curr_stack]
			mov     edx, ds:[D__target_stack][edx*4]
			jmp     move_table_source
	}
}

static void __declspec(naked) checkContainerSize(void) {
	__asm {
			push    eax
			call    item_add_check
			cmp     eax, -2                              // Íå õâàòèò ìåñòà â ñóìêå/ðþêçàêå?
			pop     eax
			je      end                                  // Äà

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_add_force_]
			mov     [esp+4], edi
			pop     edi
			retn

		end:
			xor     eax, eax
			dec     eax
			retn
	}
}

static void __declspec(naked) proto_ptr_call(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_proto_ptr_]
			call    edi
			pop     edi
			mov     edx, [esp+0x4]
			mov     eax, [edx+0x70]                      // eax = p_item.size
			test    eax, eax
			jnz     end
			cmp     dword ptr [edx+0x20], item_type_container
			jne     end
			mov     eax, [edx+0x24]                      // max_size
			shr     eax, 1                               // eax = max_size/2
			inc     eax
			mov     [edx+0x70], eax                      // container.size = max_size/2 + 1
		end:
			retn
	}
}

void InventoryInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Input", "ReloadWeaponKey", "NONE", prop_value, &sfall1_ini_info_G);
	*(char *) getRealBlockAddrData(&ReloadWeaponKey) = (char) (0 == strcmp(prop_value, "NONE") ? '\0' : prop_value[0]);
	if ('\0' != *(char *) getRealBlockAddrData(&ReloadWeaponKey)) {
		HookCallEXE(0x3B975, getRealBlockAddrCode((void *) &ReloadWeaponHotKey));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AutoReloadWeapon", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		HookCallEXE(0x20D56, getRealBlockAddrCode((void *) &AutoReloadWeapon));
	}

	// "Do not bring up the quantity selection window when dragging ammo to a weapon"
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ReloadReserve", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (temp_int >= 0) {
		// Note here: I didn't have to change anything, because the jumps are relative jumps.
		writeMem8EXE(0x6940D, 0xB8);
		writeMem32EXE(0x6940E, (uint32_t) temp_int);      // "mov  eax, ReloadReserve"
		writeMem32EXE(0x69412, 0x057FC139u);              // "cmp  ecx, eax; jg   0x6941B"
		writeMem32EXE(0x69416, 0xEB40C031u);              // "xor  eax, eax; inc  eax; jmps 0x6941E"
		writeMem32EXE(0x6941A, 0x91C12903u);              // "sub  ecx, eax; xchg ecx, eax"
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StackEmptyWeapons", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", (int *) getRealBlockAddrData(&StackEmptyWeapons));
	if (0 != *(uint32_t *) getRealBlockAddrData(&StackEmptyWeapons)) {
		MakeCallEXE(0x66B6E, getRealBlockAddrCode((void *) &inven_action_cursor_hook), true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "FreeWeight", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		MakeCallEXE(0x63C20, getRealBlockAddrCode((void *) &display_inventory_hook), true);
		MakeCallEXE(0x63F1B, getRealBlockAddrCode((void *) &display_target_inventory_hook), true);

		HookCallEXE(0x683E6, getRealBlockAddrCode((void *) &display_table_inventories_hook));

		writeMem16EXE(0x6844A, 0xD231);
		MakeCallEXE(0x6847F, getRealBlockAddrCode((void *) &display_table_inventories_hook1), true);
		HookCallEXE(0x685A2, getRealBlockAddrCode((void *) &display_table_inventories_hook2));

		HookCallEXE(0x688BB, getRealBlockAddrCode((void *) &barter_inventory_hook));
		HookCallEXE(0x68CB9, getRealBlockAddrCode((void *) &barter_inventory_hook1));
	}

	// "Using chemistry from the inventory on the player's picture"
	HookCallEXE(0x64E38, getRealBlockAddrCode((void *) &inven_pickup_hook2));

	// "Show max weight in inventory"
	MakeCallEXE(0x65D1F, getRealBlockAddrCode((void *) &display_stats_hook), true);

	// ""Take All" and "Put All" Buttons"
	MakeCallEXE(0x6352A, getRealBlockAddrCode((void *) &make_loot_drop_button), false);
	MakeCallEXE(0x672C1, getRealBlockAddrCode((void *) &loot_drop_all), false);
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "OverloadedLoot", "Sorry, you cannot carry that much.",
					OverloadedLoot, &translation_ini_info_G);
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "OverloadedDrop", "Sorry, there is no space left.",
					OverloadedDrop, &translation_ini_info_G);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SuperStimExploitFix", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "SuperStimExploitMsg",
						"You cannot use a super stim on someone who is not injured!", SuperStimMsg, &translation_ini_info_G);
		MakeCallEXE(0x8B34F, getRealBlockAddrCode((void *) &protinst_use_item_on_hook), false);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Input", "UseScrollWheel", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*(bool *) getRealBlockAddrData(&UseScrollWheel) = (1 == temp_int);
	if (*(bool *) getRealBlockAddrData(&UseScrollWheel)) {
		MakeCallEXE(0x67251, getRealBlockAddrCode((void *) &loot_container_hook), false);
		MakeCallEXE(0x6896B, getRealBlockAddrCode((void *) &barter_inventory_hook2), false);
		// No idea what this below is for (I wish documentation existed on sFall1 source... ;_;), but if I enable it,
		// the game won't like it (will try to access invalid memory). Though, the scroll (and the rest of the game)
		// works if I disable it. So I'll keep it disabled. Wonder what it's supposed to do...
		//*((uint32_t *) D__max_ptr) = 100;
	}


	// Refreshing data and window (in particular in the tuck) and closing the inner bag
	HookCallEXE(0x6294E, getRealBlockAddrCode((void *) &handle_inventory_hook));

	// Whether it is necessary to ink one function
	MakeCallEXE(0x69F19, getRealBlockAddrCode((void *) &item_add_mult), true);

	// Using the correct functions n and the corresponding conditions
	HookCallEXE(0x6936E, getRealBlockAddrCode((void *) &drop_into_container_hook));
	HookCallEXE(0x64BEB, getRealBlockAddrCode((void *) &item_add_force_call));

	// P and confirm whether to use the main backpack and oka, and not the opened bag
	writeMem32EXE(0x68BF6, (uint32_t) getRealEXEAddr(D__stack));

	// When dragging a smot from the window into the bag, a new free air gap is added, but also a bellow
	HookCallEXE(0x68318, getRealBlockAddrCode((void *) &move_table_source));
	HookCallEXE(0x68388, getRealBlockAddrCode((void *) &move_table_target));

	// Report the absence of a place in the monitor window, and in the ovl window
	HookCallEXE(0x683B2, getRealEXEAddr(C_gdialog_display_msg_));
	writeMem16EXE(0x68390, 0x8EEB);             // jmps 0x4684F0

	// You can’t take it out of the bag to your hand / if it gets into the bag last day
	HookCallEXE(0x64F1E, getRealBlockAddrCode((void *) &checkContainerSize));
	HookCallEXE(0x64DD2, getRealBlockAddrCode((void *) &checkContainerSize));

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ContainerSizeFix", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		HookCallEXE(0x6A729, getRealBlockAddrCode((void *) &proto_ptr_call));
		HookCallEXE(0x6C073, getRealBlockAddrCode((void *) &proto_ptr_call));
	}
}
