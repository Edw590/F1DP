; Helper to assemble the instructions and then it's only copy the hex bytes and correct addresses and offsets


.686p
.model flat,stdcall
.stack 2048

.code

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	START_DADI590:


  push eax
  mov  edx, 12
  call whatever_dadi590                    
  pop  edx
  push eax
  xchg edx, eax                       
  call whatever_dadi590
  xchg edi, eax                       
  xor  eax, eax
  cmp  eax, 0          
  je   skip                           
  mov  eax, 0
  test eax, eax                       
  jz   skip                           
  call whatever_dadi590
skip:
  add  edi, eax
  push edi
  sub  edx, edi
  mov  edi, [esp+94h+4h]
  push edi
  push 4F92C4h                       
  lea  eax, [esp+0Ch+4h]
  push eax
  call whatever_dadi590
  add  esp, 5*4
  xor  eax, eax
  mov  al, ds:[0]              
  cmp  edx, 0
  jge  noRed
  mov  al, ds:[0]              
noRed:
  mov  ecx, 499
  mov  ebx, 120
  jmp  whatever_dadi590

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	whatever_dadi590:

end START_DADI590