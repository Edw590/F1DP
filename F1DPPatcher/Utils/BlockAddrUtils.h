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

#ifndef F1DPPATCHER_BLOCKADDRUTILS_H
#define F1DPPATCHER_BLOCKADDRUTILS_H


#include "General.h"

#define SN_CODE_SEC_BLOCK_ADDR 0x76563412 // 12 34 56 75 in little endian
#define SN_DATA_SEC_BLOCK_ADDR 0x75563412 // 12 34 56 74 in little endian
#define SN_BLOCK_ADDR 0x74563412 // 12 34 56 74 in little endian

#define GET_BD_SYM(type, symbol) (*(type *) getRealBlockAddrData(&symbol))
// For when the symbol is an array THAT IS DEREFERENCED. Example: GET_BD_ARR(uint32_t, array)[index]
#define GET_BD_ARR(type, array) ((type) getRealBlockAddrData(&array))
#define GET_BC_SYM(symbol) ((uint32_t) getRealBlockAddrCode(&symbol))

funcptr_t (getRealBlockAddrCode(volatile const funcptr_t(func_ptr)));
void *getRealBlockAddrData(volatile void const *data_ptr);


#endif //F1DPPATCHER_BLOCKADDRUTILS_H
