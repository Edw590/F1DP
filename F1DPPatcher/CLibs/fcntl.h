// Copyright 2022 Edw590
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

#ifndef F1DPPATCHER_FCNTL_H
#define F1DPPATCHER_FCNTL_H


#define O_CREAT 0x0020
#define O_WRONLY 0x0001
#define O_APPEND 0x0010
#define O_RDONLY 0x0000
#define O_BINARY 0x0200
#define O_EXCL 0x0400

#define S_IRUSR 000400
#define S_IWUSR 000200

int open(char const * path, int oflag, int mode);


#endif //F1DPPATCHER_FCNTL_H
