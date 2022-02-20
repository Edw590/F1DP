//
// Created by DADi590 on 18/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_GAMEADDRS_H
#define FALLOUT1DOSPATCHES_GAMEADDRS_H



// ///////////////////////////////////////////////////
// Functions

// When there's no register mention, it means it's on the stack (don't forget the calling convention is backwards on
// that).

// printf_(format string, ...)
#define F_printf_ 0x0CA3B0
// exit_(EAX = exit code)
#define F_exit_ 0x0CA40A
// open_(EAX = file name) EAX = file descriptor
#define F_open_ 0x0D555A
// read_(EAX = file descriptor, EBX = length to read, EDX = buffer address) EAX = number of read bytes
#define F_read_ 0x0D545F
// close_(EAX = file descriptor) EAX = error code
#define F_close_ 0x0D57CC
// filelength_(EAX = file descriptor) EAX = file length
#define F_filelength_ 0x0BF9EC
// _nmalloc_(EAX = size) EAX = allocated block address
#define F__nmalloc_ 0x0CAC68

// ///////////////////////////////////////////////////
// Strings

// "FALLOUT %d.%d"
#define S_FALLOUT_D_D 0xFD5E0



#endif //FALLOUT1DOSPATCHES_GAMEADDRS_H
