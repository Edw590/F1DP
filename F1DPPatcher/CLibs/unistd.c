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

#include <sys/types.h>
#include "../Utils/BlockAddrUtils.h"
#include "../GameAddrs/CStdFuncs.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "unistd.h"

int close(int fildes) {
	int ret_var = 0;
	__asm {
			pusha

			mov     eax, [fildes]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_close_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	return ret_var;
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	int ret_var = 0;

	// Pointer correction
	buf = getRealBlockAddrData(buf);

	__asm {
			pusha

			mov     eax, [fildes]
			mov     edx, [buf]
			mov     ebx, [nbyte]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_read_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	return ret_var;
}
