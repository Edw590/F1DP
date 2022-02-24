//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_STRING_H
#define FALLOUT1DOSPATCHES_STRING_H



void * memset(void *s, int c, size_t n);
char * strcpy(char * s1, const char * s2);
int strncmp(const char * s1, const char * s2, size_t n);
char * strncpy(char * s1, const char * s2, size_t n);
size_t strnlen(const char * s, size_t maxlen);
char * strrchr(const char * s, int c);



#endif //FALLOUT1DOSPATCHES_STRING_H
