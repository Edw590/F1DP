//
// Created by DADi590 on 22/02/2022.
//

#include "../CFuncs/stdio.h"
#include "../CFuncs/stdlib.h"
#include "../CFuncs/string.h"
#include "../Utils/IniUtils.h"
#include "SFall1Patches.h"
#include "MainMenu.h"

#define MAIN_INI "dospatch.ini"
#define DDRAW_INI "ddraw.ini"

bool initSfall1Patcher(void) {
	// Don't forget we can't initialize variables that are not initialized with just "= [number]". If it's with "= {0}"
	// or something like that, it will copy from empty memory to the destination --> memory without address correction,
	// because it's automatic.
	// So ONLY initialize if it can be initialized with "= [number]", OR ask memset to help if it can't.
	bool ret_var = true;
	struct FileContents ini_file;
	char value[MAX_PROP_VALUE_LEN];
	//bool sfall1_settings_ready = false; // Vê dentro do dospatch.ini a chave useAsSfall1Settings e vê do ddraw.ini se
	// for 1
	memset(&ini_file, 0, sizeof(ini_file));
	memset(value, 0, MAX_PROP_VALUE_LEN);

	if (!readFile(MAIN_INI, &ini_file)) {
		// todo What happens if the main INI is not found?
		printf("DOSPATCH.INI NOT FOUND!!!"NL);

		return ret_var;
	}

	printf("Checking value now..."NL);

	if (getPropValueIni(ini_file.contents, ini_file.size, "teste", value)) {
		printf("Value found: %s"NL, value);
	} else {
		printf("Value not found"NL);
	}


	//Só vai haver o dospatch.ini, mais nada. Mete lá um parâmetro chamado useAsSfall1Settings por exemplo. Porque falta
	//outro para caso se metam aqui patches do Hi-Res e outros.


	mainMenuInit(ini_file);

	printf("%s"NL, ini_file.contents);

	freeNew(ini_file.contents);

	return ret_var;
}

#include "MainMenu.c"
