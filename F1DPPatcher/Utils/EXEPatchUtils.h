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

#ifndef F1DPPATCHER_EXEPATCHUTILS_H
#define F1DPPATCHER_EXEPATCHUTILS_H



#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Calculates the real address of a game address got from IDA.
 *
 * It is done by checking if the given address is \>= than CODE_SEC_EXE_IDA_END_ADDR. If it is, sum the data section
 * address to it. Else, sum the code section address. This carries a warning...
 *
 * WARNING: PAY ATTENTION TO HOW MANY CALLS YOU DO TO THIS FUNCTION WITH THE SAME VARIABLE!!! You can't rely on it to
 * know if the sum was already done like with the getRealBlockAddr functions!
 *
 * @param addr the address seen on IDA
 *
 * @return the real address to use
 */
void *getRealEXEAddr(volatile uint32_t addr);

void writeMem32EXE(uint32_t addr, uint32_t data);
void writeMem16EXE(uint32_t addr, uint16_t data);
void writeMem8EXE(uint32_t addr, uint8_t data);

void hookCallEXE(uint32_t addr, void const *func);

void makeCallEXE(uint32_t addr, void const *func, bool jump);

void blockCallEXE(uint32_t addr);



#endif //F1DPPATCHER_EXEPATCHUTILS_H
