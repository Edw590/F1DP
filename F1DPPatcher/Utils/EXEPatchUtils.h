// Copyright 2021 DADi590
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

#ifndef F1DPPATCHER_EXEPATCHUTILS_H
#define F1DPPATCHER_EXEPATCHUTILS_H



#include <stdint.h>
#include <stdbool.h>

void writeMem32EXE(uint32_t addr, uint32_t data);
void writeMem16EXE(uint32_t addr, uint16_t data);
void writeMem8EXE(uint32_t addr, uint8_t data);
uint32_t readMem32EXE(uint32_t addr);
uint16_t readMem16EXE(uint32_t addr);
uint8_t readMem8EXE(uint32_t addr);

__declspec(noreturn) void strcpy_sEXE(uint32_t addr, const char * s2, int n);

void HookCallEXE(uint32_t addr, const void * func);

void MakeCallEXE(uint32_t addr, const void * func, bool jump);

void BlockCallJmpEXE(uint32_t addr);



#endif //F1DPPATCHER_EXEPATCHUTILS_H
