//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_STDLIB_H
#define FALLOUT1DOSPATCHES_STDLIB_H


#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

__declspec(noreturn) void exit(int status);
void free(void * ptr);
void * malloc(size_t size);



#endif //FALLOUT1DOSPATCHES_STDLIB_H
