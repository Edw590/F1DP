/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2022 Edw590

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//
// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-08-28.

#ifndef F1DPPATCHER_F_OBJECTS_H
#define F1DPPATCHER_F_OBJECTS_H


#include <stdint.h>

struct OBJStruct;

struct ITEMnode {
	struct OBJStruct *obj;
	uint32_t num;
};

struct COMBAT_DATA {
	struct OBJStruct *who_hit_me;
	int32_t currentAP;
	int32_t results;
	int32_t damage_last_turn;
	int32_t aiPacket;
	int32_t teamNum;
	uint32_t unknown01;
};

struct PUD_CRITTER {
	int32_t inv_size;              //0x00
	int32_t inv_max;               //0x04
	struct ITEMnode* item;              //0x08
	int32_t reaction_to_pc;        //0x0C
	struct COMBAT_DATA combat_data;     //0x10
	int32_t current_hp;            //0x2C
	int32_t current_rad;           //0x30
	int32_t current_poison;        //0x34
};

struct PUD_WEAPON {
	int32_t cur_ammo_quantity;
	int32_t cur_ammo_type_pid;
};
struct PUD_AMMO {
	int32_t cur_ammo_quantity;
	int32_t none;
};
struct PUD_MISC_ITEM {
	int32_t curr_charges;
	int32_t none;
};
struct PUD_KEY_ITEM {
	int32_t cur_key_code;
	int32_t none;
};
struct PUD_PORTAL {
	int32_t cur_open_flags;
	int32_t none;
};
struct PUD_ELEVATOR {
	int32_t elev_type;
	int32_t elev_level;
};
struct PUD_STAIRS {
	int32_t destMap;
	int32_t destBuiltTile;
};


union PUDS {
	struct PUD_WEAPON weapon;
	struct PUD_AMMO ammo;
	struct PUD_MISC_ITEM misc_item;
	struct PUD_KEY_ITEM key_item;
	struct PUD_PORTAL portal;
	struct PUD_ELEVATOR elevator;
	struct PUD_STAIRS stairs;
};

struct PUD_GENERAL {
	int32_t inv_size;              //0x00
	int32_t inv_max;               //0x04
	struct ITEMnode* item;              //0x08
	int32_t updated_flags;         //0x0C
	union PUDS pud;                    //0x10
	uint32_t none01;               //0x18
	uint32_t none02;               //0x1C
	uint32_t none03;               //0x20
	uint32_t none04;               //0x24
	uint32_t none05;               //0x28
	uint32_t none06;               //0x2C
	uint32_t none07;               //0x30
	uint32_t none08;               //0x34
};

union PUD {
	struct PUD_CRITTER critter;
	struct PUD_GENERAL general;
};

struct OBJStruct {
	uint32_t objID;//0x00 //pc = PF00
	int32_t hexNum;//0x04
	int32_t xShift;//0x08
	int32_t yShift;//0x0C
	int32_t viewScrnX;//0x10
	int32_t viewScrnY;//0x14
	uint32_t frameNum;//0x18
	uint32_t ori;//0x1C
	uint32_t frmID;//0x20
	uint32_t flags;//0x24 //critter 24240060 //inv 0x000000FF = 1=item1, 2 = item2 4 = armor
	int32_t level;//0x28
	union PUD pud;
	uint32_t proID;//0x64  01
	uint32_t cID;//0x68  05
	uint32_t light_dist;//0x6C 04 //Light strength of this object?   lightRadius
	uint32_t light_intensity;//0x70 0100 //Something to do with radiation?  lightIntensity
	uint32_t combatFlags;//0x74 set to =0   //only valid in combat //read and written but set to 0 on load.
	uint32_t scriptID1;//0x78   50460004  34000004 related to load time  /map scrip ID ?
	uint32_t unknown7C;//0x7C set to =0  //not read but written but set to 0 on load.
	uint32_t scriptID2;//0x80  //objScriptID?
};


#endif //F1DPPATCHER_F_OBJECTS_H
