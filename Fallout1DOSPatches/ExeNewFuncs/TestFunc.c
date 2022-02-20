//
// Created by DADi590 on 20/02/2022.
//

__declspec(naked) void testFunc(void) {
	__asm {
		mov     eax, SN_DATA_SEG_EXE_ADDR
		add     eax, S_FALLOUT_D_D
		push    eax
		mov     ecx, SN_CODE_SEG_EXE_ADDR
		add     ecx, F_printf_
		call    ecx
		add     esp, 4

		mov     eax, 123872h
		mov     ecx, SN_CODE_SEG_EXE_ADDR
		add     ecx, F_exit_
		call    ecx

		ret
	}
}
