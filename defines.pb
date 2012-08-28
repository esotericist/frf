; defines.pb -- holds all the most critical definitions for frf to function.
; some definitions that are used locally only might be found in their respective file.

XIncludeFile "atoms.pb"

;- Type Definitions
; The currently supported types.

atom(type_empty)
atom(type_atom)
atom(type_integer)
atom(type_float)
atom(type_string)
atom(type_tuple)
atom(type_stackmark)
atom(type_variable)

atom(type_indirect) ; Special
; We use _type_indirect to indicate that what's on the stack is actually a
; container To another object. Typically, a tuple or other object where it
; is critical that we have live references even when popped from the stack.
; Pretty much nothing should EVER see this aside from push and pop.

atom(type_unknown) ; catch-all, some routines will return this if they don't
; a type on an object.


; reference workaround from purebasic forums
Structure ScanPBDataTypes
  ; These take up no memory and are allowed to grow without redim
  ; when the structure is pointed at a memory buffer.
  ; ex. *buf\d[i] ; 'i' being incremented in a loop
  b.b[0]  ; byte
  w.w[0]  ; word
  l.l[0]  ; long
  i.i[0]  ; integer
  q.q[0]  ; quad
  f.f[0]  ; float
  d.d[0]  ; double
  a.a[0]  ; ascii
  c.c[0]  ; character
  u.u[0]  ; unicode
  s.s[0]  ; string
EndStructure



; This is our base object on which all other data objects are derived.
; 
Structure dataobject
  typeatom.i
  refcount.i
EndStructure

; Integers and Atoms don't require objects at this time, since they can 
; be stored directly on the stack/inside variables.

Structure datafloat Extends dataobject
  value.d
EndStructure

Structure datastring Extends dataobject
  value.s
EndStructure

; Marks are a special case: They contain no data in and of themselves.
; They only exist as a stack object, and are used to build stackranges.
Structure datamark Extends dataobject
  
EndStructure

; indirect objects are used on the stack in place of directly pushing
; container objects such as tuples or arrays (once implemented).
; They do not persist once 'pop'ed.
Structure dataindirect Extends dataobject
  target.q   
  ; on a 32bit target, this would normally be 32-bit, but for consistency 
  ; we use quads For cells. Since this is a container for a reference to 
  ; a cell, this is appropriate.
EndStructure


; tuples are like fixed-length arrays. Typically used for small structures,
; especially message passing. Idiom for many tuple uses is for the first
; element of the outermost tuple to begin with an atom.
;
; length is an integer containing the cell count of the tuple.
; content is a pointer to an allocated block the size of length * cellsize
;
; Notably, while everything works with the datatuple object directly, we 
; never directly put a tuple reference on the stack.
Structure datatuple Extends dataobject
  length.i
  *content.scanPBdatatypes[0]  ; as always, cells are quads regardless of arch.
EndStructure



; Variables are a little different; a datavariable object on the stack 
; doesn't actually have the address of the variable.
; Instead, it holds an offset for a context' variable array, starting with 0.
; Context 0 is special: It is the 'global' variable space for the process.
Structure datavariable Extends dataobject
  context.i
  offset.i
EndStructure




; The Datablock isn't used for anything, but its job is to be the allocation
; unit for our object pools.
; It should always be at least as large as the largest dataobject inheritor.
; It's measured in quads for 32bit/64bit compatibility. Better to be too big
; than too small, and we can afford the extra bytes.
Structure datablock
  a.q[4]
EndStructure

;- Compilation Structures

; variableset holds the names of the variables declared for the word.
Structure variableset
  variablecount.i
  variablelist.i[0]
EndStructure

; Codeset holds actual compiled instructions for words.
Structure Codeset
  nameatom.i
  moduleatom.i     ; Owning module name
  length.i
  InstructionCount.i
  *var.variableset
  Codestream.q[0]
; the Codestream is made of quads because we only use doubles for floats.
; Using quads everywhere ensures all cells are big enough.
; This should hypothetically be harmless on 32-bit systems, since 32-bit
; integers can be stored/retrieved in 64-bit cells safely. ideally.
; TODO: Test more thoroughly on x86.
EndStructure


Structure module
  nameatom.i
  
  refcount.i         ; It's possible for a new version of a module to be compiled
  superceded.i       ; mid-flight. These fields help track that.
  
  List requires.module()  ; a list of module pointers that are needed by this module.
  Map defines.s(512) ; this-module-only defines
  Map wordlist.codeset(512) ; all the words this module knows
  *var.variableset          ; all the variables this module knows
EndStructure

Structure variable
  nameatom.i
  cell.q
EndStructure



; not currently active

Structure compileitem
  atom.i
  source.i
  target.i
EndStructure

Structure CompileState
  parsemode.i
  compilelevel.i
  compilestack.compileitem[1024]
  *module.module
  List modulevariables.s()
  *codeword.Codeset
  List wordvariables.s()
EndStructure




;- Data and Call stacks


Structure FlowControlFrame
  *codeword.codeset
  context.i         ; matched against context data in variable references
  currentop.i
  variables.variable[0]
EndStructure

; Both of these are dynamically sized. They can be changed at runtime, and 
; can even vary from process to process.
; TODO: Have stacks start small then dynamically grow as needed. This will 
; greatly reduce process spin-up time, and most processes are unlikely to
; need a stack depth greater than 32. 16 might be enough as a default.

Structure callstack
  top.i
  max.i
  stack.FlowControlFrame[0]
EndStructure

Structure datastack
  top.i
  max.i
  stack.i[0]
EndStructure

;- ProcessState Definition
; Each ProcessState holds one process.

Structure ProcessState
  pid.i             ; Process ID. simple integer, increases from 0 each time a process is spawned.
  *Node.NodeState   ; Pointer to the node structure holding this process
  ;                   Strictly speaking, there's a single global node, but this might be useful if
  ;                   for some reason we only have the process structure on hand.
  ;
  ;                   Notably: I would have preferred to define NodeState first, but PB cannot
  ;                   forward-declare structures. So compilation of the "processpool" map in
  ;                   NodeState fails. But it's okay to put NodeState as the type of a pointer
  ;                   because pointers are inherently typeless in PB.   W. T. F.
  
  ; TODO: This should probably be moved to a separate 'compilestate' structure someday. It doesn't belong in every process.
  parsemode.i       ; Parsing Mode. Only applicable while parsing text for immediate mode or compiling.
  
  
  ; TODO: Combine executestate and errorstate, and deal with the fallout.
  errorstate.i      ; If nonzero, this process is exiting for some reason, probably an error.
  executestate.i    ; Can currently be active, inactive, or killed. 
  processlistptr.i  ; holds an address to this process' position in the active, inactive, and dead process lists.
  
  
  debugmode.i       ; 0 for off, 1 for per-instruction debug output
  instructions.i    ; The number of instructions this process has executed this timeslice.
  opmax.i           ; The maximum number of instructions this process my execute in one timeslice.
  runtime.i         ; Time in ms this process has executed so far in its life.
  prevtime.i        ; Time in ms this process had executed prior to its current task. (only meaningful for interpreted processes)

  *d.datastack
  *c.callstack
  
  nextcontext.i     ; Next ID for a context change
  context.i         ; Current context. Used for variable management.
  
  currentop.i       ; Current instruction within the codestream. While executing an instruction, will generally point to the next instruction.
  *codeword.Codeset ; Current codestream.
  
  
  
  Map processvariables.i(512) ; map of atom to index for process-scope variables.
  
  List MessageQueue.i()  ; FIFO queue of messages for this process
  ; actual content is an ID for a dataobject (probably a tuple)
  
EndStructure


;- NodeState Definition
; This is for the entire VM at present, eventually this'll need to get divided into global and per-core structures.
; That'll get exciting.

Structure NodeState
  NextProcessID.i
  Map atomtoprimtable.i(256)
  Map primtoatomtable.i(256)
  
  Map atomtowordtable.i(256)
  Map wordtoatomtable.i(256)
  
  Map defines.s(512)  ; holds global defines for the whole system, at present only settable in pb source.
  
  *interpreterprocess.ProcessState
  releasestate.i
  Map primaryreleaselist.i(50000)
  Map secondaryreleaselist.i(50000)
  
  List objectpool.datablock()
  
  Map processpool.ProcessState(50000)
  
  List activeprocesses.i()
  List inactiveprocesses.i()
  List deadprocesses.i()
EndStructure



;- Helper Macros
; These are ease-of-use macros for dealing with some of the vagaries of P twiddling.
Macro P
  *ThisProcess.ProcessState
EndMacro

Macro dstack
  *ThisProcess\d\stack
EndMacro

Macro dtop
  *ThisProcess\d\top
EndMacro

Macro cstack
  *ThisProcess\c\stack
EndMacro

Macro ctop
  *ThisProcess\c\top
EndMacro

Macro pmode
  *ThisProcess\parsemode
EndMacro

Macro cword
  *ThisProcess\codeword
EndMacro

Macro N
  *thisNode.NodeState
EndMacro

Macro debugstate
  *ThisProcess\debugmode
EndMacro  

Macro i (x)
  x >> 2
EndMacro

Macro a (x)
  x >> 2
EndMacro

Macro ptr (x)
  x & ~3
EndMacro



;- Atom Definitions
; Some atoms for process state management.
atom(active)
atom(inactive)
atom(killed)

; Some atoms for process exit management. 
atom(error)
atom(exit)
atom(killed)


atom(true)
atom(false)

;- Procedure Declarations

Declare killproc(P, reason.i, resulttext.s); declared in vm.pb
Declare ResetP(P, reasonatom.i) ; declared in vm.pb
Declare runtimefault(P, errortext.s); declared in vm.pb
Declare setactive(P)
Declare setinactive(P)
Declare setdead(P)
Declare pushstate(P)            ; declared in vm.pb
Declare popstate(P)             ; declared in vm.pb
Declare.s stacktrace(P)         ; declared in stack.pb


;- Timekeeping

CompilerIf #PB_Compiler_OS = #PB_OS_Windows   ; On windows, elapsedmilliseconds() seems to be subject to the standard windows timer foibles. Bah.
  Macro gettime()
    timeGetTime_()
    ; In theory, this shouldn't be any more precise, but somehow it is.
  EndMacro
CompilerElse
  Macro gettime()
    ElapsedMilliseconds()                       ; Once I figure out how to do high res timers on both platforms, I'll move to that. Then again, ms precision may be fine.
  EndMacro
CompilerEndIf

; IDE Options = PureBasic 4.70 Beta 1 (Windows - x64)
; CursorPosition = 150
; FirstLine = 138
; Folding = ---
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory