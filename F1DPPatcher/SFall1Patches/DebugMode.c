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

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "DebugMode.h"
#include "SFall1Patches.h"

static void __declspec(naked) debugMode(void) {
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

void DebugModeInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Debugging", "debugMode", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		uint32_t str_addr = 0;
		// This is a modification of the patch (DADi590) - this one doesn't cut code, because it's put right when main()
		// starts.
		makeCallEXE(0x728A7, getRealBlockAddrCode((void *) &debugMode), false);

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
}
