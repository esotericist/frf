; bitwise Trie wilbert, idle
; v1.3
; Walk walks over all allocated keys
; keys are always allocated 4 at a time

Structure TrieNode   ; 4 * 4 bytes
  n.l[4]
EndStructure

Structure Trie
  *vt.l
  nodes.TrieNode[256] ; offset 0
  firstMem.l          ; offset 4096
  currentMem.l        ; offset 4100
  memCount.l          ; offset 4104
  freeNodes.l         ; offset 4108
  nextNode.l          ; offset 4112
EndStructure

Structure TrieMem
  nextMem.l           ; offset 0
  m.Long[4092]        ; offset 4
EndStructure

Declare Trie_clear(*this.Trie)
Declare Trie_Free(*this.Trie)
Declare Trie_New()
Declare Trie_SetValue(*this.Trie,key.l, value.l)
Declare.l Trie_GetValue(*this.Trie, key.l)
Declare.s Trie_GetMemSize(*this.Trie)
Declare Trie_Walk(*this.Trie, *callback.l, userInfo.l = 0)

Interface Trie_Class
  Clear()
  Free()
  Set(key.l,value.l)
  Get(key.l)
  GetSize.s()
  Walk(*callback.l, userInfo.l = 0)
EndInterface

DataSection: vt_Trie:
  Data.l @Trie_Clear()
  Data.l @Trie_Free()
  Data.l @Trie_SetValue()
  Data.l @Trie_GetValue()
  Data.l @Trie_GetMemSize()
  Data.l @Trie_Walk()
EndDataSection

Procedure Trie_Clear(*this.Trie)
  Protected *mem.TrieMem, *nextMem.TrieMem, idx.l
  If *this
    *mem = *this\firstMem
    While *mem
      *nextMem = *mem\nextMem
      FreeMemory(*mem)
      *mem = *nextMem
    Wend
    FillMemory(*this\nodes[0], 4096)
    *this\firstMem = AllocateMemory(SizeOf(TrieMem))
    *this\currentMem = *this\firstMem
    *this\memCount = 1
    *this\freeNodes = 1022
    *this\nextNode = (*this\firstMem + 20) & $fffffff0
  EndIf
EndProcedure

Procedure Trie_Free(*this.Trie)
  Protected *mem.TrieMem, *nextMem.TrieMem, idx.l
  If *this
    *mem = *this\firstMem
    While *mem
      *nextMem = *mem\nextMem
      FreeMemory(*mem)
      *mem = *nextMem
    Wend
    FreeMemory(*this)
  EndIf
  ProcedureReturn 0
EndProcedure

Procedure Trie_New()
  Protected *this.Trie
  *this = AllocateMemory(SizeOf(Trie))
  If *this
    *this\vt = ?vt_Trie
    Trie_Clear(*this)
  EndIf
  ProcedureReturn *this
EndProcedure

Procedure.s Trie_GetMemSize(*this.Trie)
  Protected sz.s,size.i
  If *this
    size = SizeOf(Trie) + *this\memCount * SizeOf(TrieMem)
    If size < 1048576
      sz = Str(size/1024) + " kb"
    Else
      sz = Str(size/1048576) + " mb"
    EndIf
  EndIf
  ProcedureReturn sz
EndProcedure

Procedure Trie_SetValue(*this.Trie, key.l, value.l)
  EnableASM
  MOV eax, *this
  MOV ecx, key
  !and eax, eax
  !jnz trie_set_ok
  ProcedureReturn             ; return if *this is zero
  !trie_set_ok:
  !lea edx, [eax + 4]
  !push ebx
  !push esi
  !push ebp
  !rol ecx, 12
  !mov esi, ecx
  !and esi, 0xffc
  !add esi, edx
  !mov bl, 11
  !trie_set_loop:
  !mov eax, [esi]
  !and eax, eax                 ; check for zero pointer
  !jnz trie_set_cont
  ; ** allocate **
  !mov eax, [edx + 4112]        ; eax = *this\nextNode
  !mov [esi], eax
  !add dword [edx + 4112], 16   ; *this\nextNode + 16
  !dec dword [edx + 4108]       ; *this\freeNodes - 1
  !jnz trie_set_cont
  !push ecx
  !push edx
  CompilerSelect #PB_Compiler_OS
    CompilerCase #PB_OS_Windows
      !push dword 16372
      !call _PB_AllocateMemory@4
    CompilerCase #PB_OS_Linux
      !push dword 16372
      !call PB_AllocateMemory
      !add esp, 4
    CompilerCase #PB_OS_MacOS
      !mov ebp, esp
      !and esp, 0xfffffff0
      !sub esp, 12
      !push dword 16372
      !call _PB_AllocateMemory
      !mov esp, ebp
  CompilerEndSelect
  !pop edx
  !mov ecx, [edx + 4100]        ; ecx = *this\currentMem
  !mov [ecx], eax               ; set nextMem
  !mov [edx + 4100], eax        ; set currentMem
  !add eax, 20
  !and eax, 0xfffffff0
  !mov [edx + 4112], eax        ; set nextNode (16 bytes aligned)
  !inc dword [edx + 4104]       ; memCount + 1
  !mov dword [edx + 4108], 1022 ; freeNodes = 1022
  !pop ecx
  !mov eax, [esi]
  ; ** continue **
  !trie_set_cont:
  !rol ecx, 2
  !mov ebp, esi
  !mov esi, ecx
  !and esi, 0xc
  !add esi, eax
  !dec bl
  !jnz trie_set_loop
  !and ecx, 0xc
  !and eax, 0xfffffff0
  !add eax, ecx
  !shr ecx, 2
  !bts dword [ebp], ecx
  !pop ebp
  !pop esi
  !pop ebx
  MOV edx, value
  MOV [eax], edx
  DisableASM
EndProcedure

Procedure.l Trie_GetValue(*this.Trie, key.l)
  EnableASM
  MOV eax, *this
  MOV ecx, key
  !and eax, eax
  !jnz trie_get_ok
  ProcedureReturn       ; return if *this is zero
  !trie_get_ok:
  !lea edx, [eax + 4]
  !push esi
  !rol ecx, 12
  !mov esi, ecx
  !and esi, 0xffc
  !add esi, edx
  !mov dl, 11
  !trie_get_loop:
  !mov eax, [esi]
  !and eax, 0xfffffff0    ; check for zero pointer
  !jz trie_get_exit
  !rol ecx, 2             ; next 2 bits
  !mov esi, ecx
  !and esi, 0xc
  !add esi, eax
  !dec dl
  !jnz trie_get_loop
  !mov eax, [esi]
  !trie_get_exit:
  !pop esi
  DisableASM
  ProcedureReturn
EndProcedure

Macro Trie_WalkRecurseMacro(n)
  !push esi
  !mov eax, ebx
  !and eax, 3
  !mov esi, [esi + eax*4]
  !and esi, esi
  !jz trie_walk_recurse_skip#n
  !call trie_walk_recurse
  !trie_walk_recurse_skip#n#:
  !pop esi
EndMacro

Macro Trie_WalkValueMacro(n)
  !bt esi, n
  !jnc trie_walk_novalue#n
  !call trie_walk_handle_cb
  !trie_walk_novalue#n#:
EndMacro

Procedure Trie_Walk(*this.Trie, *callback.l, userInfo.l = 0); *callback(key.l, value.l [,userInfo.l])
  EnableASM
  MOV eax, *this
  MOV ecx, *callback      ; ecx = callback
  MOV edx, userInfo
  !and eax, eax
  !jnz trie_walk_ok
  ProcedureReturn       ; return if *this is zero
  !trie_walk_ok:
  !push ebx
  !push esi
  !push edi
  !push ebp
  !mov edi, edx           ; edi = userInfo
  !lea edx, [eax + 4]     ; edx = root table
  !xor ebx, ebx           ; ebx = key
  !trie_walk_loop:
  !mov esi, [edx + ebx*4]
  !and esi, esi
  !jz trie_walk_skip_root
  !or ebx, 0x400
  !call trie_walk_recurse
  !and ebx, 0x3ff
  !trie_walk_skip_root:
  !inc ebx
  !cmp ebx, 0x400
  !jb trie_walk_loop
  !pop ebp
  !pop edi
  !pop esi
  !pop ebx
  ProcedureReturn
  !trie_walk_recurse:
  !shl ebx, 2
  !jc trie_walk_value
  Trie_WalkRecurseMacro(0)
  !inc ebx
  Trie_WalkRecurseMacro(1)
  !inc ebx
  Trie_WalkRecurseMacro(2)
  !inc ebx
  Trie_WalkRecurseMacro(3)
  !shr ebx, 2
  !ret
  !trie_walk_value:
  Trie_WalkValueMacro(0)
  !inc ebx
  Trie_WalkValueMacro(1)
  !inc ebx
  Trie_WalkValueMacro(2)
  !inc ebx
  Trie_WalkValueMacro(3)
  !shr ebx, 2
  !bts ebx, 30
  !ret
  ; universal callback
  !trie_walk_handle_cb:
  !mov eax, ebx
  !and eax, 3
  !mov ebp, esi
  !and ebp, 0xfffffff0
  !mov eax, [ebp + eax*4]
  !push ecx
  !push edx
  !mov ebp, esp
  !and esp, 0xfffffff0
  !sub esp, 4
  !push edi     ; edi = userInfo
  !push eax     ; eax = value
  !push ebx     ; ebx = key
  !call ecx     ; ecx = callback
  !mov esp, ebp
  !pop edx
  !pop ecx
  !ret
EndProcedure

Global size = 1024*1024
Global mt.Trie_Class = Trie_New()
Global NewMap mp(size)

Procedure cb(key.l, value.l)
EndProcedure

start = ElapsedMilliseconds()
For a = 0 To size
  mt\Set(a,size-a)
Next

stop = ElapsedMilliseconds()
Trie_Add = stop-start

start = ElapsedMilliseconds()
For a = 0 To size
  mp(Str(a))=size-a; this line results in a crash on OS X when compiling without debug
Next
stop = ElapsedMilliseconds()
Map_add = Stop-start

start = ElapsedMilliseconds()
For a = 0 To size
  x = mt\Get(a)
Next

stop = ElapsedMilliseconds()
Trie_Look = stop-start

start = ElapsedMilliseconds()
For a = 0 To size
  x = mp(Str(a))
Next

stop = ElapsedMilliseconds()
map_Look = stop-start

start = ElapsedMilliseconds()
mt\Walk(@cb())
stop = ElapsedMilliseconds()
Trie_Walk = stop-start

start = ElapsedMilliseconds()
ForEach mp()
  cb(0, mp())
Next
stop = ElapsedMilliseconds()
map_Walk = stop-start

times.s = "Trie Add = " + Str(Trie_Add) + " Map Add = "+ Str(Map_Add) + #CRLF$
times   + "Trie Look = " + Str(Trie_Look) + " Map Look = "+ Str(Map_Look) + #CRLF$
times   + "Trie Walk = " + Str(Trie_Walk) + " Map Walk = "+ Str(Map_Walk)
MessageRequester("Times", times)

mem.s = mt\GetSize()
MessageRequester("", mem)

mt\Free()
; IDE Options = PureBasic 4.60 (Windows - x64)
; CursorPosition = 1
; Folding = --
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\