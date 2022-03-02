// Copyright 2021 DADi590
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

#ifndef F1DPPATCHER_GENERAL_H
#define F1DPPATCHER_GENERAL_H


// The Microsoft-chosen newline characters: CR LF
#define NL "\r\n"
// 80 includes everything (even the NULL character)
#define MAX_PATH_DOS 80
// 4 chars exactly or change the initial strings with the dashes
#define F1DP_VER_STR "v1.0"

#define freeNew(ptr) free(ptr); ptr = NULL



#endif //F1DPPATCHER_GENERAL_H
