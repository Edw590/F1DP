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

#ifndef F1DPPATCHER_F_WINDOWS_H
#define F1DPPATCHER_F_WINDOWS_H



#include "F_Objects.h"

struct ButtonStruct {
	int32_t ref;//?//0x00
	uint32_t flags;//0x04
	RECT rect;//left 0x08, top 0x0C, right 0x10, bottom 0x14
	int32_t refHvOn;//0x18
	int32_t refHvOff;//0x1C
	int32_t refDn;//0x20
	int32_t refUp;//0x24
	int32_t refDnRht;//-1//0x28 //right mouse button
	int32_t refUpRht;//-1//0x2C //right mouse button
	uint8_t *buffUp;//0x30
	uint8_t *buffDn;//0x34
	uint8_t *buffHv;//0//0x38 other pic
	uint8_t *buffUpDis;//0x3C //upDisabledPic?
	uint8_t *buffDnDis;//0x40 //downDisabledPic?
	uint8_t *buffHvDis;//0x44 //otherDisabledPic?
	uint8_t *buffCurrent;//0x48 //current pic?
	uint8_t *buffDefault;//0x4C //default pic?
	void *funcHvOn;//0x50
	void *funcHvOff;//0x54
	void *funcDn;//0x58
	void *funcUp;//0x5C
	void *funcDnRht;//0x60//right mouse button
	void *funcUpRht;//0x64//right mouse button
	void *funcDnSnd;//0x68 push sound func
	void *funcUpSnd;//0x6C lift sound func
	uint32_t unknown70;//0x70
	struct ButtonStruct *nextButton;//0x74
	struct ButtonStruct *prevButton;//0x78
};

//#pragma pack(4)
struct WinStruct {
	int32_t ref;//0x00
	uint32_t flags;//0x04
	RECT rect;//left 0x08, top 0x0C, right 0x10, bottom 0x14
	int32_t width;//0x18
	int32_t height;//0x1C
	uint32_t colour;//0x20//colour index offset?
	uint32_t unknown24;//0x24//x?
	uint32_t unknown28;//0x28//y?
	uint8_t *buff;//0x2C         // uint8_ts frame data ref to palette
	struct ButtonStruct *ButtonList;//0x30//button struct list?
	uint32_t unknown34;//0x34
	uint32_t unknown38;//0x38
	uint32_t unknown3C;//0x3C
	void (*pBlit)(uint8_t *fBuff, int32_t subWidth, int32_t subHeight, int32_t fWidth, uint8_t *int32_t, uint32_t tWidth);//0x40//drawing func address
};

uint8_t* __cdecl FReallocateMemory(uint8_t * mem, uint32_t sizeuint8_ts);

struct WinStruct* __cdecl GetWinStruct(int32_t WinRef);



#endif //F1DPPATCHER_F_WINDOWS_H
