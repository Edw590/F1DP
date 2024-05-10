/*
 *    sfall
 *    Copyright (C) 2009, 2010  Mash (Matt Wells, mashw at bigpond dot net dot au), 2022 DADi590
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/General.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "AnimationsAtOnceLimit.h"
#include "SFall1Patches.h"
#include <stdint.h>

static int AnimationsLimit = 21;

//pointers to new animation struct arrays
static uint8_t *anim_set = NULL;
static uint8_t *sad = NULL;

static uint32_t const AnimMove[5] = {
		0x1673B+2, 0x1688E+2, 0x16A6D+2, 0x16B82+6, 0x172E2+2,
};

static uint32_t const AnimMaxCheck[7] = {
		0x134AE+2, 0x1351B+2, 0x1363B+2, 0x13946+2, 0x139DC+2, 0x17FD7+2,
};

static uint32_t const AnimMaxSizeCheck[4] = {
		0x134E8+2, 0x13721+2, 0x13850+2, 0x17851+2,
};

static uint32_t const fake_anim_set_C[2] = {
		0x413534, 0x41389C,
};

static uint32_t const anim_set_0[46] = {
		0x413646, 0x413716, 0x4137B7, 0x4138C1, 0x413946, 0x4139D3, 0x413A8A,
		0x413BA3, 0x413D23, 0x413E30, 0x413FA3, 0x4140A3, 0x4141D5, 0x4142CA,
		0x4143B0, 0x414492, 0x414576, 0x41465A, 0x4146DC, 0x41475C, 0x4147DC,
		0x414864, 0x4148F4, 0x414986, 0x414A1C, 0x414AAA, 0x414B2A, 0x414BAA,
		0x414C28, 0x414D02, 0x414DE6, 0x414E66, 0x414EEA, 0x414F9A, 0x4150A8,
		0x41518F, 0x4151A4, 0x4151BD, 0x4151FE, 0x41520D, 0x4155F5, 0x41563C,
		0x4156AF, 0x41578E, 0x4157B3, 0x4158DF,
};

static uint32_t const anim_set_4[9] = {
		0x4137CE, 0x4151CF, 0x415652, 0x41565F, 0x415734, 0x415848, 0x415866,
		0x415873, 0x4158F4,
};

static uint32_t const anim_set_8[14] = {
		0x413726, 0x41375F, 0x4137BD, 0x413956, 0x413984, 0x4139E3, 0x413A00,
		0x413A1A, 0x4151C3, 0x415659, 0x41571F, 0x41575F, 0x41577F, 0x4158CE,
};

static uint32_t const anim_set_C[25] = {
		0x4135C6, 0x4135CF, 0x4135DF, 0x4135F9, 0x413614, 0x413652, 0x41365B,
		0x413670, 0x41367D, 0x4136F5, 0x413743, 0x4137C8, 0x4137D7, 0x413808,
		0x413829, 0x41388D, 0x413973, 0x41508D, 0x4155BC, 0x4158AA, 0x4158EE,
		0x415921, 0x41592D, 0x41788D, 0x417898,
};

static uint32_t const anim_set_10[6] = {
		0x41373A, 0x41396A, 0x4139F7, 0x41572B, 0x415826, 0x415854,
};

static uint32_t const anim_set_14[6] = {
		0x413732, 0x413962, 0x4139EF, 0x415745, 0x41581E, 0x41585D,
};

static uint32_t const anim_set_28[3] = {
		0x4137E3, 0x4151DC, 0x4151EF,
};

static uint32_t const sad_0[23] = {
		0x4167C6, 0x4167D2, 0x416839, 0x416914, 0x416928, 0x416A41, 0x416AE3,
		0x416B72, 0x416C21, 0x416EFF, 0x4171BE, 0x417216, 0x41729F, 0x417362,
		0x4173EE, 0x41751F, 0x417639, 0x4176BC, 0x41777C, 0x4177A4, 0x417919,
		0x41791F, 0x417934,
};

static uint32_t const sad_4[10] = {
		0x41588D, 0x4167BC, 0x41690B, 0x416D07, 0x4170EF, 0x41714F, 0x4172EB,
		0x417429, 0x4174D8, 0x417575,
};

static uint32_t const sad_8[10] = {
		0x416834, 0x416847, 0x416922, 0x41697B, 0x4169AB, 0x417161, 0x41742F,
		0x417443, 0x4175CB, 0x41760B,
};

static uint32_t const sad_C[2] = {
		0x416874, 0x416D6A,
};

static uint32_t const sad_10[5] = {
		0x41683F, 0x4169B1, 0x41743D, 0x4174E3, 0x417502,
};

static uint32_t const sad_14[4] = {
		0x416854, 0x4169E2, 0x417470, 0x4174F0,
};

static uint32_t const sad_18[10] = {
		0x4156F2, 0x416868, 0x4169E8, 0x417114, 0x41730C, 0x417437, 0x4174FC,
		0x417598, 0x4176DB, 0x4177E0,
};

static uint32_t const sad_1C[15] = {
		0x4162C6, 0x4162CE, 0x41630D, 0x416355, 0x41639B, 0x4163A3, 0x416884,
		0x4168B2, 0x416A78, 0x416F19, 0x417057, 0x41720F, 0x417297, 0x417482,
		0x417516,
};

static uint32_t const sad_20[35] = {
		0x4156FA, 0x415895, 0x4162DF, 0x4167F6, 0x41688E, 0x41699F, 0x416A82,
		0x416D01, 0x416DB0, 0x416E36, 0x416F23, 0x416F89, 0x417050, 0x41705D,
		0x417087, 0x417103, 0x417149, 0x41716E, 0x417209, 0x41721C, 0x417273,
		0x417291, 0x4172C9, 0x4172FB, 0x417476, 0x4174C8, 0x417560, 0x41757D,
		0x417589, 0x4175B3, 0x4176E1, 0x417799, 0x4178C0, 0x4178E9, 0x41792E,
};

static uint32_t const sad_24[5] = {
		0x416332, 0x41636D, 0x416378, 0x41685D, 0x416F0B,
};

static uint32_t const sad_28[4] = {
		0x416D4A, 0x416E3D, 0x416F6D, 0x417099,
};

__declspec(naked) static void anim_set_end_hook(void) {
	__asm {
			push    esi
			mov     esi, SN_DATA_SEC_EXE_ADDR
			lea     edi, [esi+D__anim_set]
			pop     esi
			push    edi
			mov     edi, SN_DATA_SEC_BLOCK_ADDR
			cmp     dword ptr [edi+AnimationsLimit], 21
			pop     edi
			jbe     skip
			push    esi
			mov     esi, SN_DATA_SEC_BLOCK_ADDR
			mov     edi, [esi+anim_set]
			pop     esi
			add     edi, 1936                            // We take into account the dummy
		skip:
			xor     edx, edx
			dec     edx
			mov     [edi+0x4][esi], edx                  // counter
			mov     dl, [edi+0xC][esi]                   // flags
			test    dl, 0x2                              // Battle flag set?
			jz      end                                  // No
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_combat_anim_finished_]
			call    edi
			pop     edi
		end:
			mov     dword ptr [edi][esi], -1000          // curr_anim

			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+0x158B2]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}

/*__declspec(naked) static void op_anim_hook(void) {
	__asm {
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			test    ds:[edi+D__combat_state], al         // In battle?
			pop     edi
			jz      skip                                 // No
			inc     eax                                  // RB_RESERVED
		skip:
			lea     esp, [esp-4] // [DADi590] Reserve space for the jump address
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_register_begin_]
			mov     [esp+4], edi
			pop     edi
			retn
	}
}*/

// todo Recheck all writes to memory here. They seem weird. A casts to pointers from a position of an array (?).
//  After that, replace with write??MemEXE calls.

void AnimationsAtOnceInit(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_SFALL1, "Misc", "AnimationsAtOnceLimit", "21", prop_value, &sfall1_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*(uint32_t *) getRealBlockAddrData(&AnimationsLimit) = (uint32_t) temp_int > 127 ? 127 : (uint32_t) temp_int;
	if (temp_int > 21) {
		int i = 0;
		int AnimationsLimit_local = 0;
		uint32_t (*AnimMove_local)[5] = NULL;
		uint32_t (*AnimMaxCheck_local)[7] = NULL;
		uint32_t (*AnimMaxSizeCheck_local)[4] = NULL;
		uint32_t (*fake_anim_set_C_local)[2] = NULL;
		uint32_t (*anim_set_0_local)[46] = NULL;
		uint32_t (*anim_set_4_local)[9] = NULL;
		uint32_t (*anim_set_8_local)[14] = NULL;
		uint32_t (*anim_set_C_local)[25] = NULL;
		uint32_t (*anim_set_10_local)[6] = NULL;
		uint32_t (*anim_set_14_local)[6] = NULL;
		uint32_t (*anim_set_28_local)[3] = NULL;
		uint32_t (*sad_0_local)[23] = NULL;
		uint32_t (*sad_4_local)[10] = NULL;
		uint32_t (*sad_8_local)[10] = NULL;
		uint32_t (*sad_C_local)[2] = NULL;
		uint32_t (*sad_10_local)[5] = NULL;
		uint32_t (*sad_14_local)[4] = NULL;
		uint32_t (*sad_18_local)[10] = NULL;
		uint32_t (*sad_1C_local)[15] = NULL;
		uint32_t (*sad_20_local)[35] = NULL;
		uint32_t (*sad_24_local)[5] = NULL;
		uint32_t (*sad_28_local)[4] = NULL;

		AnimationsLimit_local = *(int *) getRealBlockAddrData(&AnimationsLimit);
		AnimMove_local = getRealBlockAddrData(&AnimMove);
		AnimMaxCheck_local = getRealBlockAddrData(&AnimMaxCheck);
		AnimMaxSizeCheck_local = getRealBlockAddrData(&AnimMaxSizeCheck);
		fake_anim_set_C_local = getRealBlockAddrData(&fake_anim_set_C);
		anim_set_0_local = getRealBlockAddrData(&anim_set_0);
		anim_set_4_local = getRealBlockAddrData(&anim_set_4);
		anim_set_8_local = getRealBlockAddrData(&anim_set_8);
		anim_set_C_local = getRealBlockAddrData(&anim_set_C);
		anim_set_10_local = getRealBlockAddrData(&anim_set_10);
		anim_set_14_local = getRealBlockAddrData(&anim_set_14);
		anim_set_28_local = getRealBlockAddrData(&anim_set_28);
		sad_0_local = getRealBlockAddrData(&sad_0);
		sad_4_local = getRealBlockAddrData(&sad_4);
		sad_8_local = getRealBlockAddrData(&sad_8);
		sad_C_local = getRealBlockAddrData(&sad_C);
		sad_10_local = getRealBlockAddrData(&sad_10);
		sad_14_local = getRealBlockAddrData(&sad_14);
		sad_18_local = getRealBlockAddrData(&sad_18);
		sad_1C_local = getRealBlockAddrData(&sad_1C);
		sad_20_local = getRealBlockAddrData(&sad_20);
		sad_24_local = getRealBlockAddrData(&sad_24);
		sad_28_local = getRealBlockAddrData(&sad_28);

		//allocate memory to store larger animation struct arrays + pacifier
		*(uint8_t **) getRealBlockAddrData(&anim_set) = malloc(1936 * ((size_t) AnimationsLimit_local + 1)); // + pacifier
		*(uint8_t **) getRealBlockAddrData(&sad) = malloc(3240 * ((size_t) AnimationsLimit_local + 1));

		//reserved animation limit check (old 13)
		writeMem8EXE(0x13665 + 2, (uint8_t) AnimationsLimit_local - 8, true);

		//movement animation limit checks (old 16)
		temp_int = sizeof(*AnimMove_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint8_t *) (*AnimMove_local[i]) = (uint8_t) AnimationsLimit_local - 5;
		}

		//Max animation limit checks (old 21) AnimationsLimit
		temp_int = sizeof(*AnimMaxCheck_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint8_t *) (*AnimMaxCheck_local[i]) = (uint8_t) AnimationsLimit_local;
		}

		//Max animations checks - animation struct size * max num of animations (old 1936*21=40656)
		temp_int = sizeof(*AnimMaxSizeCheck_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*AnimMaxSizeCheck_local[i]) = 1936 * (uint32_t) AnimationsLimit_local;
		}

		//divert old animation structure list pointers to newly alocated memory

		//struct array 1///////////////////

		//old addr 0x55FB84
		writeMem32EXE(0x134DC + 2, (uint32_t) anim_set, true);

		//old addr 0x55FB90
		temp_int = sizeof(*fake_anim_set_C_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*fake_anim_set_C_local[i]) = 12 + (uint32_t) anim_set;
		}

		//old addr 0x560314
		temp_int = sizeof(*anim_set_0_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*anim_set_0_local[i]) = 1936 + (uint32_t) anim_set;
		}

		//old addr 0x560318
		temp_int = sizeof(*anim_set_4_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*anim_set_4_local[i]) = 1936 + 4 + (uint32_t) anim_set;
		}

		//old addr 0x56031C
		temp_int = sizeof(*anim_set_8_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*anim_set_8_local[i]) = 1936 + 8 + (uint32_t) anim_set;
		}

		//old addr 0x560320
		temp_int = sizeof(*anim_set_C_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*anim_set_C_local[i]) = 1936 + 12 + (uint32_t) anim_set;
		}

		//old addr 0x560324
		temp_int = sizeof(*anim_set_10_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*anim_set_10_local[i]) = 1936 + 16 + (uint32_t) anim_set;
		}

		//old addr 0x560328
		temp_int = sizeof(*anim_set_14_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*anim_set_14_local[i]) = 1936 + 20 + (uint32_t) anim_set;
		}

		//old addr 0x560338
		writeMem32EXE(0x139B7 + 2, 1936 + 36 + (uint32_t) anim_set, true);

		//old addr 0x56033C
		temp_int = sizeof(*anim_set_28_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*anim_set_28_local[i]) = 1936 + 40 + (uint32_t) anim_set;
		}

		//old addr 0x560348
		writeMem32EXE(0x156EA + 2, 1936 + 52 + (uint32_t) anim_set, true);

		//struct array 2///////////////////

		//old addr 0x540014
		temp_int = sizeof(*sad_0_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_0_local[i]) = (uint32_t) sad;
		}

		//old addr 0x540018
		temp_int = sizeof(*sad_4_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_4_local[i]) = 4 + (uint32_t) sad;
		}

		//old addr 0x54001C
		temp_int = sizeof(*sad_8_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_8_local[i]) = 8 + (uint32_t) sad;
		}

		//old addr 0x540020
		temp_int = sizeof(*sad_C_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_C_local[i]) = 12 + (uint32_t) sad;
		}

		//old addr 0x540024
		temp_int = sizeof(*sad_10_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_10_local[i]) = 16 + (uint32_t) sad;
		}

		//old addr 0x540028
		temp_int = sizeof(*sad_14_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_14_local[i]) = 20 + (uint32_t) sad;
		}

		//old addr 0x54002C
		temp_int = sizeof(*sad_18_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_18_local[i]) = 24 + (uint32_t) sad;
		}

		//old addr 0x540030
		temp_int = sizeof(*sad_1C_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_1C_local[i]) = 28 + (uint32_t) sad;
		}

		//old addr 0x540034
		temp_int = sizeof(*sad_20_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_20_local[i]) = 32 + (uint32_t) sad;
		}

		//old addr 0x540038
		temp_int = sizeof(*sad_24_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_24_local[i]) = 36 + (uint32_t) sad;
		}

		//old addr 0x54003A
		writeMem32EXE(0x16309 + 3, 38 + (uint32_t) sad, true);

		//old addr 0x54003B
		writeMem32EXE(0x162CF + 2, 39 + (uint32_t) sad, true);

		//old addr 0x54003C
		temp_int = sizeof(*sad_28_local) / 4;
		for (i = 0; i < temp_int; ++i) {
			*(uint32_t *) (*sad_28_local[i]) = 40 + (uint32_t) sad;
		}
	}

	makeCallEXE(0x1588D, &anim_set_end_hook, true);
	//hookCallEXE(0x4FE65, &op_anim_hook); - [DADi590] Already commented out
}

void AnimationsAtOnceExit(void) {
	free(anim_set);
	*(uint8_t **) getRealBlockAddrData(&anim_set) = NULL;

	free(sad);
	*(uint8_t **) getRealBlockAddrData(&sad) = NULL;
}
