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

#ifndef F1DPPATCHER_F_MAPPER_H
#define F1DPPATCHER_F_MAPPER_H



#include "F_Objects.h"
#include <stdbool.h>

struct MAPedge {
	int32_t x;
	int32_t y;
	RECT sqrEdge;
	RECT visEdge;
	RECT tileEdge;
	RECT angleEdge;
	uint32_t angleFlag;
	struct MAPedge *prevArea;
	struct MAPedge *nextArea;
};

struct MAPdata {
	struct MAPedge mapEDGE[3];
	struct MAPedge *currentEDGES;

};

struct OBJInfo {
	uint32_t flags;
	struct OBJStruct *obj;
};


int32_t __cdecl ScrnSqr2HexPosMove(int32_t x, int32_t y, bool axis);
void __cdecl ScrnHexPos2Sqr(int32_t hexPos, int32_t* px, int32_t* py);
void __cdecl GetHexSqrXY(int32_t hexPos, int32_t *px, int32_t *py);

int32_t __cdecl SetViewPos(int32_t hexPos, uint32_t flags);
extern struct MAPdata M_CURRENT_MAP;

extern int32_t *pVIEW_HEXPOS;
extern int32_t *pMAP_LEVEL;

extern uint32_t *pPC_SCROLL_LIMIT_FLAG;

extern struct OBJStruct *pCombatOutlineList[500];
extern int combatOutlineCount;


extern uint32_t *pSCROLL_BLOCK_FLAG;

extern char workingMapName[32];
extern char *pCurrentMapName;

extern int32_t EDGE_OFF_X;
extern int32_t EDGE_OFF_Y;

extern int FOG_OF_WAR;

extern uint32_t* pDRAW_VIEW_FLAG;

void __cdecl SetMapGlobals();
void __cdecl SetMapperScrollInfo();
void __cdecl ReDrawViewWin();
int32_t __cdecl SetMapLevel(int32_t level);
void __cdecl F_DrawFloorTile(uint32_t fid, int32_t x, int32_t y, RECT *rect);
void __cdecl GetObjectRect(struct OBJStruct *obj, RECT *rcObj);
void __cdecl F_DrawMapArea(RECT *rect, int level);


int32_t __cdecl ToggleMapRooves();
void __cdecl ShowMapRooves();
void __cdecl HideMapRooves();
int32_t __cdecl AreMapRoovesVisible();


int32_t __cdecl ToggleMapHexes();
void __cdecl ShowMapHexes();
void __cdecl HideMapHexes();
int32_t __cdecl AreMapHexesVisible();

void __cdecl F_IntFace_Init();
void __cdecl F_IntFace_Destroy();
void __cdecl F_SetMouseModeFlag(uint32_t flag);
void __cdecl F_ResetIfaceBar();
void __cdecl F_LoadMap( char *mapName);
void __cdecl F_SaveMap( char *mapName);

void __cdecl SaveMap(char *mapName);

void __cdecl F_ResetNewMap();
void __cdecl F_CheckGameInput(int keyCode, uint32_t flags);
void __cdecl F_InitMapArea();
int __cdecl F_FalloutSetup(int pathLength, char *path);

int32_t __cdecl F_SelectHexpos();
void __cdecl DeleteMapObjs(struct OBJStruct **lpObj1, struct OBJStruct **lpObj2);
int32_t __cdecl Get_GRID001_ListNum();

int32_t __cdecl GetNextHexPos(int32_t hexPos, uint32_t direction, int32_t distance);
int32_t __cdecl GetHexDistance(int32_t hexStart, int32_t hexEnd);

bool __cdecl FogOfWarMap_CopyFiles(char *pToPath, char *pFromPath);
int32_t __cdecl FogOfWarMap_DeleteTmps(char *path);
bool __cdecl FogOfWarMap_Save(char *MapName);
bool __cdecl FogOfWarMap_Load(char *MapName);

void __cdecl SetMapBorders(char *MapName);
int32_t __cdecl ClipEdgeRect(RECT *rect1, RECT *rect2, RECT *rect3);

bool __cdecl isHexWithinMapEdges(int32_t hexPos);
int32_t __cdecl SetViewPos(int32_t hexPos, uint32_t flags);

void __cdecl GetTileXY(int32_t scrnX, int32_t scrnY, int32_t level, int32_t *tileX, int32_t *tileY);
int32_t __cdecl GetScrnXYTile(int32_t tileNum, int32_t *scrnX, int32_t *scrnY, int32_t level);
int32_t __cdecl GetFloorHexLight(int32_t elev, int32_t hexNum, int32_t globalLight);

void __cdecl DrawFloorTiles(RECT *rect, int32_t level);
uint32_t __cdecl CheckAngledTileEdge(int x, int y, uint32_t tileLstNum);

void __cdecl CheckAngledObjEdge(RECT *rect, uint32_t isUpper);
uint32_t __cdecl CheckAngledRoofTileEdge(int32_t xPos, int32_t yPos, uint32_t tileLstNum);

void __cdecl DrawObjects(RECT *rect, int32_t level);
void __cdecl DrawDialogView(uint8_t *frmBuff, int32_t subWidth, int32_t subHeight, int32_t frmWidth, uint8_t *toBuff, int32_t toWidth);

int32_t __cdecl GetObjectsAtPos(int32_t xPos, int32_t yPos, int32_t level, int32_t type, struct OBJInfo **lpObjInfoArray);

int32_t __cdecl MergeRects(RECT *rect1, RECT *rect2, RECT *newRect);

void __cdecl SetMapTile(uint32_t frmID, uint32_t proID);
void __cdecl SetMapObject(uint32_t frmID, uint32_t proID);

int32_t __cdecl F_CopySaveFile(char* toPath, char *fromPath);

bool __cdecl ReSizeMaps();
void __cdecl FMapperSetup();

int32_t __cdecl F_GetScrnHexPos(int32_t scrnX, int32_t scrnY, int32_t level);

bool __cdecl IsInLineOfSightBlocked(int32_t hexStart, int32_t hexEnd);



#endif //F1DPPATCHER_F_MAPPER_H
