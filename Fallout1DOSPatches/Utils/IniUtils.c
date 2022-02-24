//
// Created by DADi590 on 21/02/2022.
//

#include "../CFuncs/fcntl.h"
#include "../CFuncs/io.h"
#include "../CFuncs/stdio.h"
#include "../CFuncs/stdlib.h"
#include "../CFuncs/string.h"
#include "../CFuncs/unistd.h"
#include "BlockAddrUtils.h"
#include "IniUtils.h"

#define PRIM_INI_FILE "dospatch.ini"
#define SEC_INI_FILE "ddraw.ini"

/**
 * @brief Reads an INI file to the specified buffer.
 *
 * @param file_path the path to the file to read
 * @param file a struct pointer for its values to be assigned internally. Only upon successful return are its
 * contents modified.
 *
 * @return a struct with all attributes set, or one with none set (all attributes set to default values) if an error
 * occurred
 */
bool readFile(const char *file_path, struct FileContents *file) {
	char * file_contents = NULL;
	unsigned long file_size = 0;
	unsigned file_descriptor = 0;
	long temp = 0;

	// Pointer correction
	file_path = getRealBlockAddrData(file_path);
	file = getRealBlockAddrData(file);

	temp = open(file_path, O_RDONLY | O_TEXT | O_EXCL, S_IRUSR);
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

	if (-1 == read((int) file_descriptor, file_contents, (size_t) file_size)) {
		freeNew(file_contents);
		close((int) file_descriptor);

		return false;
	}

	close((int) file_descriptor);

	file->contents = file_contents;
	file->size = file_size;

	return true;
}

/**
 * @brief Looks for a property in the INI_FILE_NAME file.
 *
 * @param name the property name
 * @param value a buffer where to put the property's value
 *
 * @return true if the property was found with a value on it, false otherwise
 */
bool getPropValueIni(const char * ini_contents, unsigned long ini_len, const char * prop_name, char * prop_value) {
	unsigned long i = 0;
	char line[MAX_LINE_LEN];
	memset(line, 0, MAX_LINE_LEN);

	// todo Missing the section search ("Main", "Misc", "Sound"...). There can be repeated properties in different
	//  sections

	// Pointer correction
	ini_contents = getRealBlockAddrData(ini_contents);
	prop_name = getRealBlockAddrData(prop_name);
	prop_value = getRealBlockAddrData(prop_value);

	// Terminate the string before looking for it, so if it's not found, an empty string is returned.
	for (i = 0; i < ini_len; /*Nothing*/) {
		int j = 0;
		bool comment_line = false;

		{
			// ////////////////////////
			// Line reader
			bool first_decent_char_passed = false;

			while ((i < ini_len) && ('\n' != ini_contents[i])) {
				if (('\r' != ini_contents[i]) && (j < (MAX_LINE_LEN - 1))) {
					// If \r is present, it always comes before \n (\r\n - CR LF).
					// Also, if the line in the file is bigger than the maximum allowed line length here, ignore everything
					// that comes after the maximum length, but get to the end of the line on the file.
					if ((!first_decent_char_passed) && (' ' != line[j]) && ('\t' != line[j])) {
						// Nothing here
					} else {
						first_decent_char_passed = true;
					}
					line[j] = ini_contents[i];
					++j;
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
			if (index_equals_sign >= 0) {
				int k = 0;
				int prop_name_temp_len = 0;
				char prop_name_temp[MAX_PROP_NAME_LEN];
				char * prop_name_temp_real = NULL;
				memset(prop_name_temp, 0, MAX_PROP_NAME_LEN);

				strncpy(prop_name_temp, line, (size_t) index_equals_sign);
				prop_name_temp[index_equals_sign] = '\0';
				prop_name_temp_len = (int) strnlen(prop_name_temp, MAX_PROP_NAME_LEN);

				// //////////////////////////
				// Ignore the spaces before and after the property name
				for (k = 0; k < prop_name_temp_len; ++k) {
					if ((' ' != prop_name_temp[k]) && ('\t' != prop_name_temp[k])) {
						prop_name_temp_real = prop_name_temp + k;
						break;
					}
				}
				for (k = (prop_name_temp_len - 1); k >= 0; --k) {
					if ((' ' != prop_name_temp[k]) && ('\t' != prop_name_temp[k])) {
						prop_name_temp[k + 1] = '\0'; // End the string on first whitespace
						break;
					}
				}
				if (NULL == prop_name_temp_real) {
					// For any reason, no idea. Maybe if the line is empty or the key has no name and there's just an
					// equals sign? Didn't check, but good to listen to the warnings (in this case NULL dereference), I
					// guess.
					continue;
				}
				// //////////////////////////

				if (0 == strncmp(prop_name_temp_real, prop_name, MAX_PROP_NAME_LEN)) {
					char prop_value_temp[MAX_PROP_VALUE_LEN];
					size_t prop_value_len = 0;
					int prop_value_real_len = 0;
					int index_prop_value_real_begin = 0;

					// Below, -1 from the equals sign
					prop_value_len = strnlen(line, MAX_LINE_LEN) - strnlen(prop_name_temp_real, MAX_PROP_NAME_LEN) - 1;
					strncpy(prop_value_temp, &line[index_equals_sign + 1], prop_value_len);
					prop_value_temp[prop_value_len] = '\0';

					// //////////////////////////
					// Ignore the spaces before and after the property value
					for (k = 0; k < prop_name_temp_len; ++k) {
						if ((' ' != prop_value_temp[k]) && ('\t' != prop_value_temp[k])) {
							index_prop_value_real_begin = k;
							break;
						}
					}
					for (k = (prop_name_temp_len - 1); k >= 0; --k) {
						if ((' ' != prop_value_temp[k]) && ('\t' != prop_value_temp[k])) {
							prop_value_real_len = (k + 1) - index_prop_value_real_begin;
							prop_value[k + 1] = '\0'; // End the string on first whitespace
							break;
						}
					}
					// //////////////////////////

					if (prop_value_real_len > 0) {
						strncpy(prop_value, &prop_value_temp[index_prop_value_real_begin], (size_t) prop_value_real_len);

						return true;
					}

					return false;
				}
			}
		}
	}

	return false;
}
