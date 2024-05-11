// Copyright 2022 Edw590
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


// This could use some optimizations, but it will only run once every game start. So no big deal being a bit slower,
// but more readable xD. For example the while loop could have the condition put outside, but that's one more variable
// on the beginning of realMain() and that's a mess (I also don't want to create scopes just for that - no need).

// WARNING: I'm not paying much attention to how much of the stack I'm using... This MUST be loaded right after main()
// starts on the game, before more functions are called and the stack starts to get more full.

// WARNING 2: ALL GLOBAL VARIABLES MUST BE INITIALIZED AND ---NEVER--- UNINITIALIZED!!!!!! Else, it will go to a segment
// which is not on the file, and hence, can't be copied and, therefore, allocated.
// This is also just because I don't want to be appending NULL bytes or hard-coding a number on the Loader for it to
// always allocate space for the uninitialized data section. So just initialize everything (easy thing to do anyways).

// WARNING 3: NO SWITCH CASES MUST BE IN THIS ENTIRE PROJECT!!!!!! For those, there are things called Jump Tables. They
// use absolute addresses --> no way. Use IF statements ¯\_(ツ)_/¯.

#include "CLibs/stdio.h"
#include "CLibs/stdlib.h"
#include "CLibs/string.h"
#include "FixtPatches/FixtPatches.h"
#include "GameAddrs/CStdFuncs.h"
#include "HighResPatches/HighResPatches.h"
#include "PatcherPatcher/PatcherPatcher.h"
#include "SFall1Patches/SFall1Patches.h"
#include "TeamXPatches/TeamXPatches.h"
#include "Utils/BlockAddrUtils.h"
#include "Utils/EXEPatchUtils.h"
#include "Utils/General.h"
#include "Utils/GlobalEXEAddrs.h"
#include "Utils/IniUtils.h"
#include "CLibs/conio.h"

#define SN_MAIN_FUNCTION 0x78563412 // 12 34 56 78 in little endian

struct FileInfo f1dpatch_ini_info_G = {0};

// The spaces in the end are to pad the string so that it doesn't overlap with the bit of white background
static char version_str[] = "F1DP v"F1DP_VER_STR" by Edw590      ";

bool realMain(void);
static void patchVerStr(void);
bool applyPatches(void);
void fixPSPNotSaving(void);

__declspec(naked) int main(void) {
	// The special number declaration below is for the loader to look for it and start the execution right there.
	// Useful since I copy the entire code segment, and there are things above main(). The number of things before
	// main() may or may not change, depending on what is declared before it (like in headers or strings).
	// This way that's protected for sure. Just look for a weird number and start the execution 4 bytes after.

	// Also, implementing this function is just to be able to run the program as a normal EXE in case it's needed for
	// any reason. Though, this is unnecessary if it's for the patch to be tested only when ran inside the EXE. In that
	// case, the special number could be put in the beginning of realMain().
	// EDIT: actually no, because to put the DD and JMP statements on realMain(), without making it naked, it wouldn't
	// be the real start of the function (PUSHes and POPs first, aside from the part that local variables must be above
	// all other things); and putting it naked, would make it a pain to be sure it's working well. This is MUCH easier.
	// So this is still needed. Though, the 1st jump is still unnecessary and is only there to run this as a normal
	// program and nothing else.

	__asm {
			jmp     main1 // Jump over the data in case this program's EXE is executed normally
			dd      SN_MAIN_FUNCTION
		main1: // Where code execution begins for the Loader
			// Right before ANYthing else (before the EDI register, which contains the address of the allocated block) is
			// used somewhere), patch the patcher itself - and also before any operations that need the Special Numbers
			// already replaced.
			mov     eax, SN_BASE
			call    patcherPatcher
			jmp     realMain
	}

	// No return value here on main() because this function never returns - it's realMain() that returns, so the return
	// value is there, and may or may not of int type (the Loader is the one interpreting that value, so it's decided
	// by the programmer, not by C's specifications).
}

/**
 * @brief This is the real main() function, since the original one just redirects to this one as soon as the Patcher has
 * everything ready for normal functioning.
 *
 * @return true if everything went alright, false if at least one error occurred
 */
bool realMain(void) {
	// Don't forget we can't initialize variables that are not initialized with just "= [number]". If it's with "= {0}"
	// or something like that, it will copy from empty memory to the destination --> memory without address correction,
	// because it's automatic.
	// So ONLY initialize if it can be initialized with "= number", OR if it can't, ask for memset()'s help.
	bool ret_var = true;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	memset(&f1dpatch_ini_info_G, 0, sizeof(f1dpatch_ini_info_G));

	((struct FileInfo *) getRealBlockAddrData(&f1dpatch_ini_info_G))->is_main_ini = true;

	printlnStr("  /--- F1DP v"F1DP_VER_STR" Patcher ---\\");

	// Open the main INI file
	if (!readFile(F1DP_MAIN_INI, &f1dpatch_ini_info_G)) {
		printlnStr(LOGGER_ERR_STR "Main settings file \""F1DP_MAIN_INI"\" not found! Aborting all operations...");

		ret_var = false;
		goto funcEnd;
	}

	printlnStr(LOGGER_STR "Initialization successful.");

	// Leave these 3 turned on. Might be useful for anyone to debug something wrong with the game or whatever.
	printf(LOGGER_STR "Block code section at: 0x%X; Block data section at: 0x%X;"NL, SN_CODE_SEC_BLOCK_ADDR, SN_DATA_SEC_BLOCK_ADDR);
	printf("    Game code section at: 0x%X; Game data section at: 0x%X;"NL, SN_CODE_SEC_EXE_ADDR, SN_DATA_SEC_EXE_ADDR);
	printf("    Block at: 0x%X."NL, SN_BLOCK_ADDR);

	// Check if the version of the INI file is the same as of the Patcher
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "F1DPVersion", NULL, prop_value, &f1dpatch_ini_info_G)) {
		if (0 != strcmp(prop_value, F1DP_VER_STR)) {
			// There was a float comparison here, but what if I decide to launch 1.1.1, for example?
			// But actually there's no problem at all. I had it there because of spaces after the numbers. But that's
			// being ignored with the getPropValueIni() function. So it's all good just checking the value string.
			printlnStr(LOGGER_ERR_STR "Wrong INI settings file version. Aborting the Patcher...");

			ret_var = false;
			goto funcEnd;
		}
	} else {
		printlnStr(LOGGER_ERR_STR "F1DPVersion not specified. Aborting the Patcher...");

		ret_var = false;
		goto funcEnd;
	}

	// If the INI file is present and is of the correct version, patch the version string to indicate the Patcher is
	// loaded and working normally.
	patchVerStr();

	ret_var = ret_var && applyPatches();


	funcEnd:

	free(((struct FileInfo *) getRealBlockAddrData(&f1dpatch_ini_info_G))->contents);

	printlnStr("  \\--- F1DP v"F1DP_VER_STR" Patcher ---/");

	return ret_var;
}

__declspec(naked) static void getverstr_hook(void) {
	__asm {
			push    edx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edi+version_str]
			pop     edi

			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]
			push    edx
			mov     edx, eax
			push    eax
			mov     [esp+2*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_sprintf_]
			call    edi
			add     esp, 2*4
			pop     edi

			mov     eax, edx
			pop     edx
			ret
	}
}

/**
 * @brief Patches the FALLOUT 1.2 string on the main menu in the bottom right corner with F1DP's own version string.
 */
static void patchVerStr(void) {
	// This replaces a call to the original getverstr_ function by the one above. The other call to that function
	// remains untouched, which is used when Ctrl+V is pressed in-game and the version will appear on the Pip-Boy.
	// The function above just replaces the original string with a custom one from inside the Patcher and adjusts
	// whatever is necessary for that to happen (like the stack).
	// Also, below I'm changing the height of the string so that it doesn't overlap with the rest of the bottom strings.

	writeMem32EXE(0x73373 + 1, 0x1BD, true); // Change the string height (445)

	hookCallEXE(0x73358, &getverstr_hook);
}

/**
 * @brief Applies the patches according to the INI file settings.
 *
 * @return true if everything went fine, false if at least one error occurred
 */
bool applyPatches(void) {
	bool ret_var = true;
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);



	// APPLY THE PATCHES IN CHRONOLOGICAL ORDER!!!!!!!!!!!!!!!
	//
	// Meaning: TeamX's patches came first. Other patches may have been build on top of those (which are permanent
	// patches, not dynamic with a DLL). So those are the first ones to be applied.
	// Then come Fixt patches, which are also permanent, and came after TeamX's. Finally there's sFall1 patches which
	// are dynamic, so they are always applied on top of whatever was already on the EXE (the 2 patches mentioned above).
	// Update: after this there's High-Res patch. I'm putting it after sFall1 because High-Res patch INI file mentions
	// sFall stuff, and the opposite doesn't happen - so High-Res patch after sFall1.



	// Enable or disable TeamX's patches
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "TeamXPatches", NULL, prop_value, &f1dpatch_ini_info_G)) {
		sscanf(prop_value, "%d", &temp_int);
		if (0 == temp_int) {
			printlnStr(LOGGER_STR "TeamX's patches disabled.");
		} else if (1 == temp_int) {
			printlnStr(LOGGER_STR "TeamX's patches enabled.");

			initTeamXPatches();
		} else {
			printlnStr(LOGGER_ERR_STR "'TeamXPatches' has an invalid value. Aborting Fallout Fixt patches...");

			ret_var = false;
		}
	} else {
		printlnStr(LOGGER_ERR_STR "'TeamXPatches' not specified. Aborting Fallout Fixt patches...");

		ret_var = false;
	}

	// Enable or disable the Fallout Fixt patches
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "FixtPatches", NULL, prop_value, &f1dpatch_ini_info_G)) {
		sscanf(prop_value, "%d", &temp_int);
		if (0 == temp_int) {
			printlnStr(LOGGER_STR "Fallout Fixt patches disabled.");
		} else if (1 == temp_int) {
			printlnStr(LOGGER_STR "Fallout Fixt patches enabled.");

			initFixtPatches();
		} else {
			printlnStr(LOGGER_ERR_STR "'FixtPatches' has an invalid value. Aborting Fixt patches...");

			ret_var = false;
		}
	} else {
		printlnStr(LOGGER_ERR_STR "'FixtPatches' not specified. Aborting Fixt patches...");

		ret_var = false;
	}

	// Enable or disable Crafty's sFall1 patches
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "CraftySFall1Patches", NULL, prop_value, &f1dpatch_ini_info_G)) {
		sscanf(prop_value, "%d", &temp_int);
		if (0 == temp_int) {
			printlnStr(LOGGER_STR "Crafty's sFall1 patches disabled.");
		} else if (1 == temp_int) {
			printlnStr(LOGGER_STR "Crafty's sFall1 patches enabled.");

			ret_var = ret_var && initSFall1Patches();
		} else {
			printlnStr(LOGGER_ERR_STR "'CraftySFall1Patches' has an invalid value. Aborting Crafty's sFall1 patches...");

			ret_var = false;
		}
	} else {
		printlnStr(LOGGER_ERR_STR "'CraftySFall1Patches' not specified. Aborting Crafty's sFall1 patches...");

		ret_var = false;
	}

	// Enable or disable Mash's High-Res patches
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "HighResPatches", NULL, prop_value, &f1dpatch_ini_info_G)) {
		sscanf(prop_value, "%d", &temp_int);
		if (0 == temp_int) {
			printlnStr(LOGGER_STR "High-Res patches disabled.");
		} else if (1 == temp_int) {
			printlnStr(LOGGER_STR "High-Res patches enabled.");

			initHighResPatches();
		} else {
			printlnStr(LOGGER_ERR_STR "'HighResPatches' has an invalid value. Aborting High-Res patches...");

			ret_var = false;
		}
	} else {
		printlnStr(LOGGER_ERR_STR "'HighResPatches' not specified. Aborting High-Res patches...");

		ret_var = false;
	}

	return ret_var;
}

// This below was used to try to see why the PSP wouldn't save the game in the conditions I say in
// https://www.reddit.com/r/PSP/comments/crfeuf/running_fallout_1_on_the_psp_with_dosbox_fast/.
// Though, while on this, I found another build of DOSBox for PSP, which finally fixed the problem, and I stopped
// with this - and I left the code here in case it's ever useful again.

// It counts the number of opened files and the error flag in case it was not possible to open or close some file, and
// also prints a usage of db_fopen_().

/*uint32_t curr_opened_files = 0;
int prev_was_inc = 0;
char const fmt_str_inc[] = "\n+F:%d; sopen_; previous errno=%d\n";
char const fmt_str_dec[] = "\n-F:%d; __close_; previous errno=%d\n";
void printNumOpenFiles(bool inc) {
	char *fmt_str = NULL;
	int curr_opened_files_local = 0;
	int prev_errno = *(int *) getRealEXEAddr(0x2AE668);

	*(uint32_t *) getRealEXEAddr(0x2AE668) = 0;

	if (0 == prev_errno) {
		if (1 == *(int *) getRealBlockAddrData(&prev_was_inc)) {
			++(*(uint32_t *) getRealBlockAddrData(&curr_opened_files));
		} else if (2 == *(int *) getRealBlockAddrData(&prev_was_inc)) {
			--(*(uint32_t *) getRealBlockAddrData(&curr_opened_files));
		}
	}
	if (inc) {
		*(int *) getRealBlockAddrData(&prev_was_inc) = 1;
		curr_opened_files_local = *(uint32_t *) getRealBlockAddrData(&curr_opened_files);
		fmt_str = getRealBlockAddrData(&fmt_str_inc);

		__asm {
				lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]

				push    [prev_errno]
				push    [curr_opened_files_local]
				push    [fmt_str]

				mov     [esp+5*4], edi // [Edw590: "PUSH EDI"]
				mov     edi, SN_CODE_SEC_EXE_ADDR
				lea     edi, [edi+C_debug_printf_]
				call    edi
				pop     edi
				add     esp, 5*4
		}
	} else {
		*(int *) getRealBlockAddrData(&prev_was_inc) = 2;
		curr_opened_files_local = *(uint32_t *) getRealBlockAddrData(&curr_opened_files);
		fmt_str = getRealBlockAddrData(&fmt_str_dec);

		__asm {
				lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]

				push    [prev_errno]
				push    [curr_opened_files_local]
				push    [fmt_str]

				mov     [esp+4*4], edi // [Edw590: "PUSH EDI"]
				mov     edi, SN_CODE_SEC_EXE_ADDR
				lea     edi, [edi+C_debug_printf_]
				call    edi
				pop     edi
				add     esp, 4*4
		}
	}
}
__declspec(naked) static void sopen_hook(void) {
	__asm {
			pusha

			mov     al, 1
			call    printNumOpenFiles

			popa

			// Function stuff
			push    ebx
			push    ecx
			push    edx
			push    esi
			push    edi

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0xD5583]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}
__declspec(naked) static void __close_hook(void) {
	__asm {
			pusha

			mov     al, 0
			call    printNumOpenFiles

			popa

			// Function stuff
			push    ebx
			push    ecx
			push    edx
			mov     ecx, eax

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0xDF494]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

char const fmt_str_db_fopen[] = "\ndb_fopen_(%s, %s) %d; errno=%d\n";
static int db_fopen_map_save_hook(char *filename, char *mode) {
	int ret_var = 0;
	int errno = 0;
	char *fmt_str = getRealBlockAddrData(&fmt_str_db_fopen);

	*(uint32_t *) getRealEXEAddr(0x2AE668) = 0;

	__asm {
			mov     eax, [filename]
			mov     edx, [mode]

			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_db_fopen_]
			call    edi
			pop     edi

			mov     [ret_var], eax
	}
	errno = *(uint32_t *) getRealEXEAddr(0x2AE668);
	__asm {
			pusha

			lea     esp, [esp-4] // [Edw590: reserve space to "PUSH EDI"]

			push    [errno]
			push    [ret_var]
			push    [mode]
			push    [filename]
			push    [fmt_str]

			mov     [esp+5*4], edi // [Edw590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_debug_printf_]
			call    edi
			pop     edi
			add     esp, 5*4

			popa
	}

	return ret_var;
}

void fixPSPNotSaving(void) {
	(void *) __close_hook;
	(void *) sopen_hook;
	(void *) db_fopen_map_save_hook;

	writeMem8EXE(C_gmovie_play_, 0xC3);

	makeCallEXE(0xDF48F, &__close_hook, true);
	makeCallEXE(0xD557E, &sopen_hook, true);

	hookCallEXE(0x757D4, &db_fopen_map_save_hook);
}*/
