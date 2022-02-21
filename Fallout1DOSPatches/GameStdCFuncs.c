//
// Created by DADi590 on 20/02/2022.
//

#include "Headers/GameStdCFuncs.h"


int open(const char *const pathname, const int flags) {
	int ret_var = 0;
	__asm {
		pusha

		push    0
		push    [flags]
		mov     eax, [pathname]
		push    eax
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_open_
		call    ecx
		add     esp, 0Ch
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

int filelength(const int fd) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, [fd]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_filelength_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

void * malloc(const size_t size) {
	void * ret_var = NULL;
	__asm {
		pusha

		mov     eax, [size]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F__nmalloc_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

int read(const int fd, const void *buf, const size_t count) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, [fd]
		mov     ebx, [count]
		mov     edx, [buf]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_read_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

int close(const int fd) {
	int ret_var = 0;
	__asm {
		pusha

		mov     eax, [fd]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_close_
		call    ecx
		mov     [ret_var], eax

		popa
	}

	return ret_var;
}

void exit(const int status) {
	__asm {
		// No need for pusha and popa --> exit() does not return
		mov     eax, [status]
		mov     ecx, SN_CODE_SEC_EXE_ADDR
		add     ecx, F_exit_
		call    ecx
	}
}
