

; Helper to assemble the instructions and then it's only copy the hex bytes and correct addresses and offsets


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

pusha

mov     edi, [esi+8] ; Data section address
push    edi
mov     edi, [esi+4] ; Code section address
push    edi
mov     edi, [esi+8]
lea     edi, [edi+0FD5E0h] ; FALLOUT %d.%d string
push    edi
mov     edi, [esi+4]
lea     edi, [edi+0CA3B0h] ; printf()
call    edi
add     esp, 12
;mov     eax, 12345678h
;mov     edi, [esi+4]
;lea     edi, [edi+0CA40Ah] ; exit()
;call    edi

popa

ret


  ; Primeiro push é a cor
  ; Segundo push é posição Y
  ; No EAX vai a janela onde escrever
  ; No EBX vai a font da letra (ele tem 0 na string do sFall)
  ; No edi vai a posição X
  ; No EDX vai o endereço da string no CS
                    
  ;sub  dword ptr [esp], 12
  ;pop  edi
  ;push ds
  ;push cs
  ;pop  ds
  ;mov  edi, 10
  ;mov  dword ptr [esp+0Ch], 12345
  ;rep  movsb
  ;pop  ds
  ;pop  edi
  ;call whateverDadi590
  ;jmp  whateverDadi590


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