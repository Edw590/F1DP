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
#include "GameAddrs/CStdFuncs.h"
#include "Utils/General.h"
#include "Utils/GlobalEXEAddrs.h"
#include "PatcherPatcher/PatcherPatcher.h"
#include "SFall1Patches/SFall1Patches.h"
#include "Utils/BlockAddrUtils.h"
#include "Utils/EXEPatchUtils.h"
#include "Utils/IniUtils.h"
#include <stdbool.h>

#define SN_MAIN_FUNCTION 0x78563412 // 12 34 56 78 in little endian

bool prop_logPatcher_G = true;
struct FileInfo dospatch_ini_info_G = {0};

static char version_str[] = "F1DP v"F1DP_VER_STR" by DADi590";

bool realMain(void);
static void patchVerStr(void);

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
		main1: // Where code execution begins for the loader
			// Right before ANYthing else (before the ESI register, which contains the address of the allocated block) is
			// used somewhere), patch the patcher itself - and also before any operations that need the special numbers
			// already replaced.
			mov     eax, SN_BASE
			call    patchPatcher
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
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];

	memset(&dospatch_ini_info_G, 0, sizeof(dospatch_ini_info_G));
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	((struct FileInfo *) getRealBlockAddrData(&dospatch_ini_info_G))->is_main_ini = true;

	printlnStr("  /---- F1DP v"F1DP_VER_STR" Patcher ----\\");

	// Open the main INI file
	if (!readFile(F1DP_MAIN_INI, &dospatch_ini_info_G)) {
		printlnStr(LOGGER_ERR_STR "DOSPATCH.INI NOT FOUND!!!");
		printlnStr(LOGGER_STR "Press any key to create a new dospatch.ini file with default values...");

		// todo Make a function to create the INI file with default values

		ret_var = false;
		goto funcEnd;
	}

	printlnStr(LOGGER_STR "Initialization successful.");
	logf(LOGGER_STR "Code section at: 0x%X; Data section at: 0x%X."NL, SN_CODE_SEC_BLOCK_ADDR, SN_DATA_SEC_BLOCK_ADDR);

	// Check if the version of the INI file is the same as of the Patcher
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "F1DPVersion", NULL, prop_value, &dospatch_ini_info_G)) {
		float temp_float = 0;
		float temp_float2 = 0;
		sscanf(prop_value, "%f", &temp_float);
		sscanf(F1DP_VER_STR, "%f", &temp_float2);
		if (temp_float != temp_float2) {
			// Ignore floating-point comparision errors. It's the same value, no operations are made in different orders
			// or something like that. It's just convert the version number and again, that same version number. And
			// then compare both. Must be equal, I guess (I'm not doing math here to influence the precision of the
			// result).
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

	// Enable or disable the logger
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "logPatcher", NULL, prop_value, &dospatch_ini_info_G)) {
		sscanf(prop_value, "%d", &temp_int);
		*(bool *) getRealBlockAddrData(&prop_logPatcher_G) = (0 == temp_int ? false : true);
		if (0 == temp_int) {
			loglnStr(LOGGER_STR "Patcher logger disabled."); // Will never print (logger = false), but anyway.
		} else if (1 == temp_int) {
			loglnStr(LOGGER_STR "Patcher logger enabled.");
		} else {
			printlnStr(LOGGER_ERR_STR "'logPatcher' has a wrong value. Using 1 as default...");

			ret_var = false;
		}
	} else {
		*(bool *) getRealBlockAddrData(&prop_logPatcher_G) = true;
		printlnStr(LOGGER_ERR_STR "'logPatcher' not specified. Using 1 as default...");

		ret_var = false;
	}

	// Enable or disable the sFall1 patches
	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "sFall1Enable", NULL, prop_value, &dospatch_ini_info_G)) {
		// Clang-Tidy is complaining of "Comparison length is too long and might lead to a buffer overflow" with the 2
		// below, but both are null-terminated strings, so ignore that - and I need to check if the string starts and
		// ends with "1", so I need to check the NULL character too (2 characters total).
		sscanf(prop_value, "%d", &temp_int);
		if (0 == temp_int) {
			loglnStr(LOGGER_STR "sFall1 1.7.6 patches disabled.");
		} else if (1 == temp_int) {
			loglnStr(LOGGER_STR "sFall1 1.7.6 patches enabled.");

			ret_var = ret_var && initSfall1Patcher();
		} else {
			printlnStr(LOGGER_ERR_STR "'sFall1Enable' has a wrong value. Aborting sFall1 1.7.6 patches...");

			ret_var = false;
		}
	} else {
		printlnStr(LOGGER_ERR_STR "'sFall1Enable' not specified. Aborting sFall1 1.7.6 patches...");

		ret_var = false;
	}


	funcEnd:

	freeNew(((struct FileInfo *) getRealBlockAddrData(&dospatch_ini_info_G))->contents);

	printlnStr(ret_var ? NL LOGGER_STR "true" : NL LOGGER_STR "false");
	printlnStr("  \\---- F1DP v"F1DP_VER_STR" Patcher ----/");

	return false;
}

static __declspec(naked) void getverstr_hook(void) {
	__asm {
			push    edx
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			lea     edx, [edi+version_str]
			pop     edi

			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    edx
			mov     edx, eax
			push    eax
			mov     [esp+5*4], edi // [DADi590: "PUSH EDI"]
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

static void patchVerStr(void) {
	// This replaces a call to the original getverstr_ function by the one above. The other call to that function
	// remains untouched, which is used when Ctrl+V is pressed in-game and the version will appear on the Pip-Boy.
	// The function above just replaces the original string with a custom one from inside the Patcher and adjusts
	// whatever is necessary for that to happen (like the stack).
	// Also, below I'm changing the height of the string so that it doesn't overlap with the rest of the bottom strings.

	writeMem32EXE(0x73373+1, 0x1BD); // Change the string height (445)

	HookCallEXE(0x73358, getRealBlockAddrCode((void *) &getverstr_hook));
}
