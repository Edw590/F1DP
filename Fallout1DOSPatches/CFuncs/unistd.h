//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_UNISTD_H
#define FALLOUT1DOSPATCHES_UNISTD_H



// ssize_t is in sys/types.h for Watcom
#include <sys/types.h>

int close(int fildes);
ssize_t read(int fildes, void * buf, size_t nbyte);



#endif //FALLOUT1DOSPATCHES_UNISTD_H
