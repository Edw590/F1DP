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

// This had no copyright notice on it. Hopefully, the author doesn't mind me using it. I'm keeping
// the same license as in the other files of the project on it though (I'm just licensing it only
// to myself because I can't put the original authors there, whoever they were - no notice).
//
// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../Utils/General.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "AmmoMod.h"
#include "AnimationsAtOnceLimit.h"
#include "Criticals.h"
#include "Define.h"
#include "FalloutEngine.h"
#include "Inventory.h"
#include "SFall1Patches.h"
#include "SFall1Main.h"
#include "../CLibs/ctype.h"
#include "MainMenu.h"
#include "Credits.h"
#include "LoadGameHook.h"
#include "../CLibs/conio.h"
#include "PartyControl.h"


// ADVICE: don't try to understand the functions in each patch... Infinite EDI register uses there to be able to have
// the special numbers in use. If you want to understand what was done, just go to the sFall1 source and see there. If
// you think anything is wrong with the ones here, redo from scratch, don't try to fix. It's a mess with this way of
// Special Numbers, but I don't have better ideas without making a relocation table for the EXE, parsing it and applying
// it (as long as this way works, I think it's much better than going that route, which would take infinity).

/*
 * // ///////////////////////////////////////////////////////////////
 *         RULES TO FOLLOW IN THE ASSEMBLY PORTS!!!!!!!!!!!
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
 *   all function PUSHes (you want to replace a CALL right there), you can move all the parameters for the function on
 *   the stack down 4 bytes, and put EDI on the 4 bytes that are now free. Example on how to do that, for 6 parameters:
 * 			lea     esp, [esp-4] // Reserve space on the stack for the last function PUSH
 * 			push    edi
 * 			mov     edi, [(esp+4)+4]
 * 			mov     [(esp+4)+0], edi
 * 			mov     edi, [(esp+4)+8]
 * 			mov     [(esp+4)+4], edi
 * 			mov     edi, [(esp+4)+12]
 * 			mov     [(esp+4)+8], edi
 * 			mov     edi, [(esp+4)+16]
 * 			mov     [(esp+4)+12], edi
 * 			mov     edi, [(esp+4)+20]
 * 			mov     [(esp+4)+16], edi
 * 			mov     edi, [(esp+4)+24]
 * 			mov     [(esp+4)+20], edi
 * 			pop     edi
 * 			mov     [esp+24], edi // This will be the "PUSH" before all the function PUSHes
 *
 * 	 WARNING ABOUT THIS ABOVE!!!! Do NOT make a real PUSH before all the function PUSHes if the arguments are passed on
 * 	 the stack!!!! If you PUSH EDI before everything else and then EDI is changed inside, the value that will be
 * 	 recovered will be the old EDI value before the modifications after the PUSH and before the function call!!!!
 * 	 So a `sub esp, 4` must be done instead, and right before where EDI is used, a `mov [esp+?*4], edi` issued. The "?"
 * 	 is the number of stack parameters the function takes - that will put EDI before all others).
 * 	 ALWAYS CHECK IF ESP IS USED WITHIN THE SUBTRACTION AND THE POP!!!!! If it is, ESP will need 4 added to it every
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
 *   The SUB instruction is has the same treatment. Do the same for any others required. The flags must not be changed
 *   when porting the code!
 *
 * --> Steps to have to do the above:
 * - Find all references to the macros and put C_ or D_ on them.
 * - Go on FalloutEngine.h and go do what is required there for every macro (as a start, put the address there).
 * - Put on Ctrl+F (to highlight) "edi" so that it's easy to see where that's being used, because if it's being used
 *   on a `mov edi, ds:[0x23948]`, for example, then EDI can't be used for the SN and must be ESI (in this case).
 * - Find all references to global variables inside the Patcher and use SN_DATA_SEC_BLOCK_ADDR on them.
 * - Find all CALLs and pay attention to the stack (check the documentation of the macro to see what the "Args" are. Use
 *   the corresponding SN depending on the C_ or D_ prefix.
 * - Find all "0x"s, check if they are addresses, and if they are, use the appropriate SN.
 * - Go check all the C_s, D_s and F_s that remained untouched and use the appropriate SN.
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

static void __declspec(naked) op_set_global_var_hook(void) {
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
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_game_set_global_var_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) intface_item_reload_hook(void) {
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

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_gsound_play_sfx_file_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static uint32_t RetryCombatMinAP = 0;

static void __declspec(naked) combat_turn_hook(void) {
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
			retn
	}
}

static void __declspec(naked) intface_rotate_numbers_hook(void) {
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

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x563F6]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) DebugMode(void) {
	__asm {
			pusha
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_debug_register_env_]
			call    edi
			pop     edi
			popa

			mov     ecx, 1
			ret
	}
}

static void __declspec(naked) obj_outline_all_items_on(void) {
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
			test    eax, eax                             // This ObjType_Item?
			jnz     nextObject                           // No
			cmp     [ecx+0x7C], eax                      // Does it belong to someone?
			jnz     nextObject                           // Yes
			test    [ecx+0x74], eax                      // Already illuminated?
			jnz     nextObject                           // Yes
			mov     edx, 0x10                            // yellow
			test    [ecx+0x25], dl                       // Is NoHighlight_ set (is it a container)?
			jz      NoHighlight                          // No
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     [edi+TurnHighlightContainers], eax   // Highlight containers?
			pop     edi
			je      nextObject                           // No
			mov     edx, 0x4                             // Gray
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
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_tile_refresh_display_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) obj_outline_all_items_off(void) {
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
			test    eax, eax                             // This ObjType_Item?
			jnz     nextObject                           // No
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
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_tile_refresh_display_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) gmouse_bk_process_hook(void) {
	__asm {
			test    eax, eax
			jz      end
			test    byte ptr [eax+0x25], 0x10            // NoHighlight_
			jnz     end
			mov     dword ptr [eax+0x74], 0
		end:
			mov     edx, 0x40

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_outline_object_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) obj_remove_outline_hook(void) {
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
			retn
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

	return (toupper(c) == toupper(*(char *) getRealBlockAddrData(&toggleHighlightsKey)));
}

static void __declspec(naked) get_input_hook(void) {
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
			retn
	}
}

static void __declspec(naked) pipboy_hook(void) {
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
			retn
	}
}

static const char *_nar_31 = "nar_31";

static void __declspec(naked) Brotherhood_final(void) {
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
			mov     edx, 0x38B45
			test    eax, eax
			jz      RhombusDead
			mov     edx, 0x38B18
		RhombusDead:
			jmp     edx
		nar_31:
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    0
			mov     edx, 317                             // SEQ5D.FRM
			mov     eax, ObjType_Intrface
			xor     ecx, ecx
			xor     ebx, ebx
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     edx, [edi+_nar_31]
			pop     edi

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x38B5F]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static uint32_t Educated = 0;
static uint32_t Lifegiver = 0;
static uint32_t Tag_ = 0;
static uint32_t Mutate_ = 0;

static void __declspec(naked) editor_design_hook(void) {
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
			retn
	}
}

static void __declspec(naked) editor_design_hook2(void) {
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

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_RestorePlayer_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) perks_dialog_hook(void) {
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
			retn
	}
}

static void __declspec(naked) perk_can_add_hook(void) {
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

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x866D1]
			mov     [esp+4], edi
			pop     edi
			retn

		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x866D8]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) FirstTurnAndNoEnemy(void) {
	__asm {
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    byte ptr ds:[edi+D__combat_state], 1
			pop     edi
			jz      end                                  // Not in battle
			cmp     _combatNumTurns, eax
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
			retn
		nextCritter:
			add     edi, 4                               // To the next character on the list
			loop    loopCritter                          // Looping through the entire list
			popad
		end:
			retn
	}
}

static void __declspec(naked) FirstTurnCheckDist(void) {
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
			retn
	}
}

static void __declspec(naked) check_move_hook(void) {
	__asm {
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			jnz     skip                                 // Yes
			cmp     dword ptr [ecx], -1
			je      end
			retn

		skip:
			xor     esi, esi
			dec     esi
		end:
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x179BF]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) gmouse_bk_process_hook1(void) {
	__asm {
			xchg    ebp, eax
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			jnz     end                                  // Yes
			xchg    ebp, eax
			cmp     eax, [edx+0x40]
			jg      end
			retn

		end:
			pop     eax                                  // Destroying the return address

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x43625]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) FakeCombatFix1(void) {
	__asm {
			push    eax                                  // _obj_dude
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			pop     eax
			jz      end                                  // No
			call    FirstTurnCheckDist
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_action_get_an_object_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) FakeCombatFix2(void) {
	__asm {
			push    eax                                  // _obj_dude
			call    FirstTurnAndNoEnemy
			test    eax, eax                             // Is this the first move in combat and there are no enemies?
			pop     eax
			jz      end                                  // No
			call    FirstTurnCheckDist
		end:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_action_loot_container_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

void __declspec(naked) FakeCombatFix3(void) {
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
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_action_use_an_item_on_object_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) combat_begin_hook(void) {
	__asm {
			xor     eax, eax
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+_combatNumTurns], eax
			pop     edi
			dec     eax
			retn
	}
}

static void __declspec(naked) combat_reset_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+_combatNumTurns], edx
			pop     edi
			mov     edx, STAT_max_move_points
			retn
	}
}

static void __declspec(naked) combat_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     dword ptr [edi+_combatNumTurns]
			pop     edi

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_should_end_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static uint32_t MaxPCLevel = 21;

static void __declspec(naked) stat_pc_min_exp_hook(void) {
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
			retn
	}
}

/*FILETIME ftCurr, ftPrev;
static void __stdcall _GetFileTime(char *filename) {
	char fname[65];
	sprintf_s(fname, "%s%s", "data\\", filename);
	HANDLE hFile = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		GetFileTime(hFile, NULL, NULL, &ftCurr);
		CloseHandle(hFile);
	} else {
		ftCurr.dwHighDateTime = 0;
		ftCurr.dwLowDateTime = 0;
	};
}

static const char *commentFmt = "%02d/%02d/%d  %02d:%02d:%02d";
static void __stdcall createComment(char *bufstr) {
	SYSTEMTIME stUTC, stLocal;
	char buf[30];
	GetSystemTime(&stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
	sprintf_s(buf, commentFmt, stLocal.wDay, stLocal.wMonth, stLocal.wYear, stLocal.wHour, stLocal.wMinute,
			  stLocal.wSecond);
	strcpy(bufstr, buf);
}

static uint32_t AutoQuickSave = 0;

static void __declspec(naked) SaveGame_hook(void) {
	__asm {
	pushad
	mov  ecx, ds:[_slot_cursor]
	mov  ds:[_flptr], eax
	test eax, eax
	jz   end                                  // This is an empty slot, you can write
	call db_fclose_
	push ecx
	push edi
	call _GetFileTime
	pop  ecx
	mov  edx, ftCurr.dwHighDateTime
	mov  ebx, ftCurr.dwLowDateTime
	jecxz nextSlot                            // You're welcome
	cmp  edx, ftPrev.dwHighDateTime
	ja   nextSlot                             // current to recorded earlier
	jb   end                                  // The current slot is written before the previous one
	cmp  ebx, ftPrev.dwLowDateTime
	jbe  end
	nextSlot:
	mov  ftPrev.dwHighDateTime, edx
	mov  ftPrev.dwLowDateTime, ebx
	inc  ecx
	cmp  ecx, AutoQuickSave                   // Last slot+1?
	ja   firstSlot                            // Yes
	mov  ds:[_slot_cursor], ecx
	popad
	push 0x46DEB8
	retn
	firstSlot:
	xor  ecx, ecx
	end:
	mov  ds:[_slot_cursor], ecx
	mov  eax, ecx
	shl  eax, 4
	add  eax, ecx
	shl  eax, 3
	add  eax, _LSData+0x3D                   // eax->_LSData[_slot_cursor].Comment
	push eax
	call createComment
	popad
	xor  edx, edx
	inc  edx
	mov  ds:[_quick_done], edx
	push 0x46DF33
	retn
	}
}*/

static uint32_t RemoveFriendlyFoe = 0;
static uint32_t ColorLOS = 0;

static void __declspec(naked) combat_update_critter_outline_for_los(void) {
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
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    eax
			mov     eax, esi
			xchg    ecx, eax                             // ecx = target, eax=source (_obj_dude)
			mov     ebx, [ecx+0x4]                       // target_tile
			mov     edx, [eax+0x4]                       // source_tile
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
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
			retn
	}
}

static void __declspec(naked) obj_move_to_tile_hook(void) {
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

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x7C2D0]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

void __declspec(naked) queue_find_first_(void) {
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
			retn
	}
}

static void OnExit(void) {
	//ConsoleExit();
	AnimationsAtOnceExit();
}

static void __declspec(naked) _WinMain_hook(void) {
	__asm {
			call    OnExit

			// [DADi590: no need to call exit() here. That's done in the Loader code already.]
			retn
	}
}

void DllMain2(void) {
	uint32_t i = 0;
	int temp_int = 0;
	uint32_t temp_uint32 = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	// DO NOT REMOVE THIS PRINTF!!!! Interesting story about this... It will crash (at least at the moment I'm writing
	// this) if this thing is not here.... DOSBox will completely crash with an error:
	// "Exit to error: CPU_SetSegGeneral: Stack segment beyond limits". Reason why this works? Yes. (wtf, this was
	// random, I just wanted to print until I'd get to the error....) This was all because of the line
	// `static uint32_t real_perk_lev[PERK_count] = {0};` in PartyControl.c. No idea why this fixed it, wtf...
	// I was told DOSBox is buggy --> cool to know how much, I guess? ahahahah
	//printf("\0\0\0\0\0\0\0\0\0\0");

	// Make a call just before the game exits, which is inside the Loader code, in the NOPs I left there for this.
	// Those NOPs that are replaced here are just before this block of code is freed by free().
	MakeCallEXE(0xEA2DA, getRealBlockAddrCode((void *) &_WinMain_hook), false);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Main", "TranslationsINI", "./Translations.ini", prop_value, &sfall1_ini_info_G);
	// If it fails, the struct will have 0s and the file won't be read, so the default values will be used as sFall1 does.
	readFile(prop_value, &translation_ini_info_G);

	AmmoModInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartingMap", "", mapName, &sfall1_ini_info_G);
	if (0 != strcmp(mapName, "")) {
		writeMem32EXE(0x72995, (uint32_t) getRealBlockAddrData(mapName));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "VersionString", "", versionString, &sfall1_ini_info_G);
	if (0 != strcmp(mapName, "")) {
		writeMem32EXE(0xA10E7+1, (uint32_t) getRealBlockAddrData(versionString));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "WindowName", "", windowName, &sfall1_ini_info_G);
	if (0 != strcmp(windowName, "")) {
		writeMem32EXE(0x72B86+1, (uint32_t) getRealBlockAddrData(windowName));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ConfigFile", "", configName, &sfall1_ini_info_G);
	if (0 != strcmp(configName, "")) {
		writeMem32EXE(0x3DE14+1, (uint32_t) getRealBlockAddrData(configName));
		writeMem32EXE(0x3DE39+1, (uint32_t) getRealBlockAddrData(configName));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MaleDefaultModel", "hmjmps", dmModelName, &sfall1_ini_info_G);
	writeMem32EXE(0x183BD+1, (uint32_t) getRealBlockAddrData(dmModelName));

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "FemaleDefaultModel", "hfjmps", dfModelName, &sfall1_ini_info_G);
	writeMem32EXE(0x183E0+1, (uint32_t) getRealBlockAddrData(dfModelName));

	for (i = 0; i < 14; ++i) {
		char ininame[8];
		MovieNames[(i * 65) + 64] = 0;
		strcpy(ininame, "Movie");
		itoa((int) i + 1, &ininame[5], 10);
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", ininame, origMovieNames[i], &MovieNames[i * 65],
						&sfall1_ini_info_G);
		writeMem32EXE(0x1055F0 + (i * 4), (uint32_t) getRealBlockAddrData(&MovieNames[i * 65]));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartYear", "-1", prop_value, &sfall1_ini_info_G);
	// I'd use stdtol() for the conversion, but that's not available on the game EXE...
	sscanf(prop_value, "%d", &temp_int);
	if (temp_int >= 0) {
		writeMem32EXE(0x9175A+2, (uint32_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartMonth", "-1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 1) && (temp_int <= 12)) {
		writeMem32EXE(0x91771+1, (uint32_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartDay", "-1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 1) && (temp_int <= 31)) {
		writeMem8EXE(0x91744+2, (uint8_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "LocalMapXLimit", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x9DFB9+4, (uint32_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "LocalMapYLimit", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x9DFC7+4, (uint32_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DialogueFix", "1", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x3EFA4+2, 0x31);
	}

	CritInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DisplayKarmaChanges", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "KarmaGain", "You gained %d karma.", KarmaGainMsg, &translation_ini_info_G);
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "KarmaLoss", "You lost %d karma.", KarmaLossMsg, &translation_ini_info_G);
		HookCallEXE(0x4CED4, getRealBlockAddrCode((void *) &op_set_global_var_hook));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "PlayIdleAnimOnReload", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		HookCallEXE(0x563D9, getRealBlockAddrCode((void *) &intface_item_reload_hook));
	}

	// Disabled
	//idle = GetPrivateProfileIntA("Misc", "ProcessorIdle", -1, ini);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SkipOpeningMovies", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem16EXE(0x728C3, 0x13EB);            // jmps 0x472A88
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "NPCsTryToSpendExtraAP", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*(uint32_t *) getRealBlockAddrData(&RetryCombatMinAP) = temp_uint32;
	if (temp_uint32 > 0) {
		// Apply retry combat patch
		HookCallEXE(0x20ABA, getRealBlockAddrCode((void *) &combat_turn_hook));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "RemoveWindowRounding", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem16EXE(0xA4BD0, 0x04EB);// jmps 0x4A50C6
	}

	// Disabled
	//ConsoleInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SpeedInterfaceCounterAnims", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (1 == temp_int) {
		MakeCallEXE(0x563F1, getRealBlockAddrCode((void *) &intface_rotate_numbers_hook), true);
	} else if (2 == temp_int) {
		writeMem32EXE(0x56406, 0x90DB3190u);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SpeedInventoryPCRotation", "166", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int != 166) && (temp_int <= 1000)) {
		writeMem32EXE(0x6415A+1, temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "RemoveCriticalTimelimits", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x91412+1, 0x0);
		writeMem8EXE(0x91453+1, 0x0);
	}

	// Patch ereg call - I think this is only for Windows, so I've disabled it here
	//BlockCallEXE(0x3B25F);

	// todo The below is disabled until ALL the addresses are corrected for the DOS EXE
	/*getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AnimationsAtOnceLimit", "21", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &AnimationsLimit);
	if (*(int *) getRealBlockAddrData(&AnimationsLimit) > 21) {
		AnimationsAtOnceInit();
	}*/

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "CombatPanelAnimDelay", "1000", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 0) && (temp_int <= 65535)) {
		writeMem32EXE(0x55385+1, (uint32_t) temp_int);
		writeMem32EXE(0x554DA+1, (uint32_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DialogPanelAnimDelay", "33", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 0) && (temp_int <= 255)) {
		writeMem32EXE(0x400FF+1, (uint32_t) temp_int);
		writeMem32EXE(0x401C1+1, (uint32_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Debugging", "DebugMode", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		uint32_t str_addr = 0;
		// This is a modification of the patch (DADi590) - this one doesn't cut code
		MakeCallEXE(0x728A7, getRealBlockAddrCode((void *) &DebugMode), false);

		writeMem8EXE(0xB308B, 0xB8);               // mov  eax, offset ???
		if (1 == temp_int) {
			str_addr = (uint32_t) getRealEXEAddr(0xFE1EC); // "gnw"
		} else if (2 == temp_int) {
			str_addr = (uint32_t) getRealEXEAddr(0xFE1D0); // "log"
		} else if (3 == temp_int) {
			str_addr = (uint32_t) getRealEXEAddr(0xFE1E4); // "screen"
		} else if (4 == temp_int) {
			str_addr = (uint32_t) getRealEXEAddr(0xFE1C8); // "mono"
		}
		writeMem32EXE(0xB308B+1, str_addr);
	}

	// I don't think this applies for MS-DOS...
	//if (GetPrivateProfileIntA("Misc", "SingleCore", 1, ini)) {
	//	dlog("Applying single core patch.", DL_INIT);
	//	HANDLE process = GetCurrentProcess();
	//	SetProcessAffinityMask(process, 1);
	//	CloseHandle(process);
	//	dlogr(" Done", DL_INIT);
	//}

	//Bodypart hit chances
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Head", "0xFFFFFFD8", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEE84)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Left_Arm", "0xFFFFFFE2", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEE88)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Right_Arm", "0xFFFFFFE2", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEE8C)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Torso", "0x00000000", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEE90)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Right_Leg", "0xFFFFFFEC", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEE94)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Left_Leg", "0xFFFFFFEC", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEE98)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Eyes", "0xFFFFFFC4", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEE9C)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Groin", "0xFFFFFFE2", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEEA0)) = temp_uint32;
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "BodyHit_Uncalled", "0x00000000", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*((uint32_t *) getRealEXEAddr(0xFEEA4)) = temp_uint32;

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Input", "ToggleItemHighlightsKey", "NONE", prop_value, &sfall1_ini_info_G);
	*(char *) getRealBlockAddrData(&toggleHighlightsKey) = (char) (0 == strcmp(prop_value, "NONE") ? '\0' : prop_value[0]);
	if ('\0' != *(char *) getRealBlockAddrData(&toggleHighlightsKey)) {
		HookCallEXE(0x43715, getRealBlockAddrCode((void *) &gmouse_bk_process_hook));
		HookCallEXE(0x4398A, getRealBlockAddrCode((void *) &obj_remove_outline_hook));
		HookCallEXE(0x46155, getRealBlockAddrCode((void *) &obj_remove_outline_hook));
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Input", "TurnHighlightContainers", "0", prop_value, &sfall1_ini_info_G);
		sscanf(prop_value, "%ud", (uint32_t *) getRealBlockAddrData(&TurnHighlightContainers));
	}

	HookCallEXE(0x72D37, getRealBlockAddrCode((void *) &get_input_hook));       //hook the main game loop
	HookCallEXE(0x2082E, getRealBlockAddrCode((void *) &get_input_hook));       //hook the combat loop

	MainMenuInit();

	CreditsInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DisablePipboyAlarm", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x88E44, 0xC3);// retn
	}

	LoadGameHookInit();

	InventoryInit();

	// todo I think this is not working. Check everything on it again --> EVERYthing
	PartyControlInit(); // todo In the end, check if this works already (or still not and go fix it somehow...)

	// This is for Russian language. Last thing to do (maybe no one requests it and it's one less thing to do?)
	/*char xltcodes[512];
	if (GetPrivateProfileStringA("Misc", "XltTable", "", xltcodes, 512, ini) > 0) {
		int count = 0;
		char *xltcode = strtok(xltcodes, ",");
		while (xltcode && count < 94) {
			int _xltcode = atoi(xltcode);
			if (_xltcode < 32 || _xltcode > 255) break;
			XltTable[count++] = (BYTE) _xltcode;
			xltcode = strtok(0, ",");
		}
		if (count == 94) {
			XltKey = GetPrivateProfileIntA("Misc", "XltKey", 4, ini);
			if (XltKey != 4 && XltKey != 2 && XltKey != 1) XltKey = 4;
			MakeCall(0x42E0B6, &get_input_str_hook, true);
			SafeWrite8(0x42E04E, 0x7D);
			MakeCall(0x471784, &get_input_str2_hook, true);
			SafeWrite8(0x47171C, 0x7D);
			MakeCall(0x4B7060, &kb_next_ascii_English_US_hook, true);
			if (*((DWORD *) 0x442BC3) == 0x89031488) {
				HookCall(0x442B86, &about_process_input_hook);
				HookCall(0x442FF2, &stricmp_hook);
				HookCall(0x44304C, &stricmp_hook);
			}
		}
	}*/

	// Navigation keys in pipboy
	HookCallEXE(0x86A5E, getRealBlockAddrCode((void *) &pipboy_hook));

	// Third ending for the Brotherhood of Steel
	MakeCallEXE(0x38B00, getRealBlockAddrCode((void *) &Brotherhood_final), true);

	// Disable the disappearance of an unused perk
	writeMem8EXE(0x361CB, 0x80);                // add  byte ptr ds:_free_perk, 1
	writeMem16EXE(0x36222, 0x0DFE);             // dec  byte ptr ds:_free_perk
	writeMem8EXE(0x36228+1, 0xB1);              // jmp  0x436263
	HookCallEXE(0x2C64E, getRealBlockAddrCode((void *) &editor_design_hook));
	HookCallEXE(0x2CBE4, getRealBlockAddrCode((void *) &editor_design_hook2));
	HookCallEXE(0x368EC, getRealBlockAddrCode((void *) &perks_dialog_hook));
	MakeCallEXE(0x866C2, getRealBlockAddrCode((void *) &perk_can_add_hook), true);

	// Raise the Perk Window
	writeMem8EXE(0x3641E+1, 31);                  // 91-60=31
	writeMem8EXE(0x369D2+2, 74);                  // 134-60=74

	// Decrease the distance to switch to walking when clicking on an item
	for (i = 0; i < (sizeof(WalkDistance) / 4); ++i) {
		writeMem8EXE(((uint32_t *) getRealBlockAddrData(WalkDistance))[i], 1);
	}

	// Fix "Pressing A to enter combat before anything else happens, thus getting infinite free running"
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "FakeCombatFix", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*(uint32_t *) getRealBlockAddrData(&FakeCombatFix) = temp_uint32;
	if (0 != temp_uint32) {
		MakeCallEXE(0x17952, getRealBlockAddrCode((void *) &check_move_hook), false);
		MakeCallEXE(0x43609, getRealBlockAddrCode((void *) &gmouse_bk_process_hook1), false);
		HookCallEXE(0x43D98, getRealBlockAddrCode((void *) &FakeCombatFix1));       // action_get_an_object_
		HookCallEXE(0x443CB, getRealBlockAddrCode((void *) &FakeCombatFix1));       // action_get_an_object_
		HookCallEXE(0x43E41, getRealBlockAddrCode((void *) &FakeCombatFix2));       // action_loot_container_
		HookCallEXE(0x443B7, getRealBlockAddrCode((void *) &FakeCombatFix2));       // action_loot_container_
		HookCallEXE(0x11F7B, getRealBlockAddrCode((void *) &FakeCombatFix3));       // action_use_an_object_
		HookCallEXE(0x43F80, getRealBlockAddrCode((void *) &FakeCombatFix3));       // gmouse_handle_event_
		MakeCallEXE(0x1FEDA, getRealBlockAddrCode((void *) &combat_begin_hook), false);
		MakeCallEXE(0x1FA53, getRealBlockAddrCode((void *) &combat_reset_hook), false);
		HookCallEXE(0x20D29, getRealBlockAddrCode((void *) &combat_hook));
	}

	// Some people were not that happy this "feature", so I've put it optional, and disabled by default (it's wtf anyway)
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "F1DPExpandedAges", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		// Minimum player age
		writeMem8EXE(0x108364 + 0xC, 8);

		// Maximum Player Age
		writeMem8EXE(0x108364 + 0x10, 60);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "EnableMusicInDialogue", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x3E45B+1, 0x00);
		// This below was already commented out, probably because of the above which makes the code never get to where
		// this patch is.
		//BlockCall(0x481A7);
	}

	// Max player level
	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "MaxPCLevel", "21", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*(uint32_t *) getRealBlockAddrData(&MaxPCLevel) = temp_uint32;
	if ((temp_uint32 != 21) && (temp_uint32 >= 1) && (temp_uint32 <= 99)) {
		writeMem8EXE(0x3611B+2, (uint8_t) temp_uint32);
		writeMem8EXE(0x9CCBC+2, (uint8_t) temp_uint32);
		MakeCallEXE(0x9CB82, getRealBlockAddrCode((void *) &stat_pc_min_exp_hook), false);
		// Max Perks Gained
		writeMem8EXE(0x36197+2, 33);
		writeMem8EXE(0x361A2+2, 33);
		writeMem8EXE(0x37D4A+2, 33);
		// cap hitpoints at 999 when leveling up
		writeMem32EXE(0x1080F4+0x10, 999);
	}

	// todo Too much work. Disabled for now. Complete it.
	/*getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AutoQuickSave", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*(uint32_t *) getRealBlockAddrData(&AutoQuickSave) = temp_uint32;
	if ((temp_uint32 >= 1) && (temp_uint32 <= 10)) {
		--*(uint32_t *) getRealBlockAddrData(&AutoQuickSave);
		SafeWrite16(0x46DEAB, 0xC031);            // xor  eax, eax
		SafeWrite32(0x46DEAD, 0x505A50A3);        // mov  ds:_slot_cursor, eax
		SafeWrite16(0x46DEB2, 0x04EB);            // jmp  0x46DEB8
		MakeCall(0x46DF13, &SaveGame_hook, true);
	}*/

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DontTurnOffSneakIfYouRun", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem8EXE(0x17A2F, 0xEB);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "ColorLOS", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", &temp_uint32);
	*(uint32_t *) getRealBlockAddrData(&ColorLOS) = temp_uint32;
	if ((2 == temp_uint32) || (4 == temp_uint32) || (16 == temp_uint32) || (32 == temp_uint32)) {
		HookCallEXE(0x242FF, getRealBlockAddrCode((void *) &combat_update_critter_outline_for_los));
		HookCallEXE(0x24399, getRealBlockAddrCode((void *) &combat_update_critter_outline_for_los));
		MakeCallEXE(0x7C2CA, getRealBlockAddrCode((void *) &obj_move_to_tile_hook), true);
	}


	freeNew(((struct FileInfo *) getRealBlockAddrData(&translation_ini_info_G))->contents);
}
