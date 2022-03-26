/*
 *    sfall
 *    Copyright (C) 2016  The sfall team
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
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/General.h"
#include "../Utils/IniUtils.h"
#include "Quests.h"
#include "SFall1Patches.h"

// Entire file updated to version 1.8

void QuestsInit(void) {
	char buf[MAX_DOS_PATH_LEN - 3];
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "QuestsFile", "", buf, &sfall1_ini_info_G);
	if (strlen(buf) > 0) {
		int location = 0;
		struct FileInfo iniQuests_info;
		char iniQuests[MAX_DOS_PATH_LEN];
		char section[4];
		char thread[8];
		short *questsTable = (short *) getRealEXEAddr(D__sthreads);
		memset(&iniQuests_info, 0, sizeof(iniQuests_info));
		memset(&iniQuests, 0, MAX_DOS_PATH_LEN);
		memset(&section, 0, 4);
		memset(&thread, 0, 8);

		sprintf(iniQuests, ".\\%s", buf);
		readFile(iniQuests, &iniQuests_info);

		for (location = 0; location < 12; ++location) {
			int quest = 0;
			itoa((location * 10) + 701, section, 10);
			for (quest = 0; quest < 9; ++quest) {
				short gvar_index = 0;
				sprintf(thread, "Quest%d", (location * 10) + 701 + quest);
				getPropValueIni(NULL, section, thread, "-1", prop_value, &iniQuests_info);
				sscanf(prop_value, "%hd", &gvar_index);
				if (gvar_index != -1) {
					questsTable[(location * 18) + (quest * 2)] = gvar_index;
				}
			}
		}

		free(iniQuests_info.contents);
	}
}
