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


int32_t ScrnSqr2HexPosMove(int32_t x, int32_t y, bool axis);
void ScrnHexPos2Sqr(int32_t hexPos, int32_t* px, int32_t* py);
void GetHexSqrXY(int32_t hexPos, int32_t *px, int32_t *py);

int32_t SetViewPos(int32_t hexPos, uint32_t flags);
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

void SetMapGlobals();
void SetMapperScrollInfo();
void ReDrawViewWin();
int32_t SetMapLevel(int32_t level);
void F_DrawFloorTile(uint32_t fid, int32_t x, int32_t y, RECT *rect);
void GetObjectRect(struct OBJStruct *obj, RECT *rcObj);
void F_DrawMapArea(RECT *rect, int level);


int32_t ToggleMapRooves();
void ShowMapRooves();
void HideMapRooves();
int32_t AreMapRoovesVisible();


int32_t ToggleMapHexes();
void ShowMapHexes();
void HideMapHexes();
int32_t AreMapHexesVisible();

void F_IntFace_Init();
void F_IntFace_Destroy();
void F_SetMouseModeFlag(uint32_t flag);
void F_ResetIfaceBar();
void F_LoadMap( char *mapName);
void F_SaveMap( char *mapName);

void SaveMap(char *mapName);

void F_ResetNewMap();
void F_CheckGameInput(int keyCode, uint32_t flags);
void F_InitMapArea();
int F_FalloutSetup(int pathLength, char *path);

int32_t F_SelectHexpos();
void DeleteMapObjs(struct OBJStruct **lpObj1, struct OBJStruct **lpObj2);
int32_t Get_GRID001_ListNum();

int32_t GetNextHexPos(int32_t hexPos, uint32_t direction, int32_t distance);
int32_t GetHexDistance(int32_t hexStart, int32_t hexEnd);

bool FogOfWarMap_CopyFiles(char *pToPath, char *pFromPath);
int32_t FogOfWarMap_DeleteTmps(char *path);
bool FogOfWarMap_Save(char *MapName);
bool FogOfWarMap_Load(char *MapName);

void SetMapBorders(char *MapName);
int32_t ClipEdgeRect(RECT *rect1, RECT *rect2, RECT *rect3);

bool isHexWithinMapEdges(int32_t hexPos);
int32_t SetViewPos(int32_t hexPos, uint32_t flags);

void GetTileXY(int32_t scrnX, int32_t scrnY, int32_t level, int32_t *tileX, int32_t *tileY);
int32_t GetScrnXYTile(int32_t tileNum, int32_t *scrnX, int32_t *scrnY, int32_t level);
int32_t GetFloorHexLight(int32_t elev, int32_t hexNum, int32_t globalLight);

void DrawFloorTiles(RECT *rect, int32_t level);
uint32_t CheckAngledTileEdge(int x, int y, uint32_t tileLstNum);

void CheckAngledObjEdge(RECT *rect, uint32_t isUpper);
uint32_t CheckAngledRoofTileEdge(int32_t xPos, int32_t yPos, uint32_t tileLstNum);

void DrawObjects(RECT *rect, int32_t level);
void DrawDialogView(uint8_t *frmBuff, int32_t subWidth, int32_t subHeight, int32_t frmWidth, uint8_t *toBuff, int32_t toWidth);

int32_t GetObjectsAtPos(int32_t xPos, int32_t yPos, int32_t level, int32_t type, struct OBJInfo **lpObjInfoArray);

int32_t MergeRects(RECT *rect1, RECT *rect2, RECT *newRect);

void SetMapTile(uint32_t frmID, uint32_t proID);
void SetMapObject(uint32_t frmID, uint32_t proID);

int32_t F_CopySaveFile(char* toPath, char *fromPath);

bool ReSizeMaps();
void FMapperSetup();

int32_t F_GetScrnHexPos(int32_t scrnX, int32_t scrnY, int32_t level);

bool IsInLineOfSightBlocked(int32_t hexStart, int32_t hexEnd);



#endif //F1DPPATCHER_F_MAPPER_H
