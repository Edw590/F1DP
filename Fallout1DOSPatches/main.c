


// This could use some optimizations, but it will only run once every game start. So no big deal being a bit slower,
// but more readable xD. For example the while loop could have the condition put outside, but that's one more variable
// on the beginning of realMain() and that's a mess (I also don't want to create scopes just for that - no need).

#define _CRT_INSECURE_DEPRECATE

#include "Headers/GameStdCFuncs.h"

// Special 32-bit numbers ("SN") that are not supposed to appear ANYwhere else in the entire code copied to the BIN
// file. They're weird enough for me to think they won't appear. (Too much coincidence?)
#define SN_MAIN_FUNCTION 0x78563412 // 12 34 56 78 in little endian
#define SN_CODE_SEG_EXE_ADDR 0x79563412 // 12 34 56 79 in little endian
#define SN_DATA_SEG_EXE_ADDR 0x77563412 // 12 34 56 77 in little endian
#define SN_BLOCK_ADDR 0x76563412 // 12 34 56 76 in little endian

#define DOS_MAX_FILE_NAME_LEN 12

void realMain(void);
void patchPatcher(uint32_t block_address);
__declspec(naked) void testFunc(void);

// The sole purpose of this string below is for me to know where main() is so I know I need to start copying from 3
// bytes above the NOPs that are generated AHAHAH.
// This because the string is placed on the code segment and right above main(), so it's a good way to know where to
// start without looking for the special number.
const char copy_here[] = "Start copying 3 bytes above before the NOPs above this!";

const char patches_file_name[DOS_MAX_FILE_NAME_LEN+1] = "dospatch.bin";

__declspec(naked) int main(void) {
	// The special number declaration below is for the loader to look for it and start the execution right there.
	// Useful since I copy the entire code segment, and there are things above main(). The number of things before
	// main() may or may not change, depending on what is declared before it (like in headers or strings).
	// This way that's protected for sure. Just look for a weird number and start the execution 4 bytes after.

	// Also, implementing this function is just to be able to run the program as a normal EXE in case it's needed for
	// any reason. Though, this is unnecessary if it's for the patch to be tested only when ran inside the EXE. In that
	// case, the special number could be put in the beginning of realMain().
	// EDIT: actually no, because if it's for realMain() not to be naked, then the first thing must be declarations of
	// local variables. So this is still needed. Though, the 1st jump is still unnecessary and is only there to run this
	// as a normal program and nothing else.

	__asm {
		jmp     main1 // Jump over the data in case the EXE is executed normally
		dd      SN_MAIN_FUNCTION
		main1:
		jmp     realMain // Where code execution begins for the loader
	}
}

void realMain(void) {
	// It's important that the addresses stay as non-pointers. This way we can do arithmetic with them byte by byte and
	// not sizeof by sizeof.
	uint32_t block_address = 0;
	// NO INITIALIZATIONS/ASSIGNMENTS ABOVE THE __asm DECLARATION!!!!! THE ESI REGISTER MUST BE INTACT INSIDE IT!!!!
	__asm {
		mov     [block_address], esi // Now the block address is decently stored before any code messes with ESI
	}
	patchPatcher(block_address); // Right before anything else, patch the patch itself.


	/*{
		printfBefore(block_address)
		__asm mov eax, offset test_str;
		printf((char *) getRealAddrDataEax(block_address));
		printfAfter(block_address)
	}*/

	/*{
		char * fmt_str = NULL;
		int * arg1 = 0;
		printfBefore(block_address)
		getRealAddrData(block_address, &format_d, fmt_str);
		__asm push eax;
		getRealAddrData(block_address, &teste, arg1);
		__asm push eax;
		printf();
		printfAfter(block_address, 2)
	}

	{
		int num = 0;
		getRealAddrData(block_address, &teste, &num);
		++*((int *) num);
	}

	{
		char * fmt_str = NULL;
		int * arg1 = 0;
		printfBefore(block_address)
		getRealAddrData(block_address, &format_d, fmt_str);
		getRealAddrData(block_address, &teste, arg1);
		printf();
		printfAfter(block_address, 2)
	}*/

	/*__asm {
	pusha

	mov     eax, BLOCK_DATA_SEG_OFFSET
	add     eax, offset teste
	add     eax, block_address
	push    eax
	}
	printf(code_seg_addr, 1);

	++teste;

	__asm {
	pusha

	mov     eax, BLOCK_DATA_SEG_OFFSET
	add     eax, offset teste
	add     eax, block_address
	push    eax
	}
	printf(code_seg_addr, 1);*/


	//exit(code_seg_addr, 0);

	testFunc();
}

// THIS MUST BE ABOVE ANY PATCH FUNCTIONS!!!!!!!!!!
// Because of the special numbers check (don't patch the FIRST occurrence, which must be in this function).
void patchPatcher(uint32_t block_address) {
	int i = 0;
	int patches_file_len = 0;
	int file_descriptor = 0;
	bool first_sn_code_seg_addr_passed = false;
	bool first_sn_data_seg_addr_passed = false;
	bool first_sn_block_addr_passed = false;
	uint32_t code_seg_addr = getSegAddr(block_address, CODE_SEG_ADDR_OFFSET);
	uint32_t data_seg_addr = getSegAddr(block_address, DATA_SEG_ADDR_OFFSET);

	file_descriptor = open(code_seg_addr, (char const *) getRealAddrCode(block_address, &patches_file_name), 0x200);
	patches_file_len = filelength(code_seg_addr, file_descriptor);
	close(code_seg_addr, file_descriptor);

	// Below, -4 because we'll be reading 4 by 4. So the last 4 bytes are the last ones to be ready. Not the last 3 or
	// 2 or 1, which would cause bad things to happen (reading wrong places in memory - because we read 4 by 4).
	i = 0;
	while (i <= (patches_file_len - 4)) {
		uint32_t * curr_addr = (uint32_t *) (block_address + i);
		uint32_t curr_value = *curr_addr;

		// Below, I check if the first occurrence of the special numbers has passed already - which is the one below,
		// needed to remain there to check for the rest of the occurrences (can't patch the checker xD).
		if (SN_CODE_SEG_EXE_ADDR == curr_value) {
			if (first_sn_code_seg_addr_passed) {
				*curr_addr = code_seg_addr;
			} else {
				first_sn_code_seg_addr_passed = true;
			}
		} else if (SN_DATA_SEG_EXE_ADDR == curr_value) {
			if (first_sn_data_seg_addr_passed) {
				*curr_addr = data_seg_addr;
			} else {
				first_sn_data_seg_addr_passed = true;
			}
		} else if (SN_BLOCK_ADDR == curr_value) {
			if (first_sn_block_addr_passed) {
				*curr_addr = block_address;
			} else {
				first_sn_block_addr_passed = true;
			}
		}

		++i;
	}
}

// This MUST be after patchPatcher(), and MUST include C files and not headers. The functions' CODE must be here, after
// the mentioned function. So here below MUST be all the code, but I'm including it because I don't want to have here
// infinite lines of code when I can just include the C files.
#include "ExeNewFuncs.c"
