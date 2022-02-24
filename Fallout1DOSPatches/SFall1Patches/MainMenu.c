//
// Created by DADi590 on 20/02/2022.
//

#include "../CFuncs/stdio.h"
#include "../Utils/EXEPatchUtils.h"
#include "MainMenu.h"

void mainMenuInit() {
	// Version string
	writeMem32EXE(0xA10E3, 0x90909090); // Remove the 2 pushes
	writeMem32EXE(0xA10E7 + 1, (uint32_t) getRealBlockAddrData("DOSPatcher v0.2")); // Change the string address
	writeMem32EXE(0x73373 + 1, 0x1BD); // 445
	writeMem8EXE(0xA10F2 + 2, 0x8);
}
