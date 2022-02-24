//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_FCNTL_H
#define FALLOUT1DOSPATCHES_FCNTL_H



#define O_RDONLY 0x0000
#define O_TEXT 0x0100
#define O_EXCL 0x0400

#define S_IRUSR 000400

int open(const char * path, int oflag, int mode);



#endif //FALLOUT1DOSPATCHES_FCNTL_H
