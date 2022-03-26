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

// This had no copyright notice on it. I'm keeping the same license as in the other files of the project on it though
// (I'm just licensing it only to myself because I can't put the original authors - no notice).
//
// Original code modified by me, DADi590, to adapt it to this project, starting on 2022-03-02.
// NOTE: I don't see mention to Crafty in the copyright notices, but I'll just say here that this code was taken from
// his modification of the original sFall1.

#ifndef F1DPPATCHER_SFALL1MAIN_H
#define F1DPPATCHER_SFALL1MAIN_H



#include "../Utils/IniUtils.h"

extern struct FileInfo translation_ini_info_G;

extern uint32_t _combatNumTurns;
extern uint32_t _tmpQNode;
extern uint32_t FakeCombatFix;

void FakeCombatFix3(void);
void queue_find_first_(void);
void queue_find_next_(void);

void DllMain2(void);



#endif //F1DPPATCHER_SFALL1MAIN_H
