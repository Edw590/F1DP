/*
 * Copyright 2022 DADi590
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

package main

import (
	"crypto/md5"
	"fmt"
	"os"
)

func showInitialScreen() {
	fmt.Println("-------------------")
	fmt.Println("F1DP Installer v1.0")
	fmt.Println("-------------------")
	fmt.Println()
	fmt.Println("This program will install the F1DP Loader to the game EXE and will create a new file containing the " +
		"F1DP Patcher on it, which the Loader will call each time the game starts. After completing, this program " +
		"will be no longer necessary and you may delete it if you want.")
	fmt.Println()
	fmt.Println("-----Credits-----")
	fmt.Println("[Not sure what the order should be in this case, as it's all very important, so alphabetical order.]")
	fmt.Println()
	fmt.Println("- DADi590 (me), for making F1DP and adapting all patches to the DOS EXE")
	fmt.Println("- Crafty for his sFall1 modifications")
	fmt.Println("- Timeslip for the original sFall1 mod")
	fmt.Println("- xttl on the Doomworld forums for his posted knowledge, which helped making my life MUCH easier " +
		"while trying to figure out how to get the patcher to work on DOS")
	fmt.Println()
	fmt.Println("Link for sFall by Timeslip: http://timeslip.chorrol.com/sfallchangelog.html")
	fmt.Println("Link for Crafty's sFall1 thread: http://fforum.kochegarov.com/index.php?showtopic=29288")
	fmt.Println()
	fmt.Println("Credits completed, now press ENTER to continue to the Installer...")
	_, err := fmt.Scanln()
	if err != nil {
	}

	fmt.Println("------------------------------------------------------------------------")
	fmt.Println()
}

const ERR_NONE int = 0
const ERR_MISSING_PARAM int = 1
const ERR_FILE_NOT_FOUND int = 2
const ERR_CANT_WRITE_FILE int = 3
const ERR_WRONG_MD5_PATCHES int = 4

func main() {
	showInitialScreen()

	var argv []string = os.Args
	if 2 != len(argv) {
		fmt.Println("Missing one parameter: the path to the file to patch. Either drag the file to patch to this " +
			"program, or call it with the file path as parameter.")
		fmt.Println()

		exit(ERR_MISSING_PARAM)
	}
	var file_path = argv[1]

	file_bytes, err := os.ReadFile(file_path)
	if nil != err {
		fmt.Println("File \"" + file_path + "\" not found")

		exit(ERR_FILE_NOT_FOUND)
	}

	var file_md5 string = fmt.Sprintf("%X", md5.Sum(file_bytes))
	fmt.Println("File path: \"" + file_path + "\"")
	fmt.Println("File MD5 hash: " + file_md5)
	fmt.Println("The file must be one of the following:")
	//fmt.Println("- Fallout 1 DOS EXE v1.1 official; MD5 hash: 6A41C641B789B44FD7BE1805CE030C9B")
	fmt.Println("- Fallout 1 DOS EXE v1.2 semi-official by TeamX; MD5 hash: 3DCF41FA6784030BD5C71BE81954C899")
	fmt.Println()

	if "3DCF41FA6784030BD5C71BE81954C899" != file_md5 {
		// Only the Fallout 1 DOS EXE v1.2 semi-official by TeamX. Else I need to be recalculating everything for the
		// other EXE too --> not happening, at least so soon. One already takes infinity.

		fmt.Println("MD5 NOT matched! No operation was done. Aborting the Installer...")

		exit(ERR_WRONG_MD5_PATCHES)
	} else {
		fmt.Println("MD5 matched!")
	}
	fmt.Println("------------------")
	fmt.Println()

	prepareLoader(file_bytes)

	var new_file_path string = file_path + "_patched.exe"
	err = os.WriteFile(new_file_path, file_bytes, 0644)
	if nil != err {
		fmt.Println("Error - Could not write to and/or create the patched DOS file.")

		exit(ERR_CANT_WRITE_FILE)
	} else {
		fmt.Println("Loader installed successfully to a new file: \"" + new_file_path + "\".")
	}
	fmt.Println()

	fmt.Println("------------------")
	fmt.Println()

	//////////////////////////////////////
	// Create the BIN file

	fmt.Println("The Installer will now create the BIN file containing the Patcher in the Installer's directory.")
	fmt.Println("Please move it to the folder where the Fallout DOS program is.")

	var bin_file_path string = "dospatch.bin"
	var bin_file_bytes []byte = []byte{} // todo
	err = os.WriteFile(bin_file_path, bin_file_bytes, 0644)
	if nil != err {
		fmt.Println("Error - Could not write to and/or create the BIN file.")

		exit(ERR_CANT_WRITE_FILE)
	} else {
		fmt.Println("BIN file created: \"" + bin_file_path + "\".")
	}

	//////////////////////////////////////
	// Create the INI file

	// todo Here or on the Patcher? Would be best on the Patcher, each time the file is missing...

	exit(ERR_NONE)
}

func prepareLoader(file_bytes []byte) bool {
	var initial_offset int = 0
	var new_bytes []byte = nil

	fmt.Println("Patches made to the game EXE:")

	///////////////////////
	fmt.Println("- Mark the code section as RWX") // Data executable for the allocated block, and Code writable to patch it
	// Making them writable and executable or not is actually the same, because they already are because it works either
	// way (xtll at Doomforums thinks the LE loader ignores the flags). But always good to do it just in case.
	initial_offset = 0x2D5C
	new_bytes = []byte{0x47}
	patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x2D74
	new_bytes = []byte{0x47}
	//patchBytes(file_bytes, initial_offset, new_bytes) - Can't make Data executable, an exception will be thrown (?)

	///////////////////////
	fmt.Println("- Extend the code section to its maximum size") // (required for the patches)
	initial_offset = 0x2D54
	new_bytes = []byte{0x00, 0xB0}
	patchBytes(file_bytes, initial_offset, new_bytes)
	fmt.Println("- Mark the start and end of the extension with 2 NOPs, one in each side") // (to be well visible where
	// it's possible to write stuff and where's not)
	initial_offset = 0x11E2D9
	new_bytes = []byte{0x90}
	patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x11EFFF
	new_bytes = []byte{0x90}
	patchBytes(file_bytes, initial_offset, new_bytes)

	///////////////////////
	fmt.Println("- Give a name to the code and data sections - csec and dsec (just because yes)") // (they had no name)
	initial_offset = 0x2D68
	new_bytes = []byte{0x63, 0x73, 0x65, 0x63}
	patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x2D80
	new_bytes = []byte{0x64, 0x73, 0x65, 0x63}
	patchBytes(file_bytes, initial_offset, new_bytes)

	///////////////////////
	fmt.Println("- Redirect 2 calls on main_() to the Loader")
	// Call for the Loader to load the external code
	initial_offset = 0xA68A3
	new_bytes = []byte{0x5D, 0x7A, 0x07, 0x00}
	patchBytes(file_bytes, initial_offset, new_bytes)
	// Call for the Loader to unload the external code
	initial_offset = 0x115B8B
	new_bytes = []byte{0x4B, 0x87, 0x00, 0x00}
	patchBytes(file_bytes, initial_offset, new_bytes)

	///////////////////////
	fmt.Println("- Write RSVD at the end of the code section (reserved) for special use within the Loader")
	// Call for the Loader to load the external code
	initial_offset = 0x11EFFB
	new_bytes = []byte{0x52, 0x53, 0x56, 0x44}
	patchBytes(file_bytes, initial_offset, new_bytes)

	///////////////////////
	fmt.Println("- Install the Loader")
	initial_offset = 0x11E2DA
	new_bytes = []byte{} // todo
	patchBytes(file_bytes, initial_offset, new_bytes)

	return true
}

/*
patchBytes replaces the bytes in a byte array by the given ones

-----------------------------------------------------------

> Params:

- file_bytes – the bytes slice header

- initial_offset – the file offset from where to start replacing the bytes

- new_bytes – a slice containing the bytes to replace starting on the given offset (can be as many as wanted, as long as
they don't go over the length of 'file_bytes' - this is NOT checked here)


> Returns:

- nothing
*/
func patchBytes(file_bytes []byte, initial_offset int, new_bytes []byte) {
	for i, offset := 0, initial_offset; i < len(new_bytes); offset, i = offset+1, i+1 {
		file_bytes[offset] = new_bytes[i]
	}
}

func exit(exit_code int) {
	fmt.Println()
	if 0 != exit_code {
		fmt.Println("ATTENTION: the Installer did not complete successfully. Please check the console for more information.")
		fmt.Println("Exiting with error code:", exit_code)
	} else {
		fmt.Println("The Installer completed all operations successfully!")
	}
	fmt.Println("Press ENTER to exit...")
	_, err := fmt.Scanln()
	if err != nil {
	}

	os.Exit(exit_code)
}
