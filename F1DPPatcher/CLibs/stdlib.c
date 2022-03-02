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

#include "../GameAddrs/CStdFuncs.h"
#include "../OtherHeaders/General.h"
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "stdlib.h"

__declspec(noreturn) void exit(int status) {
	__asm {
	// No need for pusha and popa --> exit() does not return. But I'll put them anyway.
		pusha

		mov     eax, dword ptr [status]
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F_exit_
		call    edi

		popa
	}
}

void free(void *ptr) {
	// ptr is a reserved word for MASM...

	// Pointer correction
	void *pointer = getRealBlockAddrData(ptr);

	__asm {
		pusha

		mov     eax, dword ptr [pointer]
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F__nfree_
		call    edi

		popa
	}
}

void *malloc(size_t size) {
	void *ret_var = NULL;
	__asm {
		pusha

		mov     eax, dword ptr [size]
		mov     edi, SN_CODE_SEC_EXE_ADDR
		add     edi, F__nmalloc_
		call    edi
		mov     dword ptr [ret_var], eax

		popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}
