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
#include "GlobalEXEAddrs.h"
#include "EXEPatchUtils.h"

// And right after, the Data section begins
#define CODE_SEC_EXE_IDA_END_ADDR 0xEAFFF

// All these functions suppose `addr` is an address value seen on IDA as the value if the EXE would load in its supposed
// address, 0x10000 (for the code section), and something else for the data section. This is important because they
// automatically they care of seeing if the address is supposed to be from the Data section or the Code section and it
// is corrected internally. So do NOT correct it externally, or it WILL be wrong inside the functions.

void writeMem32EXE(uint32_t addr, uint32_t data) {
	*(uint32_t *) getRealEXEAddr(addr) = data;
}
void writeMem16EXE(uint32_t addr, uint16_t data) {
	*(uint16_t *) getRealEXEAddr(addr) = data;
}
void writeMem8EXE(uint32_t addr, uint8_t data) {
	*(uint8_t *) getRealEXEAddr(addr) = data;
}

uint32_t readMem32EXE(uint32_t addr) {
	return *(uint32_t *) getRealEXEAddr(addr);
}
uint16_t readMem16EXE(uint32_t addr) {
	return *(uint16_t *) getRealEXEAddr(addr);
}
uint8_t readMem8EXE(uint32_t addr) {
	return *(uint8_t *) getRealEXEAddr(addr);
}


void HookCallEXE(uint32_t addr, void const *func) {
	writeMem32EXE(addr + 1, (uint32_t) func - ((uint32_t) getRealEXEAddr(addr) + 5));
}

void MakeCallEXE(uint32_t addr, void const *func, bool jump) {
	writeMem8EXE(addr, jump ? 0xE9 : 0xE8);
	HookCallEXE(addr, func);
}

void BlockCallEXE(uint32_t addr) {
	// 5 is the length of the CALL near instruction with opcode E8 or the JMP near instruction with opcode E9.
	memset(getRealEXEAddr(addr), 0x90, 5);
}


void *getRealEXEAddr(volatile uint32_t addr) {
	if (addr <= CODE_SEC_EXE_IDA_END_ADDR) {
		return (void *) (addr + SN_CODE_SEC_EXE_ADDR);
	} else {
		return (void *) (addr + SN_DATA_SEC_EXE_ADDR);
	}
}
