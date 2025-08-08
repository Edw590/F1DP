/*
The MIT License (MIT)
Copyright © 2022 Matt Wells
Copyright © 2025 Edw590

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

extern struct OBJStruct *pCombatOutlineList[500];
extern int combatOutlineCount;

extern int FOG_OF_WAR;

void SetMapGlobals();
void __declspec(naked) ReDrawViewWin();

void __declspec(naked) GetHexDistance();

void __declspec(naked) FogOfWarMap_CopyFiles();
void __declspec(naked) FogOfWarMap_DeleteTmps();
void __declspec(naked) FogOfWarMap_Save();
void __declspec(naked) FogOfWarMap_Load();

void __declspec(naked) SetMapBorders();

void __declspec(naked) GetTileXY();
void __declspec(naked) GetScrnXYTile();
void __declspec(naked) GetFloorHexLight();

void __declspec(naked) DrawFloorTiles();
void __declspec(naked) CheckAngledTileEdge();

void __declspec(naked) DrawObjects();

void __declspec(naked) GetObjectsAtPos();

void __declspec(naked) F_CopySaveFile();

void __declspec(naked) IsInLineOfSightBlocked();


#endif //F1DPPATCHER_F_MAPPER_H
