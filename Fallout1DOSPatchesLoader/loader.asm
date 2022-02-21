

; Loader to load (wow) the external code to memory inside the code process


.686p
.model flat
.stack 100h

.code

	; To be easy to know where the code begins - search for 5 NOPs with a hex editor.
	nop
	nop
	nop
	nop
	nop

	START_DADI590:
	; These things below are to be used for things only known when the code is in place. They're noticeble enough.
	; One is a weird number, the other, all near calls and some jumps are to the same function - suspicious.
	;whateverDadi590
	;12345678h


; WARNING - The offsets are ONLY for: Fallout 1 DOS EXE v1.2 semi-official by TeamX; MD5 hash: 3DCF41FA6784030BD5C71BE81954C899.
; I did NOT check the Fallout 1 DOS EXE v1.1 official; MD5 hash: 6A41C641B789B44FD7BE1805CE030C9B, or any other that may exist.

; Note: I could call all functions with the relative offset, but I prefer to have it hard-coded here so less bugs happen.
; This will only run in the beginning anyway, so no big deal with performance.

pusha ; Save all registers

call    rightAfter
rightAfter:
pop     esi
sub     esi, 0EA2E0h ; ESI = Code section address --> ATTENTION: if you put the loader anywhere else in the game, change this value!!!

; Allocate the code block here
push    0
push    200h ; O_RDONLY | O_BINARY
lea     edi, [esi+12345678h] ; PATCHES_BIN_FILE
push    edi
lea     ecx, [esi+0D555Ah] ; open_(file name) EAX = file handle
call    ecx
add     esp, 0Ch
cmp     eax, -1
je      errorOpen
push    eax ; Save the file handle for read()

lea     ecx, [esi+0BF9ECh] ; filelength_(EAX = file handle) EAX = file length
call    ecx
cmp     eax, -1
je      errorFileLength
push    eax ; Save the file length for read()

lea     ecx, [esi+0CAC68h] ; _nmalloc_(EAX = file length) EAX = allocated block address
call    ecx
test    eax, eax
jz      errorMalloc

mov     edx, eax
mov     eax, [esp+4]
mov     ebx, [esp]
lea     ecx, [esi+0D545Fh] ; read(EAX = file handle, EBX = length to read, EDX = buffer address) EAX = number of read bytes
call    ecx
cmp     eax, [esp]
jne     errorRead

mov     eax, [esp+4]
lea     ecx, [esi+0D57CCh] ; close(EAX = file handle) EAX = error code
call    ecx
; I'll not check for file not closed here. I don't think it's that bad anyway.
; The patches are already in place really.

cmp     dword ptr [edx], 0 ; [!!!] VERSION TYPE HERE
jne     errorWrongVer

mov     [edx+4], esi ; Code section address stored in the block in the 2nd 4 bytes
mov     eax, [esi+728F6h+1] ; Get the address of a mov with the "07desert" string from main()
sub     eax, 0F9F4Ch ; EAX = Data section address
mov     [edx+8], eax ; Data section address stored in the block in the 3rd 4 bytes
mov     ebx, [esp]
mov     [edx+12], ebx ; Patcher file length stored in the block in the 4th 4 bytes

add     esp, 8 ; Remove the saved things from the stack

;mov     [esi+0EAFFBh], edx ; Allocated buffer address stored in the end of the code section
nop
nop
nop
nop

;;;;;;;;;;;;;;;;;;;
push esi

; Parameters for the patcher
mov     esi, edx ; ESI = block address

mov     eax, edx ; EAX = block address
dec     eax ; So that it's "-1". Then it starts at 0 and no jumps are needed.
loop_find_main:
	inc     eax
	cmp     dword ptr [eax], 78563412h ; This constant is defined in the patcher
	loopne  loop_find_main
add     eax, 4 ; EAX = main external function address

call    eax

pop     esi
;;;;;;;;;;;;;;;;;;;

test    eax, eax
jz      printSuccess
jmp     printErrors

errorOpen:
	call    printErrorP1
	push    edi ; File name
	lea     eax, [esi+12345678h] ; ERR_OPEN_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 8
	jmp     printErrorP2

errorFileLength:
	add     esp, 4 ; Remove the saved things from the stack
	call    printErrorP1
	push    edi ; File name
	lea     eax, [esi+12345678h] ; ERR_FILE_LENGTH_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 8
	jmp     printErrorP2

errorMalloc:
	add     esp, 8 ; Remove the saved things from the stack
	call    printErrorP1
	push    ebx ; File length
	lea     eax, [esi+12345678h] ; ERR_MALLOC_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 8
	jmp     printErrorP2

errorRead:
	add     esp, 8 ; Remove the saved things from the stack
	call    printErrorP1
	push    edi ; File name
	lea     eax, [esi+12345678h] ; ERR_READ_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 8
	jmp     printErrorP2

errorWrongVer:
	call    printErrorP1
	push    [edx] ; Patches file version
	push    edi ; File name
	lea     eax, [esi+12345678h] ; ERR_WRNG_VER_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 12
	jmp     printErrorP2

printErrorP1:
	lea     eax, [esi+12345678h] ; ERR_IN_PATCHER_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 4
	ret

printErrorP2:
	lea     eax, [esi+12345678h] ; PRESS_ENTER_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 4
	lea     ecx, [esi+0DE2DEh] ; getch() EAX = entered character in ASCII
	call    ecx
	jmp     end1

printSuccess:
	lea     eax, [esi+12345678h] ; PATCHER_SUCCESS
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 4
	jmp     end1

printErrors:
	lea     eax, [esi+12345678h] ; PATCHER_ERRORS
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf(string)
	call    ecx
	add     esp, 4
	lea     ecx, [esi+0DE2DEh] ; getch() EAX = entered character in ASCII
	call    ecx
	jmp     end1

end1:

popa ; Restore all registers

; Execute removed main() call, now with offsets and not effective addresses (no ESI to help here)
call    whateverDadi590 ; sub_13450

ret

PATCHES_BIN_FILE db "dospatch.bin", 0
ERR_OPEN_STR db "--> Error opening the patches file %s",0Dh,0Ah, 0
ERR_FILE_LENGTH_STR db "--> Error getting the length of the patches file %s",0Dh,0Ah, 0
ERR_MALLOC_STR db "--> Error allocating RAM memory space (%d bytes) for the patches",0Dh,0Ah, 0
ERR_READ_STR db "--> Error reading the file %s",0Dh,0Ah, 0
ERR_WRNG_VER_STR db "--> Wrong patches file %s version type. Expecting 0, but got %d",0Dh,0Ah, 0 ; [!!!] VERSION TYPE HERE
ERR_IN_PATCHER_STR db 0Dh,0Ah,"Attention - error loading Fallout 1 DOS Patcher",0Dh,0Ah, 0
PRESS_ENTER_STR db 0Dh,0Ah,"Press any key to continue with game execution WITHOUT patches...",0Dh,0Ah, 0
PATCHER_SUCCESS db 0Dh,0Ah,"Fallout 1 DOS Patcher exited successfully! The game will now start automatically.",0Dh,0Ah, 0
PATCHER_ERRORS db 0Dh,0Ah,"Fallout 1 DOS Patcher exited with errors! Please check the console.",0Dh,0Ah,
				  "Press any key to proceed loading the game...",0Dh,0Ah, 0


  	; Do NOT remove NOPs from here! I've tested them and it's needs to be this much for the last instruction, if it's
  	; a jump, to be a near jump, not a short jump --> which I don't want, or I'll have to move all the code when I
  	; edit it (like I'll do right now...). Of course, the other jumps count too, but these many NOPs will ensure even
  	; the last one is seen as a near jump and not a short one.
  	; Aside from this, it's easy to see where the code ends - just look for infinite NOPs.
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	whateverDadi590:

end START_DADI590