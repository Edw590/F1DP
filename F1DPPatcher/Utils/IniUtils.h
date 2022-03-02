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

#ifndef F1DPPATCHER_INIUTILS_H
#define F1DPPATCHER_INIUTILS_H


#include <stdbool.h>

#define MAX_LINE_LEN 1000
#define MAX_SEC_NAME_LEN 1000
#define MAX_PROP_KEY_LEN 100
#define MAX_PROP_VALUE_LEN 900

#define F1DP_INI_SPEC_SEC_MAIN "F1DP-Main"
#define F1DP_INI_SPEC_SEC_SFALL1 "F1DP-sFall1"

#define F1DP_MAIN_INI "dospatch.ini"

struct FileInfo {
	char *contents;
	unsigned long size;
};

/**
 * @brief Reads an INI file to the specified buffer.
 *
 * @param file_path the path to the file to read
 * @param file a struct pointer for its values to be assigned internally - only upon successful return are its
 * contents modified.
 *
 * @return true if everything went alright, false otherwise
 */
bool readFile(const char *file_path, struct FileInfo *file);

/**
 * @brief Looks for a property value in an INI file given a key, possibly a given section name, and possible a given
 * Special Section name (which are sections specifically for Fallout 1 DOS Patcher, which can contain normal INI
 * sections).
 *
 * It ignores any whitespaces before and after the key and value. It does NOT ignore whitespaces between other
 * characters (like "value 1   		=   my name       3   	", the key is "value 1" and the value is "my name       3").
 * It also detects inline-comments and ignores them, and is able to search by sections (which can contain ; and =, but
 * it's not recommended as that's not supposed to be there and I'm only allowing it because I don't want to be more time
 * coding this parser...).
 * Finally, it is not case sensitive on the section (special or not) and property keys. The property values will be
 * returned exactly as written on the file.
 * Special Sections start with "[>F1DP-" and end with "<]", and have the same characteristics as Sections do.
 *
 * @param ini_info the information of the INI file where to search the value in
 * @param prop_spec_section_name the name of the special section where the given key is, or NULL to ignore
 * @param prop_section_name the name of the section where the given key is, or NULL to ignore
 * @param prop_key the name of the key to get the value from
 * @param def_value a string to copy to *prop_value* in case the key is not found, or NULL to don't touch in
 * *prop_value* in that case
 * @param prop_value an array with enough space to copy the found value to (which is only modified if this function
 * returns true - will mean the found value is on the array)
 *
 * @return true if the property was found with a value on it, false otherwise
 */
bool getPropValueIni(struct FileInfo ini_info, const char *prop_spec_section_name, const char *prop_section_name,
					 const char *prop_key, const char *def_value, char *prop_value);



#endif //F1DPPATCHER_INIUTILS_H
