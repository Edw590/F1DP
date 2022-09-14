/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2022 DADi590

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
// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-08-28.

#include "../CLibs/stdio.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../GameAddrs/FalloutEngine.h"
#include "../Utils/BlockAddrUtils.h"
#include "../Utils/EXEPatchUtils.h"
#include "../Utils/GlobalEXEAddrs.h"
#include "../Utils/IniUtils.h"
#include "F_Mapper.h"
#include "F_Objects.h"
#include "HighResPatches.h"
#include "F_Art.h"
#include "F_File.h"
#include "F_Windows.h"


struct bitset {
	uint32_t numBits;
	uint32_t *dwords;
	uint32_t numDwods;
};

void bitset(struct bitset *bit_set, uint32_t nBits) {
	bit_set->numBits=nBits;
	bit_set->numDwods = ((bit_set->numBits >> 5) + 1);
	bit_set->dwords = (uint32_t*)malloc(sizeof(uint32_t) * bit_set->numDwods);
	memset(bit_set->dwords, 0x0, sizeof(uint32_t) * bit_set->numDwods);
}
void unbitset(struct bitset *bit_set) {
	free(bit_set->dwords);
	bit_set->dwords = NULL;
}

void set1(struct bitset *bit_set, uint32_t bit) {
	uint32_t bindex = bit >> 5;
	uint32_t boffset= bit&0x1F;
	if(bindex < bit_set->numDwods)
		bit_set->dwords[bindex] |= (1 << boffset);
}
void set(struct bitset *bit_set) {
	memset(bit_set->dwords, 0xFF, sizeof(uint32_t) * bit_set->numDwods);
}
void clear1(struct bitset *bit_set, uint32_t bit) {
	uint32_t bindex = bit >> 5;
	uint32_t boffset= bit&0x1F;
	if(bindex < bit_set->numDwods)
		bit_set->dwords[bindex] &= ~(1 << boffset);
}
void clear(struct bitset *bit_set) {
	memset(bit_set->dwords, 0x00, sizeof(uint32_t) * bit_set->numDwods);
}
uint32_t get(struct bitset *bit_set, uint32_t bit) {
	uint32_t bindex = bit >> 5;
	uint32_t boffset= bit&0x1F;
	if(bindex < bit_set->numDwods)
		return bit_set->dwords[bindex] & (1 << boffset);
	else
		return 0;
}


// Used to make the replacements for global variable pointer correction: (*(var_type *) getRealBlockAddrData(&))

int FOG_OF_WAR = false;
int32_t fogLight = 0x1000;
struct bitset *fogHexMapBits = NULL;
bool isRecordingObjFog = false;//needed to prevent obj discovery before true pcObj position set.

int32_t *pMAP_LEVEL = NULL;

int32_t *pNUM_HEX_X = NULL;
int32_t *pNUM_HEX_Y = NULL;
int32_t *pNUM_HEXES = NULL;

int32_t* pNUM_TILE_Y = NULL;
int32_t* pNUM_TILE_X = NULL;
int32_t* pNUM_TILES = NULL;

int32_t * pAmbientLightIntensity = NULL;
int32_t *pLightHexArray = NULL;

struct OBJNode** pMapObjNodeArray = NULL;
struct OBJNode* upperMapObjNodeArray[40000] = {0};
struct OBJStruct* pCombatOutlineList[500] = {0};
int combatOutlineCount = 0;

uint32_t*** pMapTileLevelOffset = NULL;

struct OBJStruct** lpObj_PC = NULL;
struct OBJStruct** lpObjSpecial = NULL;

struct MAPdata M_CURRENT_MAP = {0};


//mark and display other wall objects that are 1 hex away from current wall object - to reduce blockiness
//_____________________________________________________________
void MarkVisibleWalls(struct OBJStruct* objViewer, struct OBJStruct* objWall) {
	int32_t hexPos = objWall->hexNum;
	struct OBJNode* objNode = NULL;
	struct OBJStruct* obj2 = NULL;
	int32_t type = 0;
	int ori = 0;
	for (ori = 0; ori < 6; ori++) {
		hexPos = GetNextHexPos(objWall->hexNum, ori, 1);
		objNode = (*(struct OBJNode** *) getRealBlockAddrData(&pMapObjNodeArray))[hexPos];
		while (objNode) {
			obj2 = objNode->obj;
			if (obj2->level <= objWall->level) {
				type = (obj2->frmID & 0x0F000000) >> 24;
				if (obj2->level == objWall->level && !(obj2->flags & FLG_MarkedByPC)) {
					if (type == ART_WALLS) {
						obj2->flags = obj2->flags | FLG_MarkedByPC;
						if (!IsInLineOfSightBlocked(objViewer->hexNum, obj2->hexNum))
							MarkVisibleWalls(objViewer, obj2);
					}
					else if (type == ART_SCENERY) {//treat doors as walls
						struct PROTOscenery* pro = NULL;
						F_GetPro(obj2->proID, (struct PROTOall**)&pro);
						if (pro->sceneryType == FLG_Portal) {
							obj2->flags = obj2->flags | FLG_MarkedByPC;
							if (!IsInLineOfSightBlocked(objViewer->hexNum, obj2->hexNum))
								MarkVisibleWalls(objViewer, obj2);
						}
					}
				}
				objNode = objNode->next;
			}
			else
				objNode = NULL;
		}
	}
}


//________________________________________________
int32_t GetHexDistance(int32_t hexStart, int32_t hexEnd) {

	int32_t xSquStart = 0, ySquStart = 0;
	int32_t xSquEnd = 0, ySquEnd = 0;
	int32_t xDiff = 0;
	int32_t yDiff = 0;

	if (hexStart == -1)
		return 9999;
	if (hexEnd == -1)
		return 9999;

	GetHexSqrXY(hexStart, &xSquStart, &ySquStart);
	GetHexSqrXY(hexEnd, &xSquEnd, &ySquEnd);

	xDiff = abs(xSquEnd - xSquStart);
	yDiff = abs(ySquEnd - ySquStart);

	if (yDiff >= xDiff)
		return yDiff;
	else
		return (xDiff + yDiff) >> 1;///2;
}


//Check is object is visible to PC. returns 1=display normaly, 0=display but darken, -1=dont display.
//_______________________________
int IsVisibleByPC(struct OBJStruct* obj) {
	struct OBJStruct* pObj_PC = *(*(struct OBJStruct** *) getRealBlockAddrData(&lpObj_PC));
	int32_t type = (obj->frmID & 0x0F000000) >> 24;

	if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)))
		return 1;
	if (!*(uint32_t *) getRealEXEAddr(D__refresh_enabled))
		return 1;

	if (!(*(bool *) getRealBlockAddrData(&isRecordingObjFog)))
		return 1;

	if (obj == pObj_PC) {
		obj->flags = obj->flags | FLG_MarkedByPC;
		return 1;
	}

	if (type != ART_CRITTERS) {
		//display marked objects - already seem by PC
		if (obj->flags & FLG_MarkedByPC)
			return 1;
			//if line of sight between PC and object is not blocked or object is a wall less than 6 hexes away.
		else if (!IsInLineOfSightBlocked(pObj_PC->hexNum, obj->hexNum) || (GetHexDistance(pObj_PC->hexNum, obj->hexNum) < 2 && type == ART_WALLS)) {
			obj->flags = obj->flags | FLG_MarkedByPC;
			//mark and display other wall objects that are 1 hex away from current wall object - to reduce blockiness
			if (type == ART_WALLS)
				MarkVisibleWalls(pObj_PC, obj);
			return 1;
		}
		else if (GetHexDistance(pObj_PC->hexNum, obj->hexNum) < 2 && type == ART_SCENERY) {
			struct PROTOscenery* pro = NULL;
			F_GetPro(obj->proID, (struct PROTOall**)&pro);
			if (pro->sceneryType == 0) {
				obj->flags = obj->flags | FLG_MarkedByPC;
				return 1;
			}
		}
			//display these objects at low light until found - to reduce pop in uglyness
		else if (type == ART_WALLS || type == ART_SCENERY || type == ART_MISC) {
			return 0;
		}
		else
			return -1;
	}
	else if (type == ART_CRITTERS) {
		obj->flags = obj->flags | FLG_MarkedByPC;
		//if is on PC's team.
		if (obj->pud.critter.combat_data.teamNum == 0)
			return 1;
			//if in combat mode and is visible in combat line of sight system.
		else if ((obj->combatFlags & FLG_IsNotPC) && !(obj->combatFlags & FLG_IsNotFightable) && !(obj->combatFlags & FLG_NotVisByPC))
			return 1;
			//if is in line of sight of PC.
		else if (!IsInLineOfSightBlocked(pObj_PC->hexNum, obj->hexNum))
			return 1;
	}
	return -1;
}


//__________________________________________________
int32_t F_GetHexLightIntensity(int32_t level, int32_t hexPos) {
	int32_t retVal = 0;
	__asm {
			mov     edx, hexPos
			mov     eax, level
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_light_get_tile_]
			call    edi
			pop     edi
			mov     retVal, eax
	}
	return retVal;
}


//______________________________________________________________
void F_DrawObj(struct OBJStruct* obj, RECT* rect, uint32_t lightIntensity) {
	__asm {
			mov     ebx, lightIntensity
			mov     edx, rect
			mov     eax, obj
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_render_object_]
			call    edi
			pop     edi
	}
}



//_______________________________________________________________
uint32_t F_CheckObjectFrmAtPos(struct OBJStruct* obj, int32_t xPos, int32_t yPos) {
	uint32_t flags = 0;
	__asm {
			mov     ebx, yPos
			mov     edx, xPos
			mov     eax, obj
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_obj_intersects_with_]
			call    edi
			pop     edi
			mov     flags, eax
	}
	return flags;
}


//__________________________________________________________________________
void GetTileXY(int32_t scrnX, int32_t scrnY, int32_t level, int32_t* tileX, int32_t* tileY) {
	(void *) level;
	__asm {
			PUSH    ESI                                //GET_TILE_COOR(EAX scrnX, EDX scrnY, EBX level, ECX *tileX, Arg1 *tileY)
			PUSH    EDI
			//PUSH    EBP
			//MOV     EBX, tileY
			MOV     ECX, tileX
			MOV     EDI, scrnX
			MOV     ESI, scrnY
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_offy]
			pop     edi
			MOV     EBX, DWORD PTR DS : [EAX]            // tileViewYPos
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_offx]
			pop     edi
			MOV     EDX, DWORD PTR DS : [EAX]            // tileViewXPos
			SUB     ESI, EBX                              // scrnY-tileViewYPos
			SUB     EDI, EDX                              // scrnX-tileViewXPos
			SUB     ESI, 0xC                              // scrnY-12
			LEA     EDX, [EDI * 0x4]
			LEA     EAX, [ESI * 0x4]
			SUB     EDX, EDI
			SUB     EDX, EAX
			MOV     DWORD PTR DS : [ECX] , EDX               // tileX = ((scrnX-tileViewXPos)*3)-((scrnY-tileViewYPos -12)*4)
			TEST    EDX, EDX                             // if(tileX>=0)
			JGE     J0x4B1FDA
			INC     EDX                                  // tileX++
			MOV     EBX, 0xC0
			MOV     EAX, EDX
			SAR     EDX, 0x1F
			IDIV    EBX                                 // tileX=tileX/192
			DEC     EAX                                  // tileX--
			JMP     J0x4B1FE6
		J0x4B1FDA:
			MOV     EBX, 0xC0
			MOV     EAX, EDX
			SAR     EDX, 0x1F
			IDIV    EBX                                 // tileX=tileX/192
		J0x4B1FE6:
			MOV     DWORD PTR DS : [ECX] , EAX               // store tileX
			SHL     ESI, 0x2                                // (scrnY-tileViewYPos -12)*4
			ADD     EDI, ESI
			MOV     EBX, tileY
			MOV     DWORD PTR DS : [EBX] , EDI               // tileY = ((scrnX-tileViewXPos)*4) + ((scrnY-tileViewYPos -12)*4)
			TEST    EDI, EDI                             // if(tileY>=0)
			JGE     J0x4B2006
			LEA     EDX, [EDI + 0x1]                          // tileY++
			MOV     EAX, EDX
			SAR     EDX, 0x1F
			SHL     EDX, 0x7
			SBB     EAX, EDX
			SAR     EAX, 0x7                                // tileY=tileY/128
			DEC     EAX                                  // tileY--
			JMP     J0x4B2015
		J0x4B2006:
			MOV     EDX, EDI
			MOV     EAX, EDI
			SAR     EDX, 0x1F
			SHL     EDX, 0x7
			SBB     EAX, EDX
			SAR     EAX, 0x7                                // tileY=tileY/128
		J0x4B2015:
			MOV     EBX, tileY
			MOV     DWORD PTR DS : [EBX] , EAX               // store tileY
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_x]
			pop     edi
			MOV     EAX, DWORD PTR DS : [EAX]            // currentTilePosX
			ADD     DWORD PTR DS : [ECX] , EAX               // tileX=tileX+currentTilePosX
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_y]
			pop     edi
			MOV     EAX, DWORD PTR DS : [EAX]            // currentTilePosY
			MOV     EDI, DWORD PTR DS : [EBX]
			ADD     EDI, EAX
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_width]
			pop     edi
			MOV     EAX, DWORD PTR DS : [EAX]            // numTilesMapX(100)
			MOV     DWORD PTR DS : [EBX] , EDI               // tileY=tileY+currentTilePosY
			DEC     EAX
			MOV     EBX, DWORD PTR DS : [ECX]
			SUB     EAX, EBX
			MOV     DWORD PTR DS : [ECX] , EAX               // tileX = numTilesX-1 - tileX
		    //POP     EBP
			POP     EDI
			POP     ESI
	}

}


//_________________________________________________________________________
int32_t GetScrnXYTile(int32_t tileNum, int32_t* scrnX, int32_t* scrnY, int32_t level) {
	int32_t retVal = 0;

	(void *) level;
	__asm {
			PUSH     ESI                                 // int GET_SCRN_COOR_TILE(EAX tileOffset, EDX *scrnX, EBX *scrnY, ECX level)
			PUSH     EDI
			//PUSH     EBP
			MOV     EAX, tileNum
			MOV     ESI, EAX
			//MOV     EDI,scrnX
			//MOV     EBX, scrnY
			TEST    EAX, EAX
			JL      J0x4B1DD3
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EDX, [edi+D__square_size]
			pop     edi
			CMP     EAX, DWORD PTR DS : [EDX]            // numTiles
			JL      J0x4B1DDC
		J0x4B1DD3:
			MOV     EAX, -1
			JMP     endFunc
			//POP     EBP
			//POP     EDI
			//POP     ESI
			//RETN

		J0x4B1DDC:
			MOV     EDX, EAX
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     ECX, [edi+D__square_width]
			pop     edi
			MOV     ECX, DWORD PTR DS : [ECX]            // numTilesMapX(100)
			SAR     EDX, 0x1F
			IDIV    ECX
			DEC     ECX
			MOV     EAX, ESI
			SUB     ECX, EDX
			MOV     EDX, ESI
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     ESI, [edi+D__square_width]
			pop     edi
			MOV     ESI, DWORD PTR DS : [ESI]            // numTilesMapX(100)
			SAR     EDX, 0x1F
			IDIV    ESI
			MOV     EDX, EAX
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_offx]
			pop     edi
			MOV     EAX, DWORD PTR DS : [EAX]            // tileViewXPos
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EBX, [edi+D__square_x]
			pop     edi
			MOV     EBX, DWORD PTR DS : [EBX]            // currentTilePosX
			MOV     EDI, scrnX
			MOV     DWORD PTR DS : [EDI] , EAX
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_offy]
			pop     edi
			MOV     EAX, DWORD PTR DS : [EAX]            // tileViewYPos
			SUB     ECX, EBX
			MOV     EBX, scrnY
			MOV     DWORD PTR DS : [EBX] , EAX
			MOV     EAX, ECX
			LEA     ESI, [ECX * 0x4]
			SUB     ESI, ECX
			SHL     ESI, 0x4
			MOV     ECX, DWORD PTR DS : [EDI]
			ADD     ECX, ESI
			MOV     ESI, EAX
			MOV     DWORD PTR DS : [EDI] , ECX
			SHL     ESI, 0x2
			MOV     EDI, DWORD PTR DS : [EBX]
			SUB     ESI, EAX
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     EAX, [edi+D__square_y]
			pop     edi
			MOV     EAX, DWORD PTR DS : [EAX]            // currentTilePosY
			SHL     ESI, 0x2
			SUB     EDX, EAX
			SUB     EDI, ESI
			MOV     EAX, EDX
			MOV     DWORD PTR DS : [EBX] , EDI
			SHL     EDX, 0x5
			MOV     ESI, EAX
			MOV     EDI, scrnX
			MOV     ECX, DWORD PTR DS : [EDI]
			SHL     ESI, 0x2
			ADD     ECX, EDX
			SUB     ESI, EAX
			MOV     DWORD PTR DS : [EDI] , ECX
			SHL     ESI, 0x3
			MOV     EDI, DWORD PTR DS : [EBX]
			ADD     EDI, ESI
			XOR     EAX, EAX
			MOV     DWORD PTR DS : [EBX] , EDI
		endFunc:
			//POP     EBP
			POP     EDI
			POP     ESI
			mov     retVal, EAX
	}
	return retVal;
}


//________________________________________________
void CheckAngledObjEdge(RECT* rect, uint32_t isUpper) {

	int32_t xMax;
	int32_t yMax;
	int32_t tilePosNum = 0;
	int32_t scrnX = 0, scrnY = 0;
	RECT* prcEdge;
	uint32_t* pFlags;
	int32_t yOff;
	int32_t tempVal = 0;

	int32_t yPos = 0;
	int32_t xPos = 0;

	RECT rcTileGrid = { 0,0,0,0 };

	xMax = *(int32_t *) getRealEXEAddr(D__square_width);
	yMax = *(int32_t *) getRealEXEAddr(D__square_length);

	GetTileXY(rect->left, rect->top, 0, &tempVal, &rcTileGrid.top);
	GetTileXY(rect->right, rect->top, 0, &rcTileGrid.right, &tempVal);
	GetTileXY(rect->left, rect->bottom, 0, &rcTileGrid.left, &tempVal);
	GetTileXY(rect->right, rect->bottom, 0, &tempVal, &rcTileGrid.bottom);

	rcTileGrid.left += 1;
	rcTileGrid.top -= 1;
	rcTileGrid.right -= 1;
	rcTileGrid.bottom += 1;

	if (rcTileGrid.right < 0)
		rcTileGrid.right = 0;
	else if (rcTileGrid.right >= xMax)
		rcTileGrid.right = xMax - 1;
	if (rcTileGrid.top < 0)
		rcTileGrid.top = 0;
	else if (rcTileGrid.top >= yMax)
		rcTileGrid.top = yMax - 1;

	if (rcTileGrid.left > xMax)
		rcTileGrid.left = xMax;
	if (rcTileGrid.bottom > yMax)
		rcTileGrid.bottom = yMax;

	if (rcTileGrid.bottom < rcTileGrid.top)
		return;

	prcEdge = &(*(struct MAPdata *) getRealBlockAddrData(&M_CURRENT_MAP)).currentEDGES->angleEdge;
	pFlags = &(*(struct MAPdata *) getRealBlockAddrData(&M_CURRENT_MAP)).currentEDGES->angleFlag;
	yOff = rcTileGrid.top * xMax;
	for (yPos = rcTileGrid.top; yPos <= rcTileGrid.bottom; yPos++) {
		if (rcTileGrid.left >= rcTileGrid.right) {
			for (xPos = rcTileGrid.right; xPos <= rcTileGrid.left; xPos++) {
				tilePosNum = yOff + xPos;
				if (xPos > prcEdge->left && (*pFlags & 0x01000000) >> 24 == isUpper
					|| yPos < prcEdge->top && (*pFlags & 0x00010000) >> 16 == isUpper
					|| xPos < prcEdge->right && (*pFlags & 0x00000100) >> 8 == isUpper
					|| yPos > prcEdge->bottom && (*pFlags & 0x00000001) == isUpper) {
					GetScrnXYTile(tilePosNum, &scrnX, &scrnY, 0);
					F_DrawFloorTile(0x04000001, scrnX, scrnY, rect);
				}
			}
		}
		yOff += xMax;
	}
}


//______________________________________
void DrawObjects(RECT* rect, int32_t level) {
	struct OBJNode* mapObj = NULL;
	int32_t hexLight = 0;
	int upperObjCount = 0;
	int objNum = 0;

	int isVisPC = 0;
	int32_t hexPos = 0;

	int32_t globalLight = *(*(int32_t * *) getRealBlockAddrData(&pAmbientLightIntensity));

	(*(int *) getRealBlockAddrData(&combatOutlineCount)) = 0;

	//draw flat objects first
	for (hexPos = 0; hexPos < *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)); hexPos++) {
		hexLight = F_GetHexLightIntensity(level, hexPos);
		if (hexLight < globalLight)
			hexLight = globalLight;
		mapObj = (*(struct OBJNode** *) getRealBlockAddrData(&pMapObjNodeArray))[hexPos];

		while (mapObj) {
			if (mapObj->obj->level <= level) {
				if (mapObj->obj->level == level) {
					if ((mapObj->obj->flags & FLG_Flat)) {
						if (!(mapObj->obj->flags & 0x1)) {
							isVisPC = IsVisibleByPC(mapObj->obj);
							if (isVisPC == 0 && (hexLight < (*(int32_t *) getRealBlockAddrData(&fogLight)) || (*(int32_t *) getRealBlockAddrData(&fogLight)) == 0))
								isVisPC = 1;
							if (isVisPC == 1)
								F_DrawObj(mapObj->obj, rect, hexLight);
							else if (isVisPC == 0)
								F_DrawObj(mapObj->obj, rect, (*(int32_t *) getRealBlockAddrData(&fogLight)));

							if ((mapObj->obj->combatFlags & FLG_IsNotPC) && !(mapObj->obj->combatFlags & FLG_IsNotFightable)) {
								if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)) || mapObj->obj->pud.critter.combat_data.teamNum == 0 || !(mapObj->obj->combatFlags & FLG_NotVisByPC)) {
									if ((*(int *) getRealBlockAddrData(&combatOutlineCount)) < 500) {
										(*(struct OBJStruct** *) getRealBlockAddrData(&pCombatOutlineList))[(*(int *) getRealBlockAddrData(&combatOutlineCount))] = mapObj->obj;
										(*(int *) getRealBlockAddrData(&combatOutlineCount))++;
									}
								}
							}
						}
					}
					else {
						(*(struct OBJNode** *) getRealBlockAddrData(&upperMapObjNodeArray))[upperObjCount] = mapObj;
						upperObjCount++;
						mapObj = NULL;
					}
				}
			}
			else
				mapObj = NULL;
			if (mapObj)
				mapObj = mapObj->next;
		}
	}

	CheckAngledObjEdge(rect, 0);
	//draw non flat objects
	for (objNum = 0; objNum < upperObjCount; objNum++) {
		mapObj = (*(struct OBJNode** *) getRealBlockAddrData(&upperMapObjNodeArray))[objNum];
		if (mapObj)
			hexPos = mapObj->obj->hexNum;

		hexLight = F_GetHexLightIntensity(level, hexPos);
		if (hexLight < globalLight)
			hexLight = globalLight;

		while (mapObj) {
			if (mapObj->obj->level <= level) {
				if (mapObj->obj->level == level) {
					if (!(mapObj->obj->flags & 0x1)) {
						isVisPC = IsVisibleByPC(mapObj->obj);
						if (isVisPC == 0 && (hexLight < (*(int32_t *) getRealBlockAddrData(&fogLight)) || (*(int32_t *) getRealBlockAddrData(&fogLight)) == 0))
							isVisPC = 1;
						if (isVisPC == 1)
							F_DrawObj(mapObj->obj, rect, hexLight);
						else if (isVisPC == 0)
							F_DrawObj(mapObj->obj, rect, (*(int32_t *) getRealBlockAddrData(&fogLight)));

						if ((mapObj->obj->combatFlags & FLG_IsNotPC) && !(mapObj->obj->combatFlags & FLG_IsNotFightable)) {
							if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)) || mapObj->obj->pud.critter.combat_data.teamNum == 0 || !(mapObj->obj->combatFlags & FLG_NotVisByPC)) {
								if ((*(int *) getRealBlockAddrData(&combatOutlineCount)) < 500) {
									(*(struct OBJStruct** *) getRealBlockAddrData(&pCombatOutlineList))[(*(int *) getRealBlockAddrData(&combatOutlineCount))] = mapObj->obj;
									(*(int *) getRealBlockAddrData(&combatOutlineCount))++;
								}
							}
						}
					}
				}
			}
			else
				mapObj = NULL;
			if (mapObj)
				mapObj = mapObj->next;
		}
	}
	CheckAngledObjEdge(rect, 1);
}


//Check if object is visible -for mouse selection.
//______________________________
bool IsNotFogged(struct OBJStruct* obj) {
	struct OBJStruct* pObj_PC = *(*(struct OBJStruct** *) getRealBlockAddrData(&lpObj_PC));
	int32_t objType = (obj->frmID & 0x0F000000) >> 0x18;

	if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)))
		return true;

	//allow critters if in line of sight for normal or combat modes.
	if (objType == ART_CRITTERS) {
		if (obj->pud.critter.combat_data.teamNum == 0)
			return true;
		else if ((obj->combatFlags & FLG_IsNotPC) && !(obj->combatFlags & FLG_IsNotFightable) && !(obj->combatFlags & FLG_NotVisByPC))
			return true;
		else if (IsInLineOfSightBlocked(pObj_PC->hexNum, obj->hexNum))
			return false;
	}
		//allow wall and scenery if fog is set to 0;
	else if (!(*(int32_t *) getRealBlockAddrData(&fogLight)) && (objType == ART_WALLS || objType == ART_SCENERY || objType == ART_MISC))
		return true;
		//allow if obj is visible.
	else if (!(obj->flags & FLG_MarkedByPC))
		return false;

	return true;
}


//find the object who's frm lies under mouse cursor.
//_________________________________________________________________________________________
int32_t GetObjectsAtPos(int32_t xPos, int32_t yPos, int32_t level, int32_t type, struct OBJInfo** lpObjInfoArray) {

	struct OBJNode* mapObj = NULL;
	struct OBJInfo* pObjInfoArray = NULL;
	int32_t hexPos = 0;

	uint32_t objInfoArraySize = 0;

	bool exitLoop = false;
	int32_t objType = 0;
	uint32_t flags = 0;
	int32_t numObjects = 0;

	*lpObjInfoArray = pObjInfoArray;

	for (hexPos = 0; hexPos < *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)); hexPos++) {

		mapObj = (*(struct OBJNode** *) getRealBlockAddrData(&pMapObjNodeArray))[hexPos];
		objInfoArraySize = sizeof(struct OBJInfo) * (numObjects + 1);

		while (mapObj && !exitLoop) {
			if (mapObj->obj->level <= level) {
				if (mapObj->obj->level == level) {
					if (type != -1) {
						objType = mapObj->obj->frmID & 0x0F000000;
						objType = objType >> 0x18;
					}
					else
						objType = -1;

					if (objType == type && mapObj->obj != *(*(struct OBJStruct** *) getRealBlockAddrData(&lpObjSpecial)) && IsNotFogged(mapObj->obj)) {
						flags = F_CheckObjectFrmAtPos(mapObj->obj, xPos, yPos);
						if (flags) {
							pObjInfoArray = (struct OBJInfo*)FReallocateMemory((uint8_t *)pObjInfoArray, objInfoArraySize);
							if (pObjInfoArray) {
								pObjInfoArray[numObjects].obj = mapObj->obj;
								pObjInfoArray[numObjects].flags = flags;
								numObjects++;
								objInfoArraySize += sizeof(struct OBJInfo);
							}
						}
					}
				}
				mapObj = mapObj->next;
			}
			else
				exitLoop = true;
		}
		exitLoop = false;
	}
	*lpObjInfoArray = pObjInfoArray;

	return numObjects;
}

//copy a file to or from save game slot  -returns 0 pass, -1 fail.
//_______________________________________________
int32_t F_CopySaveFile(char *toPath, char *fromPath) {
	int32_t retVal = 0;
	__asm {
			mov     edx, fromPath
			mov     eax, toPath
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_copy_file_]
			call    edi
			pop     edi
			mov     retVal, eax
	}
	return retVal;
}

//step adjustment array ori 0-5 {evenHex y, oddHex y, x}
int32_t hexStep[6][3] = {
		{0,-1, -1}, {1, 0,-1},  {1, 1, 0},  {1, 0, 1},  {0,-1, 1}, {-1,-1,0}
};
int32_t hexStep2[6][3] = {
		{0,-200, -1}, {200, 0,-1},  {200, 200, 0},  {200, 0, 1},  {0,-200, 1}, {-200,-200,0}
};

//____________________________________________________________
int32_t GetNextHexPos(int32_t hexPos, uint32_t direction, int32_t distance) {
	int32_t xMax = 0;
	int32_t yMax = 0;
	int32_t y = 0;
	int32_t x = 0;
	int32_t hexNew = hexPos;

	//if(hexPos<=0 || hexPos>40000) return hexPos;

	xMax = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	yMax = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_Y));
	y = hexPos / xMax;
	x = hexPos % xMax;

	while (distance > 0) {

		y += (*(int32_t** *) getRealBlockAddrData(&hexStep))[direction][(x & 0x01)];
		hexNew += (*(int32_t** *) getRealBlockAddrData(&hexStep2))[direction][(x & 0x01)];
		x += (*(int32_t** *) getRealBlockAddrData(&hexStep))[direction][2];
		hexNew += (*(int32_t** *) getRealBlockAddrData(&hexStep2))[direction][2];

		if (x < 0 || x >= xMax)
			return hexPos;
		if (y < 0 || y >= yMax)
			return hexPos;

		hexPos = hexNew;

		distance--;
	}
	return hexPos;
}

//__________________________________________________
void GetHexSqrXY(int32_t hexPos, int32_t* px, int32_t* py) {
	//grid 1x =16pixels, 1y =12pixels
	//x must be divided by 2 to get hex width of 32pixels
	int32_t xMax = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	int32_t y = hexPos / xMax;
	int32_t x = hexPos % xMax;

	*py = y + (x >> 1);
	*px = (x << 1) - *py;

}


//____________________________________
bool CheckHexTransparency(int32_t hexNum) {
	uint32_t flags = FLG_LightThru | FLG_ShootThru | FLG_TransNone;//|FLG_Flat;

	struct OBJNode* objNode = 0;
	int32_t objType = 0;

	objNode = (*(struct OBJNode** *) getRealBlockAddrData(&pMapObjNodeArray))[hexNum];
	objType = 0;

	if (hexNum < 0 || hexNum >= *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES))) {
		return true;
	}

	if ((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)) && !get((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)), *(*(int32_t * *) getRealBlockAddrData(&pMAP_LEVEL)) * *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)) + hexNum))
		set1((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)), *(*(int32_t * *) getRealBlockAddrData(&pMAP_LEVEL)) * *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)) + hexNum);

	while (objNode) {
		if (objNode->obj->level <= *(*(int32_t * *) getRealBlockAddrData(&pMAP_LEVEL))) {
			if (objNode->obj->level == *(*(int32_t * *) getRealBlockAddrData(&pMAP_LEVEL))) {
				objType = (objNode->obj->frmID & 0x0F000000) >> 0x18;

				if (objType == ART_WALLS && !(objNode->obj->flags & flags) || objType == ART_SCENERY && !(objNode->obj->flags & flags))// && !(objNode->obj->flags & OBJFLAG_TransNone) && !(objNode->obj->flags & OBJFLAG_Flat))
					return true;
			}
			objNode = objNode->next;
		}
		else
			objNode = NULL;
	}

	return false;
}

//__________________________________________
bool GetHexXY(int32_t hexPos, int32_t* x, int32_t* y) {
	if (hexPos < 0 || hexPos >= *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)))
		return false;

	*y = hexPos / *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	*x = hexPos % *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));

	return true;
}


//_______________________________________
void F_DrawMapArea(RECT* rect, int level) {

	__asm {
			push    ebx
			mov     edx, level
			mov     eax, rect
			push    edi
			mov     edi, SN_DATA_SEC_EXE_ADDR
			lea     ebx, [edi+D__tile_refresh]
			pop     edi
			call    dword ptr ds : [ebx]
			pop     ebx
	}
}


//__________________
void ReDrawViewWin() {
	struct WinStruct* win;
	if (*(uint32_t *) getRealEXEAddr(D__display_win) == -1)
		return;
	win = GetWinStruct(*(uint32_t *) getRealEXEAddr(D__display_win));
	if (win && *(uint32_t *) getRealEXEAddr(D__refresh_enabled))
		F_DrawMapArea(&win->rect, *(*(int32_t * *) getRealBlockAddrData(&pMAP_LEVEL)));
}


//________________________________________________________
bool FogOfWarMap_CopyFiles(char* pToPath, char* pFromPath) {
	char* fromPath = NULL;
	char* toPath = NULL;
	void* FileStream_From = NULL;
	void* FileStream_To = NULL;
	uint32_t dVal = 0;
	uint32_t numuint32_ts = 0;
	uint32_t i = 0;

	if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)))
		return false;

	fromPath = malloc(sizeof(char) * 256);
	sprintf(fromPath, "%s", pFromPath);
	memcpy(strchr(fromPath, '.'), ".fog\0", 5);
	toPath = malloc(sizeof(char) * 256);
	sprintf(toPath, "%s", pToPath);
	memcpy(strchr(toPath, '.'), ".fog\0", 5);


	FileStream_From = F_fopen(fromPath, "rb");
	if (FileStream_From == NULL)
		return false;
	FileStream_To = F_fopen(toPath, "wb");
	if (FileStream_To == NULL) {
		F_fclose(FileStream_From);
		return false;
	}

	dVal = 0;
	numuint32_ts = 0;

	F_fread32(FileStream_From, &dVal);
	if (dVal == 0x464F474F) {
		F_fwrite32(FileStream_To, 0x464F474F);//"FOGO"
		F_fread32(FileStream_From, &dVal);
		if (dVal == 0x46574152)
			F_fwrite32(FileStream_To, 0x46574152);//"FWAR"
		else {
			F_fclose(FileStream_From);
			F_fclose(FileStream_To);
			return false;
		}
		F_fread32(FileStream_From, &dVal);//version
		if (dVal == 0x00000001)//version1
			F_fwrite32(FileStream_To, 0x00000001);//version1
		else {
			F_fclose(FileStream_From);
			F_fclose(FileStream_To);
			return false;
		}
	}
	F_fread32(FileStream_From, &dVal);//numBits
	F_fwrite32(FileStream_To, dVal);
	F_fread32(FileStream_From, &numuint32_ts);
	F_fwrite32(FileStream_To, numuint32_ts);
	for (i = 0; i < numuint32_ts; i++) {
		F_fread32(FileStream_From, &dVal);
		F_fwrite32(FileStream_To, dVal);
	}
	F_fclose(FileStream_From);
	F_fclose(FileStream_To);

	return true;
}


//______________________________________
int32_t FogOfWarMap_DeleteTmps(char* path) {
	if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)))
		return false;
	//return numFiles
	return FDeleteTmpSaveFiles(path, "fog");
}


//__________________________________
bool FogOfWarMap_Save(char* MapName) {
	char mapPath[256];
	uint32_t i;
	void* FileStream;

	if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)))
		return false;

	sprintf(mapPath, "%s", MapName);
	memcpy(strchr(mapPath, '.'), ".fog\0", 5);
	FileStream = F_fopen(mapPath, "rb");

	if (FileStream == NULL)
		return false;

	if ((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))) {
		F_fwrite32(FileStream, 0x464F474F);//"FOGO"
		F_fwrite32(FileStream, 0x46574152);//"FWAR"
		F_fwrite32(FileStream, 0x00000001);//version1
		F_fwrite32(FileStream, (*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->numBits);
		F_fwrite32(FileStream, (*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->numDwods);
		for (i = 0; i < (*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->numDwods; i++)
			F_fwrite32(FileStream, (*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->dwords[i]);
	}

	F_fclose(FileStream);
	///MessageBox(NULL, mapPath, "Fog Saved",MB_ICONEXCLAMATION | MB_OK);
	return true;
}


//__________________________________
bool FogOfWarMap_Load(char* MapName) {
	char mapPath[256];
	void* FileStream;
	uint32_t dVal = 0;
	uint32_t i = 0;

	if (!(*(int *) getRealBlockAddrData(&FOG_OF_WAR)))
		return false;
	(*(bool *) getRealBlockAddrData(&isRecordingObjFog)) = true;
	//reset map hex width in case changed.
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[0][1] = -*(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[1][0] = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[2][0] = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[2][1] = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[3][0] = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[4][1] = -*(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[5][0] = -*(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));
	(*(int32_t** *) getRealBlockAddrData(&hexStep2))[5][1] = -*(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));


	if ((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)) != NULL) {
		free((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)));
		(*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)) = NULL;
	}

	sprintf(mapPath, "maps\\%s", MapName);
	memcpy(strchr(mapPath, '.'), ".fog\0", 5);
	FileStream = F_fopen(mapPath, "rb");
	if (FileStream == NULL) {
		(*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)) = malloc(sizeof(struct bitset));
		set1((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)), *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)) * 3);
		return false;
	}

	if (!(*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))) {
		F_fread32(FileStream, &dVal);//0x464F474F);//"FOGO"
		if (dVal == 0x464F474F) {
			F_fread32(FileStream, &dVal);//0x47574152);//"FWAR"
			if (dVal != 0x46574152) {
				F_fclose(FileStream);
				return false;
			}
			F_fread32(FileStream, &dVal);
			if (dVal != 0x00000001) {//version1
				F_fclose(FileStream);
				return false;
			}
		}
		F_fread32(FileStream, &dVal);//(*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->numBits);
		(*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)) = malloc(sizeof(struct bitset));
		set1((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)), dVal);
		F_fread32(FileStream, &dVal);//(*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->numuint32_ts);
		for (i = 0; i < (*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->numDwods; i++)
			F_fread32(FileStream, &(*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))->dwords[i]);
	}

	F_fclose(FileStream);
	return true;
}

//_____________________________________________________
bool IsInLineOfSightBlocked(int32_t hexStart, int32_t hexEnd) {

	int32_t hexCurrent = hexStart;
	//int32_t ori = 0;

	int32_t hexStartX = 0, hexStartY = 0;
	int32_t hexEndX = 0, hexEndY = 0;
	int32_t hexCurrentX = 0, hexCurrentY = 0;

	int32_t squStartX = 0, squStartY = 0;
	int32_t squEndX = 0, squEndY = 0;
	int32_t squCurrentX = 0, squCurrentY = 0;

	int32_t hexesW = *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X));

	GetHexXY(hexStart, &hexStartX, &hexStartY);
	GetHexXY(hexEnd, &hexEndX, &hexEndY);
	GetHexXY(hexCurrent, &hexCurrentX, &hexCurrentY);

	GetHexSqrXY(hexStart, &squStartX, &squStartY);
	GetHexSqrXY(hexEnd, &squEndX, &squEndY);
	GetHexSqrXY(hexCurrent, &squCurrentX, &squCurrentY);


	if (hexCurrentX == hexEndX) {//hex grid up and down (diagonal right/down to left/up)
		while (hexCurrent != hexEnd) {
			if (hexCurrentY < hexEndY)
				hexCurrent += hexesW;
			else
				hexCurrent -= hexesW;
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}
	else if (hexCurrentY == hexEndY) {//hex grid left and right
		while (hexCurrent != hexEnd) {
			if (hexCurrentX < hexEndX)
				hexCurrent += 1;
			else
				hexCurrent -= 1;
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}
	else if (squCurrentY == squEndY) {//square left and right
		while (hexCurrent != hexEnd) {
			if (squCurrentX > squEndX) {
				if (hexCurrent & 0x1)
					hexCurrent -= 1;
				else
					hexCurrent += hexesW - 1;
			}
			else {
				if (hexCurrent & 0x1)
					hexCurrent -= hexesW - 1;
				else
					hexCurrent += 1;
			}
			if (CheckHexTransparency(hexCurrent))
				return true;
		}

	}
	else if (squCurrentX == squEndX) {//square up and down
		while (hexCurrent != hexEnd) {
			if (squCurrentY < squEndY) {
				if (hexCurrent & 0x1) {
					if (CheckHexTransparency(hexCurrent + hexesW) && CheckHexTransparency(hexCurrent + 1))
						return true;
					hexCurrent += hexesW + 1;
				}
				else {
					if (CheckHexTransparency(hexCurrent + hexesW) && CheckHexTransparency(hexCurrent + hexesW + 1))
						return true;
					hexCurrent += hexesW + hexesW + 1;
				}
			}
			else {
				if (hexCurrent & 0x1) {
					if (CheckHexTransparency(hexCurrent - hexesW) && CheckHexTransparency(hexCurrent - hexesW - 1))
						return true;
					hexCurrent -= (hexesW + hexesW + 1);
				}
				else {
					if (CheckHexTransparency(hexCurrent - hexesW) && CheckHexTransparency(hexCurrent - 1))
						return true;
					hexCurrent -= (hexesW + 1);
				}
			}
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}

	else if ((squEndY - squCurrentY) - (squEndX - squCurrentX) == 0) {//diagonal left/down to right/up
		while (hexCurrent != hexEnd) {
			if (squCurrentX < squEndX) {
				if (hexCurrent & 0x1)
					hexCurrent += 1;
				else
					hexCurrent += hexesW + 1;
			}
			else {
				if (hexCurrent & 0x1)
					hexCurrent -= (hexesW + 1);
				else
					hexCurrent -= 1;
			}
			if (CheckHexTransparency(hexCurrent))
				return true;
		}
	}
	else {
		int32_t distY = abs(squEndY - squCurrentY);
		int32_t distX = abs(squEndX - squCurrentX);
		int32_t distY2 = 0;//distY*distY;
		int32_t distX2 = 0;//distX*distY;
		int32_t error = 0;

		if (distY <= distX) {
			//int32_t xPos = 0;
			distY2 = distY * distY;
			distX2 = distX * distY;
			if (squCurrentY < squEndY && squCurrentX < squEndX) {//square left/down shallow
				bool stepX = true;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (error <= distX2) {
						if (CheckHexTransparency(hexCurrent))
							return true;
					}

					if (error > distX2) {
						error -= distX2;
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
						else {
							hexCurrent += hexesW;
							stepX = true;
						}
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;

						error += distY2;
					}
				}
			}
			else if (squCurrentY < squEndY && squCurrentX > squEndX) {//square right/down shallow
				bool stepX = false;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (error <= distX2) {
						if (CheckHexTransparency(hexCurrent))
							return true;
					}
					if (error > distX2) {
						error -= distX2;
						if (!stepX) {
							hexCurrent += hexesW;
							stepX = true;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
					}
					else {
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
						error += distY2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX < squEndX) {//square left/up shallow
				bool stepX = true;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distX2) {
						error -= distX2;
						if (stepX) {
							hexCurrent -= hexesW;
							stepX = false;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;

						error += distY2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX > squEndX) {//square right/up shallow
				bool stepX = false;
				error = distX2 - distY2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distX2) {
						error -= distX2;
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
						else {
							hexCurrent -= hexesW;
							stepX = false;
						}
					}
					else {
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
						error += distY2;
					}
				}
			}
			else
				return true;
		}
		else if (distY > distX) {
			distY2 = distY * distX;
			distX2 = distX * distX;
			if (squCurrentY < squEndY && squCurrentX < squEndX) {//square left/down steep
				bool stepX = true;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
						else {
							hexCurrent += hexesW;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else if (squCurrentY < squEndY && squCurrentX > squEndX) {//square right/down steep
				bool stepX = false;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
					}
					else {
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent += 1;
							else
								hexCurrent += (hexesW + 1);
							stepX = false;
						}
						else {
							hexCurrent += hexesW;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX > squEndX) {//square right/up steep
				bool stepX = false;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (!stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= 1;
							else
								hexCurrent += (hexesW - 1);
							stepX = true;
						}
						else
							stepX = false;
					}
					else {
						if (stepX) {
							hexCurrent -= hexesW;
							stepX = false;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else if (squCurrentY > squEndY && squCurrentX < squEndX) {//square left/up steep
				bool stepX = true;
				error = distY2 - distX2;
				while (hexCurrent != hexEnd) {
					if (CheckHexTransparency(hexCurrent))
						return true;

					if (error > distY2) {
						error -= distY2;
						if (stepX) {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW - 1);
							else
								hexCurrent += 1;
							stepX = false;
						}
						else
							stepX = true;
					}
					else {
						if (stepX) {
							hexCurrent -= hexesW;
							stepX = false;
						}
						else {
							if (hexCurrent & 0x1)
								hexCurrent -= (hexesW + 1);
							else
								hexCurrent -= 1;
							stepX = true;
						}
						error += distX2;
					}
				}
			}
			else
				return true;
		}
		else
			return true;
	}
	return false;
}

//_____________________________________________________________
int32_t GetFloorHexLight(int32_t elev, int32_t hexNum, int32_t globalLight) {
	int32_t elevOffset = elev * *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES));
	int32_t light = (*(int32_t * *) getRealBlockAddrData(&pLightHexArray))[elevOffset + hexNum];

	if ((elev < 0) || (elev >= 3)) {
		return 0;
	}
	if ((hexNum < 0) || (hexNum >= *(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)))) {
		return 0;
	}

	if (light < globalLight)
		light = globalLight;

	if ((*(int *) getRealBlockAddrData(&FOG_OF_WAR)) && light > (*(int32_t *) getRealBlockAddrData(&fogLight)) && (*(int32_t *) getRealBlockAddrData(&fogLight)) != 0) {
		struct OBJStruct* pObj_PC = *(*(struct OBJStruct** *) getRealBlockAddrData(&lpObj_PC));
		if ((*(struct OBJStruct** *) getRealBlockAddrData(&lpObj_PC)) && pObj_PC->hexNum != -1 && (*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits))) {
			if (get((*(struct bitset * *) getRealBlockAddrData(&fogHexMapBits)), elevOffset + hexNum) == 0 && IsInLineOfSightBlocked(pObj_PC->hexNum, hexNum))
				return (*(int32_t *) getRealBlockAddrData(&fogLight));
		}
	}

	if (light > 0x10000)
		return 0x10000;
	else
		return light;
}


//_______________________________________________________
uint32_t CheckAngledTileEdge(int x, int y, uint32_t tileLstNum) {
	(void *) x;
	(void *) y;

	/* DADi590: commented this because it's not important to FOW
	if (isAngledEdges) {
		struct MAPedge* currentEdges = M_CURRENT_MAP.currentEDGES;
		if (x > currentEdges->angleEdge.left || x < currentEdges->angleEdge.right || y > currentEdges->angleEdge.bottom || y < currentEdges->angleEdge.top)
			tileLstNum = 1;
	}*/

	return F_GetFrmID(ART_TILES, tileLstNum, 0, 0, 0);
}


//_________________________________________
void DrawFloorTiles(RECT* rect, int32_t level) {

	RECT rcTileGrid = { 0,0,0,0 };
	int32_t tempVal = 0;

	uint32_t** levelOffset;
	int32_t tileLstNum = 0;
	int32_t tilePosNum = 0;
	int32_t tileFlag = 0;
	int32_t scrnX = 0, scrnY = 0;
	uint32_t fID = 0;

	int32_t yPos;
	int32_t xPos;

	int32_t yOff;

	GetTileXY(rect->left, rect->top, level, &tempVal, &rcTileGrid.top);
	GetTileXY(rect->right, rect->top, level, &rcTileGrid.right, &tempVal);
	GetTileXY(rect->left, rect->bottom, level, &rcTileGrid.left, &tempVal);
	GetTileXY(rect->right, rect->bottom, level, &tempVal, &rcTileGrid.bottom);

	if (rcTileGrid.right < 0)
		rcTileGrid.right = 0;
	else if (rcTileGrid.right >= *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_X)))
		rcTileGrid.right = *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_X)) - 1;
	if (rcTileGrid.top < 0)
		rcTileGrid.top = 0;
	else if (rcTileGrid.top >= *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_Y)))
		rcTileGrid.top = *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_Y)) - 1;

	if (rcTileGrid.left > *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_X)))
		rcTileGrid.left = *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_X));
	if (rcTileGrid.bottom > *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_Y)))
		rcTileGrid.bottom = *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_Y));

	if (rcTileGrid.bottom < rcTileGrid.top)
		return;

	levelOffset = *(*(uint32_t*** *) getRealBlockAddrData(&pMapTileLevelOffset));
	yOff = rcTileGrid.top * *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_X));
	for (yPos = rcTileGrid.top; yPos <= rcTileGrid.bottom; yPos++) {
		if (rcTileGrid.left >= rcTileGrid.right) {
			for (xPos = rcTileGrid.right; xPos <= rcTileGrid.left; xPos++) {
				tilePosNum = yOff + xPos;
				if (tilePosNum >= 0 && tilePosNum < *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILES))) {
					tileLstNum = levelOffset[level][tilePosNum];
					tileFlag = (tileLstNum & 0x0000F000) >> 12;
					tileLstNum = tileLstNum & 0x00000FFF;

					if (!(tileFlag & 0x1)) {
						GetScrnXYTile(tilePosNum, &scrnX, &scrnY, level);
						fID = CheckAngledTileEdge(xPos, yPos, tileLstNum);
						F_DrawFloorTile(fID, scrnX, scrnY, rect);
					}
					///else MessageBox(nullptr, "tile flag set", "Hi-Res Patch Error",MB_ICONEXCLAMATION | MB_OK);
				}
			}
		}
		yOff += *(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_X));
	}
}


//_________________________________________________________
void F_DrawFloorTile(uint32_t fid, int32_t x, int32_t y, RECT* rect) {

	__asm {
			mov     ecx, rect
			mov     ebx, y
			mov     edx, x
			mov     eax, fid
			push    edi
			mov     edi, SN_CODE_SEC_EXE_ADDR
			lea     edi, [edi+C_floor_draw_]
			call    edi
			pop     edi
	}
}


//__________________
void SetMapGlobals(void) {
	int temp_int = 0;
	char prop_value[MAX_PROP_VALUE_LEN];
	memset(prop_value, 0, MAX_PROP_VALUE_LEN);

	getPropValueIni(MAIN_INI_SPEC_SEC_HIGHRES_PATCH, "MAPS", "FOG_OF_WAR", "0", prop_value, &high_res_patch_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	*(int *) getRealBlockAddrData(&FOG_OF_WAR) = (0 != temp_int);

	getPropValueIni(MAIN_INI_SPEC_SEC_HIGHRES_PATCH, "MAPS", "FOG_LIGHT_LEVEL", "0", prop_value, &high_res_patch_ini_info_G);
	sscanf(prop_value, "%d", &temp_int);
	if ((temp_int < 1) || (temp_int > 10)) {
		(*(int32_t *) getRealBlockAddrData(&fogLight)) = 0;
	} else {
		(*(int32_t *) getRealBlockAddrData(&fogLight)) = temp_int * 0x1000;
	}
}

void FMapperSetup(void) {
	(*(uint32_t*** *) getRealBlockAddrData(&pMapTileLevelOffset)) = getRealEXEAddr(D__squares);

	(*(struct OBJStruct** *) getRealBlockAddrData(&lpObjSpecial)) = getRealEXEAddr(D__obj_egg);
	(*(int32_t * *) getRealBlockAddrData(&pAmbientLightIntensity)) = getRealEXEAddr(D__ambient_light);

	(*(int32_t * *) getRealBlockAddrData(&pLightHexArray)) = getRealEXEAddr(D__tile_intensity);

	(*(struct OBJNode** *) getRealBlockAddrData(&pMapObjNodeArray)) = getRealEXEAddr(D__objectTable);

	(*(int32_t * *) getRealBlockAddrData(&pMAP_LEVEL)) = getRealEXEAddr(D__map_elevation);

	(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_X)) = getRealEXEAddr(D__grid_width);
	(*(int32_t * *) getRealBlockAddrData(&pNUM_HEX_Y)) = getRealEXEAddr(D__grid_length);
	(*(int32_t * *) getRealBlockAddrData(&pNUM_HEXES)) = getRealEXEAddr(D__grid_size);

	(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_Y)) = getRealEXEAddr(D__square_length);
	(*(int32_t * *) getRealBlockAddrData(&pNUM_TILE_X)) = getRealEXEAddr(D__square_width);
	(*(int32_t * *) getRealBlockAddrData(&pNUM_TILES)) = getRealEXEAddr(D__square_size);

	(*(struct OBJStruct** *) getRealBlockAddrData(&lpObj_PC)) = getRealEXEAddr(D__obj_dude);
}
