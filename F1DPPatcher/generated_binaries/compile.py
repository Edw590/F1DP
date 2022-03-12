from pathlib import Path
import os

result = list(Path("../.").rglob("*.c"))

all_C_files = []
for i in result:
	all_C_files.append(str(i))

all_C_files_ready = ""
for i in all_C_files:
	if not i.startswith("..\\cmake"):
		all_C_files_ready += i + " "

print("List of files to compile:")
for i in all_C_files_ready.split():
	print("- " + i)
print()

# The exit() call below is so that this script exits with the same error code as Watcom.
exit(os.system("wcl386.exe " + all_C_files_ready + " -bt=DOS -l=dos4g -fpd -mf -oi -ol+ -oneatx -ot -wcd=119 -we -wo -wx -ze"))

# Example of Warning 119: "Warning! W119: Address of static function '[something]' has been taken". The program needs the
# functions static and it's needed to get their address for the game EXE. So ignore the warning.
#
# Warning 202 is for unreferenced symbols - if I were programming myself, yes. I'm porting and needing to have all
# variables already there ready for use when I port the code that uses them. It's just easier to have them all there and
# not be putting there only when they become needed.
