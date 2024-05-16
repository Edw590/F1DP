// Copyright 2022 Edw590
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

#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../Utils/EXEPatchUtils.h"
#include "FixtPatches.h"
#include "../Utils/IniUtils.h"
#include "../Utils/GlobalVars.h"

void initFixtPatches(void) {
	char prop_value[MAX_PROP_VALUE_LEN];
	struct FileInfo translation_ini_info_G;
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);
	memset(&translation_ini_info_G, 0, sizeof(struct FileInfo));

	getPropValueIni(MAIN_INI_SPEC_SEC_FIXT, "Main", "TranslationsINI", "TRANSLAT.INI", prop_value, &f1dpatch_ini_info_G);
	// If it fails, the struct will have 0s and the file won't be read, so the default values will be used as Fixt does.
	readFile(prop_value, &translation_ini_info_G);

	char string1[100];
	char string2[50];
	char string3[50];

	getPropValueIni(NULL, "Fixt", "String1", "not played because being", string1, &translation_ini_info_G);
	getPropValueIni(NULL, "Fixt", "String2", "NOT", string2, &translation_ini_info_G);
	getPropValueIni(NULL, "Fixt", "String3", "IS", string3, &translation_ini_info_G);

	// strnpy() is being used instead of strcpy() because we don't want to copy the NULL character, so the number of
	// characters to copy must be equal to the number of characters inside the quotation marks (which don't include the
	// NULL one).
	strncpy(getRealEXEAddr(0xF32FC), string1, strlen(string1));

	strncpy(getRealEXEAddr(0xFC0CD), string2, strlen(string2));
	strncpy(getRealEXEAddr(0xFC0F5), string3, strlen(string3));

	// No idea what this patch does
	writeMem32EXE(0x22050, 0xC003C003u, true);
	writeMem32EXE(0x22050 + 4, 0x04244429u, true);
	writeMem32EXE(0x22050 + 8, 0x90909090u, true);
}
