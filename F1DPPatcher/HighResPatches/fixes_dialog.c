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
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/IniUtils.h"
#include "F_Objects.h"
#include "HighResPatches.h"
#include "fixes_dialog.h"
#include <stddef.h>

//__________________________________________________________________
void InvInsertItem(struct OBJStruct* item, int32_t numItems, struct PUD_GENERAL* inv) {
	int i = 0;
	int32_t numNodes = inv->inv_max;
	struct ITEMnode* itemNode = inv->item;
	if (NULL != itemNode && numNodes > 1) {
		for (i = (numNodes - 1); i > 0; --i) {
			itemNode[i].obj = itemNode[i-1].obj;
			itemNode[i].num = itemNode[i-1].num;
		}
	}
	itemNode[0].obj = item;
	itemNode[0].num = (uint32_t) numItems;
}


//______________________________________
__declspec(naked) void inv_insert_item(void) {

	__asm {
			mov     edx, dword ptr ss:[esp+0x20]//num items to insert
			pushad
			push    ecx//pObjPud_To
			push    edx
			push    edi//*pObjItem
			mov     eax, edi
			mov     ebx, ecx
			call    InvInsertItem
			add     esp, 0xC
			popad
			ret
	}
}


void DialogInventoryFixes(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_HIGHRES_PATCH, "OTHER_SETTINGS", "INV_ADD_ITEMS_AT_TOP", "0", prop_value, &high_res_patch_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		makeCallEXE(0x6A1EB, &inv_insert_item, false);
		//0006A1F0     /EB 12         JMP SHORT 0006A204                       ; invItemList->invItem[invItemList->inv_size].obj = obj_item
		writeMem16EXE(0x6A1F0, 0x12EB, true);
		writeMem8EXE(0x6A1F0 + 2, 0x90, true);
	}

	getPropValueIni(MAIN_INI_SPEC_SEC_HIGHRES_PATCH, "OTHER_SETTINGS", "BARTER_PC_INV_DROP_FIX", "1", prop_value, &high_res_patch_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 != temp_int) {
		writeMem32EXE(0x682CE+1, 164, true);
		writeMem32EXE(0x682DA + 1, 100, true);
	}
}
