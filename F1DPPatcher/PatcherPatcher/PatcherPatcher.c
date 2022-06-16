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

#include "PatcherPatcher.h"

#define CODE_SEC_EXE_ADDR_OFFSET 4
#define DATA_SEC_EXE_ADDR_OFFSET 8
#define BIN_FILE_LEN_OFFSET 12

// These are intrinsically related to their SN_ constant relatives. If ANY of those is changed, these must be checked.
// The O in SNO means offset (relative to SN_BASE).
#define SNO_CODE_SEC_EXE_ADDR 0x09000000
#define SNO_DATA_SEC_EXE_ADDR 0x07000000
#define SNO_CODE_SEC_BLOCK_ADDR 0x06000000
#define SNO_DATA_SEC_BLOCK_ADDR 0x05000000
#define SNO_BLOCK_ADDR 0x04000000

// The code segment begins in the 16th byte, not on the 0th
#define CODE_SEC_BLOCK_ADDR_OFFSET 0x10
// The data segment begins some pages (each of 0x1000 bytes) after the code segment. This number can change and must be
// updated manually each time the code needs one more page in the EXE.
#define DATA_SEC_BLOCK_ADDR_OFFSET (0x10000 + CODE_SEC_BLOCK_ADDR_OFFSET)

// Explanation of this file and all the SN_ constants thing around the project files
//
// The function below patches the patcher so that, first I don't need to pass the addresses as parameters to every
// function on this program, and second and MUCH more important (the actual main reason): when the EXE calls this file's
// strings and functions, they already have the correct addresses on them and there is no need to pass addresses as
// parameters from the EXE to the new functions (or else I'd need a wrapper for every function I'd want to patch on the
// EXE that would go see where this block is allocated with a stored address on the code section of the EXE, do a little
// math to get this file's sections, and send them to the new functions inside this file --> wtf; so the new way is MUCH
// better).
// How does the program know how to patch itself? Wherever an address known only at run-time is needed, I place a
// special constant (SN_-started constant) and the program at run-time, looks for those constants and replaces them all
// in memory. So the patcher iterates its own memory and patches itself. Therefore the values must be unique, else it
// would patch something else. So I've chosen (in little endian - important): 12345678h and small variants of those.
// This also means there can be no opimizations ANYwhere where these values are. Volatile helps on that on PatchUtils,
// for example.
// How does the program not patch itself when comparing values? Because I'm putting the patcherPatcher() function with a
// constant parameter (a base value, SN_BASE) which it then sums to macros to get the final value to compare, every time
// it wants to compare values. It's not a decent idea, but it works, because Open Watcom doesn't optimize it, so I guess
// it's good enough.
// EDIT: thinking a bit more, I think the idea is the same as relocations ("dynamic relocations"? xD). I just don't have
// a table nor I want to make one xD - my way is kind of simpler, I just have to hope (😂) that the Special Numbers are
// never used anywhere else than where I put them.

// This function must NOT use ANY other function from this project to achieve its goal. It must be all internal to this
// file. That way, for sure it's not using something that needs to be replaced (which can only happen after this
// function ends its job successfully and without external help).

// The sn_base parameter MUST be SN_BASE. Constant parameter ALWAYS. This is to disable sum optimizations. I need the
// function to not have any of the SN_ constants on it, else it will replace them (not good to replace the replacer...).
void patcherPatcher(uint32_t sn_base) {
	uint32_t offset = 0;
	uint32_t block_addr = 0;
	uint32_t code_sec_exe_addr = 0;
	uint32_t data_sec_exe_addr = 0;
	uint32_t bin_file_len = 0;
	uint32_t code_sec_block_addr = 0;
	uint32_t data_sec_block_addr = 0;
	int temp1 = 0;
	int temp2 = 0;
	int temp3 = 0;
	int temp4 = 0;
	int temp5 = 0;
	// NO ASSIGNMENTS ABOVE THE __asm DECLARATION!!!!! THE EDI REGISTER MUST BE INTACT INSIDE IT!!!!
	__asm {
			mov     [block_addr], edi // Now the block address is decently stored before any code messes with EDI
	}
	code_sec_exe_addr = *(uint32_t *) (block_addr + CODE_SEC_EXE_ADDR_OFFSET);
	data_sec_exe_addr = *(uint32_t *) (block_addr + DATA_SEC_EXE_ADDR_OFFSET);
	bin_file_len = *(uint32_t *) (block_addr + BIN_FILE_LEN_OFFSET);
	code_sec_block_addr = block_addr + CODE_SEC_BLOCK_ADDR_OFFSET;
	data_sec_block_addr = block_addr + DATA_SEC_BLOCK_ADDR_OFFSET;

	offset = 0;
	// Below, -4 because we'll be reading 4 by 4. So the last 4 bytes are the last ones to be ready. Not the last 3 or
	// 2 or 1, which would cause bad things to happen (reading wrong places in memory - because we read 4 by 4).
	while (offset <= (bin_file_len - 4)) {
		uint32_t curr_addr = block_addr + offset;
		uint32_t curr_value = *(uint32_t *) curr_addr;

		if (curr_value == (sn_base + SNO_CODE_SEC_EXE_ADDR)) {
			*(uint32_t *) curr_addr = code_sec_exe_addr;
			++temp1;
		} else if (curr_value == (sn_base + SNO_DATA_SEC_EXE_ADDR)) {
			*(uint32_t *) curr_addr = data_sec_exe_addr;
			++temp2;
		} else if (curr_value == (sn_base + SNO_CODE_SEC_BLOCK_ADDR)) {
			*(uint32_t *) curr_addr = code_sec_block_addr;
			++temp3;
		} else if (curr_value == (sn_base + SNO_DATA_SEC_BLOCK_ADDR)) {
			*(uint32_t *) curr_addr = data_sec_block_addr;
			++temp4;
		} else if (curr_value == (sn_base + SNO_BLOCK_ADDR)) {
			*(uint32_t *) curr_addr = block_addr;
			++temp5;
		} else {
			// Do nothing if none of the special numbers are found
		}

		++offset;
	}
}
