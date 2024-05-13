/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2022 Edw590

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//
// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-08-28.

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "HighResPatches.h"
#include "fixes_other.h"

static uint32_t destHexPos = 0;

//_______________________________________________
void __declspec(naked) double_click_running(void) {

	//in PC_WALK(EAX actionPoints) function.
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     eax, ds:[edi+destHexPos]
			pop     edi
			jne     exitFunc
			mov     eax, dword ptr ss:[esp + 0x4]//actionPoints

			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_dude_run_]
			mov     [esp+4], edi
			pop     edi
			ret

			exitFunc:
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+destHexPos], eax
			pop     edi
			mov     eax, 0x2
			ret
	}
}


//_______________
void __cdecl OtherFixes(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_HIGHRES_PATCH, "OTHER_SETTINGS", "DOUBLE_CLICK_RUNNING", "1", prop_value, &high_res_patch_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		makeCallEXE(0x179E3, &double_click_running, false);
	}

	//Bypass hard drive space check - can cause a false error.
	//"Not enough free hard disk space.  Fallout requires at least %.1f megabytes of free hard disk space."
	writeMem16EXE(C_game_check_disk_space_, 0xC031, true);//xor eax, eax
	writeMem8EXE(C_game_check_disk_space_ + 2, 0xC3, true);//return
}
