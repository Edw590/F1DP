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

// This had no copyright notice on it. I'm keeping the same license as in the other files of the project on it though
// (I'm just licensing it only to myself because I can't put the original authors - no notice).
//
// Original code modified by me, Edw590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#include "dinput.h"

bool KeyDown(uint16_t key) {
	uint32_t curr_addr = 0;
	//uint16_t buffer_key = 0;
	//uint8_t flags = true;
	bool any_case_letter = false;

	// Note: these 2 Interrupts check/return the FIRST key on the buffer. 01h checks the FIRST, not the entire buffer.
	/*
	 * INT 16h / AH = 00h - get keystroke from keyboard (no echo).
	 * 	return:
	 * 		AH = BIOS scan code.
	 * 		AL = ASCII character.
	 * 		(if a keystroke is present, it is removed from the keyboard buffer).
	 *
	 * INT 16h / AH = 01h - check for keystroke in the keyboard buffer.
	 * 	return:
	 * 		ZF = 1 if keystroke is not available.
	 * 		ZF = 0 if keystroke available.
	 * 		AH = BIOS scan code.
	 * 		AL = ASCII character.
	 * 		(if a keystroke is present, it is not removed from the keyboard buffer).
	 */

	// I think this is equivalent to GetAsyncKeyState(). It checks if the key is in the buffer. If it's not, leaves. If
	// it is, removes it, compares it, and returns the result. All without blocking the UI, like GetAsyncKeyState().
	// I just don't know if GetAsyncKeyState() removes the key from the buffer or not (if there is such a thing on
	// Windows and is not some different implementation).
	// EDIT: from http://cplusplus.com/forum/windows/21799/, I got this:
	// "GetAsyncKeyState doesn't work with a buffer. That's what the "Async" (asynchronous) part of its name means. It
	// just checks if the key is pressed when you call it."
	// So I'll not remove anything from the buffer and will just check it without modifying. This is still not the same
	// as GetAsyncKeyState(), because I think it sees if the key is in the queue - not if it is the first one on the
	// queue like INT 16h / 00h and 01h do, but I think that's the best we can do.
	// EDIT 2: I'm now iterating over the keyboard buffer (which seems to be on a fixed and known address), instead of
	// getting the first key on the buffer without checking the others. So if I clicked T and then R, called this
	// function to check if I clicked R, it will say no because T is the one there at the moment. R is behind. This
	// implementation takes care of that.
	// Idea gotten from: https://stackoverflow.com/a/40963163/8228163.

	any_case_letter = (0x00FF == (key & (uint16_t) 0x00FF));

	// The 2 if statements could be inside a unique for loop, but this is for optimization (loop without many checks).
	if (any_case_letter) {
		for (curr_addr = 0x0000041E; curr_addr <= 0x0000043C; curr_addr += 2) {
			uint16_t buffer_key = *(uint16_t *) curr_addr;
			if ((key & (uint16_t) 0xFF00) == (buffer_key & (uint16_t) 0xFF00)) { // Check only the most significant 8 bits
				return true;
			}
		}
	} else {
		for (curr_addr = 0x0000041E; curr_addr <= 0x0000043C; curr_addr += 2) {
			uint16_t buffer_key = *(uint16_t *) curr_addr;
			if (key == buffer_key) { // Check all bits
				return true;
			}
		}
	}

	return false;

	/*__asm {
			mov     ah, 0x01
			int     0x16

			mov     [buffer_key], ax // Store AX before it's changed below
			lahf
			mov     [flags], ah // Put all flags in a C variable for the Zero Flag to be analyzed in C
	}
	if (0 != (flags & (uint8_t) 0x40)) { // 0x40 == 0100 0000 b
		// ZF == 1, no keystroke available in the keyboard buffer

		return false;
	}

	__asm {
			mov     ah, 0x00
			int     0x16
	}

	printf("1: %X / 2: %X\n", *(uint16_t *) 0x00000480, *(uint16_t *) 0x00000482);

	if (any_case_letter) {
		return (key & (uint16_t) 0xFF00) == (buffer_key & (uint16_t) 0xFF00);
	} else {
		return key == buffer_key;
	}*/
}
