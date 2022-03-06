
# Fallout 1 DOS Patcher (F1DP)
A patcher for the MS-DOS version of Fallout 1 including Crafty's sFall1 patches and maybe more

## Introduction

Hi everyone. I've decided to attempt to port all patches made to the Windows version of Fallout 1 to the MS-DOS one. Some Windows patches are DLL mods, and even if DOS recognizes DLLs with help of extenders or whatever it is, the DLL code is for the Windows version only (but anyway, I've no idea how to load and even program a DLL, at least yet). But I've went around that. This way, those who need to play with the DOS version like me for whatever reason can enjoy the patches that the Windows version players can.

This repository contains the code for everything involved with the patcher:
- the Installer, in Go (a GoLand project - this repository -, which includes inside it the below 2 things);
- the Patcher, in C and Inline-Assembly (a CLion project);
- and Loader, in Assembly (a file compiled from Sublime Text 3 using MASM 9.0 and using the compilation assistant and Sublime Text stuff from this repository of mine: https://github.com/DADi590/Chrome-Dino-Game-in-Assembly).

## Download

Download any version you'd like (preferably the most recent one) here: https://github.com/DADi590/F1DP/releases.

## History of ideas for this to work
Notes:
- The versions are in the opposite order - the first is in the end.
- They are big texts, but which explain everything that went through my head to get everything to be how it is, however that is.

<details> 
  <summary><h3>Second version</h3></summary>

Carrying on from the last thing I said on the first version, I did find out a way to go around editing the relocations table. I got the idea from @xttl user from the Doomworld forum, more precisely this thread: https://www.doomworld.com/forum/topic/86380-exe-hacking (thank you infinitely for posting your knowledge there). This was an AMAZING find and I loved the idea. The idea there is to load a BIN file with binary instructions, then jump to it and execute the instructions. The only thing permanently patched in the EXE (by editing the EXE with a hex editor) is the loader. When the game starts, it loads the patch file and executes the instructions. After that, it comes back to the game and unloads the patch from memory. Inside the patch, there could be only simple patches, like patching a number or a call to NOPs, for example. No adding new functions or strings or other things. There could not be any strings on it or global variables. Nothing outside the functions. So an idea I had first to go around having no strings was to allocate on the stack space for the characters and then assign char by char, index by index manually --> awful, but worked 😂. A char is an int (8-bit but doesn't matter for this), so it would create strings on the stack xD.

Still, I tried to think on ideas to improve that, and seems all worked! Spoiler: the patches execute like a DLL and I can patch dynamically, and go from the EXE to the patch and back to the EXE and mention strings in the patch and modify them and whatever is needed, just like is done with Crafty's sFall1 mod. Took me some work to get to this though xD. I'll just more or less copy what I wrote on the mentioned forum.

#### Patch explanation

The first 12 bytes of the BIN file are to store the version of the patch (or the type, I haven't seen too much of that yet), and code and data segments address (these last 2 are put there by the loader - the loader must also provide in a register the address of the allocated code block to store the BIN file's contents). The last 4 bytes of the 16 are for anything else I'd need. The rest is a full copy of the Fallout1DOSPatches LE EXE starting from the code segment until the end of the file --> copy and paste, just that.

The code generated by the compiler for the C project comes with absolute references, expecting to be corrected when the EXE is loaded (relocations). So I can't rely on ANY absolute references produced by the compiler. Then the idea is to patch those references before they are needed! The patch patches itself with correct values only known at run-time (reads its own memory and patches it self XD ahahah), and then does whatever it wants with the new values - like patching the EXE with CALLs and JMPs to inside the patch...!

The contents of the BIN file remain in memory so that the EXE can jump or call or read its contents wherever it wants. It can jump or call to the EXE functions, and if inside the BIN file those functions need references to the EXE, they have them all already and there's no need to pass parameters. Just do the patch normally without thinking in passing parameters specifically for segment addresses and other stuff (not even an idea I had of main function parameter to know which sub-function to execute --> the EXE just calls the functions on the BIN file, because the patch patches the EXE's jumps and calls to go to the BIN file functions).

One can also have global variables/strings and use the data segment and whatever segment is needed and the code will run just fine. I haven't made any patches to the EXE yet, but I've been testing from the call to the patch from the loader. The test function I'm using is a naked function and I pass no parameters to it at all. I managed to call printf from it to print a string in the EXE's data segment ("FALLOUT %p.%p") with the code and data segment addresses as parameters, print a "Hi!" string in the data segment of the BIN file, print a global int value (with "%d") in the BIN file's data segment, then increment it and print it again, and call exit(), and all worked perfectly!!! (Being printf() and exit() functions from inside the EXE.)

I can also call the EXE's functions from C. I made C functions with the same signature as the original standard ones with an Assembly implementation and I opened a file, read its contents to a local C variable and closed the file. In this case I can even put the patches to be choosable inside an INI file (like on Craft's sFall1) and let people edit the file easily to choosse which patches they want applied!

So I think this can be used exactly like DLL injection! I load, let it be loaded, access stuff from inside its memory space, call its functions with references to the EXE's functions, whatever else is needed. At least for a beginner on these things seems to be wow ahahah.

PS: this doesn't mess on how the BIN file is loaded to memory. It just needs to pass 3 addresses in 3 registers or pass one of them (the block one - the base of everything else) and store the others in the block and the rest it's however the loader is implemented. I haven't touched the loader since I began trying these things (2 days without touching on it, I think, while I'd find about these things).

**Update:** now the loader is writing the BIN file length on the 4th 4 bytes of the BIN file (only in memory, nothing permanent). That's required to know up to what address to look up when the pathces patches itself. The patcher patches itself as soon as it starts now (now I mean really when it starts - a direct call after the jump), and the rest of the code has all the values needed already ready for use. That way I don't need to pass the addresses as parameters to every function of the file. Seems that all really works, so cool!
  
</details>

<details> 
  <summary><h3>First version</h3></summary>

#### How I've been doing this

I've been checking this repository: https://github.com/Aramatheus/sfall_1. It has the sFall1 1.20 source with small modifications from Sduibek (Fallout Fixt's creator). If this one gets working well, I go over to sFall1 1.3 entirely by Crafty and then (or right to) 1.7.6. There's also version 1.8, but there's no public source that I have found. That one requires reverse engineering unless the source is found somewhere. The reason I'm not already using version 1.7.6 is because the Loot/Drop all patch doesn't work when I port it. No idea if it needs other patches to work, but at least on 1.20 it's working, so it seems a good place to start.

Also, MASM or NASM or whatever is preferred to use is very useful to assemble the instructions Crafty has on the source. Then the assembled EXE or OBJ file can be opened in a hex editor and the bytes copied to the DOS EXE, and finally the only thing left to do is correct all addresses and offsets. I've been going on the Windows EXE distributed with Fallout Fixt, go on all addresses I find on sFall1's source, and then use the hex bytes near that address to find the correct function on the DOS EXE. So far, this worked well for the Loot/Drop all feature, so that's what I've been doing.

Before this I started by writing the opcodes myself (that's painful...). Then I found out IDA (The Interactive Disassembler) can assemble individual instructions and I went that route. I had to kind of bet with myself if jumps were short or near (short because there's not much space, so it must be saved as much as possible), but not really a problem. Except when I'd make a wrong bet and I had to move all the code backwards or forwards and correct all offsets --> wtf. So when I finally used my head to a decent state, I realized that MASM (I already had it installed) could be very helpful. I just copy the Assembly code, paste it on an editor, assemble with MASM and do what I described above. Much simpler, faster, and bug-free on wrongly copied instructions (already happened to me when it was manual, I can't even copy things on my own, wow...).

#### How I fixed some weird bugs

There were bugs on the ported code. For example (this was written in the time I found them, so it's written in present verb form, not past):
- One very noticeable is that when I create a new game, go on the dead body and click D when exchanging items, an error dialog appears --> with no error written. I've no idea why that happens. I'm pointing to a readable place in memory, so wtf. Doesn't do anything bad, just doesn't show the message, but still very weird.
- Other errors include when clicking D with fewer items to exchange with the body, all disappears. But only from the screen. It's all on the dead body's inventory if I go back and exchange items again. So again doesn't do anything bad, but wtf. Curious to know why it happens.
- Or the current and total weight not showing on the inventory. As it replaces the format string from "%s %d" to "%s %d/%d" and that one doesn't work, it breaks the original functionality. That must also be fixed somehow. I'll try and see if I can, but I have no experience in fixing stuff like this, so any help is very appreciated. As this is not working and is even breaking original functionality (wow), it's disabled on the patcher.

##### How I fixed those

There was also an interesting "bug" when I was trying to replace "FALLOUT 1.2" on the right bottom corner of the main menu by a string somewhere else (no space where the original string is to put what I wanted). When I'd put the address of the new string, the old address would get back there even though the new was written on the file. Magic? No... (seemed for a while though).

It seems I learned about relocations... Not much, but enough to understand that that's the issue. The game is relocated at a different address depending on the DOS extender. DOS/4GW puts it at one address, DOS/32A puts it elsewhere. The EXE is prepared to handle that normally because there's a table of addresses to relocate, and all the needed addresses are fixed as soon as the EXE is loaded. The added code is not in that table nor will I put it unless I have a very strong reason, because I'd need to move all the contents of the EXE down to add bytes to the table.

So plan B. I'm assuming the game is running at the address DOS/32A puts it running in, and all the addresses on the code are relative to that address, not the one the game is supposed to be loaded at (which is what IDA assumes, as a start). So if the EXE is loaded in IDA, various things will start with "unk_" (unknown - makes sense). Maybe I can get IDA to move the segments to the DOS/32A addresses so I can see the references, but I don't know how that's done (was trying but it says not enough space, and as I don't need it, I won't try to find out how to do it).

**PS:** I'm now noticing my tablet with DOS/32A is not loading the game at the same address as on PC, because the patches are not working there... So it's not fixed with DOS/32A, I guess. That means the patch works only with Luck (depends on how many points one has in real life 😂). I'll think of some way around, without messing with the relocations table... Else, I guess will be fun to write some program to do it, because if I'm to do it, I won't do it manually, no way (at minimum because I'm careless).

</details>

## For developers
### - Compiling the Installer
I'm letting GoLand take care of that, but for manual mode, go on the main.c folder and just do: `go build` and that's it.
### - Compiling the Patcher
Command I'm using to compile (use ONLY Open Watcom - I assume its Calling Convention in various things!): please see the `PROGRAM_PARAMS` attribute inside the file `F1DPPatcher/.idea/runConfigurations/Compile_with_Open_Watcom__for_C_.xml` (or a similar one in case I change its name).

At the moment of writing this (02-03-2022), I don't have all the target C files in the compile command. With CLion, targets are automatically added to CMakeLists, which is then given to the compiler. As I haven't got CMake to work with Open Watcom, I can't have that automatic addition of files. So I'm including C files in other C files and just compiling main.c 😄...
### - Compiling the Loader
Command I'm using to compile: `C:\MASM615\BIN\ml_9.00.21022.08_x86.EXE /omf /Fl /Fm /FR /errorReport:send /Sf /W3 /WX "%BASE_FILE_NAME%.asm"`.

## About
### - Contributing

Anyone is free to contribute to the repository with new patch ports, new patches entirely, port fixes, whatever that is useful.

### - Licensing

- This project and included sFall1 source code is licensed under GNU GPL-3.0-or-later - https://www.gnu.org/licenses/.
- All my code is licensed under Apache 2.0 License - http://www.apache.org/licenses/LICENSE-2.0.

## Support
If you have any questions, try the options below:
- Create an Issue here: https://github.com/DADi590/F1DP/issues
- Create a Discussion here: https://github.com/DADi590/F1DP/discussions

## Final notes
Hope you like it! It seems to me that this was an amazing thing to try to do because I've been learning a lot with this!!!
