// Copyright (C) 2022 DADi590
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// This had no copyright notice on it. I'm keeping the same license as in the other files of the project on it though
// (I'm just licensing it only to myself because I can't put the original authors - no notice).
//
// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#ifndef F1DPPATCHER_LOADGAMEHOOK_H
#define F1DPPATCHER_LOADGAMEHOOK_H



#include <stdint.h>

extern uint32_t InLoop;

void LoadGameHookInit(void);

// [DADi590:] I had to swap the bitwise operations with the actual values because Watcom won't get their value before
// feeding them into the assembler for some reason, and then it's an error because the assembler doesn't know what 1<<31
// is...

#define WORLDMAP    0x1         //  (1<<0)
#define LOCALMAP    0x2         //  (1<<1)    No point hooking this: would always be 1 at any point at which scripts are running
#define DIALOG      0x4         //  (1<<2)
#define ESCMENU     0x8         //  (1<<3)
#define SAVEGAME    0x10        //  (1<<4)
#define LOADGAME    0x20        //  (1<<5)
#define COMBAT      0x40        //  (1<<6)
#define OPTIONS     0x80        //  (1<<7)
#define HELP        0x100       //  (1<<8)
#define CHARSCREEN  0x200       //  (1<<9)
#define PIPBOY      0x400       //  (1<<10)
#define PCOMBAT     0x800       //  (1<<11)
#define INVENTORY   0x1000      //  (1<<12)
#define AUTOMAP     0x2000      //  (1<<13)
#define SKILLDEX    0x4000      //  (1<<14)
#define INTFACEUSE  0x8000      //  (1<<15)
#define INTFACELOOT 0x10000     //  (1<<16)
#define BARTER      0x20000     //  (1<<17)
#define RESERVED    0x80000000  //  (1<<31)



#endif //F1DPPATCHER_LOADGAMEHOOK_H
