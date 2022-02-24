//
// Created by DADi590 on 20/02/2022.
//

#include "../CFuncs/stdio.h"
#include "BlockAddrUtils.h"

// If the address is greater than SN_CODE_SEC_EXE_ADDR, then that means it's already a real address somewhere. So don't
// touch it (even if we'd add some offset to it to get it to some section we wanted, it would overflow the EXE's space
// address). If it's smaller than that, correct it.
// This is useful in case by mistake, a malloc'ed string is given and then it's hours to find out the bug (which is we
// already have the correct address but we give it to printlnStr() which calls these functions that add an address to
// the already correct address without checking that it might have already been corrected --> this ends with that).

// About the 'const' keyword on the functions... Technically it's constant, because the pointers mean to point to
// something. They do not because they're using wrong values, and this just corrects them to point to what they actually
// want to point to in the first place. And I don't have 'const' in the return type because of casting problems in the
// rest of the project files, so I just cast it internally.

// Also, there isn't only one function like getRealEXEAddr() because "mov offset dword_????" is something like 0x17 (so
// a small number), and from the code segment, the same thing. I can't differentiate. So must be the programmer saying
// to which one the address is supposed to belong - unlike the EXE addresses, which when seen on IDA, they have decent
// values and it's just compare if it's inside the code section of the data section and add the corresponding offset.

// And hopefully, no valid address will return 0, since I'm checking if they're 0 or not, and if they are (meaning
// NULL), then the result is the unmodified function argument. From what I see on HxD, the Data section of this program
// always begins with 01 01 01 01 (or something like that, but starts with a padding even if the section starts at the
// beginning of a 16-byte line, meaning paragraph-aligned), so at minimum, real data starts address 0x4. As long as it
// stays that way, cool and this will always work. Useful for example for free() with a NULL pointer.

void * getRealBlockAddrCode(const void * addr) {
	if (((uint32_t) addr >= SN_CODE_SEC_EXE_ADDR) || (NULL == addr)) {
		return (void *) addr;
	}

	return (void *) ((uint32_t) addr + SN_CODE_SEC_BLOCK_ADDR);
}

void * getRealBlockAddrData(const void * addr) {
	if (((uint32_t) addr >= SN_CODE_SEC_EXE_ADDR) || (NULL == addr)) {
		return (void *) addr;
	}

	return (void *) ((uint32_t) addr + SN_DATA_SEC_BLOCK_ADDR);
}
