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
print("\n----- DOS -----")
print("----- DOS -----")
print("----- DOS -----\n")
try:
	os.remove("F1DPINST.EXE")
except:
	pass
compile_err = os.system("wcl386.exe " + all_C_files_ready + " -bt=DOS -l=dos4g -fpd -mf -oi -ol+ -oneatx -ot -wcd=119 -wcd=201 -we -wo -wx -ze") == 0
try:
	os.rename("main.exe", "F1DPINST.EXE")
except:
	pass
print("\n----- NT -----")
print("----- NT -----")
print("----- NT -----\n")
try:
	os.remove("F1DPInstallerWin.exe")
except:
	pass
compile_err = compile_err and (os.system("wcl386.exe " + all_C_files_ready + " -fpd -mf -oi -ol+ -oneatx -ot -wcd=119 -wcd=201 -we -wo -wx -ze") == 0)
try:
	os.rename("main.exe", "F1DPInstallerWin.exe")
except:
	pass
# For testing (no optimizations, so errors come up decently, as already happened an important crash not happening
# with optimizations, but happen without them, so disable them for debug and testing and only enable for releases (or
# not even that, doesn't really matter in such a small program).
#exit(os.system("wcl386.exe " + all_C_files_ready + " -bt=DOS -l=dos4g -fpd -mf -od -wcd=119 -we -wo -wx -ze"))

# Example of Warning 119: "Warning! W119: Address of static function '[something]' has been taken". The program needs the
# functions static and it's needed to get their address for the game EXE. So ignore the warning.
# Warning 201 is for unreachable code. I have a macro for compiling for DOS and Windows, and it chooses which code to
# run - then of course there is unreachable code. Luckily, CLion already checked and highlights unreachable code.

print("\n")
os.system("F1DPInstallerWin.exe")
print("\n")

# 0 for no error, 1 for error (check the console for more info)
exit(not compile_err)
