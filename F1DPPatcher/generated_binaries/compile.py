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

# For releases
exit(os.system("wcl386.exe " + all_C_files_ready + " -bt=DOS -l=dos4g -fpd -mf -oi -ol+ -oneatx -ot -wcd=119 -we -wo -wx -ze"))
# For testing (no optimizations, so errors come up decently, as already happened an important crash not happening
# with optimizations, but happen without them, so disable them for debug and testing and only enable for releases (or
# not even that, doesn't really matter in such a small program).
#exit(os.system("wcl386.exe " + all_C_files_ready + " -bt=DOS -l=dos4g -fpd -mf -od -wcd=119 -we -wo -wx -ze"))

# Example of Warning 119: "Warning! W119: Address of static function '[something]' has been taken". The program needs the
# functions static and it's needed to get their address for the game EXE. So ignore the warning.
