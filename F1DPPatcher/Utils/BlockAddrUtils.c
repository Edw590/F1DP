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

#include "GlobalEXEAddrs.h"
#include "BlockAddrUtils.h"
#include <stddef.h>
#include <stdint.h>

// If the address is greater than SN_CODE_SEC_EXE_ADDR, then that means it's already a real address somewhere. So don't
// touch it (even if we'd add some offset to it to get it to some section we wanted, it would overflow the EXE's space
// address). If it's smaller than that, correct it.
// This is useful in case by mistake, a malloc'ed string is given and then it's hours to find out the bug (which is we
// already have the correct address but we give it to printlnStr() which calls these functions that add an address to
// the already correct address without checking that it might have already been corrected --> this ends with that).

// About the 'const' keyword on the functions... Technically it's constant, because the pointers mean to point to
// something. They do not because they're using wrong values, and this just corrects them to point to what they actually
// want to point to in the first place. And I don't have 'const' in the return type because of casting problems in the
// rest of the project files, so I just cast it internally.

// Also, there isn't only one function like getRealEXEAddr() because "mov offset dword_????" is something like 0x17 (so
// a small number), and from the code segment, the same thing. I can't differentiate. So must be the programmer saying
// to which one the address is supposed to belong - unlike the EXE addresses, which when seen on IDA, they have decent
// values and it's just compare if it's inside the code section of the data section and add the corresponding offset.

// And hopefully, no valid address will return 0 (wtf), since I'm checking if they're 0 or not, and if they are (meaning
// NULL), then the result is the unmodified function argument. From what I see on HxD, the Data section of this program
// always begins with 01 01 01 01 (or something like that, but starts with a padding even if the section starts at the
// beginning of a 16-byte line, meaning paragraph-aligned), so at minimum, real data starts address 0x4. As long as it
// stays that way, cool and this will always work. Useful for example for free() with a NULL pointer - not supposed to
// add anything to a NULL pointer if it's supposed to be NULL (0).

// The volatile keywords around the functions that use the Special Numbers, like the 2 below, is just for me to be 100%
// sure Watcom will not optimize the functions and inline all values, adding or subtracting the Special Numbers to
// others if it is an obvious things that "should" be optimized --> except there can be no optimizations here, or the
// patcherPatcher() won't replace them because the Special Number is gone.

//void (*getRealBlockAddrCode(volatile const void(*func_ptr)(void)))(void) { - the wtf way without typedef...
funcptr_t (getRealBlockAddrCode(volatile const funcptr_t (func_ptr))) {
	// Inside the EXE segments address space already, or it's a NULL pointer? Then return the original. Else, correct it.
	// A more in-depth explanation is in the text above.
	if (((uint32_t) func_ptr >= SN_CODE_SEC_EXE_ADDR) || (NULL == func_ptr)) {
		return (funcptr_t()) func_ptr;
	}

	return (funcptr_t()) ((uint32_t) func_ptr + SN_CODE_SEC_BLOCK_ADDR);
}

void *getRealBlockAddrData(volatile void const *data_ptr) {
	if (((uint32_t) data_ptr >= SN_CODE_SEC_EXE_ADDR) || (NULL == data_ptr)) {
		return (void *) data_ptr;
	}

	return (void *) ((uint32_t) data_ptr + SN_DATA_SEC_BLOCK_ADDR);
}
