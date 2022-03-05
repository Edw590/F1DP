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

#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../OtherHeaders/General.h"
#include "../Utils/BlockAddrUtils.h"
#include "Inventory.h"
#include "SFall1Patches.h"
#include "sFall1Main.h"


// ADVICE: don't try to understand the functions in each patch... Infinite EDI register uses there to be able to have
// the special numbers in use. If you want to understand what was done, just go to the sFall1 source and see there. If
// you think anything is wrong with the ones here, redo from scratch, don't try to fix. It's a mess with this way of
// Special Numbers, but I don't have better ideas without making a relocation table for the EXE, parsing it and applying
// it (as long as this way works, I think it's much better than going that route, which would take infinity).

/*
 * /////////////////////////////////////////////////////////////////
 *             RULES TO FOLLOW IN THE PORTS!!!!!!!!!!!
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
 * - Go check all the C_s and D_s that remained untouched and use the appropriate SN.
 *
 * /////////////////////////////////////////////////////////////////
 */


struct FileInfo translation_ini_info_G = {0};

void DllMain2(void) {
	char prop_value[MAX_PROP_VALUE_LEN];

	memset(prop_value, 0, MAX_PROP_VALUE_LEN);


	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Main", "TranslationsINI", "./Translations.ini", prop_value, &sfall1_ini_info_G);
	// If it fails, the struct will have 0s and the file won't be read, so the default values will be used as sFall1 does.
	readFile(prop_value, &translation_ini_info_G);

	InventoryInit();


	freeNew(((struct FileInfo *) getRealBlockAddrData(&translation_ini_info_G))->contents);
}
