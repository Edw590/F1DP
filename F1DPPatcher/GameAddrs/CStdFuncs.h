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

// getch(void) EAX
#define F_getch_ 0xDE2DE

// /////////////////
// ctype.h

// tolower(EAX = c) EAX
#define F_tolower_ 0xCA3D2
// toupper(EAX = c) EAX
#define F_toupper_ 0xCA367

// /////////////////
// fcntl.h

// open(path, oflag, ...) EAX
#define F_open_ 0xD555A

// /////////////////
// io.h

// filelength(EAX = handle) EAX
#define F_filelength_ 0xBF9EC

// /////////////////
// math.h

// __stdcall modf(x, iptr) EAX
#define F_modf_ 0xE9CFE

// /////////////////
// stdio.h

// printf(format, ...) EAX
#define F_printf_ 0xCA3B0
// sprintf(s, format, ...) EAX
#define F_sprintf_ 0xBCEF2
// sscanf(s, format, ...) EAX
#define F_sscanf_ 0xCA48C


// /////////////////
// stdlib.h

// exit(EAX = status) (void)
#define F_exit_ 0xCA40A
// itoa(EAX = value, EDX = str, EBX = base) (void)
#define F_itoa_ 0xC6DCA
// _nfree(EAX = ptr) (void)
#define F__nfree_ 0xCADE9
// _nmalloc(EAX = size) EAX
#define F__nmalloc_ 0xCAC68

// /////////////////
// string.h

// memset(EAX = s1, EDX = s2, EBX = n) EAX
#define F_memcpy_ 0xE3F4E
// memset(EAX = s, EDX = c, EBX = n) EAX
#define F_memset_ 0xB5730
// strcmp(EAX = s1, EDX = s2) EAX
#define F_strcmp_ 0xC6040
// strcpy(EAX = s1, EDX = s2) EAX
#define F_strcpy_ 0xE2B66
// strncmp(EAX = s1, EDX = s2, EBX = c) EAX
#define F_strncmp_ 0xCA3E0
// strncpy(EAX = s1, EDX = s2, EBX = n) EAX
#define F_strncpy_ 0xBDBCA
// strrchr(EAX = s, EDX = c) EAX
#define F_strrchr_ 0xBDBEF
// strupr(EAX = string) EAX
#define F_strupr_ 0xCA38B

// /////////////////
// unistd.h

// close(EAX = fildes) EAX
#define F_close_ 0xD57CC
// read(EAX = fildes, EDX = buf, EBX = nbyte) EAX
#define F_read_ 0xD545F



#endif //F1DPPATCHER_CSTDFUNCS_H
