// Copyright 2021 DADi590
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

#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../OtherHeaders/General.h"
#include "../OtherHeaders/GlobalVars.h"
#include "Inventory.h"
#include "SFall1Patches.h"

void sFall1Patches(struct FileInfo sfall1_ini_info);

bool initSfall1Patcher(struct FileInfo dospatch_ini_info) {
	bool ret_var = true;
	struct FileInfo sfall1_ini_info;
	char ini_prop_value[MAX_PROP_VALUE_LEN];
	bool main_ini_for_settings = true;

	memset(&sfall1_ini_info, 0, sizeof(sfall1_ini_info));
	memset(ini_prop_value, 0, MAX_PROP_VALUE_LEN);

	if (getPropValueIni(dospatch_ini_info, F1DP_INI_SPEC_SEC_MAIN, NULL, "sFall1SettingsFile", NULL, ini_prop_value)) {
		if (0 != strcmp(ini_prop_value, F1DP_MAIN_INI)) { // Don't reopen the main INI file, use the one already open.
			if (readFile(ini_prop_value, &sfall1_ini_info)) {
				logf(LOGGER_STR "File \"%s\" opened for sFall1 settings."NL, ini_prop_value);
				main_ini_for_settings = false;
			} else {
				logf(LOGGER_STR "File \"%s\" not found for sFall1 settings. Aborting sFall1 patches."NL, ini_prop_value);
				memset(&sfall1_ini_info, 0, sizeof(sfall1_ini_info));

				ret_var = false;
				goto funcEnd;
			}
		} else {
			sfall1_ini_info = dospatch_ini_info;
		}
	} else {
		loglnStr(LOGGER_ERR_STR "No file specified for sFall1 settings. Aborting sFall1 patches.");

		ret_var = false;
		goto funcEnd;
	}


	sFall1Patches(sfall1_ini_info);


	funcEnd:

	// If the file was opened here, release its contents before leaving the function.
	if (!main_ini_for_settings) {
		freeNew(sfall1_ini_info.contents);
	}

	return ret_var;
}

// ADVICE: don't try to understand the functions in each patch... Infinite EDI register uses there to be able to have
// the special numbers in use. If you want to understand what was done, just go to the sFall1 source and see there. If
// you think anything is wrong with the ones here, redo from scratch, don't try to fix. It's a mess with this way of
// Special Numbers, but I don't have better ideas without making a relocation table for the EXE, parsing it and applying
// it (as long as this way works, I think it's much better than going that route which would take infinity).

void sFall1Patches(struct FileInfo sfall1_ini_info) {
	//char prop_value[MAX_PROP_VALUE_LEN];

	InventoryInit(sfall1_ini_info);
}

#include "Inventory.c"
