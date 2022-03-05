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

//
// Created by DADi590 on 05/03/2022.
//

#include "../CLibs/stdio.h"
#include "../Utils/EXEPatchUtils.h"
#include "Criticals.h"
#include "FalloutEngine.h"
#include <stdint.h>

struct CritStruct {
	union {
		struct {
			uint32_t DamageMultiplier;
			uint32_t EffectFlags;
			uint32_t StatCheck;
			uint32_t StatMod;
			uint32_t FailureEffect;
			uint32_t Message;
			uint32_t FailMessage;
		};
		uint32_t values[7];
	};
};

#define SetEntry(a,b,c,d,e) defaultTable[a*9*6 + b*6 + c].values[d]=e

void CritInit(void) {
	struct CritStruct *defaultTable = (struct CritStruct *) getRealEXEAddr(D__crit_succ_eff);

	// Critical table bug fixes

	// Crit_children.Leg_Right.Effect2
	SetEntry(2,4,1,4,0); // FailureEffect, (DAM_CRIP_ARM_RIGHT||DAM_BLIND||DAM_ON_FIRE||DAM_EXPLODE)->0
	SetEntry(2,4,1,5,5216); // Message, 5000->5216
	SetEntry(2,4,1,6,5000); // FailMessage, 0->5000

	// Crit_children.Leg_Right.Effect3
	SetEntry(2,4,2,4,0); // FailureEffect, (DAM_CRIP_ARM_RIGHT||DAM_BLIND||DAM_ON_FIRE||DAM_EXPLODE)->0
	SetEntry(2,4,2,5,5216); // Message, 5000->5216
	SetEntry(2,4,2,6,5000); // FailMessage, 0->5000

	// Crit_children.Leg_Left.Effect2
	SetEntry(2,5,1,4,0); // FailureEffect, (DAM_CRIP_ARM_RIGHT||DAM_BLIND||DAM_ON_FIRE||DAM_EXPLODE)->0
	SetEntry(2,5,1,5,5216); // Message, 5000->5216
	SetEntry(2,5,1,6,5000); // FailMessage, 0->5000

	// Crit_children.Leg_Left.Effect3
	SetEntry(2,5,2,4,0); // FailureEffect, (DAM_CRIP_ARM_RIGHT||DAM_BLIND||DAM_ON_FIRE||DAM_EXPLODE)->0
	SetEntry(2,5,2,5,5216); // Message, 5000->5216
	SetEntry(2,5,2,6,5000); // FailMessage, 0->5000

	// Crit_super_mutant.Leg_Left.Effect2.FailMessage
	SetEntry(3,5,1,6,5306); // 5312->5306

	// Crit_ghoul.Head.Effect5.StatCheck
	SetEntry(4,0,4,2,-1); // STAT_st->NoSTAT

	// Crit_brahmin.Head.Effect5.StatCheck
	SetEntry(5,0,4,2,-1); // STAT_st->NoSTAT

	// Crit_radscorpion.Leg_Right.Effect2.FailureEffect
	SetEntry(6,4,1,4,2); // 0->DAM_KNOCKED_DOWN

	// Crit_radscorpion.Leg_Left.Effect2.FailureEffect
	SetEntry(6,5,1,4,2); // 0->DAM_KNOCKED_DOWN

	// Crit_radscorpion.Leg_Left.Effect3.FailMessage
	SetEntry(6,5,2,6,5608); // 5008->5608

	// Crit_centaur.Torso.Effect4.FailureEffect
	SetEntry(9,3,3,4,2); // 0->DAM_KNOCKED_DOWN

	// Crit_deathclaw.Leg_Left
	SetEntry(13,5,1,4,4); // Effect2.FailureEffect, DAM_CRIP_LEG_RIGHT->DAM_CRIP_LEG_LEFT
	SetEntry(13,5,2,4,4); // Effect3.FailureEffect, DAM_CRIP_LEG_RIGHT->DAM_CRIP_LEG_LEFT
	SetEntry(13,5,3,4,4); // Effect4.FailureEffect, DAM_CRIP_LEG_RIGHT->DAM_CRIP_LEG_LEFT
	SetEntry(13,5,4,4,4); // Effect5.FailureEffect, DAM_CRIP_LEG_RIGHT->DAM_CRIP_LEG_LEFT
	SetEntry(13,5,5,4,4); // Effect6.FailureEffect, DAM_CRIP_LEG_RIGHT->DAM_CRIP_LEG_LEFT
}
