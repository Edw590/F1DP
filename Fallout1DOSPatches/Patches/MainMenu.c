//
// Created by DADi590 on 20/02/2022.
//

#include "../Headers/BlockAddrUtils.h"
#include "../Headers/EXEPatchUtils.h"

void mainMenuInit() {
	// Version string
	writeMem32(0xA10E3, 0x90909090); // Remove the 2 pushes
	writeMem32(0xA10E7 + 1, getRealBlockAddrData("DOSPatcher v0.2")); // Change the string address
	writeMem32(0x73373 + 1, 0x1BD); // 445
	writeMem8(0xA10F2 + 2, 0x8);
}
