# Fallout1DOSPatcher
A patcher for the MS-DOS version of Fallout 1 including Crafty's sFall1 patches and maybe more

Hi everyone. I've decided to attempt to port all patches made to the Windows version of Fallout 1 to the MS-DOS one (some Windows patches are DLL mods, and DOS doesn't know what a DLL is, so the patches are not used when playing with the DOS EXE). This way, those who need to play with the DOS version like me for whatever reason can enjoy the patches that the Windows version players can.

**NOTE:** You need to have dos32a.exe in the Fallout directory instead of dos4gw.exe. This is one of the patches, and that other program (DOS/32A) corrects an issue with one of the patches (aside from being faster). Download it here: https://sourceforge.net/projects/dos32a.

Why Go and not Python or C or something? I prefer Go over Python (I like compilers complaining over types and other things aside from Go being easy to compile to an EXE), and C is too much hassle for such a simple thing.

## Download

Download any version you'd like (preferably the most recent one) here: https://github.com/DADi590/Fallout1DOSPatcher/releases.

## Contributing

Anyone is free to contribute to the repository with new patch ports, new patches entirely, port fixes, whatever that is useful.

## How I've been doing this

I've been checking this repository: https://github.com/Aramatheus/sfall_1. It has the sFall1 1.20 source with small modifications from Sduibek (Fallout Fixt's creator). If this one gets working well, I go over to sFall1 1.3 entirely by Crafty and then (or right to) 1.7.6. There's also version 1.8, but there's no public source that I have found. That one requires reverse engineering unless the source is found somewhere. The reason I'm not already using version 1.7.6 is because the Loot/Drop all patch doesn't work when I port it. No idea if it needs other patches to work, but at least on 1.20 it's working, so it seems a good place to start.

Also, MASM or NASN or whatever you prefer to use is very useful to assemble the instructions Crafty has on the source. Then the assembled EXE or OBJ file can be opened in a hex editor and the bytes copied to the DOS EXE, and finally the only thing left to do is correct all addresses and offsets. I've been going on the Windows EXE distributed with Fallout Fixt, go on all addresses I find on sFall1's source, and then use the hex bytes near that address to find the correct function on the DOS EXE. So far, this worked well for the Loot/Drop all feature, so that's what I've been doing.

Before this I started by writing the opcodes myself (that's painful...). Then I found out IDA (The Interactive Disassembler) can assemble individual instructions and I went that route. I had to kind of bet with myself if jumps were short or near (short because there's not much space, so it must be saved as much as possible), but not really a problem. Except when I'd make a wrong bet and I had to move all the code backwards or forwards and correct all offsets --> wtf. So when I finally used my head to a decent state, I realized that MASM (I already had it installed) could be very helpful. I just copy the Assembly code, paste it on an editor, assemble with MASM and do what I described above. Much simpler, faster, and bug-free on wrongly copied instructions (already happened to me when it was manual, I can't even copy things on my own, wow...).

## How I fixed some weird bugs

There were bugs on the ported code. For example (this was written in the time I found them, so it's written in present verb form, not past):
- One very noticeable is that when you create a new game, go on the dead body and click D when exchanging items, an error dialog appears --> with no error written. I've no idea why that happens. I'm pointing to a readable place in memory, so wtf. Doesn't do anything bad, just doesn't show the message, but still very weird.
- Other errors include when clicking D with fewer items to exchange with the body, all disappears. But only from the screen. It's all on the dead body's inventory if you go back and exchange items again. So again doesn't do anything bad, but wtf. Curious to know why it happens.
- Or the current and total weight not showing on the inventory. As it replaces the format string from "%s %d" to "%s %d/%d" and that one doesn't work, it breaks the original functionality. That must also be fixed somehow. I'll try and see if I can, but I have no experience in fixing stuff like this, so any help is very appreciated. As this is not working and is even breaking original functionality (wow), it's disabled on the patcher.

### How I fixed those

There was also an interesting "bug" when I was trying to replace "FALLOUT 1.2" on the right bottom corner of the main menu by a string somewhere else (no space where the original string is to put what I wanted). When I'd put the address of the new string, the old address would get back there even though the new was written on the file. Magic? No... (seemed for a while though).

It seems I learned about relocations... Not much, but enough to understand that that's the issue. The game is relocated at a different address depending on the DOS extender. DOS/4GW puts it at one address, DOS/32A puts it elsewhere. The EXE is prepared to handle that normally because there's a table of addresses to relocate, and all the needed addresses are fixed as soon as the EXE is loaded. The added code is not in that table nor will I put it unless I have a very strong reason, because I'd need to move all the contents of the EXE down to add bytes to the table.

So plan B. I'm assuming the game is running at the address DOS/32A puts it running in, and all the addresses on the code are relative to that address, not the one the game is supposed to be loaded at (which is what IDA assumes, as a start). So if the EXE is loaded in IDA, various things will start with "unk_" (unknown - makes sense). Maybe you can get IDA to move the segments to the DOS/32A addresses so you can see the references, but I don't know how that's done (was trying but it says not enough space, and as I don't need it, I won't try to find out how to do it).

**PS:** I'm now noticing my tablet with DOS/32A is not loading the game at the same address as on PC, because the patches are not working there... So it's not fixed with DOS/32A, I guess. That means the patch works only with Luck (depends on how many points one has in real life 😂). I'll think of some way around, without messing with the relocations table... Else, I guess will be fun to write some program to do it, because if I'm to do it, I won't do it manually, no way (at minimum because I'm careless).
