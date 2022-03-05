// Copyright 2022 DADi590
//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

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
