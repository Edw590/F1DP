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

#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "TeamXPatches.h"

static void __declspec(naked) patch1(void) {
	__asm {
			mov     eax, 0x10
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_game_get_global_var_]
			call    edi
			pop     edi
			test    eax, eax
			jnz     something
			mov     eax, 0x25D
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_game_get_global_var_]
			call    edi
			pop     edi
			test    eax, eax
			jz      otherJump

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x38B18]
			mov     [esp+4], edi
			pop     edi
			retn

		otherJump:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x38B34]
			mov     [esp+4], edi
			pop     edi
			retn

		something:
			lea     esp, [esp-4] // [DADi590: reserve space to "PUSH EDI"]
			push    0
			mov     edx, 0x150
			mov     eax, 6
			xor     ecx, ecx
			xor     ebx, ebx
			mov     [esp+1*4], edi // [DADi590: "PUSH EDI"]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_art_id_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     edx, [edi+0xF131C]
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

void initTeamXPatches(void) {
	// From 1.2 EXE
	// I'm supposing the only thing they did with the semi-official patch to the EXE was to get children back. Though,
	// as the DOS EXE this Patcher patches is from the US release, version 1.2, it has the children enabled already, so
	// no worries about that, I guess.

	// From 1.2.1 EXE
	// Nothing related to the DOS EXE (it's only the SizeOfImage in the PE header that changed).

	// From Restoration Mod EXE
	makeCallEXE(0x38AFB, getRealBlockAddrCode((void *) &patch1), true);
	strncpy(getRealEXEAddr(0xF131C), "nar_31\0\0", 8);
	// There's another patch, but that has to do with screen resolutions, so I'll leave it.
}
