


// This could use some optimizations, but it will only run once every game start. So no big deal being a bit slower,
// but more readable xD. For example the while loop could have the condition put outside, but that's one more variable
// on the beginning of realMain() and that's a mess (I also don't want to create scopes just for that - no need).

#define _CRT_INSECURE_DEPRECATE

#include "Headers/GameStdCFuncs.h"
#include "Headers/AddressUtils.h"
#include "Headers/Logger.h"
#include "Headers/EXEPatchUtils.h"
#include "Headers/General.h"
#include "Headers/PatchPatcher.h"
#include "Headers/MainMenu.h"
#include "Headers/BlockAddrUtils.h"

int realMain(void);

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
	(void) logger_str; // To disable the unused code warning
}

int realMain(void) {
	// Right before ANYthing else (except declarations with initialization), patch the patch itself (aside from needing
	// the ESI register intact).
	patchPatcher(SN_BASE);
	{
		uint32_t str_addr = getRealBlockAddrData("Fallout 1 DOS Patcher initialized successfully!");
		__asm {
			push    ecx
			push    dword ptr [str_addr]
		}
		printf(1);
	}

	mainMenuInit();

	// According to Open Watcom's calling convention, the return value will be put in EAX. The loader checks if it's 0
	// or not (not specific values). So just use EXIT_SUCCESS and EXIT_FAILURE.
	return EXIT_SUCCESS;
}

// This is just because I don't want to be adding and removing targets on the compiler flags...
// Also, if main() is in the beginning, I can just search for the special name and know right there where the segment
// starts (3 bytes before the NOPs that come before the special number and the jump that comes before it).
#include "ExeNewFuncs.c"
#include "GameStdCFuncs.c"
#include "EXEPatchUtils.c"
#include "BlockAddrUtils.c"
#include "PatchPatcher.c"
