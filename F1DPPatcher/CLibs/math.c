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
#include "../OtherHeaders/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "math.h"

double modf(double x, double *iptr) {
	double ret_var = 0;

	// Pointer correction
	iptr = getRealBlockAddrData(iptr);

	__asm {
			pusha

			push    [iptr]
			push    [x]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_modf_]
			call    edi
			mov     dword ptr [ret_var], eax

			popa
	}

	return ret_var;
}
