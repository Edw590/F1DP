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

#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/General.h"
#include "../Utils/GlobalVars.h"
#include "SFall1Main.h"
#include "SFall1Patches.h"

struct FileInfo sfall1_ini_info_G = {0};

bool initSfall1Patcher(void) {
	bool ret_var = true;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	if (getPropValueIni(MAIN_INI_SPEC_SEC_MAIN, NULL, "sFall1SettingsFile", NULL, prop_value, &dospatch_ini_info_G)) {
		if (0 != strcmp(prop_value, F1DP_MAIN_INI)) { // Don't reopen the main INI file, use the one already open.
			if (readFile(prop_value, &sfall1_ini_info_G)) {
				logf(LOGGER_STR "File \"%s\" opened for sFall1 settings."NL, prop_value);
				((struct FileInfo *) getRealBlockAddrData(&sfall1_ini_info_G))->is_main_ini = false;
			} else {
				logf(LOGGER_STR "File \"%s\" not found for sFall1 settings. Aborting sFall1 patches."NL, prop_value);
				memset(&sfall1_ini_info_G, 0, sizeof(sfall1_ini_info_G));

				ret_var = false;
				goto funcEnd;
			}
		} else {
			*(struct FileInfo *) getRealBlockAddrData(&sfall1_ini_info_G) =
					*(struct FileInfo *) getRealBlockAddrData(&dospatch_ini_info_G);
		}
	} else {
		loglnStr(LOGGER_ERR_STR "No file specified for sFall1 settings. Aborting sFall1 patches.");

		ret_var = false;
		goto funcEnd;
	}

	DllMain2();


	funcEnd:

	// If the file was opened here, release its contents before leaving the function (unless it's the main one, which is
	// taken care of by realMain()).
	if (!((struct FileInfo *) getRealBlockAddrData(&sfall1_ini_info_G))->is_main_ini) {
		freeNew(((struct FileInfo *) getRealBlockAddrData(&sfall1_ini_info_G))->contents);
	}

	return ret_var;
}
