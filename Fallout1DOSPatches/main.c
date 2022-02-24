


// This could use some optimizations, but it will only run once every game start. So no big deal being a bit slower,
// but more readable xD. For example the while loop could have the condition put outside, but that's one more variable
// on the beginning of realMain() and that's a mess (I also don't want to create scopes just for that - no need).

#include "CFuncs/stdio.h"
#include "OtherHeaders/PatcherPatcher.h"
//#include "SFall1Patches/SFall1Patches.h"
#include <stdbool.h>
#include "SFall1Patches/MainMenu.h"

#define SN_MAIN_FUNCTION 0x78563412 // 12 34 56 78 in little endian

bool realMain(void);

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
		jmp     main1 // Jump over the data in case this program's EXE is executed normally
		dd      SN_MAIN_FUNCTION
		main1: // Where code execution begins for the loader
		// Right before ANYthing else (before the ESI register is used somewhere), patch the patch itself. And also
		// before any operations that need the special numbers already replaced.
		mov     eax, SN_BASE
		call    patchPatcher
		jmp     realMain
	}

	// No return value here on main() because this function never returns - it's realMain() that returns, so the return
	// value is there.
}

bool realMain(void) {

	printf("Fallout 1 DOS Patcher initialized successfully!"NL);

	// Start the sFall1 patcher
	//initSfall1Patcher();
	mainMenuInit();
	//ret_var = ret_var && initSfall1Patcher(); --> Optimizations... Get it back when ret_var = true in the beginning.


	// According to Open Watcom's calling convention, the return value will be put in EAX. The loader checks if it's 0
	// or not (not specific values). So just use EXIT_SUCCESS and EXIT_FAILURE. The loader will always load the game
	// though. It will just show that there were errors or not (and this patcher must be the one to print them).
	return true;
}

// This is just because I don't want to be adding and removing targets on the compiler flags...
// Also, if main() is in the beginning, I can just search for the special name and know right there where the segment
// starts (3 bytes before the NOPs that come before the special number and the jump that comes before it).
#include "CFuncs/CFuncs.c"
#include "PatcherPatcher.c"
//#include "SFall1Patches/SFall1Patches.c"
#include "SFall1Patches/MainMenu.c"
#include "Utils/BlockAddrUtils.c"
#include "Utils/EXEPatchUtils.c"
#include "Utils/IniUtils.c"
