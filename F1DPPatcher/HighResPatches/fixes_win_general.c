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
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/IniUtils.h"
#include "HighResPatches.h"
#include "fixes_win_general.h"

union bytes8 {
	double d;
	uint64_t i;
};

//____________________
void WinGeneralFixes(void) {
	union bytes8 temp_double;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);
	memset(&temp_double, 0, 8);

	getPropValueIni(MAIN_INI_SPEC_SEC_HIGHRES_PATCH, "OTHER_SETTINGS", "FADE_TIME_MODIFIER", "60", prop_value, &high_res_patch_ini_info_G);
	sscanf(prop_value, "%lf", &temp_double);
	writeMem64EXE((uint32_t) getRealEXEAddr(0xFB3DC), temp_double.i, false);
}
