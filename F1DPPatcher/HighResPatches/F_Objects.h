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

// Microsoft's RECT structure in windef.h
typedef struct tagRECT {
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

struct DAMAGEstats
{
	uint32_t normal;//0x04
	uint32_t laser;//0x08
	uint32_t fire;//0x0C
	uint32_t plasma;//0x10
	uint32_t electrical;//0x14
	uint32_t emp;//0x18
	uint32_t explosive;//0x1C
};

struct PRIMEStats
{
	uint32_t Strength;//0x24        0
	uint32_t Perception;//0x28      1
	uint32_t Endurance;//0x2C       2
	uint32_t Charisma;//0x30        3
	uint32_t Intelligence;//0x34    4
	uint32_t Agility;//0x38         5
	uint32_t Luck;//0x3C            6
};

struct SECONDStats
{
	uint32_t HitPoints;//0x40               7
	uint32_t ActionPoints;//0x44            8
	uint32_t ArmorClass;//0x48              9
	uint32_t Unused4C;//0x4C                A
	uint32_t MeleeDamage;//0x50             B
	uint32_t CarryWeight;//0x54             C
	uint32_t Sequence;//0x58                D
	uint32_t HealingRate;//0x5C             E
	uint32_t CriticalChance;//0x60          F
	uint32_t CriticalHitModifier;//0x64     10
	struct DAMAGEstats damageDT;//
	//uint32_t DTnormal;//0x68 //DT section 11
	//uint32_t DTlaser;//0x6C               12
	//uint32_t DTfire;//0x70                13
	//uint32_t DTplasma;//0x74              14
	//uint32_t DTelectrical;//0x78          15
	//uint32_t DTemp;//0x7C                 16
	//uint32_t DTexplosive;//0x80           17
	struct DAMAGEstats damageDR;
	//uint32_t DRnormal;//0x84 //DR section 18
	//uint32_t DRlaser;//0x88               19
	//uint32_t DRfire;//0x8C                1A
	//uint32_t DRplasma;//0x90              1B
	//uint32_t DRelectrical;//0x94          1C
	//uint32_t DRemp;//0x98                 1D
	//uint32_t DRexplosive;//0x9C           1E
	uint32_t RadiationResistance;//0xA0     1F
	uint32_t PoisonResistance;//0xA4        20
	uint32_t Age;//0xA8                     21
	uint32_t Gender;//0xAC                  22
};

struct SKILLS
{
	uint32_t smallGuns ;//0x13C
	uint32_t bigGuns;//0x140
	uint32_t energyWeapons;//0x144
	uint32_t unarmed;//0x148
	uint32_t melee;//0x14C
	uint32_t throwing;//0x150
	uint32_t firstAid;//0x154
	uint32_t doctor ;//0x158
	uint32_t sneak;//0x15C
	uint32_t lockpick;//0x160
	uint32_t steal;//0x164
	uint32_t traps;//0x168
	uint32_t science;//0x16C
	uint32_t repair;//0x170
	uint32_t speech;//0x174
	uint32_t barter;//0x178
	uint32_t gambling;//0x17C
	uint32_t outdoorsman;//0x180
};


struct PROTOall
{
	uint32_t objID;//0x00
	uint32_t txtID;//0x04
	uint32_t frmID;//0x08
	uint32_t lightRadius;//0x0C
	uint32_t lightIntensity;//0x10
	uint32_t lightFlags;//0x14

	///uint32_t actionFlags;//0x18
	///uint32_t scrtptID;//0x1C
	///uint32_t crittFlags;//0x20
	///PRIMEStats primeStats;//0x24
	///SECONDStats secondStats;//0x40
	///PRIMEStats primeStatsBonus;//0xB0
	///SECONDStats secondStatsBonus;//0xCC
	///SKILLS skills;//0x13C
	///uint32_t bodyType;//0x184
	///uint32_t expVal;//0x188
	///uint32_t killType;//0x18C
	///uint32_t damageType;//0x190
	///uint32_t frmIDHead; //0x194
	///uint32_t aiPacket; //0x198
	///uint32_t teamNum; //0x19C

};





struct ITEMTYPEdata
{
	//	                    //0 armor           1 Containers    2 Drugs
	uint32_t data01;//0x24       AC                  MaxSize
	uint32_t data02;//0x28       DR DAMAGEstats      OpenFlags
	uint32_t data03;//0x2C       --                  //
	uint32_t data04;//0x30       --                  //
	uint32_t data05;//0x34       --                  //
	uint32_t data06;//0x38       --                  //
	uint32_t data07;//0x3C       DT DAMAGEstats      //
	uint32_t data08;//0x40       --                  //
	uint32_t data09;//0x44       --                  //
	uint32_t data10;//0x48       --                  //
	uint32_t data11;//0x4C       --                  //
	uint32_t data12;//0x50       --                  //
	uint32_t data13;//0x54       --                  //
	uint32_t data14;//0x58       Perk                //
	uint32_t data15;//0x5C       MaleFID             //
	uint32_t data16;//0x60       FemaleFID           //
	uint32_t data17;//0x64       //
	uint32_t data18;//0x68       //
};

struct PROTOitem
{
	uint32_t objID;//0x00
	uint32_t txtID;//0x04
	uint32_t frmID;//0x08
	uint32_t lightRadius;//0x0C
	uint32_t lightIntensity;//0x10
	uint32_t lightFlags;//0x14
	uint32_t actionFlags;//0x18
	uint32_t scriptID;//0x1C

	uint32_t itemType;//0x20
	struct ITEMTYPEdata itemTypeData;//0x24
	uint32_t materialID;//0x6C
	uint32_t size;//0x70
	uint32_t weight;//0x74
	uint32_t cost;//0x78
	uint32_t invFrmID;//0x7C
	uint8_t soundID;//80
};


struct PROTOcritter
{
	uint32_t objID;//0x00
	uint32_t txtID;//0x04
	uint32_t frmID;//0x08
	uint32_t lightRadius;//0x0C
	uint32_t lightIntensity;//0x10
	uint32_t lightFlags;//0x14
	uint32_t actionFlags;//0x18
	uint32_t scriptID;//0x1C

	uint32_t crittFlags;//0x20
	struct PRIMEStats primeStats;//0x24
	struct SECONDStats secondStats;//0x40
	struct PRIMEStats primeStatsBonus;//0xB0
	struct SECONDStats secondStatsBonus;//0xCC
	struct SKILLS skills;//0x13C
	uint32_t bodyType;//0x184
	uint32_t expVal;//0x188
	uint32_t killType;//0x18C
	uint32_t damageType;//0x190
	uint32_t frmIDHead; //0x194
	uint32_t aiPacket; //0x198
	uint32_t teamNum; //0x19C
};


struct SCENERYTYPEdata
{
	uint32_t data01;//0x24  //generic == 0xCCCCCCCC, doors == 0x0, ladders & stairs == 0xFFFFFFFF
	uint32_t data02;//0x28

};

struct PROTOscenery
{
	uint32_t objID;//0x00
	uint32_t txtID;//0x04
	uint32_t frmID;//0x08
	uint32_t lightRadius;//0x0C
	uint32_t lightIntensity;//0x10
	uint32_t lightFlags;//0x14
	uint32_t actionFlags;//0x18
	uint32_t scriptID;//0x1C

	uint32_t sceneryType;//0x20
	struct SCENERYTYPEdata sceneryTypeData;//0x24
	uint32_t materialID;//0x2C
	uint32_t unknown; //30
	uint8_t soundID;//34
};


struct PROTOwall
{
	uint32_t objID;//0x00
	uint32_t txtID;//0x04
	uint32_t frmID;//0x08
	uint32_t lightRadius;//0x0C
	uint32_t lightIntensity;//0x10
	uint32_t lightFlags;//0x14
	uint32_t actionFlags;//0x18
	uint32_t scriptID;//0x1C

	uint32_t materialID;//0x20
};

struct PROTOtile // same as misc
{
	uint32_t objID;//0x00
	uint32_t txtID;//0x04
	uint32_t frmID;//0x08
	uint32_t lightRadius;//0x0C
	uint32_t lightIntensity;//0x10
	uint32_t lightFlags;//0x14
	uint32_t materialID;//0x18  //misc = unknown
};



union PROTO {
	struct PROTOall *all;
	struct PROTOitem *item;
	struct PROTOcritter *critter;
	struct PROTOscenery *scenery;
	struct PROTOwall *wall;
	struct PROTOtile *tile;
	struct PROTOtile *misc;
};



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


struct OBJNode {
	struct OBJStruct*obj;
	struct OBJNode*next;
};


///OBJStruct.flags-------
#define FLG_Disabled       0x00000001 //???
#define FLG_Flat           0x00000008
#define FLG_NoBlock        0x00000010
#define FLG_MultiHex       0x00000800
#define FLG_NoHighlight    0x00001000
#define FLG_TransRed       0x00004000
#define FLG_TransNone      0x00008000
#define FLG_TransWall      0x00010000
#define FLG_TransGlass     0x00020000
#define FLG_TransSteam     0x00040000
#define FLG_TransEnergy    0x00080000
#define FLG_LightThru      0x20000000
#define FLG_ShootThru      0x80000000
#define FLG_WallTransEnd   0x10000000

#define FLG_MarkedByPC  0x40000000
//items
#define FLG_IsHeldSlot1 0x01000000
#define FLG_IsHeldSlot2 0x02000000
#define FLG_IsWornArmor 0x04000000


///OBJStruct.combatFlags-------
#define FLG_NotVisByPC           0x00000020
#define FLG_PCTeamMem            0x00000008
#define FLG_NonPCTeamMem         0x00000001
#define FLG_IsNotPC              0x00FFFFFF //check if any above flags set
#define FLG_IsPC                 0x00000000
#define FLG_IsNotFightable       0x80000000


///Scenery Types--------
#define FLG_Portal           0x00000000
#define FLG_Stairs           0x00000001
#define FLG_Elevators        0x00000002
#define FLG_LadderBottom     0x00000003
#define FLG_LadderTop        0x00000004
#define FLG_Generic          0x00000005


#endif //F1DPPATCHER_F_OBJECTS_H
