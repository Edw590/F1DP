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

#include "../CLibs/ctype.h"
#include "../CLibs/fcntl.h"
#include "../CLibs/io.h"
#include "../CLibs/stdlib.h"
#include "../CLibs/string.h"
#include "../CLibs/unistd.h"
#include "General.h"
#include "BlockAddrUtils.h"
#include "IniUtils.h"

bool readFile(char const *file_path, struct FileInfo *file) {
	char *file_contents = NULL;
	unsigned long file_size = 0;
	unsigned file_descriptor = 0;
	long temp = 0;

	// Pointer correction
	file_path = getRealBlockAddrData(file_path);
	file = getRealBlockAddrData(file);

	if (NULL == file_path) {
		return false;
	}

	temp = open(file_path, O_RDONLY | O_BINARY | O_EXCL, S_IRUSR);
	if (temp < 0) {
		return false;
	}
	file_descriptor = (unsigned long) temp; // temp is >= 0 here

	temp = filelength((int) file_descriptor);
	if (temp < 0) {
		close((int) file_descriptor);

		return false;
	}
	file_size = (unsigned) temp; // temp is >= 0 here

	file_contents = malloc(file_size * sizeof(*file_contents));

	if (temp != read((int) file_descriptor, file_contents, (size_t) file_size)) {
		free(file_contents);
		close((int) file_descriptor);

		return false;
	}
	close((int) file_descriptor);

	file->contents = file_contents;
	file->size = file_size;

	return true;
}

bool getPropValueIni(char const *prop_spec_section_name, char const *prop_section_name, char const *prop_key,
					 char const *def_value, char *prop_value, struct FileInfo const *ini_info) {
	unsigned long i = 0;
	size_t temp = 0;
	char const *ini_contents = NULL;
	unsigned long ini_len = 0;
	char curr_section_name[MAX_SEC_NAME_LEN];
	char curr_spec_section_name[MAX_SEC_NAME_LEN];
	char line[MAX_LINE_LEN];
	char prop_spec_section_name_lower[MAX_SEC_NAME_LEN];
	char prop_section_name_lower[MAX_SEC_NAME_LEN];
	char prop_key_lower[MAX_PROP_KEY_LEN];

	memset(curr_section_name, 0, MAX_SEC_NAME_LEN);
	memset(curr_spec_section_name, 0, MAX_SEC_NAME_LEN);
	memset(line, 0, MAX_LINE_LEN);
	memset(prop_spec_section_name_lower, 0, MAX_SEC_NAME_LEN);
	memset(prop_section_name_lower, 0, MAX_SEC_NAME_LEN);
	memset(prop_key_lower, 0, MAX_PROP_KEY_LEN);

	//
	// WARNING: This function is a mess. I'm sorry about it. For me this is kind of boring to do, so I'll leave it like
	// this as long as it works...
	//

	// Pointer correction
	prop_spec_section_name = getRealBlockAddrData(prop_spec_section_name);
	prop_section_name = getRealBlockAddrData(prop_section_name);
	prop_key = getRealBlockAddrData(prop_key);
	def_value = getRealBlockAddrData(def_value);
	prop_value = getRealBlockAddrData(prop_value);
	ini_info = getRealBlockAddrData(ini_info);

	// A pointer to the struct intead of the struct itself is because if it is declared globally, a pointer is much
	// easier to pass then to be calling getRealBlockAddrData() each time manually (while this function does it by itself).
	ini_contents = ini_info->contents;
	ini_len = ini_info->size;

	if (!ini_info->is_main_ini) {
		// If it's not the main INI, ignore the Special Section parameter (those sections exist only in the main INI).
		prop_spec_section_name = NULL;
	}

	// Convert the given key and section names to lowercase
	if (NULL != prop_spec_section_name) {
		strcpy(prop_spec_section_name_lower, prop_spec_section_name);
		temp = strnlen(prop_spec_section_name_lower, MAX_SEC_NAME_LEN);
		for (i = 0; i < temp; ++i) {
			prop_spec_section_name_lower[i] = (char) tolower(prop_spec_section_name_lower[i]);
		}
	}
	if (NULL != prop_section_name) {
		strcpy(prop_section_name_lower, prop_section_name);
		temp = strnlen(prop_section_name_lower, MAX_SEC_NAME_LEN);
		for (i = 0; i < temp; ++i) {
			prop_section_name_lower[i] = (char) tolower(prop_section_name_lower[i]);
		}
	}
	strcpy(prop_key_lower, prop_key);
	temp = strnlen(prop_key_lower, MAX_PROP_KEY_LEN);
	for (i = 0; i < temp; ++i) {
		prop_key_lower[i] = (char) tolower(prop_key_lower[i]);
	}

	// Terminate the string before looking for it, so if it's not found, an empty string is returned.
	for (i = 0; i < ini_len; /*Nothing*/) {
		int j = 0;
		bool comment_line = false;

		{
			// ////////////////////////
			// Line reader
			bool non_init_whitespace_reached = false;
			bool semicolon_reached = false;

			// In case you think (again) that it would be cool for the loop not start if the line starts right away with
			// a semicolong, think again, because it needs to reach the end of the line with the 'i' variable...
			while ((i < ini_len) && ('\n' != ini_contents[i])) {
				if (('\r' != ini_contents[i]) && (j < (MAX_LINE_LEN - 1))) {
					// If \r is present, it always comes before \n (\r\n - CR LF).
					// Also, if the line in the file is bigger than the maximum allowed line length here, ignore everything
					// that comes after the maximum length, but get to the end of the line on the file.
					if (!non_init_whitespace_reached) {
						if ((' ' != ini_contents[i]) && ('\t' != ini_contents[i])) {
							non_init_whitespace_reached = true;
						} else {
							// Go to the next iteration until a decent character is found
						}
					}
					if (non_init_whitespace_reached && (!semicolon_reached)) {
						// After reaching the first decent char and until a semicolon is found (a comment just began),
						// keep copying things
						if (';' == ini_contents[i]) {
							semicolon_reached = true;
						}
						if (!semicolon_reached) {
							line[j] = ini_contents[i];
							++j;
						}
					}
				}
				++i;
			}
			line[j] = '\0'; // The last character is a NULL byte
			++i; // So that next time, it begins with the character after \n
			// ////////////////////////
		}

		{
			int k = 0;
			// Ignore spaces and tabs and check if a semicolon is the first character after those 2
			for (k = 0; k < j; ++k) {
				if ((' ' != line[k]) && ('\t' != line[k])) {
					if (';' == line[k]) {
						comment_line = true;
					} else {
						comment_line = false;
					}
					break;
				}
			}
		}

		if (!comment_line) {
			int index_equals_sign = strrchr(line, '=') - line; // Subtraction of addresses to get the index
			if ((j >= 2) && ('[' == line[0]) && (']' == line[j-1])) { // (j >= 2) == 2 characters at least ("[]")
				// Check if the first characters of the section name are the ones for the Special Sections names.
				// If not, then it's a normal Section. Also, check only the first 7 chars (the string chars) and not the
				// entire string with the NULL character (8 chars or strcmp), because that would mean it would end right
				// there and it doesn't - it's just a prefix.
				if ((j >= 9) && (0 == strncmp(line, "[>F1DP-", 7)) && ('<' == line[j-2])) { // (j >= 9) == 9 characters at least ("[>F1DP-<]")
					int k = 0;
					int length = (j-2)-2; // The -2s is for the 2 ignored characters "[>" and "<]"
					strncpy(curr_spec_section_name, &line[2], (size_t) length);
					curr_spec_section_name[length] = '\0';

					// Convert the name to lowercase
					for (k = 0; k < length; ++k) {
						curr_spec_section_name[k] = (char) tolower(curr_spec_section_name[k]);
					}
				} else {
					int k = 0;
					int length = (j-1)-1; // The -1s is for the 2 ignored characters "[" and "]"
					strncpy(curr_section_name, &line[1], (size_t) length);
					curr_section_name[length] = '\0';

					// Convert the name to lowercase
					for (k = 0; k < length; ++k) {
						curr_section_name[k] = (char) tolower(curr_section_name[k]);
					}
				}
			} else if (index_equals_sign >= 0) {
				int k = 0;
				int prop_key_temp_len = 0;
				char prop_key_temp[MAX_PROP_KEY_LEN];
				char *prop_key_temp_real = NULL; // This will be a pointer to the middle of prop_key_temp
				memset(prop_key_temp, 0, MAX_PROP_KEY_LEN);

				strncpy(prop_key_temp, line, (size_t) index_equals_sign);
				prop_key_temp[index_equals_sign] = '\0';
				prop_key_temp_len = (int) strnlen(prop_key_temp, MAX_PROP_KEY_LEN);

				// //////////////////////////
				// Ignore the spaces before and after the property name
				for (k = 0; k < prop_key_temp_len; ++k) {
					if ((' ' != prop_key_temp[k]) && ('\t' != prop_key_temp[k])) {
						prop_key_temp_real = prop_key_temp + k;
						break;
					}
				}
				for (k = (prop_key_temp_len - 1); k >= 0; --k) {
					// The \0 is in case there are NULL characters in the end of the string (there can be if there's
					// nothing after it - like if there is no = and no value) - already happened with the value, so I
					// figured here could happen too.
					if ((' ' != prop_key_temp[k]) && ('\t' != prop_key_temp[k]) && ('\0' != prop_key_temp[k])) {
						// End the string on first whitespace
						// We're changing prop_key_temp and not prop_key_temp_real, but don't forget prop_key_temp is
						// a pointer to the middle of prop_key_temp, so changing prop_key_temp, changes
						// prop_key_temp_real, as long as the changes overlap (which they will - we're changing the
						// end of the string).
						prop_key_temp[k + 1] = '\0';
						break;
					}
				}
				if (NULL == prop_key_temp_real) {
					// For any reason, no idea. Maybe if the line is empty or the key has no name and there's just an
					// equals sign? Didn't check, but good to listen to the warnings (in this case NULL dereference), I
					// guess.
					continue;
				}
				// Convert the name to lowercase
				temp = strnlen(prop_key_temp_real, MAX_PROP_KEY_LEN);
				for (k = 0; k < (int) temp; ++k) {
					prop_key_temp_real[k] = (char) tolower(prop_key_temp_real[k]);
				}
				// //////////////////////////

				// Check or not if the Special Section names match.
				if ((NULL != prop_spec_section_name) && (0 != strncmp(curr_spec_section_name,
																	  prop_spec_section_name_lower, MAX_SEC_NAME_LEN))) {
					continue;
				}
				// Check or not if the section names match.
				if ((NULL != prop_section_name) && (0 != strncmp(curr_section_name, prop_section_name_lower,
																 MAX_SEC_NAME_LEN))) {
					continue;
				}

				// Compare the new property name (a pointer to the original but with the beginning changed and the end
				// changed as well as it did on the original one - which didn't have the beginning changed though).
				if (0 == strncmp(prop_key_temp_real, prop_key_lower, MAX_PROP_KEY_LEN)) {
					char prop_value_temp[MAX_PROP_VALUE_LEN];
					size_t prop_value_len = 0;
					int prop_value_real_len = 0;
					int index_prop_value_real_begin = 0;

					// Below, -1 from the equals sign
					prop_value_len = strnlen(line, MAX_LINE_LEN) - strnlen(prop_key_temp_real, MAX_PROP_KEY_LEN) - 1;
					strncpy(prop_value_temp, &line[index_equals_sign + 1], prop_value_len);
					prop_value_temp[prop_value_len] = '\0';

					// //////////////////////////
					// Ignore the spaces before and after the property value
					for (k = 0; k < (int) prop_value_len; ++k) {
						if ((' ' != prop_value_temp[k]) && ('\t' != prop_value_temp[k])) {
							index_prop_value_real_begin = k;
							break;
						}
					}
					for (k = ((int) prop_value_len - 1); k >= 0; --k) {
						if ((' ' != prop_value_temp[k]) && ('\t' != prop_value_temp[k]) && ('\0' != prop_value_temp[k])) {
							// On the line below, the +1 accounts for the NULL character (so that strnlen can copy it).
							prop_value_real_len = (k + 1) - index_prop_value_real_begin;
							prop_value_temp[k + 1] = '\0'; // End the string on first whitespace
							break;
						}
					}
					// //////////////////////////

					if (prop_value_real_len > 0) {
						// Copy the string with the new beginning and the new limit (by where the new \0 was put, no
						// matter any others that was already there). +1 for the \0.
						strncpy(prop_value, &prop_value_temp[index_prop_value_real_begin], (size_t) prop_value_real_len + 1);
					} else {
						strcpy(prop_value, "");
					}

					return true;
				}
			} else {
				// It's not a section nor has an equals sign, ignore the line.
			}
		}
	}

	if (NULL != def_value) {
		// Copy the default value to the property value in case it's not NULL and in case the key was not found on the
		// INI.
		strcpy(prop_value, def_value);
	}

	return false;
}
