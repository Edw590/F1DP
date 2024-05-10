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

#include "../GameAddrs/CStdFuncs.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "ctype.h"

int tolower(int c) {
	int ret_var = 0;

	__asm {
			pusha

			mov     eax, [c]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_tolower_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	return ret_var;
}

int toupper(int c) {
	int ret_var = 0;

	__asm {
			pusha

			mov     eax, [c]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_toupper_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	return ret_var;
}
