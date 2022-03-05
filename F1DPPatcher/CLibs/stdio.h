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

#ifndef F1DPPATCHER_STDIO_H
#define F1DPPATCHER_STDIO_H



// 4 chars
#define LOGGER_STR "  - "
// 8 chars
#define LOGGER_ERR_STR "  - [X] "

__declspec(naked) int printf(const char * format, ...);
__declspec(naked) int sscanf(const char* s, const char *format, ...);

// //////////////////////////////////////////////////////////////
// Non-standard functions

// Print a string and add newline characters to it (for one without them, use printf)
int printlnStr(const char * string);

// Use exactly like printf. Will only print if the logger is enabled on the INI like the other log functions.
__declspec(naked) int logf(const char *format, ...);
// Log a string (if the logger is enabled on the INI) and add newline characters to it
int loglnStr(const char *string);



#endif //F1DPPATCHER_STDIO_H
