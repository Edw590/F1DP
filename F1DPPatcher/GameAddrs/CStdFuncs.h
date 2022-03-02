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

#ifndef F1DPPATCHER_CSTDFUNCS_H
#define F1DPPATCHER_CSTDFUNCS_H



// All (or most of) these functions and file names belong to POSIX 1003.1 (filelength doesn't), the same used by Watcom,
// the compiler used for the game.
// Keep them ordered alphabetically (as well as on the CLibs folder).

// When there's no register mention on the parameters, it means it's on the stack (don't forget the calling convention
// is backwards on that).
// The return value is either a register name, or [stack] or (void). To know what's returned, go to the official docs.

// /////////////////
// conio.h

// getch_(void) EAX
#define F_getch_ 0x0DE2DE

// /////////////////
// ctype.h

// getch_(void) EAX
#define F_tolower_ 0x0CA3D2
#define F_toupper_ 0x0CA367

// /////////////////
// fcntl.h

// open_(path, oflag, ...) EAX
#define F_open_ 0x0D555A

// /////////////////
// io.h

// filelength_(EAX = handle) EAX
#define F_filelength_ 0x0BF9EC

// /////////////////
// stdio.h

// printf_(format, ...) EAX
#define F_printf_ 0x0CA3B0

// /////////////////
// stdlib.h

// exit_(EAX = status) (void)
#define F_exit_ 0x0CA40A
// _nfree_(EAX = ptr) (void)
#define F__nfree_ 0x0CADE9
// _nmalloc_(EAX = size) EAX
#define F__nmalloc_ 0x0CAC68

// /////////////////
// string.h

// memset_(EAX = s, EDX = c, EBX = n) EAX
#define F_memset_ 0x0B5730
// strcmp_(EAX = s1, EDX = s2) EAX
#define F_strcmp_ 0x0C6040
// strcpy_(EAX = s1, EDX = s2) EAX
#define F_strcpy_ 0x0E2B66
// strncmp_(EAX = s1, EDX = s2, EBX = c) EAX
#define F_strncmp_ 0x0CA3E0
// strncpy_(EAX = s1, EDX = s2, EBX = n) EAX
#define F_strncpy_ 0x0BDBCA
// strrchr_(EAX = s, EDX = c) EAX
#define F_strrchr_ 0x0BDBEF

// /////////////////
// unistd.h

// close_(EAX = fildes) EAX
#define F_close_ 0x0D57CC
// read_(EAX = fildes, EDX = buf, EBX = nbyte) EAX
#define F_read_ 0x0D545F



#endif //F1DPPATCHER_CSTDFUNCS_H
