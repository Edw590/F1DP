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

#ifndef F1DPPATCHER_GLOBALEXEADDRS_H
#define F1DPPATCHER_GLOBALEXEADDRS_H



// Special 32-bit numbers ("SN") that are not supposed to appear ANYwhere else in the entire code copied to the BIN
// file. They're weird enough for me to think they won't appear. (Too much coincidence?)
#define SN_CODE_SEC_EXE_ADDR 0x79563412 // 12 34 56 79 in little endian
#define SN_DATA_SEC_EXE_ADDR 0x77563412 // 12 34 56 77 in little endian
// ATTENTION: these are NOT all the ones that exist. Seach for "SN_" on the entire project files to see the others. Most
// of them will be in the PatchPatcher.



#endif //F1DPPATCHER_GLOBALEXEADDRS_H
