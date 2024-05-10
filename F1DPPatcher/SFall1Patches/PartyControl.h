/*
 * sfall
 * Copyright (C) 2013 The sfall team, 2022 Edw590
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#ifndef F1DPPATCHER_PARTYCONTROL_H
#define F1DPPATCHER_PARTYCONTROL_H



#include <stdint.h>

extern uint32_t IsControllingNPC;
extern uint32_t HiddenArmor;
extern uint32_t DelayedExperience;

void PartyControlInit(void);
void __stdcall PartyControlExit(void);
__declspec(naked) void RestoreDudeState(void);
__declspec(naked) void SaveDudeState(void);



#endif //F1DPPATCHER_PARTYCONTROL_H
