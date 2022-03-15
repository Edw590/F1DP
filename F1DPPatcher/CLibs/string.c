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
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/BlockAddrUtils.h"
#include "string.h"
#include <stddef.h>

void *memset(void *s, int c, size_t n) {
	void *ret_var = NULL;

	// Pointer correction
	s = getRealBlockAddrData(s);

	__asm {
			pusha

			mov     eax, [s]
			mov     edx, [c]
			mov     ebx, [n]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_memset_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}

int strcmp(char const *s1, char const *s2) {
	int ret_var = 0;

	// Pointer correction
	s1 = getRealBlockAddrData(s1);
	s2 = getRealBlockAddrData(s2);

	__asm {
			pusha

			mov     eax, [s1]
			mov     edx, [s2]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_strcmp_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	return ret_var;
}

int strncmp(char const *s1, char const *s2, size_t n) {
	int ret_var = 0;

	// Pointer correction
	s1 = getRealBlockAddrData(s1);
	s2 = getRealBlockAddrData(s2);

	__asm {
			pusha

			mov     eax, [s1]
			mov     edx, [s2]
			mov     ebx, [n]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_strncmp_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	return ret_var;
}

char *strcpy(char *s1, char const *s2) {
	char *ret_var = NULL;

	// Pointer correction
	s1 = getRealBlockAddrData(s1);
	s2 = getRealBlockAddrData(s2);

	__asm {
	pusha

	mov     eax, [s1]
	mov     edx, [s2]
	mov     edi, SN_CODE_SEC_EXE_ADDR
	lea     edi, [edi+F_strcpy_]
	call    edi
	mov     [ret_var], eax

	popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}

char *strncpy(char *s1, char const *s2, size_t n) {
	char *ret_var = NULL;

	// Pointer correction
	s1 = getRealBlockAddrData(s1);
	s2 = getRealBlockAddrData(s2);

	__asm {
			pusha

			mov     eax, [s1]
			mov     edx, [s2]
			mov     ebx, [n]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_strncpy_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}

int strcpy_s(char *dest, size_t dest_size, char const *src) {
	int ret_var = 0;

	// Pointer correction
	dest = getRealBlockAddrData(dest);
	src = getRealBlockAddrData(src);

	ret_var = NULL != strncpy(dest, src, dest_size);
	dest[strnlen(src, dest_size)] = '\0'; // Don't forget strnlen() excludes the NULL character from the count...

	return ret_var;
}

size_t strlen(char const *s) {
	size_t len = 0;

	// Pointer correction
	s = getRealBlockAddrData(s);

	while ('\0' != s[len]) {
		++len;
	}

	return len;
}

size_t strnlen(char const *s, size_t maxlen) {
	size_t len = 0;

	// I don't find the function on the EXE. I only find __far_strlen, but I don't like the "far" part there. That's
	// related to going from one segment to others. None of the other functions I'm using have that and they work. So I
	// guess I'll just implement it myself.
	// EDIT: since it's my implementation, might as well go right away for strnlen() instead of strlen().

	// Pointer correction
	s = getRealBlockAddrData(s);

	for (len = 0; len < maxlen ; ++len) {
		if ('\0' == s[len]) {
			break;
		}
	}

	return len;
}

char *strchr(char const *s, int c) {
	size_t s_len = 0;
	size_t i = 0;

	// Pointer correction
	s = getRealBlockAddrData(s);

	s_len = strlen(s);
	for (i = 0; i < s_len; ++i) {
		if (s[i] == c) {
			return (char *) ((size_t) s + i);
		}
	}

	return NULL;
}

char *strrchr(char const *s, int c) {
	char *ret_var = NULL;

	// Pointer correction
	s = getRealBlockAddrData(s);

	__asm {
			pusha

			mov     eax, [s]
			mov     edx, [c]
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+F_strrchr_]
			call    edi
			mov     [ret_var], eax

			popa
	}

	ret_var = getRealBlockAddrData(ret_var);

	return ret_var;
}
