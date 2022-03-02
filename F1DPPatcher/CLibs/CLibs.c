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

// Here are all the C standard functions that can be used on this program. More can be added, either with a manual
// implementation (as last resource in case the game doesn't include the function), or with adding an Assembly
// implementation to call the game's included functions.d

// All this is defined in IEEE P1003.1™ (POSIX 1003.1) - the same that Watcom uses (some functions may not be on the
// standard).
// Notice: all functions assume that when not ready addresses are given to them, that they meant to point to the data
// section. So any pointers are automatically corrected with getRealBlockAddrData().
// Note about the notice above: if the pointer is already corrected to point to the code section (because something in
// use is in the code section), then that pointer will NOT be re-corrected (because of getRealBlockAddrData()'s
// implementation of not correcting anything >= SN_CODE_SEC_BLOCK_ADDR).

// The main register used to call the functions is EDI because as a start, the 4 main ones are used to pass parameters
// to the functions (in Watcom's calling convention order, EAX, EDX, EBX, ECX from left to right), and then because ESI
// vs EDI, EDI I see it less used, so why not that one.

#include "conio.c"
#include "ctype.c"
#include "fcntl.c"
#include "io.c"
#include "stdio.c"
#include "stdlib.c"
#include "string.c"
#include "unistd.c"
