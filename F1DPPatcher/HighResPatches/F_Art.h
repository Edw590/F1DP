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

#ifndef F1DPPATCHER_F_ART_H
#define F1DPPATCHER_F_ART_H



#define ART_ITEMS 0
#define ART_CRITTERS 1
#define ART_SCENERY 2
#define ART_WALLS 3
#define ART_TILES 4
#define ART_MISC 5
#define ART_INTRFACE 6
#define ART_INVEN 7
#define ART_HEADS 8
#define ART_BACKGRND 9
#define ART_SKILLDEX 10

uint32_t F_GetFrmID(uint32_t objType, uint32_t lstNum, uint32_t id2, uint32_t id1, uint32_t id3);



#endif //F1DPPATCHER_F_ART_H
