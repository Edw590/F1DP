// Copyright (C) 2022 Edw590
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
// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/conio.h"
#include "../CLibs/ctype.h"
#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/General.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "AI.h"
#include "AmmoMod.h"
#include "AnimationsAtOnceLimit.h"
#include "Bugs.h"
#include "Credits.h"
#include "Criticals.h"
#include "DebugMode.h"
#include "Define.h"
#include "Inventory.h"
#include "LoadGameHook.h"
#include "MainMenu.h"
#include "PartyControl.h"
#include "Quests.h"
#include "SFall1Main.h"
#include "SFall1Patches.h"
#include "Sound.h"


// ADVICE: don't try to understand the functions in each patch... Infinite EDI register uses there to be able to have
// the special numbers in use. If you want to understand what was done, just go to the sFall1 source and see there. If
// you think anything is wrong with the ones here, redo from scratch, don't try to fix. It's a mess with this way of
// Special Numbers, but I don't have better ideas without making a relocation table for the EXE, parsing it and applying
// it (as long as this way works, I think it's much better than going that route, which would take infinity).

/*
 * // ///////////////////////////////////////////////////////////////
 *         RULES TO FOLLOW IN THE ASSEMBLY PORTS!!!!!!!!!!!
 *
 * Description of prefixes used below:
 * - F_: macro containing an address for a standard C function on the code section of the EXE
 * - C_: macro containing an address for anything else on the code section of the EXE
 * - D_: macro containing an address for anything on the data section of the EXE
 *
 * - All absolute addresses must have added to them a Special Number corresponding to where that absolute address refers
 *   to:
 *   - SN_CODE_SEC_EXE_ADDR if it's from the Code section of the game EXE (for example a function)
 *   - SN_DATA_SEC_EXE_ADDR if it's from the Data section of the game EXE (for example a string)
 *   - SN_CODE_SEC_BLOCK_ADDR if it's from the Code section of this patcher (for example a function)
 *   - SN_DATA_SEC_BLOCK_ADDR if it's from the Data section of this patcher (for example a string)
 * - Use the EDI register where possible, else use ESI. If for some reason both of those are needed for the code, use
 *   whatever other register you want. But first use the other 2 in that order so that it's easy to see what was done.
 * - Use these instructions ALWAYS for the ports:
 * 			push    edi
 * 			mov     edi, SN_CODE_SEC_EXE_ADDR
 * 			[whatever instruction you want - but don't forget of adding EDI to it, like mov ecx, [edi+0x3842], being
 * 			0x3842 an absolute address]
 * 			pop     edi
 * - PAY ATTENTION TO THE STACK if the code inside the PUSH and POP instructions relies on the stack. If it's a function
 *   and that function requires the parameters on the stack, put the PUSH before all function PUSHes. If ESP is used
 *   with something like `mov ecx, [esp+0x3842]`, don't forget of adding a +4 on it for the PUSH EDI instruction.
 *   If the function requires parameters on the stack and you don't want to make the jump or CALL sooner in code before
 *   all function PUSHes (you want to replace a CALL right there, after various PUSHes), you can move all the parameters
 *   for the function on the stack down 4 bytes, and put EDI on the 4 bytes that are now free. Example on how to do
 *   that, for 6 parameters:
 * 			lea     esp, [esp-4] // Reserve space on the stack for the last function PUSH
 * 			push    edi
 * 			mov     edi, [(esp+4)+4*1]
 * 			mov     [(esp+4)+4*0], edi
 * 			mov     edi, [(esp+4)+4*2]
 * 			mov     [(esp+4)+4*1], edi
 * 			mov     edi, [(esp+4)+4*3]
 * 			mov     [(esp+4)+4*2], edi
 * 			mov     edi, [(esp+4)+4*4]
 * 			mov     [(esp+4)+4*3], edi
 * 			mov     edi, [(esp+4)+4*5]
 * 			mov     [(esp+4)+4*4], edi
 * 			mov     edi, [(esp+4)+4*6]
 * 			mov     [(esp+4)+4*5], edi
 * 			pop     edi
 * 			mov     [esp+4*6], edi // This will be the "PUSH" before all the function PUSHes
 *
 * 	 WARNING ABOUT THIS ABOVE!!!! Do NOT make a real PUSH before all the function PUSHes if the arguments are passed on
 * 	 the stack!!!! If you PUSH EDI before everything else and then EDI is changed inside, the value that will be
 * 	 recovered will be the old EDI value before the modifications after the PUSH and before the function call!!!!
 * 	 So a `lea esp, [esp-4]` must be done instead (LEA and not SUB because LEA does not change any flags and SUB does),
 * 	 and right before where EDI is used, a `mov [esp+4*?], edi` issued. The "?" is the number of stack parameters the
 * 	 function takes - that will put EDI before all others).
 * 	 ALWAYS CHECK IF ESP IS USED BETWEEN THE SUBTRACTION AND THE POP!!!!! If it is, ESP will need 4 added to it every
 * 	 time!!!
 *
 * - If you need to PUSH an absolute address to the stack:
 *			lea     esp, [esp-4] // Reserve space for the address
 *			push    edi
 *			mov     edi, SN_CODE_SEC_EXE_ADDR
 *			lea     edi, [edi+C_text_font_]
 *			mov     [esp+4], edi
 *			pop     edi
 * - If you need to JMP to an absolute address --> do the above and add RET at the end, which will behave exactly like
 *   a JMP. If a conditional jump is required, I've not thought about it (hasn't been needed yet anyway - if it is, I'll
 *   write here, hopefully).
 * - WARNING: THERE ARE POINTERS TO FUNCTIONS IN THE DATA SECTION!!!! THE STACK MUST BE PAID ATTENTION TO IN THOSE CASES
 *   TOO!!!!! Simple idea: Ctrl+F all CALLs (also because one could put the pointer on EAX and only call it later, so
 *   Ctrl+F ALL CALLs).
 * - Don't forget near CALLs or near jumps to the Patcher functions can NOT have an SN added to them, because they're
 *   an offset, not an address!!!
 * - Do NOT use ANY instruction that changes the FLAGS registers!!! For example, ADD is one of them. Instead, use LEA.
 *   The SUB instruction has the same treatment. Do the same for any others required. The flags must NEVER be changed
 *   when porting the code (imagine they're used somewhere not immediately noticed...)!
 *
 * --> Steps to ALWAYS follow do the above:
 * 1. For ports from:
 *    - source-code: find all references to the macros and put C_ or D_ on them.
 *    - Assembly code: find all references to variables (which will contain the address of the wanted thing) and replace
 *    them with the referenced symbol name and put C_ or D_ on them. Find also all hexadecimal numbers in ????h form
 *    (highlight all "h"s) and replace with the 0x???? form.
 * 2. Go on FalloutEngine.h and go do what is required there for every macro (as a start, put the address there).
 * 3. Put on Ctrl+F (to highlight) "edi" so that it's easy to see where that's being used, because if it's being used
 *    on a `mov edi, ds:[0x23948]`, for example, then EDI can't be used for the SN and must be ESI (in this case).
 * 4. Find all references to global variables inside the Patcher and use SN_DATA_SEC_BLOCK_ADDR on them. If it's a CALL
 *    to a function internal to the Patcher, do NOT correct anything. It's a relative CALL, not an absolute one, and the
 *    offset is correct when the Patcher compiles.
 * 5. Find all CALLs and pay attention to the stack (check the documentation of the macro to see what the "Args" are.
 *    Use the corresponding SN depending on the C_ or D_ prefix.
 * 6. Find all "0x"s, check if they are addresses, and if they are, use the appropriate SN.
 * 7. Go check all the C_s, D_s and F_s that remained untouched and use the appropriate SN.
 * 8. Search for all the 4 addresses SN_ constants. For:
 *    - SN_CODE_SEC_EXE_ADDR and SN_DATA_SEC_EXE_ADDR, check if what's right below them is a macro. If it's not, correct
 *      the mistake. If it's a macro, check that it's a C_ one if it's the CODE SN in use, or a D_ one if it's the DATA
 *      SN in use.
 *    - SN_DATA_SEC_BLOCK_ADDR, check if what's below them is NOT a macro. If it is, correct the mistake. If it's not a
 *      macro, check if the name below is of global variable if it's the
 *      DATA SN in use.
 *
 * // ///////////////////////////////////////////////////////////////
 */


struct FileInfo translation_ini_info_G = {0};

static char mapName[65] = {0};
static char versionString[65] = {0};
static char windowName[65] = {0};
static char configName[65] = {0};
static char dmModelName[65] = {0};
static char dfModelName[65] = {0};
static char MovieNames[14 * 65] = {0};

static const char *origMovieNames[14] = {
		"iplogo.mve",
		"mplogo.mve",
		"intro.mve",
		"vexpld.mve",
		"cathexp.mve",
		"ovrintro.mve",
		"boil3.mve",
		"ovrrun.mve",
		"walkm.mve",
		"walkw.mve",
		"dipedv.mve",
		"boil1.mve",
		"boil2.mve",
		"raekills.mve",
};

static uint32_t objItemOutlineState = 0;
static uint32_t toggleHighlightsKey = 0;
static uint32_t TurnHighlightContainers = 0;

uint32_t _combatNumTurns = 0;
uint32_t _tmpQNode = 0;
uint32_t FakeCombatFix = 0;

static const uint32_t WalkDistance[4] = {
		0x11E61+2,                                  // action_use_an_item_on_object_
		0x12026+2,                                  // action_get_an_object_
		0x122DF+2,                                  // action_loot_container_
		0x12690+2,                                  // action_use_skill_on_
};

static const uint32_t PutAwayWeapon[5] = {
		0x11D1E,                                  // action_climb_ladder_
		0x11EB5,                                  // action_use_an_item_on_object_
		0x120AE,                                  // action_get_an_object_
		0x55F2C,                                  // intface_change_fid_animate_
		0x65F33,                                  // inven_wield_
};

// Don't delete this. Already has the correct addresses. Less work if this is useful again.
/*static const uint32_t TimedRest[6] = {
		0x89417, 0x89456, 0x894EC, 0x895F1, 0x89634, 0x896FB,
};

static const uint32_t world_map[4] = {
		0xAAEF6, 0xAB065, 0xAB144, 0xAB197,
};*/

static char KarmaGainMsg[128] = {0};
static char KarmaLossMsg[128] = {0};

static void __stdcall SetKarma(int value) {
	char buf[128];
	if (value > 0) {
		sprintf(buf, KarmaGainMsg, value);
	} else {
		sprintf(buf, KarmaLossMsg, -value);
	}
	__asm {
			lea     eax, [buf]
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_print_]
			call    edi
			pop     edi
	}
}

__declspec(naked) static void op_set_global_var_hook(void) {
	__asm {
			cmp     eax, 155                             // PLAYER_REPUTATION
			jne     end
			pushad
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_game_get_global_var_]
			call    edi
			pop     edi
			sub     edx, eax
			test    edx, edx
			jz      skip
			push    edx
			call    SetKarma
		skip:
			popad
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_game_set_global_var_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void intface_item_reload_hook(void) {
	__asm {
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_register_clear_]
			call    edi
			pop     edi
			xor     eax, eax
			inc     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_register_begin_]
			call    edi
			pop     edi
			xor     edx, edx
			xor     ebx, ebx
			dec     ebx
			pop     eax                                  // _obj_dude
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_register_object_animate_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_register_end_]
			call    edi
			pop     edi
			popad

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_gsound_play_sfx_file_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

static uint32_t RetryCombatMinAP = 0;
__declspec(naked) static void combat_turn_hook(void) {
	__asm {
			xor     eax, eax
		retry:
			xchg    ebx, eax
			mov     eax, esi
			push    edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_ai_]
			call    edi
			pop     edi
			pop     edx
		process:
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     dword ptr ds:[edi+D__combat_turn_running], 0
			pop     edi
			jle     next
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_process_bk_]
			call    edi
			pop     edi
			jmp     process
		next:
			mov     eax, [esi+0x40]                      // curr_mp
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     eax, [edi+RetryCombatMinAP]
			pop     edi
			jl      end
			cmp     eax, ebx
			jne     retry
		end:
			ret
	}
}

__declspec(naked) static void intface_rotate_numbers_hook(void) {
	__asm {
			push    edi
			push    ebp
			sub     esp, 0x54
			// ebx=old value, ecx=new value
			cmp     ebx, ecx
			je      end
			mov     ebx, ecx
			jg      decrease
			dec     ebx
			jmp     end
		decrease:
			test    ecx, ecx
			jl      negative
			inc     ebx
			jmp     end
		negative:
			xor     ebx, ebx
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x563F6]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void obj_outline_all_items_on(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__map_elevation]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_find_first_at_]
			call    edi
			pop     edi
		loopObject:
			test    eax, eax
			jz      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     eax, ds:[edi+D__outlined_object]
			pop     edi
			je      nextObject
			xchg    ecx, eax
			mov     edx, 0x10                            // yellow
			mov     eax, [ecx+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // Is it ObjType_Item?
			jz      skip                                 // Yes
			dec     eax                                  // Is it ObjType_Critter?
			jnz     nextObject                           // No
			test    byte ptr [ecx+0x44], 0x80            // source.results & DAM_DEAD?
			jz      nextObject                           // No
			push    edx
			sub     esp, 4
			mov     edx, esp
			mov     eax, [ecx+0x64]                      // eax = pid
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_proto_ptr_]
			call    edi
			pop     edi
			mov     edx, [esp]
			add     esp, 4
			xchg    eax, edx
			inc     edx
			pop     edx
			jz      nextObject
			test    byte ptr [ecx+0x20], 0x20            // Can't be stolen from?|Can't be robbed?
			jnz     nextObject                           // Yes
			or      [ecx+0x25], dl
			xor     eax, eax
		skip:
			cmp     [ecx+0x7C], eax                      // Does it belong to someone?
			jnz     nextObject                           // Yes
			test    [ecx+0x74], eax                      // Already illuminated?
			jnz     nextObject                           // Yes
			test    [ecx+0x25], dl                       // Is NoHighlight_ set (is it a container)?
			jz      NoHighlight                          // No
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+TurnHighlightContainers], eax   // Highlight containers?
			pop     edi
			je      nextObject                           // No
			shr     edx, 2                               // Gray (equals `mov edx, 0x4`)
		NoHighlight:
			mov     [ecx+0x74], edx
		nextObject:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_find_next_at_]
			call    edi
			pop     edi
			jmp     loopObject
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_tile_refresh_display_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void obj_outline_all_items_off(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__map_elevation]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_find_first_at_]
			call    edi
			pop     edi
		loopObject:
			test    eax, eax
			jz      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     eax, ds:[edi+D__outlined_object]
			pop     edi
			je      nextObject
			xchg    ecx, eax
			mov     eax, [ecx+0x20]
			and     eax, 0xF000000
			sar     eax, 0x18
			test    eax, eax                             // Is it ObjType_Item?
			jz      skip                                 // Yes
			dec     eax                                  // Is it ObjType_Critter?
			jnz     nextObject                           // No
			test    byte ptr [ecx+0x44], 0x80            // source.results & DAM_DEAD?
			jz      nextObject                           // No
		skip:
			cmp     [ecx+0x7C], eax                      // Does it belong to someone?
			jnz     nextObject                           // Yes
			mov     [ecx+0x74], eax
		nextObject:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_find_next_at_]
			call    edi
			pop     edi
			jmp     loopObject
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_tile_refresh_display_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void gmouse_bk_process_hook(void) {
	__asm {
			test    eax, eax
			jz      end
			test    byte ptr [eax+0x25], 0x10            // NoHighlight_
			jnz     end
			mov     dword ptr [eax+0x74], 0
		end:
			mov     edx, 0x40

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_outline_object_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void obj_remove_outline_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_remove_outline_]
			call    edi
			pop     edi
			test    eax, eax
			jnz     end
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     eax, [edi+objItemOutlineState]
			pop     edi
			je      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__outlined_object], eax
			pop     edi
			pushad
			call    obj_outline_all_items_on
			popad
		end:
			ret
	}
}

static bool wasToggleHighlightsKeyPressed(char c) {
	// I was checking if the wanted key was in the BIOS keyboard buffer. And that works. But not in this case, as it
	// seems the game first removes the key from the buffer before ReloadWeaponHotKey() is called. So my second thought
	// was to go check the key where it is still in the buffer and put that in a global variable which would be checked
	// in ReloadWeaponHotKey().
	// Until I realized the key is on EBX on the call to the mentioned function. It's not a key code - it's the
	// character. So a character must now be checked and not a key code - less options, but it's the easiest without
	// having to go to some place where the key is still in the buffer, check that and put in a global variable.

	return (toupper(c) == toupper(GET_BD_SYM(char, toggleHighlightsKey)));
}

__declspec(naked) static void get_input_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_get_input_]
			call    edi
			pop     edi
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     bl, byte ptr [edi+toggleHighlightsKey]
			pop     edi
			test    bl, bl
			jz      end
			call    wasToggleHighlightsKeyPressed // EAX is the parameter
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     ebx, [edi+objItemOutlineState]
			pop     edi
			test    al, al
			jz      notOurKey
			test    ebx, ebx
			jnz     end
			inc     ebx
			call    obj_outline_all_items_on
			jmp     setState
		notOurKey:
			test    ebx, ebx
			jz      end
			dec     ebx
			call    obj_outline_all_items_off
		setState:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+objItemOutlineState], ebx
			pop     edi
		end:
			popad
			ret
	}
}

__declspec(naked) static void pipboy_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_get_input_]
			call    edi
			pop     edi
			cmp     eax, '1'
			jne     notOne
			mov     eax, 0x1F4
			jmp     click
		notOne:
			cmp     eax, '2'
			jne     notTwo
			mov     eax, 0x1F8
			jmp     click
		notTwo:
			cmp     eax, '3'
			jne     notThree
			mov     eax, 0x1F5
			jmp     click
		notThree:
			cmp     eax, '4'
			jne     notFour
			mov     eax, 0x1F6
		click:
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_gsound_red_butt_press_]
			call    edi
			pop     edi
			pop     eax
		notFour:
			ret
	}
}

static const char _nar_31[] = "nar_31";
__declspec(naked) static void Brotherhood_final(void) {
	__asm {
			mov     eax, 16                              // BROTHERHOOD_INVADED
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_game_get_global_var_]
			call    edi
			pop     edi
			test    eax, eax
			jnz     nar_31
			mov     eax, 605                             // RHOMBUS_STATUS
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_game_get_global_var_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edx, [edi+0x38B45]
			pop     edi
			test    eax, eax
			jz      RhombusDead
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edx, [edi+0x38B18]
			pop     edi
		RhombusDead:
			jmp     edx
		nar_31:
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    0
			mov     edx, 317                             // SEQ5D.FRM
			mov     eax, ObjType_Intrface
			xor     ecx, ecx
			xor     ebx, ebx
			mov     [esp+1*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edi+_nar_31]
			pop     edi

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x38B5F]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

static uint32_t Educated = 0;
static uint32_t Lifegiver = 0;
static uint32_t Tag_ = 0;
static uint32_t Mutate_ = 0;
__declspec(naked) static void editor_design_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_SavePlayer_]
			call    edi
			pop     edi
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__Educated]
			mov     [esi+Educated], eax
			mov     eax, ds:[edi+D__Lifegiver]
			mov     [esi+Lifegiver], eax
			mov     eax, ds:[edi+D__Tag_]
			mov     [esi+Tag_], eax
			mov     eax, ds:[edi+D__Mutate_]
			mov     [esi+Mutate_], eax
			pop     edi
			pop     esi
			ret
	}
}

__declspec(naked) static void editor_design_hook2(void) {
	__asm {
			push    esi
			push    edi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, [esi+Educated]
			mov     ds:[edi+D__Educated], eax
			mov     eax, [esi+Lifegiver]
			mov     ds:[edi+D__Lifegiver], eax
			mov     eax, [esi+Tag_]
			mov     ds:[edi+D__Tag_], eax
			mov     eax, [esi+Mutate_]
			mov     ds:[edi+D__Mutate_], eax
			pop     edi
			pop     esi

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_RestorePlayer_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void perks_dialog_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_ListSkills_]
			call    edi
			pop     edi
			mov     eax, PERK_educated
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__Educated], eax
			pop     edi
			mov     eax, PERK_lifegiver
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__Lifegiver], eax
			pop     edi
			mov     eax, PERK_tag
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__Tag_], eax
			pop     edi
			mov     eax, PERK_mutate
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ds:[edi+D__Mutate_], eax
			pop     edi
			ret
	}
}

__declspec(naked) static void perk_can_add_hook(void) {
	__asm {
			imul    edx, eax, 3
			add     edx, [ecx+0xC]
			mov     eax, PCSTAT_level
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_pc_get_]
			call    edi
			pop     edi
			cmp     eax, edx
			jge     end

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x866D1]
			mov     [esp+4], edi
			pop     edi
			ret

		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x866D8]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void FirstTurnAndNoEnemy(void) {
	__asm {
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1
			pop     edi
			jz      end                                  // Not in battle
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+_combatNumTurns], eax
			pop     edi
			jne     end                                  // This is not the first move
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_should_end_]
			call    edi
			pop     edi
			test    eax, eax                             // Are there enemies?
			jz      end                                  // Yes
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ecx, ds:[edi+D__list_total]
			mov     edx, ds:[edi+D__obj_dude]
			pop     edi
			mov     edx, [edx+0x50]                      // team_num player support groups
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__combat_list]
			pop     esi
		loopCritter:
			mov     eax, [edi]                           // eax = the character
			mov     ebx, [eax+0x50]                      // team_num character support groups
			cmp     edx, ebx                             // Bro?
			je      nextCritter                          // Yes
			mov     eax, [eax+0x54]                      // who_hit_me
			test    eax, eax                             // Has the character been shot?
			jz      nextCritter                          // No
			cmp     edx, [eax+0x50]                      // Shot from a player's support team?
			jne     nextCritter                          // No
			popad
			dec     eax                                  // Showdown!!!
			ret
		nextCritter:
			add     edi, 4                               // To the next character on the list
			loop    loopCritter                          // Looping through the entire list
			popad
		end:
			ret
	}
}

__declspec(naked) static void FirstTurnCheckDist(void) {
	__asm {
			push    eax
			push    edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_dist_]
			call    edi
			pop     edi
			cmp     eax, 1                               // Is the distance to the object greater than 1?
			pop     edx
			pop     eax
			jle     end                                  // No
			push    edx
			push    ebx
			sub     esp, 12
			mov     edx, esp
			mov     ebx, 661                             // 'Too far away.'
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     eax, [edi+D__proto_main_msg_file]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_getmsg_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_display_print_]
			call    edi
			pop     edi
			add     esp, 12
			pop     ebx
			pop     edx
			pop     eax                                  // Destroying the return address
			xor     eax, eax
			dec     eax
		end:
			ret
	}
}

__declspec(naked) static void check_move_hook(void) {
	__asm {
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			jnz     skip                                 // Yes
			cmp     dword ptr [ecx], -1
			je      end
			ret

		skip:
			xor     esi, esi
			dec     esi
		end:
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x179BF]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void gmouse_bk_process_hook1(void) {
	__asm {
			xchg    ebp, eax
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			jnz     end                                  // Yes
			xchg    ebp, eax
			cmp     eax, [edx+0x40]
			jg      end
			ret

		end:
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x43625]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void FakeCombatFix1(void) {
	__asm {
			push    eax                                  // _obj_dude
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			pop     eax
			jz      end                                  // No
			call    FirstTurnCheckDist
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_action_get_an_object_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void FakeCombatFix2(void) {
	__asm {
			push    eax                                  // _obj_dude
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			pop     eax
			jz      end                                  // No
			call    FirstTurnCheckDist
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_action_loot_container_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) void FakeCombatFix3(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+FakeCombatFix], 0
			pop     edi
			je      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			cmp     ds:[edi+D__obj_dude], eax
			pop     edi
			jne     end
			push    eax
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			pop     eax
			jz      end                                  // No
			call    FirstTurnCheckDist
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_action_use_an_item_on_object_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void combat_begin_hook(void) {
	__asm {
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+_combatNumTurns], eax
			pop     edi
			dec     eax
			ret
	}
}

__declspec(naked) static void combat_reset_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+_combatNumTurns], edx
			pop     edi
			mov     edx, STAT_max_move_points
			ret
	}
}

__declspec(naked) static void combat_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     dword ptr [edi+_combatNumTurns]
			pop     edi

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_should_end_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

static uint32_t MaxPCLevel = 21;
__declspec(naked) static void stat_pc_min_exp_hook(void) {
	__asm {
			inc     eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     eax, [edi+MaxPCLevel]
			pop     edi
			jg      maxLevel
			push    ebx
			mov     ebx, eax
			mov     edx, eax
			sar     edx, 0x1F
			sub     eax, edx
			sar     eax, 1
			test    bl, 1
			jnz     skip
			dec     ebx
		skip:
			imul    ebx, eax
			mov     eax, ebx
			shl     eax, 5
			sub     eax, ebx
			shl     eax, 2
			add     eax, ebx
			shl     eax, 3
			pop     ebx
			jmp     end
		maxLevel:
			xor     eax, eax
			dec     eax
		end:
			pop     edx                                  // Destroying the return address
			pop     edx
			ret
	}
}

static uint32_t RemoveFriendlyFoe = 0;
static uint32_t ColorLOS = 0;
__declspec(naked) static void combat_update_critter_outline_for_los(void) {
	__asm {
			pushad
			xchg    esi, eax                             // esi = target
			mov     eax, [esi+0x64]
			shr     eax, 0x18
			cmp     eax, ObjType_Critter
			jne     end                                  // No
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ecx, ds:[edi+D__obj_dude]
			pop     edi
			cmp     ecx, esi                             // Is this a player?
			je      end                                  // Yes
			mov     eax, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_critter_is_dead_]
			call    edi
			pop     edi
			test    eax, eax                             // Is it a corpse?
			jnz     end                                  // Yes
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__combat_highlight]
			pop     esi
			push    ecx
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    eax
			mov     eax, esi
			xchg    ecx, eax                             // ecx = target, eax=source (_obj_dude)
			mov     ebx, [ecx+0x4]                       // target_tile
			mov     edx, [eax+0x4]                       // source_tile
			mov     [esp+1*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_is_shot_blocked_]
			call    edi
			pop     edi
			pop     edx
			xor     ecx, ecx
			mov     ebp, [esi+0x74]                      // outline
			and     ebp, 0xFFFFFF                        // ebp=current outline color
			test    eax, eax                             // Are there barriers?
			jnz     itsLOS                               // Yes
			// Line of sight
			inc     ecx
			cmp     ebp, ecx                             // red iridescent
			je      alreadyOutlined
			cmp     ebp, 8                               // green iridescent
			je      alreadyOutlined
			mov     ebp, ecx                             // red iridescent
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+RemoveFriendlyFoe], eax         // Ignore Friendly Foe?
			pop     edi
			jne     skipFriendlyFoe                      // Yes
			mov     eax, PERK_friendly_foe
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perk_level_]
			call    edi
			pop     edi
			test    eax, eax
			jz      noFriend
		skipFriendlyFoe:
			mov     eax, [edx+0x50]                      // team_num
			cmp     eax, [esi+0x50]                      // Bro?
			jnz     noFriend                             // No
			add     ebp, 7                               // green iridescent
		noFriend:
			jmp     setOutlined
		itsLOS:
			xchg    edx, eax                             // eax = _obj_dude
			push    eax
			mov     edx, esi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_dist_]
			call    edi
			pop     edi
			xchg    ebx, eax                             // ebx = distance from player to target
			mov     edx, STAT_pe
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_level_]
			call    edi
			pop     edi
			lea     edx, ds:0[eax*4]
			add     edx, eax                             // edx = Perception * 5
			test    byte ptr [esi+0x26], 2               // TransGlass_
			jz      noGlass
			mov     eax, edx
			sar     edx, 0x1F
			sub     eax, edx
			sar     eax, 1                               // edx = (Perception * 5) / 2
			mov     edx, eax
		noGlass:
			cmp     ebx, edx
			jg      outRange
			inc     ecx
		outRange:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     ebp, [edi+ColorLOS]
			pop     edi
			je      alreadyOutlined
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     ebp, [edi+ColorLOS]
			pop     edi
		setOutlined:
			mov     eax, esi
			xor     edx, edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_turn_off_outline_]
			call    edi
			pop     edi
			mov     eax, esi
			xor     edx, edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_remove_outline_]
			call    edi
			pop     edi
			test    ecx, ecx
			jz      end
			mov     edx, ebp
			mov     eax, esi
			xor     ebx, ebx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_outline_object_]
			call    edi
			pop     edi
			xchg    esi, eax
			xor     edx, edx
			test    edi, edi
			jz      turn_off_outline
			jmp     turn_on_outline
		alreadyOutlined:
			mov     eax, esi
			xor     edx, edx
			test    byte ptr [esi+0x77], 0x80            // OutlineOff_
			jz      turn_off_outline
		turn_on_outline:
			test    edi, edi
			jz      end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_turn_on_outline_]
			call    edi
			pop     edi
			jmp     end
		turn_off_outline:
			test    edi, edi
			jnz     end
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_turn_off_outline_]
			call    edi
			pop     edi
		end:
			popad
			ret
	}
}

__declspec(naked) static void obj_move_to_tile_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1
			pop     edi
			jz      end                                  // Not in battle
			mov     eax, [esp+0x44]
			call    combat_update_critter_outline_for_los
		end:
			mov     ebx, [esp+0x4C]
			test    ebx, ebx

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x7C2D0]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) void queue_find_first_(void) {
	__asm {
			push    ecx
			// eax = source, edx = type
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ecx, ds:[edi+D__queue]
			pop     edi
		loopQueue:
			jecxz   skip
			cmp     eax, [ecx+0x8]                       // queue.object
			jne     nextQueue
			cmp     edx, [ecx+0x4]                       // queue.type
			jne     nextQueue
			mov     eax, [ecx+0xC]                       // queue.data
			jmp     end
		nextQueue:
			mov     ecx, [ecx+0x10]                      // queue.next_queue
			jmp     loopQueue
		skip:
			xor     eax, eax
		end:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+_tmpQNode], ecx
			pop     edi
			pop     ecx
			ret
	}
}

__declspec(naked) void queue_find_next_(void) {
	__asm {
			push    ecx
			// eax = source, edx = type
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     ecx, [edi+_tmpQNode]
			pop     edi
			jecxz   skip
		loopQueue:
			mov     ecx, [ecx+0x10]                      // queue.next_queue
			jecxz   skip
			cmp     eax, [ecx+0x8]                       // queue.object
			jne     loopQueue
			cmp     edx, [ecx+0x4]                       // queue.type
			jne     loopQueue
			mov     eax, [ecx+0xC]                       // queue.data
			jmp     end
		skip:
			xor     eax, eax
		end:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+_tmpQNode], ecx
			pop     edi
			pop     ecx
			ret
	}
}

__declspec(naked) static void print_with_linebreak(void) {
	__asm {
			push esi
			push ecx
			test eax, eax                             // Is there a line?
			jz   end                                  // No
			mov  esi, eax
			xor  ecx, ecx
		loopString:
			cmp  byte ptr [esi], 0                    // End of line
			je   printLine                            // Yes
			cmp  byte ptr [esi], 0x5C                 // Perhaps a line break? '\'
			jne  nextChar                             // No
			cmp  byte ptr [esi+1], 0x6E               // Line feed exactly? 'n'
			jne  nextChar                             // No
			inc  ecx
			mov  byte ptr [esi], 0
		printLine:
			call edi
			jecxz end
			dec  ecx
			mov  byte ptr [esi], 0x5C
			inc  esi
			mov  eax, esi
			inc  eax
		nextChar:
			inc  esi
			jmp  loopString
		end:
			pop  ecx
			pop  esi
			ret
	}
}

__declspec(naked) static void display_print_with_linebreak(void) {
	__asm {
			push    edi
			push    esi
			mov     esi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [esi+C_display_print_]
			pop     esi
			call    print_with_linebreak
			pop     edi
			ret
	}
}

__declspec(naked) static void inven_display_msg_with_linebreak(void) {
	__asm {
			push    edi
			push    esi
			mov     esi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [esi+C_inven_display_msg_]
			pop     esi
			call    print_with_linebreak
			pop     edi
			ret
	}
}

static int drugExploit = 0;
__declspec(naked) static void protinst_use_item_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     dword ptr [edi+drugExploit]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_use_book_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			dec     dword ptr [edi+drugExploit]
			pop     edi
			ret
	}
}

__declspec(naked) static void UpdateLevel_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     dword ptr [edi+drugExploit]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_perks_dialog_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			dec     dword ptr [edi+drugExploit]
			pop     edi
			ret
	}
}

__declspec(naked) static void skill_level_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     dword ptr [edi+drugExploit]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_skill_level_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			dec     dword ptr [edi+drugExploit]
			pop     edi
			ret
	}
}

__declspec(naked) static void SliderBtn_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     dword ptr [edi+drugExploit]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_skill_inc_point_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			dec     dword ptr [edi+drugExploit]
			pop     edi
			ret
	}
}

__declspec(naked) static void SliderBtn_hook1(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     dword ptr [edi+drugExploit]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_skill_dec_point_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			dec     dword ptr [edi+drugExploit]
			pop     edi
			ret
	}
}

__declspec(naked) static void checkPerk(void) {
	__asm {
			inc     eax                                  // Got a perk?
			jz      end                                  // No
			dec     eax
			imul    edx, eax, 0x3C
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     eax, [edi+D__perk_data]
			pop     edi
			lea     eax, [eax+edx]
			cmp     ebx, [eax+0x10]                      // Perk.Stat
			jne     skip
			sub     ebp, [eax+0x14]                      // Perk.StatMod
		skip:
			cmp     dword ptr [eax+0x8], -1              // Perk.Ranks
			jne     end
			sub     ebp, [eax+ebx*4+0x20]                // Perk.Str
		end:
			ret
	}
}

__declspec(naked) static void stat_get_real_bonus(void) {
	__asm {
			push    edi
			xchg    ebp, eax
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_worn_]
			call    edi
			pop     edi
			test    eax, eax
			jz      noArmor
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_ar_perk_]
			call    edi
			pop     edi
			call    checkPerk
		noArmor:
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_right_hand_]
			call    edi
			pop     edi
			test    eax, eax
			jz      noRightWeapon
			mov     edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_weapon
			jne     noRightWeapon
			xchg    edx, eax                             // eax = weapon
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_perk_]
			call    edi
			pop     edi
			call    checkPerk
		noRightWeapon:
			mov     eax, ecx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_inven_left_hand_]
			call    edi
			pop     edi
			test    eax, eax
			jz      noLeftWeapon
			mov     edx, eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_get_type_]
			call    edi
			pop     edi
			cmp     eax, item_type_weapon
			jne     noLeftWeapon
			xchg    edx, eax                             // eax = weapon
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_w_perk_]
			call    edi
			pop     edi
			call    checkPerk
		noLeftWeapon:
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			mov     edi, ds:[esi+D__queue]
			pop     esi
		loopQueue:
			test    edi, edi                             // Nothing in the queue?
			jz      end                                  // Yes
			cmp     ecx, [edi+0x8]                       // source == queue.object?
			jne     nextQueue                            // No
			mov     edx, [edi+0x4]                       // edx = queue.type
			mov     esi, [edi+0xC]                       // esi = queue.data
			test    edx, edx                             // Drug?
			jz      checkDrug                            // Yes
			cmp     edx, 2                               // Addiction?
			je      checkAddict                          // Yes
			cmp     edx, 6                               // Radiation?
			jne     nextQueue                            // No
			cmp     ebx, edx                             // STAT_lu?
			je      nextQueue                            // Yes
			mov     eax, [esi]                           // eax = queue_rads.rad_level
			dec     eax
			shl     eax, 5
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			sub     ebp, ds:[edi+D__rad_bonus][eax+ebx*4]
			pop     edi
			jmp     nextQueue
		checkDrug:
			cmp     dword ptr [esi], -2                  // Special situation?
			jne     loopStats                            // No
			inc     edx
			inc     edx
		loopStats:
			cmp     ebx, [esi+edx*4]                     // stat == queue_drug.stat#?
			jne     nextStat                             // No
			add     ebp, [esi+edx*4+0xC]
		nextStat:
			inc     edx
			cmp     edx, 3
			jl      loopStats
			jmp     nextQueue
		checkAddict:
			dec     edx
			cmp     [esi], edx                           // Perk active?
			je      nextQueue                            // No
			mov     eax, [esi+0x8]                       // eax = queue_addict.perk
			call    checkPerk
		nextQueue:
			mov     edi, [edi+0x10]                      // edi = queue.next
			jmp     loopQueue
		end:
			xchg    ebp, eax
			pop     edi
			ret
	}
}

__declspec(naked) static void stat_level_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_stat_get_bonus_]
			call    edi
			pop     edi
			cmp     ebx, STAT_lu
			ja      end                                  // We test only strength-luck
			cmp  dword ptr drugExploit, 0                // Calling from the right places?
			je   end                                     // No
			call stat_get_real_bonus                     // We do not take into account temporary effects
		end:
			ret
	}
}

__declspec(naked) static void barter_attempt_transaction_hook(void) {
	__asm {
			cmp     dword ptr [eax+0x64], PID_ACTIVE_GEIGER_COUNTER
			je      found
			cmp     dword ptr [eax+0x64], PID_ACTIVE_STEALTH_BOY
			je      found
			xor     eax, eax
			dec     eax
			ret

		found:
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_item_m_turn_off_]
			call    edi
			pop     edi
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x67E18]
			mov     [esp+4], edi
			pop     edi
			ret                                         // Are there any other included items for sale?
	}
}

__declspec(naked) static void item_m_turn_off_hook(void) {
	__asm {
			and     byte ptr [eax+0x25], 0xDF            // Reset the used item flag

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_queue_remove_this_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void register_object_take_out_hook(void) {
	__asm {
			push    ecx
			push    eax
			mov     ecx, edx                             // ID1
			mov     edx, [eax+0x1C]                      // cur_rot
			inc     edx
			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    edx                                  // ID3
			xor     ebx, ebx                             // ID2
			mov     edx, [eax+0x20]                      // fid
			and     edx, 0xFFF                           // Index
			xor     eax, eax
			inc     eax                                  // ObjType
			mov     [esp+1*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			xor     ebx, ebx
			dec     ebx
			xchg    edx, eax
			pop     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_register_object_change_fid_]
			call    edi
			pop     edi
			pop     ecx
			xor     eax, eax
			ret
	}
}

// Don't delete this. It's all ready, in case it's useful again.
/*static int TimeLimit = 0;

__declspec(naked) static void game_time_date_hook(void) {
	__asm {
			test    edi, edi
			jz      end
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			add     esi, ds:[edi+D__pc_proto+0x134]            // _pc_proto.bonus_age
			pop     edi
			mov     [edi], esi
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x917AE]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void stat_level_hook1(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__fallout_game_time]
			pop     edi
			div     ecx
			add     esi, eax
			mov     eax, 99
			cmp     esi, eax
			jle     end
			xchg    esi, eax
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x9C5CC]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void inc_game_time_hook(void) {
	__asm {
			push    edx
			push    ebx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			add     ds:[edi+D__fallout_game_time], eax
			mov     eax, ds:[edi+D__fallout_game_time]
			pop     edi
			mov     ebx, 315360000
			xor     edx, edx
			div     ebx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     edx, [edi+TimeLimit]
			pop     edi
			test    edx, edx                             // TimeLimit < 0?
			jl      negative                             // Yes
			cmp     eax, edx                             // Number of years elapsed < TimeLimit?
			jb      end                                  // Yes
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_endgame_movie_]
			call    edi
			pop     edi
			jmp     end
		negative:
			cmp     eax, 13                              // Number of years elapsed < 13?
			jb      end                                  // Yes
			imul    ebx, ebx, 13                         // ebx = 315360000 * 13 = 4099680000 = 0xF45C2700
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			sub     ds:[edi+D__fallout_game_time], ebx
			add     dword ptr ds:[edi+D__pc_proto+0x134], 13   // _pc_proto.bonus_age
			mov     eax, ds:[edi+D__queue]
			pop     edi
		loopQueue:
			test    eax, eax
			jz      end
			cmp     [eax], ebx
			jnb     skip
			mov     [eax], ebx
		skip:
			sub     [eax], ebx                           // queue.time
			mov     eax, [eax+0x10]                      // queue.next_queue
			jmp     loopQueue
		end:
			pop     ebx
			pop     edx
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__fallout_game_time]
			pop     edi
			ret
	}
}

__declspec(naked) static void inc_game_time_in_seconds_hook(void) {
	__asm {
			call    inc_game_time_hook
			pop     edx
			ret
	}
}

__declspec(naked) static void script_chk_timed_events_hook(void) {
	__asm {
			test    dl, 1
			jnz     end
			xor     eax, eax
			inc     eax
			call    inc_game_time_hook
		end:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x920D7]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

__declspec(naked) static void TimedRest_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_set_game_time_]
			call    edi
			pop     edi
			push    edx
			push    eax
			xor     eax, eax
			call    inc_game_time_hook
			pop     edx
			cmp     edx, eax
			je      end
			sub     edx, ebp
			sub     eax, edx
			mov     ebp, eax
			pop     edx
			sub     edx, 315360000 * 13
			ret

		end:
			pop     edx
			ret
	}
}

__declspec(naked) static void world_map_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_set_game_time_]
			call    edi
			pop     edi
			push    esi
			push    eax
			xor     eax, eax
			call    inc_game_time_hook
			pop     esi
			cmp     esi, eax
			pop     esi
			je      end
			sub     esi, 315360000 * 13
		end:
			ret
	}
}*/

__declspec(naked) static void gdAddOptionStr_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     ecx, ds:[edi+D__gdNumOptions]
			pop     edi
			add     ecx, '1'
			push    ecx

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x3E807]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

static void OnExit(void) {
	AnimationsAtOnceExit();
	PartyControlExit();
}

__declspec(naked) static void _WinMain_hook(void) {
	__asm {
			pushad
			call    OnExit
			popad

			// [Edw590: no need to call exit() here. That's done in the Loader code already.]
			ret
	}
}

void DllMain2(void) {
	int i = 0;
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	// Make a call just before the game exits, which is inside the Loader code, in the NOPs I left there for this.
	// Those NOPs that are replaced here and called just before this block of code is freed by free() on the Loader.
	makeCallEXE(0xEA2DA, &_WinMain_hook, false);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Main", "TranslationsINI", "TRANSLAT.INI", prop_value, &sfall1_ini_info_G);
	// If it fails, the struct will have 0s and the file won't be read, so the default values will be used as sFall1 does.
	readFile(prop_value, &translation_ini_info_G);


	BugsInit();

	// Too much work and I don't see much/any gain in doing this.
	// EDIT: actually this could be useful for the PSP, to get the game to move faster, because it's really slow there.
	// But still, it's too much work and no one or almost no one will use this... (maybe not even me).
	//getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Speed", "Enable", "0", prop_value, &sfall1_ini_info_G);
	//sscanf(prop_value, "%d", &temp_int);
	//if (0 != temp_int) {
	//	AddrGetTickCount = (DWORD) &FakeGetTickCount;
	//	AddrGetLocalTime = (DWORD) &FakeGetLocalTime;
	//
	//	for (int i = 0; i < sizeof(offsetsA) / 4; i++) {
	//		SafeWrite32(offsetsA[i], (DWORD) &AddrGetTickCount);
	//	}
	//	for (int i = 0; i < sizeof(offsetsB) / 4; i++) {
	//		SafeWrite32(offsetsB[i], (DWORD) &AddrGetTickCount);
	//	}
	//	for (int i = 0; i < sizeof(offsetsC) / 4; i++) {
	//		SafeWrite32(offsetsC[i], (DWORD) &AddrGetTickCount);
	//	}
	//
	//	SafeWrite32(0x4E0CC0, (DWORD) &AddrGetLocalTime);
	//	TimerInit();
	//}

	AmmoModInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartingMap", "", mapName, &sfall1_ini_info_G);
	if (0 != strcmp(mapName, "")) {
		writeMem32EXE(0x72995, (uint32_t) getRealBlockAddrData(&mapName), true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "VersionString", "", versionString, &sfall1_ini_info_G);
	if (0 != strcmp(versionString, "")) {
		writeMem32EXE(0xA10E7 + 1, (uint32_t) getRealBlockAddrData(&versionString), true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "WindowName", "", windowName, &sfall1_ini_info_G);
	if (0 != strcmp(windowName, "")) {
		writeMem32EXE(0x72B86 + 1, (uint32_t) getRealBlockAddrData(&windowName), true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ConfigFile", "", configName, &sfall1_ini_info_G);
	if (0 != strcmp(configName, "")) {
		writeMem32EXE(0x3DE14 + 1, (uint32_t) getRealBlockAddrData(&configName), true);
		writeMem32EXE(0x3DE39 + 1, (uint32_t) getRealBlockAddrData(&configName), true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MaleDefaultModel", "hmjmps", dmModelName, &sfall1_ini_info_G);
	writeMem32EXE(0x183BD + 1, (uint32_t) getRealBlockAddrData(&dmModelName), true);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "FemaleDefaultModel", "hfjmps", dfModelName, &sfall1_ini_info_G);
	writeMem32EXE(0x183E0 + 1, (uint32_t) getRealBlockAddrData(&dfModelName), true);

	for (i = 0; i < 14; ++i) {
		char ininame[8];
		memset(ininame, 0, 8);
		GET_BD_ARR(char *, MovieNames)[(i * 65) + 64] = 0;
		strcpy(ininame, "Movie");
		itoa(i + 1, &ininame[5], 10);
		ininame[7] = '\0'; // Be sure the string is still NULL-terminated.
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", ininame, GET_BD_ARR(char **, origMovieNames)[i],
						&GET_BD_ARR(char *, MovieNames)[i * 65], &sfall1_ini_info_G);
		writeMem32EXE(0x1055F0 + ((uint32_t) i * 4),
					  (uint32_t) getRealBlockAddrData(&GET_BD_ARR(char *, MovieNames)[i * 65]), true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartYear", "-1", prop_value, &sfall1_ini_info_G);
	// I'd use stdtol() for the conversion, but that's not available on the game EXE...
	sscanf(prop_value, "%d", &temp_int);
	if (temp_int >= 0) {
		writeMem32EXE(0x9175A + 2, (uint32_t) temp_int, true);
	}
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartMonth", "-1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 1) && (temp_int <= 12)) {
		writeMem32EXE(0x91771 + 1, (uint32_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartDay", "-1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 1) && (temp_int <= 31)) {
		writeMem8EXE(0x91744 + 2, (uint8_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "LocalMapXLimit", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x9DFB9 + 4, (uint32_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "LocalMapYLimit", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x9DFC7 + 4, (uint32_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DialogueFix", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x3EFA4 + 2, 0x31, true);
	}

	CritInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DisplayKarmaChanges", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		getPropValueIni(NULL, "sfall", "KarmaGain", "You gained %d karma.", KarmaGainMsg, &translation_ini_info_G);
		getPropValueIni(NULL, "sfall", "KarmaLoss", "You lost %d karma.", KarmaLossMsg, &translation_ini_info_G);
		hookCallEXE(0x4CED4, &op_set_global_var_hook);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "PlayIdleAnimOnReload", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		hookCallEXE(0x563D9, &intface_item_reload_hook);
	}

	// Disabled
	//idle = GetPrivateProfileIntA("Misc", "ProcessorIdle", -1, ini);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SkipOpeningMovies", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem16EXE(0x728C3, 0x13EB, true);            // jmps 0x472A88
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "NPCsTryToSpendExtraAP", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, RetryCombatMinAP) = (uint32_t) temp_int;
	if (temp_int > 0) {
		// Apply retry combat patch
		hookCallEXE(0x20ABA, &combat_turn_hook);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "RemoveWindowRounding", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem16EXE(0xA4BD0, 0x04EB, true);// jmps 0x4A50C6
	}

	// Disabled
	//ConsoleInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SpeedInterfaceCounterAnims", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (1 == temp_int) {
		makeCallEXE(0x563F1, &intface_rotate_numbers_hook, true);
	} else if (2 == temp_int) {
		writeMem32EXE(0x56406, 0x90DB3190u, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SpeedInventoryPCRotation", "166", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int != 166) && (temp_int <= 1000)) {
		writeMem32EXE(0x6415A + 1, (uint32_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "RemoveCriticalTimelimits", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x91412 + 1, 0x0, true);
		writeMem8EXE(0x91453 + 1, 0x0, true);
	}

	// Patch ereg call - I think the purpose of this is only for Windows, so I've disabled it here
	//blockCallEXE(0x3B25F);

	// todo The below is disabled until ALL the addresses are corrected for the DOS EXE
	//AnimationsAtOnceInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "CombatPanelAnimDelay", "1000", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 0) && (temp_int <= 65535)) {
		writeMem32EXE(0x55385 + 1, (uint32_t) temp_int, true);
		writeMem32EXE(0x554DA + 1, (uint32_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DialogPanelAnimDelay", "33", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 0) && (temp_int <= 255)) {
		writeMem32EXE(0x400FF + 1, (uint32_t) temp_int, true);
		writeMem32EXE(0x401C1 + 1, (uint32_t) temp_int, true);
	}

	DebugModeInit();

	// I don't think this applies for MS-DOS...
	//if (GetPrivateProfileIntA("Misc", "SingleCore", 1, ini)) {
	//	dlog("Applying single core patch.", DL_INIT);
	//	HANDLE process = GetCurrentProcess();
	//	SetProcessAffinityMask(process, 1);
	//	CloseHandle(process);
	//	dlogr(" Done", DL_INIT);
	//}

	//Bodypart hit chances
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Head", "-40", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEE84)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Left_Arm", "-30", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEE88)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Right_Arm", "-30", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEE8C)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Torso", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEE90)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Right_Leg", "-20", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEE94)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Left_Leg", "-20", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEE98)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Eyes", "-60", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEE9C)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Groin", "-30", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEEA0)) = (uint32_t) temp_int;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Torso", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*((uint32_t *) getRealEXEAddr(0xFEEA4)) = (uint32_t) temp_int;

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Input", "ToggleItemHighlightsKey", "NONE", prop_value, &sfall1_ini_info_G);
	GET_BD_SYM(char, toggleHighlightsKey) = (char) (0 == strcmp(prop_value, "NONE") ? '\0' : prop_value[0]);
	if ('\0' != GET_BD_SYM(char, toggleHighlightsKey)) {
		hookCallEXE(0x43715, &gmouse_bk_process_hook);
		hookCallEXE(0x4398A, &obj_remove_outline_hook);
		hookCallEXE(0x46155, &obj_remove_outline_hook);
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Input", "TurnHighlightContainers", "0", prop_value, &sfall1_ini_info_G);
		sscanf(prop_value, "%u", &GET_BD_SYM(uint32_t, TurnHighlightContainers));
	}

	hookCallEXE(0x72D37, &get_input_hook);       //hook the main game loop
	hookCallEXE(0x2082E, &get_input_hook);       //hook the combat loop

	MainMenuInit();

	CreditsInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DisablePipboyAlarm", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x88E44, 0xC3, true);// ret
	}

	LoadGameHookInit();

	InventoryInit();

	AIInit();

	// todo I think this is not working. Check everything again, for the 3rd time?
	PartyControlInit();

	// Navigation keys in pipboy
	hookCallEXE(0x86A5E, &pipboy_hook);

	// Third ending for the Brotherhood of Steel
	makeCallEXE(0x38B00, &Brotherhood_final, true);

	// Disable the disappearance of an unused perk
	writeMem8EXE(0x361CB, 0x80, true);                // add  byte ptr ds:_free_perk, 1
	writeMem16EXE(0x36222, 0x0DFE, true);             // dec  byte ptr ds:_free_perk
	writeMem8EXE(0x36228 + 1, 0xB1, true);              // jmp  0x436263
	hookCallEXE(0x2C64E, &editor_design_hook);
	hookCallEXE(0x2CBE4, &editor_design_hook2);
	hookCallEXE(0x368EC, &perks_dialog_hook);
	makeCallEXE(0x866C2, &perk_can_add_hook, true);

	// Raise the Perk Window
	writeMem8EXE(0x3641E+1, 31, true);                  // 91-60=31
	writeMem8EXE(0x369D2 + 2, 74, true);                  // 134-60=74

	// Decrease the distance to switch to walking when clicking on an item
	for (i = 0; i < ((int) sizeof(WalkDistance) / 4); ++i) {
		*(uint8_t *) GET_BD_ARR(uint32_t *, WalkDistance)[i] = 1;
	}

	// Fix "Pressing A to enter combat before anything else happens, thus getting infinite free running"
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "FakeCombatFix", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, FakeCombatFix) = (uint32_t) temp_int;
	if (0 != temp_int) {
		makeCallEXE(0x17952, &check_move_hook, false);
		makeCallEXE(0x43609, &gmouse_bk_process_hook1, false);
		hookCallEXE(0x43D98, &FakeCombatFix1);       // action_get_an_object_
		hookCallEXE(0x443CB, &FakeCombatFix1);       // action_get_an_object_
		hookCallEXE(0x43E41, &FakeCombatFix2);       // action_loot_container_
		hookCallEXE(0x443B7, &FakeCombatFix2);       // action_loot_container_
		hookCallEXE(0x11F7B, &FakeCombatFix3);       // action_use_an_object_
		hookCallEXE(0x43F80, &FakeCombatFix3);       // gmouse_handle_event_
		makeCallEXE(0x1FEDA, &combat_begin_hook, false);
		makeCallEXE(0x1FA53, &combat_reset_hook, false);
		hookCallEXE(0x20D29, &combat_hook);
	}

	// Some people were not that happy this "feature", so I've put it optional, and disabled by default (it's wtf anyway)
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "F1DPExpandedAges", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		// Minimum player age
		writeMem8EXE(0x108364 + 0xC, 8, true);

		// Maximum Player Age
		writeMem8EXE(0x108364 + 0x10, 60, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "EnableMusicInDialogue", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x3E45B + 1, 0x00, true);
		// This below was already commented out, probably because of the above which makes the code never get to where
		// this patch is.
		//BlockCall(0x481A7);
	}

	// Max player level
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MaxPCLevel", "21", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, MaxPCLevel) = (uint32_t) temp_int;
	if ((temp_int != 21) && (temp_int >= 1) && (temp_int <= 99)) {
		writeMem8EXE(0x3611B + 2, (uint8_t) temp_int, true);
		writeMem8EXE(0x9CCBC + 2, (uint8_t) temp_int, true);
		makeCallEXE(0x9CB82, &stat_pc_min_exp_hook, false);
		// Max Perks Gained
		writeMem8EXE(0x36197 + 2, 33, true);
		writeMem8EXE(0x361A2 + 2, 33, true);
		writeMem8EXE(0x37D4A + 2, 33, true);
		// cap hitpoints at 999 when leveling up
		writeMem32EXE(0x1080F4 + 0x10, 999, true);
	}

	// todo Too much work. Disabled for now.
	/*getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AutoQuickSave", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, AutoQuickSave) = (uint32_t) temp_int;
	if ((temp_int >= 1) && (temp_int <= 10)) {
		--GET_BD_SYM(uint32_t, AutoQuickSave);
		SafeWrite16(0x46DEAB, 0xC031);            // xor  eax, eax
		SafeWrite32(0x46DEAD, 0x505A50A3);        // mov  ds:_slot_cursor, eax
		SafeWrite16(0x46DEB2, 0x04EB);            // jmp  0x46DEB8
		MakeCall(0x46DF13, &SaveGame_hook, true);
	}*/

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DontTurnOffSneakIfYouRun", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x17A2F, 0xEB, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ColorLOS", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, ColorLOS) = (uint32_t) temp_int;
	if ((2 == temp_int) || (4 == temp_int) || (16 == temp_int) || (32 == temp_int)) {
		hookCallEXE(0x242FF, &combat_update_critter_outline_for_los);
		hookCallEXE(0x24399, &combat_update_critter_outline_for_los);
		makeCallEXE(0x7C2CA, &obj_move_to_tile_hook, true);
	}

	// You can use the newline control character (\n) in the description of objects from pro_*.msg
	writeMem32EXE(0x62B99 + 6, GET_BC_SYM(display_print_with_linebreak), true);
	writeMem32EXE(0x8A2E9 + 1, GET_BC_SYM(display_print_with_linebreak), true);
	writeMem32EXE(0x66479 + 1, GET_BC_SYM(inven_display_msg_with_linebreak), true);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "RemoveFriendlyFoe", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, RemoveFriendlyFoe) = (uint32_t) temp_int;
	if (0 != temp_int) {
		writeMem32EXE(0x106D24 + 12, 100, true);
		writeMem8EXE(0x1FFD6 + 1, 0x0, true);
		writeMem8EXE(0x2436F + 1, 0x0, true);
		writeMem8EXE(0x260DD + 1, 0x0, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DrugExploitFix", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		hookCallEXE(0x8B11C, &protinst_use_item_hook);
		hookCallEXE(0x361FB, &UpdateLevel_hook);
		hookCallEXE(0x34AA0, &skill_level_hook);    // SavePlayer_
		hookCallEXE(0x355E8, &SliderBtn_hook);
		hookCallEXE(0x3564F, &skill_level_hook);    // SliderBtn_
		hookCallEXE(0x35668, &SliderBtn_hook1);
		hookCallEXE(0x9C563, &stat_level_hook);
	}

	QuestsInit();

	// Fixed inability to sell previously used "Geiger Counter" / "Invisible"
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "CanSellUsedGeiger", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x6AC2A, 0xBA, true);
		writeMem8EXE(0x6AC5C, 0xBA, true);
		makeCallEXE(0x67E23, &barter_attempt_transaction_hook, false);
		hookCallEXE(0x6BEE9, &item_m_turn_off_hook);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "InstantWeaponEquip", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		// Skip weapon stow animation
		for (i = 0; i < ((int) sizeof(PutAwayWeapon) / 4); ++i) {
			*(uint8_t *) GET_BD_ARR(uint32_t *, PutAwayWeapon)[i] = 0xEB; // jmps
		}
		blockCallEXE(0x65FFD);                      //
		blockCallEXE(0x66008);                      // inven_unwield_
		blockCallEXE(0x66018);                      //
		makeCallEXE(0x14C5C, &register_object_take_out_hook, true);
	}

	// In version 1.7.6, this seems to have a problem and the game will crash after 13 years. So do NOT attempt to port
	// it UNLESS you know how to fix it!!!
	// Go get the fix from version 1.8 with IDA's help. todo
	// Btw, the bug is also in version 1.7.20, so just go on 1.8 get it.
	/*getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "TimeLimit", "13", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	GET_BD_SYM(uint32_t, TimeLimit) = (uint32_t) temp_int;
	if ((temp_int < 14) && (temp_int > -4)) {
		int j = 0;
		if (temp_int < -1) {
			makeCallEXE(0x917A8, &game_time_date_hook, true);
			makeCallEXE(0x9C59F, &stat_level_hook1, true);
		}
		makeCallEXE(0x918BC, &inc_game_time_hook, true);
		makeCallEXE(0x918D4, &inc_game_time_in_seconds_hook, true);
		makeCallEXE(0x920C3, &script_chk_timed_events_hook, true);
		for (j = 0; j < ((int) sizeof(TimedRest) / 4); ++j) {
			*(uint32_t *) GET_BD_SYM(uint32_t, TimedRest[j] + 1) =
					&GET_BD_SYM(uint32_t, TimedRest_hook);
		}
		for (j = 0; j < ((int) sizeof(world_map) / 4); ++j) {
			*(uint32_t *) GET_BD_SYM(uint32_t, world_map[j] + 1) =
					&GET_BD_SYM(uint32_t, world_map_hook);
		}
	}*/

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "PipboyTimeAnimDelay", "-1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 0) && (temp_int <= 127)) {
		writeMem8EXE(0x894A7 + 2, (uint8_t) temp_int, true);
		writeMem8EXE(0x896B3 + 2, (uint8_t) temp_int, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "NumbersInDialogue", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0xF2368 + 2, 0x2000202E, true);
		writeMem8EXE(0x3F85B + 1, (*(uint8_t *) 0x3F85B + 1) + 1, true);
		writeMem32EXE(0xF21F8 + 2, 0x7325202E, true);
		writeMem32EXE(0x3F825, 0x1C24548B, true);          // mov  edx, [esp+0x1C]
		writeMem8EXE(0x3F825 + 4, 0x52, true);               // push edx
		writeMem32EXE(0x3F901, 0x2024548B, true);          // mov  edx, [esp+0x20]
		writeMem8EXE(0x3F901 + 4, 0x52, true);               // push edx
		makeCallEXE(0x3E807, &gdAddOptionStr_hook, true);
	}

	SoundInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Speed", "TimeScale", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int > 1) && (temp_int <= 10)) {
		writeMem32EXE(0x92093 + 1, 30000 / (uint32_t) temp_int, true);
		writeMem8EXE(0x920B2 + 2, (uint8_t) (100 / (uint32_t) temp_int), true);
	}


	free(GET_BD_SYM(struct FileInfo, translation_ini_info_G).contents);
}
