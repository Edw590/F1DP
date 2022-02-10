package main

import (
	"crypto/md5"
	"fmt"
	"os"
)

func showInitialScreen() {
	fmt.Println("------------------------------------")
	fmt.Println("Fallout 1 DOS EXE patcher by DADi590")
	fmt.Println("------------------------------------")
	fmt.Println()
	fmt.Println("-----Credits-----")
	fmt.Println()
	fmt.Println("This patcher was made by me, but the patches are not mine. Credits for all the patches go to the " +
				"creator of the original sFall, Timeslip, and the creator of the sFall1 modification, Crafty (the one I'm " +
				"using here)")
	fmt.Println()
	fmt.Println("Also a thanks for Sduibek because it seems I used a repository with commits from him to improve some " +
				"minor things, so a thank you him too. And because of this, another thanks to Aramatheus for forking a copy of " +
				"the deleted repository.")
	fmt.Println()
	fmt.Println("Link for sFall by Timeslip: http://timeslip.chorrol.com/sfallchangelog.html")
	fmt.Println("Link for Crafty's sFall1 thread: http://fforum.kochegarov.com/index.php?showtopic=29288")
	fmt.Println()
	fmt.Println("Credits completed, now press ENTER to go to the patcher...")
	_, err := fmt.Scanln()
	if err != nil {}

	fmt.Println("------------------------------------------------------------------------")
	fmt.Println("------------------------------------------------------------------------")
	fmt.Println("------------------------------------------------------------------------")
	fmt.Println()
}

const ERR_NONE int = 0
const ERR_MISSING_PARAM int = 1
const ERR_FILE_NOT_FOUND int = 2
const ERR_WRONG_MD5 int = 3
const ERR_CANT_WRITE_FILE int = 4
const ERR_WRONG_MD5_ADDIT_PATCHES int = 5

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
	var initial_offset int = 0
	var new_bytes []byte = nil

	var file_md5 string = fmt.Sprintf("%X", md5.Sum(file_bytes))
	fmt.Println("File path: \"" + file_path + "\"")
	fmt.Println("File MD5 hash: " + file_md5)
	fmt.Println("The file must be one of the following:")
	fmt.Println("- Fallout 1 DOS EXE v1.1 official; MD5 hash: 6A41C641B789B44FD7BE1805CE030C9B")
	fmt.Println("- Fallout 1 DOS EXE v1.2 semi-official by TeamX; MD5 hash: 3DCF41FA6784030BD5C71BE81954C899")
	fmt.Println()

	new_bytes = nil
	initial_offset = 0
	if "6A41C641B789B44FD7BE1805CE030C9B" == file_md5 {
		fmt.Println("The file is supported. Patching to new file...")
		initial_offset = 0x71A90
		new_bytes = []byte{0xE8, 0x3B, 0x53, 0x7, 0x0, 0xE9, 0xF4, 0x0, 0x0, 0x0, 0x90, 0x90, 0x90, 0x90}
	} else if "3DCF41FA6784030BD5C71BE81954C899" == file_md5 {
		fmt.Println("The file is supported. Patching to new file...")
		initial_offset = 0x71D00
		new_bytes = []byte{0xE8, 0x7B, 0x53, 0x7, 0x0, 0xE9, 0xF4, 0x0, 0x0, 0x0, 0x90, 0x90, 0x90, 0x90}
	} else {
		fmt.Println("File MD5 is NOT supported. No operation was done.")

		exit(ERR_WRONG_MD5)
	}
	fmt.Println()

	patchBytes(file_bytes, initial_offset, new_bytes)

	// todo Missing to put this checking if the patch was already applied or not

	fmt.Println("------------------")
	fmt.Println("Additional patches")
	fmt.Println("ATTENTION: Currently (at least), this patcher only applies these patches to the Fallout 1 DOS EXE " +
		"v1.2 semi-official by TeamX.")
	fmt.Println()
	if additionalPatches(file_bytes, file_md5) {
		fmt.Println("\nMD5 matched! Additional patches successfully applied!")
	} else {
		fmt.Println("\nMD5 NOT matched! Additional patches NOT applied!")

		exit(ERR_WRONG_MD5_ADDIT_PATCHES)
	}
	fmt.Println("------------------")
	fmt.Println()

	var new_file_path string = file_path + "_patched.exe"
	err = os.WriteFile(new_file_path, file_bytes, 0644)
	if nil != err {
		fmt.Println("Error - Could not write to and/or create the patched file.")

		exit(ERR_CANT_WRITE_FILE)
	} else {
		fmt.Println("Patch completed successfully! New file: \"" + new_file_path + "\".")
	}
	fmt.Println()

	fmt.Println("----------")
	fmt.Println("ATTENTION: Read ALL the console before you close it. I may put warnings on it and just did on " +
				"the 'Loot/Drop all' patch at the moment I'm writing this.")
	fmt.Println("----------")
	fmt.Println()

	exit(ERR_NONE)
}

func additionalPatches(file_bytes []byte, file_md5 string) bool {
	if "3DCF41FA6784030BD5C71BE81954C899" != file_md5 {
		// Only the Fallout 1 DOS EXE v1.2 semi-official by TeamX. Else I need to be recalculating everything for the
		// other EXE too --> not happening, at least so soon. One already takes infinity.

		return false
	}

	// todo Don't forget of putting each patch optional (figure that out with the MD5, maybe something saying
	// it's one of your versions?). And be careful because some patches might need others (like that sub WeightSomething
	// on the Loot/Drop all patch - so it would be dependent on some other patch if you had not removed that instruction)


	var initial_offset int = 0
	var new_bytes []byte = nil

	/////////////////////////////////////////////
	fmt.Println("-----")
	fmt.Println("Necessary patches for the other patches")
	fmt.Println("-----")


	///////////////////////
	fmt.Println("- Extend the code segment to its maximum size") // (required for the patches)
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
	fmt.Println("- Rename the code and data segments to cseg and dseg (not necessary, but why not? xD)") // (they had no name)
	initial_offset = 0x2D68
	new_bytes = []byte{0x63, 0x73, 0x65, 0x67}
	patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x2D80
	new_bytes = []byte{0x64, 0x73, 0x65, 0x67}
	patchBytes(file_bytes, initial_offset, new_bytes)


	///////////////////////
	fmt.Println("- Change the requirement of DOS/4G to DOS/32 (fixes annoying non-critical errors, aside from being faster)") // Fixes errors with the 'Loot/Drop all' patch at minimum
	initial_offset = 0x28D9
	new_bytes = []byte{0x33, 0x32, 0x61}
	patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12D8E0
	new_bytes = []byte{0x2F, 0x33, 0x32}
	patchBytes(file_bytes, initial_offset, new_bytes)
	// "DOS4GPATH" not patched on purpose in case anyone might want to use that, and also because probably that's the
	// environment variable used on documentations and manuals, so maybe better to leave it alone, I guess.


	/////////////////////////////////////////////
	fmt.Println("-----")
	fmt.Println("sFall1 1.20 fixes")
	fmt.Println("-----")


	///////////////////////
	fmt.Println("- Dialogue Fix")
	initial_offset = 0x72FA6
	new_bytes = []byte{0x31}
	patchBytes(file_bytes, initial_offset, new_bytes)


	///////////////////////
	fmt.Println("- Critical hit tables bug fixes")
	// The comments below are the same from the sFall1 1.20 source

	// Crit_children.Leg_Right.Effect2
	initial_offset = 0x12ED44; new_bytes = []byte{0x00, 0x00}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12ED48; new_bytes = []byte{0x60, 0x14}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12ED4C; new_bytes = []byte{0x88, 0x13}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_children.Leg_Right.Effect3
	initial_offset = 0x12ED60; new_bytes = []byte{0x00, 0x00}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12ED64; new_bytes = []byte{0x60, 0x14}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12ED68; new_bytes = []byte{0x88, 0x13}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_children.Leg_Left.Effect2
	initial_offset = 0x12EDEC; new_bytes = []byte{0x00, 0x00}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12EDF0; new_bytes = []byte{0x60, 0x14}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12EDF4; new_bytes = []byte{0x88, 0x13}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_children.Leg_Left.Effect3
	initial_offset = 0x12EE08; new_bytes = []byte{0x00, 0x00}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12EE0C; new_bytes = []byte{0x60, 0x14}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x12EE10; new_bytes = []byte{0x88, 0x13}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_super_mutant.Leg_Left.Effect2.FailMessage
	initial_offset = 0x12F3DC; new_bytes = []byte{0xBA}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_ghoul.Head.Effect5.StatCheck
	initial_offset = 0x12F6C0; new_bytes = []byte{0xFF, 0xFF, 0xFF, 0xFF}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_brahmin.Head.Effect5.StatCheck
	initial_offset = 0x12FCA8; new_bytes = []byte{0xFF, 0xFF, 0xFF, 0xFF}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_radscorpion.Leg_Right.Effect2.FailureEffect
	initial_offset = 0x1304E4; new_bytes = []byte{0x02}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_radscorpion.Leg_Left.Effect2.FailureEffect
	initial_offset = 0x13058C; new_bytes = []byte{0x02}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_radscorpion.Leg_Left.Effect3.FailMessage
	initial_offset = 0x1305B0; new_bytes = []byte{0xE8, 0x15}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_centaur.Torso.Effect4.FailureEffect
	initial_offset = 0x13162C; new_bytes = []byte{0x02}; patchBytes(file_bytes, initial_offset, new_bytes)

	// Crit_deathclaw.Leg_Left
	initial_offset = 0x132EE4; new_bytes = []byte{0x04}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x132F00; new_bytes = []byte{0x04}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x132F1C; new_bytes = []byte{0x04}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x132F38; new_bytes = []byte{0x04}; patchBytes(file_bytes, initial_offset, new_bytes)
	initial_offset = 0x132F54; new_bytes = []byte{0x04}; patchBytes(file_bytes, initial_offset, new_bytes)


	///////////////////////
	fmt.Println("- Dodgy Doors Fix")
	// Insert a CALL
	initial_offset = 0x452E5
	new_bytes = []byte{0x90, 0x90, 0xE8, 0x32, 0x90, 0x0D, 0x00}
	patchBytes(file_bytes, initial_offset, new_bytes)
	// DodgyDoorsFix()
	initial_offset = 0x11E31E
	new_bytes = []byte{0x8B, 0x45, 0x20, 0x8B, 0x58, 0x20, 0x81, 0xE3, 0x00, 0x00, 0x00, 0x0F, 0xC1, 0xFB, 0x18, 0x83,
		0xFB, 0x01, 0x75, 0x04, 0xF6, 0x40, 0x44, 0x03, 0xC3}
	patchBytes(file_bytes, initial_offset, new_bytes)


	///////////////////////
	fmt.Println("- Loot/Drop all")
	fmt.Println("  NOTICE 1: click 'A' to loot everything and 'D' to drop everything when exchanging inventories. I " +
		"didn't include the buttons, at least yet.")
	fmt.Println("  NOTICE 2: it won't display the error messages for some reason I don't understand. The one for the " +
		"'A' key says that you cannot carry that much, and the one for the 'D' key says there is no space left.")
	// todo Missing creating the buttons here
	// Insert a JMP
	initial_offset = 0x9B2C1
	new_bytes = []byte{0xE9, 0x71, 0x30, 0x08, 0x00}
	patchBytes(file_bytes, initial_offset, new_bytes)
	// loot_drop_all_()
	initial_offset = 0x11E337
	new_bytes = []byte{0x83, 0xF8, 0x41, 0x74, 0x19, 0x83, 0xF8, 0x61, 0x74, 0x14, 0x83, 0xF8, 0x44, 0x74, 0x69, 0x83,
		0xF8, 0x64, 0x74, 0x64, 0x3D, 0x48, 0x01, 0x00, 0x00, 0xE9, 0x71, 0xCF, 0xF7, 0xFF, 0x60, 0x83, 0x3D, 0xE0,
		0x7E, 0x10, 0x00, 0x00, 0x0F, 0x85, 0x62, 0x01, 0x00, 0x00, 0x8B, 0x8C, 0x24, 0x2C, 0x01, 0x00, 0x00, 0x89,
		0xC8, 0xE8, 0x07, 0xE4, 0xF3, 0xFF, 0x85, 0xC0, 0x0F, 0x85, 0x4C, 0x01, 0x00, 0x00, 0xBA, 0x0C, 0x00, 0x00,
		0x00, 0x89, 0xC8, 0xE8, 0x63, 0x21, 0xFB, 0xFF, 0x92, 0x89, 0xC8, 0xE8, 0x2B, 0x06, 0xF8, 0xFF, 0x29, 0xC2,
		0x89, 0xE8, 0xE8, 0x22, 0x06, 0xF8, 0xFF, 0x39, 0xD0, 0x7F, 0x09, 0x89, 0xCA, 0x89, 0xE8, 0xE9, 0xAD, 0x00,
		0x00, 0x00, 0x2E, 0x8B, 0x15, 0xCF, 0xAF, 0x0E, 0x00, 0xE9, 0xF0, 0x00, 0x00, 0x00, 0x60, 0x83, 0x3D, 0xE0,
		0x7E, 0x10, 0x00, 0x00, 0x0F, 0x85, 0x08, 0x01, 0x00, 0x00, 0x8B, 0x8C, 0x24, 0x2C, 0x01, 0x00, 0x00, 0x89,
		0xC8, 0xE8, 0xAD, 0xE3, 0xF3, 0xFF, 0x85, 0xC0, 0x0F, 0x85, 0xF2, 0x00, 0x00, 0x00, 0x8B, 0x45, 0x20, 0x25,
		0x00, 0x00, 0x00, 0x0F, 0xC1, 0xF8, 0x18, 0x85, 0xC0, 0x74, 0x3E, 0x83, 0xF8, 0x01, 0x0F, 0x85, 0xDA, 0x00,
		0x00, 0x00, 0x89, 0xE8, 0xE8, 0x86, 0xE3, 0xF3, 0xFF, 0x85, 0xC0, 0x0F, 0x85, 0xCB, 0x00, 0x00, 0x00, 0xBA,
		0x0C, 0x00, 0x00, 0x00, 0x89, 0xE8, 0xE8, 0xE2, 0x20, 0xFB, 0xFF, 0x92, 0x83, 0xEA, 0x00, 0x90, 0x90, 0x90,
		0x90, 0x89, 0xE8, 0xE8, 0xA3, 0x05, 0xF8, 0xFF, 0x29, 0xC2, 0x89, 0xC8, 0xE8, 0x9A, 0x05, 0xF8, 0xFF, 0xEB,
		0x28, 0x89, 0xE8, 0xE8, 0xC1, 0x02, 0xF8, 0xFF, 0x83, 0xF8, 0x01, 0x0F, 0x85, 0x95, 0x00, 0x00, 0x00, 0x89,
		0xE8, 0xE8, 0xF9, 0x1B, 0xF8, 0xFF, 0x92, 0x89, 0xE8, 0xE8, 0x11, 0x1C, 0xF8, 0xFF, 0x29, 0xC2, 0x89, 0xC8,
		0xE8, 0x08, 0x1C, 0xF8, 0xFF, 0x39, 0xD0, 0x7F, 0x4C, 0x89, 0xEA, 0x89, 0xC8, 0x50, 0xB8, 0xB0, 0x34, 0x0F,
		0x00, 0xE8, 0xC9, 0xF0, 0xF5, 0xFF, 0x58, 0xE8, 0x1B, 0x00, 0xF8, 0xFF, 0xB9, 0x02, 0x00, 0x00, 0x00, 0x51,
		0xB8, 0x2C, 0xB8, 0x19, 0x00, 0xBA, 0xFF, 0xFF, 0xFF, 0xFF, 0x52, 0xBB, 0x24, 0xB8, 0x19, 0x00, 0x8B, 0x04,
		0x85, 0xEC, 0xB6, 0x19, 0x00, 0xE8, 0x0D, 0x9A, 0xF7, 0xFF, 0xB8, 0x18, 0xB8, 0x19, 0x00, 0x5A, 0x5B, 0x8B,
		0x04, 0x85, 0xC4, 0xB6, 0x19, 0x00, 0xE8, 0x52, 0x95, 0xF7, 0xFF, 0xEB, 0x2D, 0x2E, 0x8B, 0x15, 0x9F, 0xAF,
		0x0E, 0x00, 0xB8, 0xB0, 0x34, 0x0F, 0x00, 0xE8, 0x7B, 0xF0, 0xF5, 0xFF, 0x31, 0xC0, 0x50, 0xA0, 0x58, 0x98,
		0x2A, 0x00, 0x50, 0x31, 0xDB, 0x53, 0x50, 0xB9, 0xA9, 0x00, 0x00, 0x00, 0x6A, 0x75, 0x31, 0xC0, 0x92, 0xE8,
		0xDB, 0x1A, 0xF3, 0xFF, 0x61, 0xE9, 0x43, 0xD2, 0xF7, 0xFF}
	patchBytes(file_bytes, initial_offset, new_bytes)
	// Add the strings to the code segment (which won't be printed anyway for some reason, but at least they're there)
	initial_offset = 0x11EF9F
	new_bytes = []byte{0x53, 0x6F, 0x72, 0x72, 0x79, 0x2C, 0x20, 0x74, 0x68, 0x65, 0x72, 0x65, 0x20, 0x69, 0x73, 0x20,
		0x6E, 0x6F, 0x74, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x20, 0x6C, 0x65, 0x66, 0x74, 0x2E, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x6F, 0x72, 0x72,
		0x79, 0x2C, 0x20, 0x79, 0x6F, 0x75, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, 0x74, 0x20, 0x63, 0x61, 0x72, 0x72,
		0x79, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x6D, 0x75, 0x63, 0x68, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	patchBytes(file_bytes, initial_offset, new_bytes)


	///////////////////////
	fmt.Println("- [!] Show maximum weight in inventory (NOT applied, because of the below)")
	fmt.Println("  NOTICE: this is not working. Currently, it will do nothing good - will delete the current weight " +
		"message when you open the Inventory (it should just add the max weight, not delete things...)")
	// todo THIS DOESN'T WORK WELL!!!!! It's supposed to only add the maximum weight, but it doesn't work and even
	//  breaks the original functionality of displaying the current weight!!!
	// Hopefully some other patch is missing for this one to work well... Else, not sure why it's not working.
	// Insert a JMP
	initial_offset = 0x99D24
	new_bytes = []byte{0xE9, 0xA2, 0x47, 0x08, 0x00}
	//patchBytes(file_bytes, initial_offset, new_bytes)
	// display_stats_hook()
	initial_offset = 0x11E4CB
	new_bytes = []byte{0x50, 0xBA, 0x0C, 0x00, 0x00, 0x00, 0xE8, 0x12, 0x20, 0xFB, 0xFF, 0x5A, 0x50, 0x92, 0xE8, 0xDA,
		0x04, 0xF8, 0xFF, 0x97, 0x33, 0xC0, 0x83, 0xF8, 0x00, 0x90, 0x90, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x85, 0xC0,
		0x90, 0x90, 0xE8, 0x48, 0x02, 0xF8, 0xFF, 0x03, 0xF8, 0x57, 0x2B, 0xD7, 0x8B, 0xBC, 0x24, 0x98, 0x00, 0x00,
		0x00, 0x57, 0x68, 0x10, 0x92, 0x0F, 0x00, 0x8D, 0x44, 0x24, 0x10, 0x50, 0xE8, 0xE2, 0x29, 0xFD, 0xFF, 0x83,
		0xC4, 0x14, 0x33, 0xC0, 0xA0, 0xF0, 0x1D, 0x2A, 0x00, 0x83, 0xFA, 0x00, 0x7D, 0x05, 0xA0, 0x10, 0x96, 0x2A,
		0x00, 0xB9, 0xF3, 0x01, 0x00, 0x00, 0xBB, 0x78, 0x00, 0x00, 0x00, 0xE9, 0x22, 0xB8, 0xF7, 0xFF}
	//patchBytes(file_bytes, initial_offset, new_bytes)


	return true
}

/*
patchBytes replaces the bytes in a byte array by the given ones

-----------------------------------------------------------

> Params:

- file_bytes – the bytes slice header

- initial_offset – the file offset from where to start replacing the bytes

- new_bytes – a slice containing the bytes to replace starting on the given offset (can be as many as wanted, as long as
they don't go over the length of file_bytes


> Returns:

- nothing
*/
func patchBytes(file_bytes []byte, initial_offset int, new_bytes []byte) {
	for i, offset := 0, initial_offset; i < len(new_bytes); offset, i = offset+1, i+1 {
		file_bytes[offset] = new_bytes[i]
	}
}

func exit(exit_code int) {
	if 0 != exit_code {
		fmt.Println("ATTENTION: the patcher did not complete successfully. Please check the console for more information.")
		fmt.Println("Exiting with error code:", exit_code)
	} else {
		fmt.Println("The patcher completed all operations successfully!")
	}
	fmt.Println("Press ENTER to exit...")
	_, err := fmt.Scanln()
	if err != nil {}

	os.Exit(exit_code)
}
