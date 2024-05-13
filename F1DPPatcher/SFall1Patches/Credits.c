/*
 *    sfall
 *    Copyright (C) 2008, 2009, 2010, 2012  The sfall team, 2022 Edw590
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

// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "Credits.h"
#include "SFall1Patches.h"
#include "version.h"
#include <stddef.h>
#include <stdint.h>

static uint32_t InCredits = 0;
static uint32_t CreditsLine = 0;

static char const *const ExtraLines[49] = {
		// The below 2 lines weren't here, but I've added, because as Sduibek mentioned, why are mod's credits before
		// game name? So, as this appears before everything else, I've added the game name to it.
		"#FALLOUT",
		"-------------",
		"#sfall1 "VERSION_STRING,
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
		"Foxx",
		"Ethereal",
		"Anyone who has used sfall1 in their own mods",
		"The bug reporters and feature requesters",
		"",
		"",
		"#FALLOUT",
		"",
};

static uint32_t ExtraLineCount = sizeof(ExtraLines) / 4;

__declspec(naked) static void credits_hook(void) {
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
			ret
	}
}

static uint32_t __stdcall CreditsNextLine(char *buf, uint32_t *font, uint32_t *colour) {
	char const *line = NULL;

	// There was a != on the line just below, which would make the credits not appear in the Credits screen (???). So
	// I've corrected(?) it to ==. Now they appear normally.
	if ((0 == *(uint32_t *) getRealBlockAddrData(&InCredits)) ||
			(*(uint32_t *) getRealBlockAddrData(&CreditsLine) >= *(uint32_t *) getRealBlockAddrData(&ExtraLineCount))) {
		return 0;
	}

	line = ((char **) getRealBlockAddrData(&ExtraLines))[(*(uint32_t *) getRealBlockAddrData(&CreditsLine))++];

	if (0 != strlen(line)) {
		if ('#' == line[0]) {
			++line;
			*font = *(uint32_t *) getRealEXEAddr(D__name_font);
			*colour = *(uint8_t *) getRealEXEAddr(0x2A6041);
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

__declspec(naked) static void credits_get_next_line_hook(void) {
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
			ret

		fail:
			lea     esp, [esp-4] // [Edw590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_credits_get_next_line_]
			mov     [esp+4], edi
			pop     edi
			ret
	}
}

// Entire file updated to version 1.8

void CreditsInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Debugging", "NoCredits", "0", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if (0 == temp_int) {
		hookCallEXE(0x72B04, &credits_hook);
		hookCallEXE(0x38CF4, &credits_hook);
		hookCallEXE(0x2752A, &credits_get_next_line_hook);
	}
}
