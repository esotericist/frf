; mem.pb -- memory management routines. If it gets allocated or deallocated, it should happen here.
;   Also includes declarations for various global and threaded variables.


XIncludeFile "defines.pb"


;Declare registerword(N, moduleatom.i, wordatom.i, *word.codeset)
Declare useobject(N, object.i)
Declare unuseobject(N, object.i)



; These are used in the object release queues
atom(primary)
atom(secondary)



Procedure.i checktype(x.i)   ; Returns atom type
  Define *objptr.dataobject, typeoutput.i
  If (x & 3) = 0
    ProcedureReturn _type_empty
  ElseIf (x & 3) = 1
    ProcedureReturn _type_integer
  ElseIf (x & 3) = 2
    ProcedureReturn _type_atom
  ElseIf (x & 3) = 3
    *objptr = (x & ~3)
    typeoutput = *objptr\typeatom
    ProcedureReturn typeoutput
  Else
    ProcedureReturn _type_unknown
  EndIf
EndProcedure

;-
;- Allocation functions


Procedure.i newobject(N)
  Define newobject.i
  newobject = AddElement(N\objectpool())
  ProcedureReturn newobject
EndProcedure

Procedure.i newstring(N, input.s)
  Define identifier.i, *newstring.datastring
  identifier = newobject(N)
  *newstring = identifier
  identifier | 3
  *newstring\typeatom = _type_string
  *newstring\value = Input
  *newstring\refcount = 1
  unuseobject(N, identifier)
  
  ProcedureReturn identifier
EndProcedure


Procedure.i newprocID(N)
  N\NextProcessID + 1
  ProcedureReturn N\NextProcessID
EndProcedure

Procedure.i newcontext(P)
  *ThisProcess\nextcontext + 1
  ProcedureReturn *ThisProcess\nextcontext
EndProcedure

Procedure.i newprocess(N,dsize.i, csize.i)
  Define *ThisProcess.ProcessState, pid.i
  pid.i = newprocID(N)
  *ThisProcess = AddMapElement(N\processpool(),Str(pid))
  InitializeStructure(*ThisProcess,ProcessState)
  *ThisProcess\pid = pid
  *ThisProcess\opmax = 10000
  *ThisProcess\c.callstack = AllocateMemory(SizeOf(callstack)+SizeOf(executeframe) * csize)
  ;Debug "New callstack: " + Str(*ThisProcess\c)
  ;  Debug SizeOf(callstack)
  ;  Debug SizeOf(integer) * csize
  *ThisProcess\c\max = csize
  *ThisProcess\d = AllocateMemory(SizeOf(datastack)+SizeOf(integer) * dsize)
  ;Debug "New datastack: " + Str(*ThisProcess\d)
  *ThisProcess\d\max = dsize
  *ThisProcess\executestate = _active
  *ThisProcess\processlistptr = AddElement(N\activeprocesses())
  *ThisProcess\Node = N
  N\activeprocesses() = *ThisProcess
  ProcedureReturn P
EndProcedure


Procedure.i newtuple(N, cellcount.i)
  
  Define identifier.i, *object.datatuple
  *object = newobject(N)
  identifier = *object | 3
  *object\typeatom = _type_tuple
  *object\refcount = 1
  *object\length = cellcount
  unuseobject(N, identifier)
  *object\content = AllocateMemory(cellcount * SizeOf(quad))
  ;  Debug *object\content
  ProcedureReturn identifier
EndProcedure


Procedure.i initnode()
  Define *thisNode.NodeState
  *thisNode = AllocateMemory(SizeOf(NodeState))
  InitializeStructure(*thisNode, NodeState)
  ResetList(*thisNode\objectpool())
  *thisNode\NextProcessID = 1
  
  *thisNode\releasestate = _primary  ; used for the release queues
  
  ;setinactive(newprocess(N, 0, 0))
  ProcedureReturn *thisNode
EndProcedure

Procedure.i newvariableset(N, size.i)
  Define *new.variableset
  *new = AllocateMemory(SizeOf(variableset)+(SizeOf(integer) * size))
  *new\variablecount = size
  ProcedureReturn *new
EndProcedure


Procedure.i newcodeword(N, size.i, wordatom)
  Define *new.codeset
  *new = AllocateMemory(SizeOf(Codeset)+(SizeOf(quad) * size))
  *new\length = size
  If wordatom
    *new\nameatom = wordatom
    ;registerword(N, wordatom,*new)
  EndIf
  ; Debug "New word: " + Str(*new)
  ProcedureReturn *new
EndProcedure


Procedure.i newmodule(N, nameatom)
  Define *new.frfModule
  *new = AllocateMemory(SizeOf(frfModule))
  InitializeStructure(*new, frfModule)
  *new\nameatom = nameatom
  ProcedureReturn *new
EndProcedure
      



Procedure.i newcompilestate(N)
  Define *new.compilestate
  *new = AllocateMemory(SizeOf(compilestate))
  InitializeStructure(*new, compilestate)
  ProcedureReturn *new
EndProcedure


;-
;- Deallocation functions


Procedure freeobject(N,*object)
  ChangeCurrentElement(N\objectpool(),*object)
  DeleteElement(N\objectpool())
EndProcedure

Procedure FreePBString(*Address)  ; Provided by freak:   http://www.purebasic.fr/english/viewtopic.php?f=13&t=23099
  Protected String.String  ; the String Structure contains one String element, which is initialized to 0 on procedure start
  PokeI(@String, *Address) ; poke our string's address into the structure
EndProcedure               ; when returning, PB's string free routine for the local structure will actually free the passed string.
; Kinda hazardous if used improperly


Procedure freetuple(N, *object.datatuple)
  Define i.i, obj.i, *otherobject.dataobject
  ; Debug *object\content
  For i = 0 To *object\length - 1
    obj = *object\content\q[i]
    *otherobject = obj & ~3
    unuseobject(N, *object\content\q[i])
  Next i
  
  FreeMemory(*object\content)
  freeobject(N, *object)
EndProcedure

Procedure freeprocess(N, P)
  Define i.i, x.i, *object.dataobject
  If N\interpreterprocess  = P
    N\interpreterprocess = 0
    DisableGadget(1, 0)
  EndIf
  
  For i = 0 To dtop
    unuseobject(N, dstack[i])
  Next i
  
  ForEach P\messagequeue()
    unuseobject(N, P\messagequeue())
  Next
  ClearList(P\messagequeue()) 
  
  FreeMemory(*ThisProcess\c)
  FreeMemory(*ThisProcess\d)
  If *ThisProcess\executestate = _active
    ChangeCurrentElement(N\activeprocesses(),*ThisProcess\processlistptr)
    DeleteElement(N\activeprocesses())
  EndIf
  If *ThisProcess\executestate = _inactive
    ChangeCurrentElement(N\inactiveprocesses(),*ThisProcess\processlistptr)
    DeleteElement(N\inactiveprocesses())
  EndIf
  If *ThisProcess\executestate = _killed
    ChangeCurrentElement(N\deadprocesses(),*ThisProcess\processlistptr)
    DeleteElement(N\deadprocesses(),1)
  EndIf
  DeleteMapElement(N\processpool(), Str(*ThisProcess\pid))
EndProcedure

;-
;- Object management


Procedure useobject(N,object.i)
  Define *object.dataobject, key.s
  If (object & 3) = 3
    *object = (object & ~3)
    If *object\refcount < 1
      *object\refcount = 1
      key=Str(object)
      DeleteMapElement(N\primaryreleaselist(),key)
      DeleteMapElement(N\secondaryreleaselist(),key)
      Debug "Unqueued"+Str(object)+"; "+AtomToString(*object\typeatom)
    Else
      *object\refcount + 1
    EndIf
    ;Debug "Usecount for "+Str(object)+" increased to "+Str(*object\refcount)+"; "+AtomToString(*object\typeatom)
  EndIf
EndProcedure

Procedure queuerelease(N, x.i)
  Define *object.dataobject, key.s
  *object = x & ~3
  key=Str(x)
  
  Select N\releasestate
      
    Case _primary
      ;Debug "primary release queued for: "+key+"; "+AtomToString(*object\typeatom)
      
      N\primaryreleaselist(key) = x
      
    Case _secondary
      ;Debug "secondary release queued for: "+key+"; "+AtomToString(*object\typeatom)
      
      N\secondaryreleaselist(key) = x
  EndSelect

EndProcedure

Procedure unuseobject(N,object.i)
  Define *object.dataobject
  If (object & 3) = 3
    *object = (object & ~3)
    *object\refcount - 1
    ;Debug "Usecount for "+Str(object)+" decreased to "+Str(*object\refcount)+"; "+AtomToString(*object\typeatom)
    
    If *object\refcount < 1
      queuerelease(N, object)
    EndIf
  EndIf
EndProcedure


Procedure releasethis(N, x.i)
  Define *object.dataobject, *stringobject.datastring, *indirect.dataindirect, typeatom
  If x & 3 = 3
    *object = x &~3
    If *object\refcount <= 0
      typeatom = checktype(x)
      ;Debug "Actual release for: "+Str(x)+"; "+AtomToString(*object\typeatom)
      
      Select typeatom
        Case _type_string
          *stringobject = *object
          FreePBString(@*stringobject.datastring\value)
          freeobject(N, *object)
        Case _type_indirect
          *indirect = *object
          unuseobject(N, *indirect\target)
          freeobject(N, *object)
        Case _type_tuple
          freetuple(N, *object)
        Default
          freeobject(N, *object)
      EndSelect
    EndIf 
  EndIf
  
EndProcedure

Procedure releaseobjects(N)
  Define x.i
  N\releasestate = _secondary
  ; Setting this to _secondary causes the queuerelease() procedure to put future releases into the secondary list
  ; We do this so that if we go to release an object and that reduces references to another object to 0, we
  ; aren't changing the size of the releasequeue we're working on. That would be awkward.
  
  If MapSize(N\primaryreleaselist())
    
    ResetMap(N\primaryreleaselist())
    ForEach N\primaryreleaselist()
      x = N\primaryreleaselist()
      releasethis(N, x)
    Next N\primaryreleaselist()
    
    ; we used to delete each element during the foreach loop, but that went badly. Now we just clear the whole map. It's faster and less error prone.
    ClearMap(N\primaryreleaselist())
  EndIf 
  
  N\releasestate = _primary
  ; Now we're done with the primary list, we set the state back so that if we have to release anything
  ; during the secondary pass, it can go in the primary queue.
  
  If MapSize(N\secondaryreleaselist())
    ResetMap(N\secondaryreleaselist())
    ForEach N\secondaryreleaselist()
      x = N\secondaryreleaselist()
      releasethis(N, x)
    Next N\secondaryreleaselist()
    ClearMap(N\secondaryreleaselist())
    ; second verse, same as the first, a little bit obtuse, a little bit recursed
    
  EndIf
  ; We keep doing this until we run out of objects. This could potentially hitch things, but fukkit, we can optimize later.
  If MapSize(N\primaryreleaselist())
    releaseobjects(N)
  EndIf
  
  ; Being explicit.
  N\releasestate = _primary
EndProcedure


;-
;- Module, Prim, and word management

;Procedure.i addflowterm(N, controlproc.i, termatom.i)
;  N\terms(Str(termatom)) = controlproc
;EndProcedure

Macro addflowterm(termatom)
  *cs\node\terms(Str(termatom)) = self
EndMacro



atom(prim)
atom(word)
Procedure.i modulelookup(N, modulenameatom.i)
;  Define *moduleptr.frfmodule
;  *moduleptr = 
  ProcedureReturn N\moduleset(Str(modulenameatom))
EndProcedure


Define moduleatom.i, primatom.i, primatomstr.s
Define *modptr.frfmodule

Macro registerprim(modulename, primname, pointer)
  moduleatom = newatom(modulename)
  primatom = newatom(primname)
  *modptr = modulelookup(N, moduleatom)
  If *modptr = 0
    *modptr = newmodule(N, moduleatom)
  EndIf
  primatomstr = Str(primatom)
  *modptr\dictionary(primatomstr)\EntryTypeAtom = _prim
  *modptr\dictionary(primatomstr)\primpointer = pointer
 
  N\primtoatomtable(Str(pointer)) = primatom
EndMacro

Procedure.i primtoatom(N, inputprim.i)
  ProcedureReturn N\primtoatomtable(Str(inputprim))
EndProcedure





;Procedure.i atomtoprim(N, inputatom.i)
;  Define *primptr, primval.i
;  *primptr = FindMapElement(N\atomtoprimtable(),Str(inputatom))
;  If *primptr
;    primval = PeekI(*primptr)
;  EndIf
;  ProcedureReturn primval
;EndProcedure

;Procedure.i primtoatom(N, inputprim.i);
;  Define *atomptr, atomval.i
;  *atomptr = FindMapElement(N\primtoatomtable(),Str(inputprim))
;  If *atomptr
;    atomval = PeekI(*atomptr)
;  EndIf
;  ProcedureReturn atomval
;EndProcedure


Procedure registerword(N, moduleatom.i, wordatom.i, *word.codeset) 
  Define *modptr.frfmodule, *dictentry.frfDictionaryEntry
  *modptr = N\moduleset(Str(moduleatom))
  If *modptr
    *dictentry = FindMapElement(*modptr\dictionary(), Str(wordatom))
    *dictentry\EntryTypeAtom = _word
    *dictentry\wordpointer = *word
  Else
    Debug "registerword error: module not found"
  EndIf
EndProcedure



;Procedure registerword(N, wordatom.i, *word.codeset)
;  If Not(FindMapElement(N\wordtoatomtable(), Str(wordatom)))
;    N\atomtowordtable(Str(wordatom)) = *word
;    N\wordtoatomtable(Str(*word)) = wordatom
;  EndIf
;EndProcedure

;Procedure unregisterword(N, wordatom.i)
;  Define *word.codeset
;  *word = N\atomtowordtable(Str(wordatom))
;  DeleteMapElement(N\atomtowordtable())
;  N\wordtoatomtable(Str(*word))
;  DeleteMapElement(N\wordtoatomtable())
;EndProcedure


;Procedure.i atomtoword(N, inputatom.i)
;  Define *wordptr, wordval.i
;  *wordptr = FindMapElement(N\atomtowordtable(),Str(inputatom))
;  If *wordptr
;    wordval = PeekI(*wordptr)
;  EndIf
;  ProcedureReturn wordval
;EndProcedure

;Procedure.i wordtoatom(N, inputword.i)
;  Define *atomptr, atomval.i
;  *atomptr = FindMapElement(N\wordtoatomtable(), Str(inputword))
;  If *atomptr 
;    atomval = PeekI(*atomptr)
;  EndIf
;  ProcedureReturn atomval
;EndProcedure



Procedure growcodeword(N, *word.codeset)
  Define *new.codeset
  If *word\InstructionCount >= *word\length -1
    *new = newcodeword(N, *word\length + 1024, *word\nameatom)
    CopyMemory(*word, *new, SizeOf(codeset) + (SizeOf(quad) * (*word\length)))
    *new\length + 1024
    ProcedureReturn *new
  EndIf
  ProcedureReturn *word
EndProcedure


; IDE Options = PureBasic 5.21 LTS (Windows - x86)
; CursorPosition = 256
; FirstLine = 194
; Folding = g+---
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory