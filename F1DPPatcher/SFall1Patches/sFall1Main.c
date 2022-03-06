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
// his modification of the original sFall1 by Timeslip.

#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../OtherHeaders/General.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "AmmoMod.h"
#include "AnimationsAtOnceLimit.h"
#include "Criticals.h"
#include "Define.h"
#include "FalloutEngine.h"
#include "Inventory.h"
#include "SFall1Patches.h"
#include "sFall1Main.h"


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
 * 			sub     esp, 4 // Reserve space on the stack for the last function PUSH
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
 *			sub     esp, 4 // Reserve space for the address
 *			push    edi
 *			mov     edi, SN_CODE_SEC_EXE_ADDR
 *			add     edi, C_text_font_
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
 *
 * --> Steps to have to do the above:
 * - Find all references to the macros and put C_ or D_ on them.
 * - Go on FalloutEngine.h and go do what is required there for every macro (as a start, put the address there).
 * - Find all references to global variables inside the Patcher and use SN_DATA_SEC_BLOCK_ADDR on them.
 * - Ctrl+F all CALLs and pay attention to the stack (check the documentation of the macro to see if the "Args" are
 *   "regs" and/or "stack". Use the corresponding SN depending on the C_ or D_ prefix.
 * - Ctrl+F all "0x"s, check if they are addresses, and if they are, use the appropriate SN.
 * - Go check all the C_s, D_s and F_s that remained untouched and use the appropriate SN.
 *
 * // ///////////////////////////////////////////////////////////////
 */


struct FileInfo translation_ini_info_G = {0};

static char mapName[65] = {0};
//static char versionString[65] = {0};
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
			add     edi, C_display_print_
			call    edi
			pop     edi
	}
}

static void __declspec(naked) op_set_global_var_hook() {
	__asm {
			cmp     eax, 155                             // PLAYER_REPUTATION
			jne     end
			pushad
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_game_get_global_var_
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
			sub     esp, 4 // [DADi590] Reserve space for the return address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_game_set_global_var_
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) intface_item_reload_hook() {
	__asm {
			pushad
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			mov     eax, ds:[edi+D__obj_dude]
			pop     edi
			push    eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_register_clear_
			call    edi
			pop     edi
			xor     eax, eax
			inc     eax
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_register_begin_
			call    edi
			pop     edi
			xor     edx, edx
			xor     ebx, ebx
			dec     ebx
			pop     eax                                  // _obj_dude
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_register_object_animate_
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_register_end_
			call    edi
			pop     edi
			popad

			sub     esp, 4 // [DADi590] Reserve space for the return address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_gsound_play_sfx_file_
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static uint32_t RetryCombatMinAP = 0;

static void __declspec(naked) combat_turn_hook() {
	__asm {
			xor     eax, eax
		retry:
			xchg    ebx, eax
			mov     eax, esi
			push    edx
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_combat_ai_
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
			add     edi, C_process_bk_
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

static void __declspec(naked) intface_rotate_numbers_hook() {
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

			sub     esp, 4 // [DADi590] Reserve space for the return address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, 0x563F6
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

static void __declspec(naked) debugModeWrapper(void) {
	__asm {
			pusha
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			add     edi, C_debug_register_env_
			call    edi
			pop     edi
			popa

			mov     ecx, 1
			ret
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


	// Make a call just before the game exits, which is inside the Loader code, in the NOPs I left there for this.
	// Those NOPs that are replaced here are just before this block of code is freed by free().
	MakeCallEXE(0xEA2DA, getRealBlockAddrCode((void *) &_WinMain_hook), false);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Main", "TranslationsINI", "./Translations.ini", prop_value, &sfall1_ini_info_G);
	// If it fails, the struct will have 0s and the file won't be read, so the default values will be used as sFall1 does.
	readFile(prop_value, &translation_ini_info_G);


	InventoryInit();

	AmmoModInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "StartingMap", "", mapName, &sfall1_ini_info_G);
	if (0 != strcmp(mapName, "")) {
		writeMem32EXE(0x72995, (uint32_t) getRealBlockAddrData(mapName));
	}

	// Disabled
	// I'm already replacing this string and moving it up. If it were on Windows, both could be there. On DOS, with the
	// other things there too, only one can (F1DP in this case).
	//getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "VersionString", "", versionString, &sfall1_ini_info_G);
	//if (0 != strcmp(mapName, "")) {
	//	writeMem32EXE(0xA10E7+1, (uint32_t) getRealBlockAddrData(versionString));
	//}

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
	if (0 != strcmp(prop_value, "0")) {
		writeMem8EXE(0x3EFA4+2, 0x31);
	}

	CritInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DisplayKarmaChanges", "0", prop_value, &sfall1_ini_info_G);
	if (0 != strcmp(prop_value, "0")) {
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "KarmaGain", "You gained %d karma.", KarmaGainMsg, &translation_ini_info_G);
		getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "sfall", "KarmaLoss", "You lost %d karma.", KarmaLossMsg, &translation_ini_info_G);
		HookCallEXE(0x4CED4, getRealBlockAddrCode((void *) &op_set_global_var_hook));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "PlayIdleAnimOnReload", "0", prop_value, &sfall1_ini_info_G);
	if (0 != strcmp(prop_value, "0")) {
		HookCallEXE(0x563D9, getRealBlockAddrCode((void *) &intface_item_reload_hook));
	}

	// Disabled
	//idle = GetPrivateProfileIntA("Misc", "ProcessorIdle", -1, ini);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SkipOpeningMovies", "0", prop_value, &sfall1_ini_info_G);
	if (0 != strcmp(prop_value, "0")) {
		writeMem16EXE(0x728C3, 0x13EB);            // jmps 0x472A88
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "NPCsTryToSpendExtraAP", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%ud", (uint32_t *) getRealBlockAddrData(&RetryCombatMinAP));
	if (*(uint32_t *) getRealBlockAddrData(&RetryCombatMinAP) > 0) {
		// Apply retry combat patch
		HookCallEXE(0x20ABA, getRealBlockAddrCode((void *) &combat_turn_hook));
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "RemoveWindowRounding", "0", prop_value, &sfall1_ini_info_G);
	if (0 != strcmp(prop_value, "0")) {
		writeMem16EXE(0xA4BD0, 0x04EB);// jmps 0x4A50C6
	}

	// Disabled
	//ConsoleInit();

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SpeedInterfaceCounterAnims", "0", prop_value, &sfall1_ini_info_G);
	if (0 == strcmp(prop_value, "1")) {
		MakeCallEXE(0x563F1, getRealBlockAddrCode((void *) &intface_rotate_numbers_hook), true);
	} else if (0 == strcmp(prop_value, "2")) {
		writeMem32EXE(0x56406, 0x90DB3190u);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "SpeedInventoryPCRotation", "166", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int != 166) && (temp_int <= 1000)) {
		writeMem32EXE(0x6415A+1, temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "RemoveCriticalTimelimits", "0", prop_value, &sfall1_ini_info_G);
	if (0 == strcmp(prop_value, "0")) {
		writeMem8EXE(0x91412+1, 0x0);
		writeMem8EXE(0x91453+1, 0x0);
	}

	// Patch ereg call - I think this is only for Windows, so I've disabled it here
	//BlockCallEXE(0x3B25F);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AnimationsAtOnceLimit", "21", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &AnimationsLimit);
	if (*(int *) getRealBlockAddrData(&AnimationsLimit) > 21) {
		AnimationsAtOnceInit();
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "CombatPanelAnimDelay", "1000", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 0) && (temp_int <= 65535)) {
		writeMem32EXE(0x55385+1, (uint32_t) temp_int);
		writeMem32EXE(0x554DA+1, (uint32_t) temp_int);
	};

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "DialogPanelAnimDelay", "33", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int >= 0) && (temp_int <= 255)) {
		writeMem32EXE(0x400FF+1, (uint32_t) temp_int);
		writeMem32EXE(0x401C1+1, (uint32_t) temp_int);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Debugging", "DebugMode", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != strcmp(prop_value, "0")) {
		uint32_t str_addr = 0;
		// This is a modification of the patch (DADi590) - this one doesn't cut code
		MakeCallEXE(0x728A7, getRealBlockAddrCode((void *) &debugModeWrapper), false);

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


	freeNew(((struct FileInfo *) getRealBlockAddrData(&translation_ini_info_G))->contents);
}
