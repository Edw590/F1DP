; Copyright 2022 DADi590
;
; Licensed to the Apache Software Foundation (ASF) under one
; or more contributor license agreements.  See the NOTICE file
; distributed with this work for additional information
; regarding copyright ownership.  The ASF licenses this file
; to you under the Apache License, Version 2.0 (the
; "License"); you may not use this file except in compliance
; with the License.  You may obtain a copy of the License at
;
;   http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing,
; software distributed under the License is distributed on an
; "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
; KIND, either express or implied.  See the License for the
; specific language governing permissions and limitations
; under the License.


; Loader to load (wow) the external code to memory inside the code process and execute it


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

; All (or most of) these functions and file names belong to POSIX 1003.1 (filelength doesn't), the same used by Watcom,
; the compiler used for the game.
; When there's no register mention on the parameters, it means it's on the stack (don't forget the calling convention
; is backwards on that).
; The return value is either a register name, or [stack] or (void). To know what's returned, go to the official docs.

; ATTENTION: I'm NOT freeing the Patcher's allocated memory ANYwhere. It ALWAYS stays in memory. Though, as it's just 10 kB,
; I'm not worrying much about it. DOSBox can have up to 63 MB, and at most 31 MB as recommended. And 10 kB is only 0.03% of
; 31 MB, so doesn't seem to be a wow leak.
; The reason I'm not freeing the memory is because I don't know where I should do it. I don't know if the program exits only
; at the end of main() or if it has other exit points. Also, this way I don't need to store anything on the code segment or
; anywhere else more obscure (as an idea from the one with the loader idea, xtll, DOS video memory).

pusha ; Save all registers
push    eax
push    ebx
;push    ecx - See the end to know why it's commented
push    edx
push    esi
push    edi
push    ebp

call    rightAfter
rightAfter:
pop     esi
sub     esi, 0EA2E0h ; ESI = Code section address --> ATTENTION: if you put the loader ANYwhere else in the game EXE, change this value!!!

lea     eax, [esi+12345678h] ; INIT_MSG
push    eax
lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
call    ecx
add     esp, 4

lea     eax, [esi+12345678h] ; PRT_BIN_FILE_NAME
push    eax
lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
call    ecx
add     esp, 4

; Allocate the code block here
push    400  ; S_IRUSR
push    600h ; O_RDONLY | O_BINARY | O_EXCL
lea     edi, [esi+12345678h] ; BIN_FILE_NAME
push    edi
lea     ecx, [esi+0D555Ah] ; open_(path, oflag, ...) EAX
call    ecx
add     esp, 12
cmp     eax, -1
je      errorOpen
push    eax ; Save the file handle for read()

lea     ecx, [esi+0BF9ECh] ; filelength_(EAX = handle) EAX
call    ecx
cmp     eax, -1
je      errorFileLength
push    eax ; Save the file length for read()

lea     ecx, [esi+0CAC68h] ; _nmalloc_(EAX = size) EAX
call    ecx
test    eax, eax
jz      errorMalloc

mov     edx, eax
mov     eax, [esp+4]
mov     ebx, [esp]
lea     ecx, [esi+0D545Fh] ; read_(EAX = fildes, EDX = buf, EBX = nbyte) EAX
call    ecx
cmp     eax, [esp]
jne     errorRead

mov     eax, [esp+4]
lea     ecx, [esi+0D57CCh] ; close_(EAX = fildes) EAX
call    ecx
; I'll not check for file not closed here (why wouldn't it close anyway? The descriptor is still valid).

; Below, "LES" + type (00h in this case), always in Little Endian (those must be the first 4 bytes of the BIN file).
; The type is the type of the Patcher. Must be the same type that this loader is supposed to load.
; Also, LES means Linear Executable Segments, because I compiled an LE EXE and copy-pasted its segments to another file.
cmp     dword ptr [edx], 0053454Ch ; [!!!] VERSION HERE
jne     errorWrongVer

mov     [edx+4], esi ; Code section address stored in the block in the 2nd 4 bytes
mov     eax, [esi+728F6h+1] ; Get the address of a mov with the "07desert" string from main()
sub     eax, 0F9F4Ch ; EAX = Data section address
mov     [edx+8], eax ; Data section address stored in the block in the 3rd 4 bytes
mov     ebx, [esp]
mov     [edx+12], ebx ; BIN file length stored in the block in the 4th 4 bytes

add     esp, 8 ; Remove the saved things from the stack

;mov     [esi+0EAFFBh], edx ; Allocated buffer address stored in the end of the code section
; This above is commented out and the NOPs are here in case I see it's a good idea to free()
; the memory at the end (and actually find a way of doing that). Though, I don't think it's
; that bad. Will just leak this bit (more whatever the game leaks, anyway, but I can't fix that,
; if it leaks anything, that is).
; But the NOPs are really here just because then I don't need to go all over the offsets
; on the instructions below if I want to change this. This way I just rewrite the NOP bytes.
nop
nop
nop
nop

lea     eax, [esi+12345678h] ; START_PATCHER_STR
push    eax
lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
call    ecx
add     esp, 4

;;;;;;;;;;;;;;;;;;;
push    esi
push    ebx

; Parameters for the Patcher
mov     esi, edx ; ESI = block address (the Patcher requires ESI to be the block address)

mov     ebx, edx ; EAX = block address (a copy to be used to find realMain())
dec     ebx ; So that it's "-1". Then it starts at 0 and no jumps are needed.
loop_find_main:
	inc     ebx
	cmp     dword ptr [ebx], 78563412h ; This constant is defined in the Patcher as SN_MAIN_FUNCTION (Special Number)
	loopne  loop_find_main
add     ebx, 4 ; EAX = main external function address (jump over the 32-bit Special Number)

; In case anything goes wrong inside the Patcher (wrong out of what is expected - not an error and return false, I mean
; a serious unexpected error related with stack or Special Numbers or something like that) and it is able to return
; here, hopefully EAX (or at least AL, in the case of a bool return value) will not be modified and will still be 0 and
; the Loader will report an error.
xor     eax, eax

call    ebx ; Call the Patcher's realMain() function

pop     ebx
pop     esi
;;;;;;;;;;;;;;;;;;;

; realMain's return type is 'bool'. That's 0 or 1, and when it's to put false, Watcom XORs AL with AL, not EAX, with EAX.
; Makes sense, because it's only 0 or 1, nothing else. So I've put the check with AL too (no need for EAX really if Watcom
; zeroes at least AL when it's to return false).
test    al, al
jnz     printSuccess  ; Returned true, then all went fine
jmp     printErrors   ; Returned false, then an error occurred

errorOpen:
	lea     eax, [esi+12345678h] ; ERR_OPEN_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 4
	jmp     printFatalError

errorFileLength:
	add     esp, 4 ; Remove the saved things from the stack

	lea     eax, [esi+12345678h] ; ERR_FILE_LENGTH_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 4
	jmp     printFatalError

errorMalloc:
	add     esp, 8 ; Remove the saved things from the stack

	push    ebx ; File length
	lea     eax, [esi+12345678h] ; ERR_MALLOC_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 8
	jmp     printFatalError

errorRead:
	add     esp, 8 ; Remove the saved things from the stack

	lea     eax, [esi+12345678h] ; ERR_READ_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 4
	jmp     printFatalError

errorWrongVer:
	add     esp, 8 ; Remove the saved things from the stack

	sub     esp, 4
	mov     eax, esp ; Store the string stack address in EAX

	; Put the 4th byte of the file in ECX (LES version) by zero extending the other 3 bytes
	movzx   cx, byte ptr [edx+3]
	push    ecx ; BIN file type

	mov     ecx, dword ptr [edx]
	mov     dword ptr [esp], ecx ; The first 3 characters of the file (must say "LES")
	mov     byte ptr [esp+3], 0  ; NULL terminate the string
	push    eax ; Push the address of where the string is on the stack (stored in EAX)

	lea     eax, [esi+12345678h] ; ERR_WRNG_TYPE_STR
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 12
	jmp     printFatalError

printFatalError:
	lea     eax, [esi+12345678h] ; ERR_LOAD_NO_PATCHES
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 4
	lea     ecx, [esi+0DE2DEh] ; getch_() EAX
	call    ecx
	jmp     end1

printSuccess:
	lea     eax, [esi+12345678h] ; PATCHER_SUCCESS
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 4
	jmp     end1

printErrors:
	lea     eax, [esi+12345678h] ; PATCHER_ERRORS
	push    eax
	lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
	call    ecx
	add     esp, 4
	lea     ecx, [esi+0DE2DEh] ; getch_(void) EAX
	call    ecx
	jmp     end1

end1:

lea     eax, [esi+12345678h] ; END_MSG
push    eax
lea     ecx, [esi+0CA3B0h] ; printf_(format, ...) EAX
call    ecx
add     esp, 4

mov     ecx, esi ; ECX = Code section address

; Restore all registers
push    ebp
push    edi
push    esi
push    edx
;push    ecx
push    ebx
push    eax

; Execute the removed main() call, now with offsets and not effective addresses (no ESI to help here).
; EDIT: since ECX is set right after returning from this code cave and sub_13450() doesn't use it,
; that means its current value is of no use. So I can just use it with ESI's value one last time.
; This is just to ease my life when testing the Loader (one less thing to fix on IDA, because I
; already have to fix all the strings manually... - boring).
lea     ecx, [ecx+13450h] ; sub_13450() as IDA calls it
call    ecx

ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;; Strings

; At most 80 chars per line, don't forget that.

INIT_MSG db "----------------------------",0Dh,0Ah,"----- F1DP Loader v1.0 -----",0Dh,0Ah, 0
END_MSG db  "----- F1DP Loader v1.0 -----",0Dh,0Ah,"----------------------------",0Dh,0Ah, 0

BIN_FILE_NAME db "dospatch.bin", 0 ; At most 12 characters excluding the NULL one (DOS' 8.3 file names - 8 name chars, one dot, 3 chars the extension)
PRT_BIN_FILE_NAME db "- BIN file containing the Patcher: dospatch.bin",0Dh,0Ah, 0
START_PATCHER_STR db "- Patcher loaded correctly. Starting it now...",0Dh,0Ah, 0

ERR_OPEN_STR db "- [X] Error opening the BIN file.",0Dh,0Ah, 0
ERR_FILE_LENGTH_STR db "- [X] Error getting the length of the BIN file.",0Dh,0Ah, 0
ERR_MALLOC_STR db "- [X] Error allocating RAM memory space (%u bytes) for the Patcher.",0Dh,0Ah, 0
ERR_READ_STR db "- [X] Error reading the BIN file.",0Dh,0Ah, 0
ERR_WRNG_TYPE_STR db "- [X] Wrong BIN file type. Expecting type LES0, but got %s%d.",0Dh,0Ah, 0 ; [!!!] VERSION HERE
ERR_LOAD_NO_PATCHES db 0Dh,0Ah,"- Press any key to proceed loading the game WITHOUT any patches...",0Dh,0Ah, 0
PATCHER_SUCCESS db 0Dh,0Ah,"- The Patcher exited successfully! The game will now start automatically.",0Dh,0Ah, 0
PATCHER_ERRORS db 0Dh,0Ah,"- The Patcher exited with errors! Please check the console.",0Dh,0Ah,
				          "  Press any key to proceed loading the game...",0Dh,0Ah, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


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
