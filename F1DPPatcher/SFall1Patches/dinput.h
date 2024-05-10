// Copyright (C) 2022 Edw590
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

#ifndef F1DPPATCHER_DINPUT_H
#define F1DPPATCHER_DINPUT_H



#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Checks if the given key is currently in the BIOS keyboard bufffer (meaning is is being/was pressed) - it is
 * wanted that this function is an equivalent to the Windows API's function GetAsyncKeyState().
 *
 * ATTENTION: this function does NOT remove keystrokes from the buffer, so getch() or equivalent must be called for that
 * purpose.
 *
 * About 0xFF on 'key': it is done that way because I don't see 0xFF used in the last 8 bits (or anywhere, actually) in
 * the BIOS Key Scan Codes lists, like on https://www.virtualbox.org/svn/vbox/trunk/src/VBox/Devices/PC/BIOS/keyboard.c,
 * so it can be used especially, I guess.
 *
 * @param key the BIOS Key Scan Code of the key. NOTE: if the key is a letter key and it's wanted that the letter can be
 *            uppercase or lowercase, then in the least significant 8 bits, put 0xFF. Example for U (or u): 0x16FF.
 *
 * @return true if the key is in the buffer, false otherwise
 */
bool KeyDown(uint16_t key);



#endif //F1DPPATCHER_DINPUT_H
