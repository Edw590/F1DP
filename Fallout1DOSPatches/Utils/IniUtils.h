//
// Created by DADi590 on 21/02/2022.
//

#ifndef FALLOUT1DOSPATCHES_INIUTILS_H
#define FALLOUT1DOSPATCHES_INIUTILS_H


#include <stdbool.h>

#define MAX_LINE_LEN 1000
#define MAX_PROP_NAME_LEN 900
#define MAX_PROP_VALUE_LEN 100

struct File {
	char * file_contents;
	unsigned long file_size;
};

bool readFile(const char *file_path, struct File *file);
bool checkPropIni(const char *ini_contents, unsigned long ini_len, const char *prop_name, char *prop_value);



#endif //FALLOUT1DOSPATCHES_INIUTILS_H
