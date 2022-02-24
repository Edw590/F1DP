//
// Created by DADi590 on 21/02/2022.
//

// Here are all the C standard functions that can be used on this program. More can be added, either with a manual
// implementation (as last resource in case the game doesn't include the function), or with adding an Assembly
// implementation to call the game's included functions.d

// All this is defined in IEEE P1003.1™ (POSIX 1003.1) - the same that Watcom uses (some functions may not be on the
// standard).
// Notice: all functions assume that when not ready addresses are given to them, that they meant to point to the data
// section. So any pointers are automatically corrected with getRealBlockAddrData().
// Note about the notice above: if the pointer is already corrected to point to the code section (because something in
// use is in the code section), then that pointer will NOT be re-corrected (because of getRealBlockAddrData()'s
// implementation of not correcting anything >= SN_CODE_SEC_BLOCK_ADDR).

#include "fcntl.c"
#include "io.c"
#include "stdio.c"
#include "stdlib.c"
#include "string.c"
#include "unistd.c"
