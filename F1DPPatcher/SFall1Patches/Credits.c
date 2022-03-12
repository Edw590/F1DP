/*
 *    sfall
 *    Copyright (C) 2008, 2009, 2010, 2012  The sfall team, 2022 DADi590
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "Credits.h"
#include "FalloutEngine.h"
#include "SFall1Patches.h"
#include "version.h"
#include <stddef.h>
#include <stdint.h>

static uint32_t InCredits = 0;
static uint32_t CreditsLine = 0;

static char const * const ExtraLines[47] = {
		"#FALLOUT",
		"----------",
		"#SFALL1 "VERSION_STRING,
		"",
		"sfall1 is free software, licensed under the GPL",
		"Copyright 2008-2016  The sfall team",
		"",
		"@Author",
		"Timeslip",
		"",
		"@Contributors",
		"ravachol",
		"Noid",
		"Glovz",
		"Dream",
		"Ray",
		"Kanhef",
		"KLIMaka",
		"Mash",
		"Helios",
		"Haenlomal",
		"NVShacker",
		"NovaRain",
		"JimTheDinosaur",
		"phobos2077",
		"Tehnokrat",
		"Mynah",
		"Crafty",
		"",
		"@Additional thanks to",
		"Nirran",
		"killap",
		"MIB88",
		"Rain man",
		"Continuum",
		"Fakeman",
		"The Master",
		"Drobovik",
		"Lexx",
		"Sduibek",
		"burn",
		"Anyone who has used sfall in their own mods",
		"The bug reporters and feature requesters",
		""
		"",
		"",
		"#FALLOUT",
		""
};

static uint32_t ExtraLineCount = sizeof(ExtraLines) / 4;

static void __declspec(naked) credits_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			mov     [edi+CreditsLine], 0
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			inc     [edi+InCredits]
			pop     edi
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_credits_]
			call    edi
			pop     edi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			dec     [edi+InCredits]
			pop     edi
			retn
	}
}

static uint32_t __stdcall CreditsNextLine(char *buf, uint32_t *font, uint32_t *colour) {
	char const *line = NULL;

	if ((0 != *(uint32_t *) getRealBlockAddrData(&InCredits)) ||
			(*(uint32_t *) getRealBlockAddrData(&CreditsLine) >= *(uint32_t *) getRealBlockAddrData(&ExtraLineCount))) {
		return 0;
	}

	line = ((char **) getRealBlockAddrData(&ExtraLines))[(*(uint32_t *) getRealBlockAddrData(&CreditsLine))++];

	if (0 != strlen(line)) {
		if ('#' == line[0]) {
			++line;
			*font = *(uint32_t *) getRealEXEAddr(D__name_font);
			*colour = *(uint8_t *) 0x6A8151;
		} else if ('@' == line[0]) {
			++line;
			*font = *(uint32_t *) getRealEXEAddr(D__title_font);
			*colour = *(uint32_t *) getRealEXEAddr(D__title_color);
		} else {
			*font = *(uint32_t *) getRealEXEAddr(D__name_font);
			*colour = *(uint32_t *) getRealEXEAddr(D__name_color);
		}
	}

	strcpy_s(buf, 256, line);

	return 1;
}

static void __declspec(naked) credits_get_next_line_hook(void) {
	__asm {
			pushad
			push    ebx
			push    edx
			push    eax
			call    CreditsNextLine
			test    eax, eax
			popad
			jz      fail
			xor     eax, eax
			inc     eax
			retn

		fail:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_credits_get_next_line_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

void CreditsInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Debugging", "NoCredits", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 == temp_int) {
		// Not sure what these 2 below do, but I've commented them so the sFall1 credits appear just before Fallout's
		// ones.
		//HookCallEXE(0x72B04, getRealBlockAddrCode((void *) &credits_hook));
		//HookCallEXE(0x38CF4, getRealBlockAddrCode((void *) &credits_hook));
		(void) credits_hook; // To ignore the unused function warnings
		HookCallEXE(0x2752A, getRealBlockAddrCode((void *) &credits_get_next_line_hook));
	}
}
