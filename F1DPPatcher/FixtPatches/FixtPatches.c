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

#include "../CLibs/string.h"
#include "../Utils/EXEPatchUtils.h"
#include "FixtPatches.h"

void initFixtPatches(void) {
	// strnpy() is being used instead of strcpy() because we don't want to copy the NULL character, so the number of
	// characters to copy must be equal to the number of characters inside the quotation marks (which don't include the
	// NULL one).

	strncpy(getRealEXEAddr(0xF32FC), "not played because being", 24);

	strncpy(getRealEXEAddr(0xFC0CD), "NOT", 3);
	strncpy(getRealEXEAddr(0xFC0F5), "IS", 2);

	// No idea what this patch does
	writeMem32EXE(0x22050, 0xC003C003u);
	writeMem32EXE(0x22050+4, 0x04244429u);
	writeMem32EXE(0x22050+8, 0x90909090u);
}
