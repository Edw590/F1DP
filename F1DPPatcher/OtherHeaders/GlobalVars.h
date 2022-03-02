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

#ifndef F1DPPATCHER_GLOBALVARS_H
#define F1DPPATCHER_GLOBALVARS_H



#include "../Utils/IniUtils.h"
#include <stdbool.h>

// Don't forget EVERY variable you put here, to get it you'll have to go through getRealBlockAddrData(), so put only
// things that won't be used that much or that will be used only in one place or something.

extern bool prop_logPatcher_G;
extern struct FileInfo dospatch_ini_info_G;



#endif //F1DPPATCHER_GLOBALVARS_H
