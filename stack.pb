; stacks.pb -- core stack definitions and manipulation routines


XIncludeFile "defines.pb"
XIncludeFile "mem.pb"

Macro needstack(x)
  If dtop < x
    runtimefault(P, "Stack underflow.")
    ProcedureReturn 0
  EndIf
EndMacro


Procedure push(P, x.i)
  Define typeatom.i, *indirect.dataindirect
  typeatom = checktype(x)  
  If typeatom = _type_indirect
    *indirect = (x & ~3)
    x = *indirect\target
  EndIf
  If dtop < *ThisProcess\d\max
    useobject(P\node, x)
    If typeatom = _type_tuple
      *indirect = newobject(P\Node)
      *indirect\typeatom = _type_indirect
      *indirect\refcount = 1
      *indirect\target = x
      x = *indirect | 3
    Else
    EndIf
    dstack[dtop] = x
    dtop + 1
  Else
    runtimefault(P, "Error: Stack overflow.")
  EndIf
EndProcedure

Procedure pushint(P, x.i)
  push(P, (x << 2)+1)
  If *ThisProcess\errorstate
    ProcedureReturn
  EndIf
EndProcedure

Procedure pushatom(P, x.i)
  push(P, (x << 2)+2)
  If *ThisProcess\errorstate
    ProcedureReturn
  EndIf
EndProcedure

Procedure pushtuple(P, x.i)
  Define *tuple.datatuple
  push(P, x)
  If *ThisProcess\errorstate
    ProcedureReturn
  EndIf
EndProcedure


Procedure pushfloat(P, x.d)
  Define identifier.i, *newfloat.datafloat
  identifier = newobject(P\Node)
  *newfloat = identifier
  identifier | 3
  *newfloat\typeatom = _type_float
  *newfloat\value = x
  push(P, identifier)
EndProcedure

Procedure pushstring(P, x.s)
  
  push(P, newstring(P\node, x))
EndProcedure

Procedure pushmark(P)
  Define identifier, *dataobject.datamark
  identifier = newobject(P\Node)
  *dataobject = identifier
  identifier | 3
  *dataobject\typeatom = _type_stackmark
  push(P, identifier)
EndProcedure
registerprim("core", "{", @pushmark())





Procedure p_pushint(P)
  pushint(P, cword\codestream[*ThisProcess\currentop])
  *ThisProcess\currentop + 1
EndProcedure

Procedure p_pushatom(P)
  pushatom(P, cword\codestream[*ThisProcess\currentop])
  *ThisProcess\currentop + 1
EndProcedure


;
; Dirty hack!
; 
; Here we have to use PokeD because Purebasic 'helpfully' does a type cast from D to I if we simply do an assignment to the structure.
; So we peek/poke the value directly using manual manipulation of the structure.
; 'codeset' is the structure that holds the stream. It's always 64-bit cells even on 32-bit systems, to accomodate double-sized floats.
;
; a matching hack is found in compiler.pb
Procedure p_pushfloat(P)
  Define value.d
  
  value = PeekD(cword + SizeOf(codeset) + *ThisProcess\currentop * SizeOf(quad))
  pushfloat(P, value)
  *ThisProcess\currentop + 1
EndProcedure
; 
;



Procedure p_pushstring(P)
  Define value.s
  value = AtomToString(cword\codestream[*ThisProcess\currentop])
  pushstring(P, value)
  *ThisProcess\currentop + 1
EndProcedure



Procedure.i pop(P)
  Define result.i, *indirect.dataindirect
  
  needstack(1)

  dtop - 1
  result = dstack[dtop]
  unuseobject(P\Node, result)
  dstack[dtop] = 0
  If checktype(result) = _type_indirect
    *indirect = (result & ~3)
    ProcedureReturn *indirect\target
  EndIf
  ProcedureReturn result
EndProcedure
registerprim("core", "pop", @pop())


Procedure.i isint(P, x.i, *out.Integer)
  If (x & 3) = 1
    *out\i = x >> 2
    ProcedureReturn 1
  Else
    ProcedureReturn 0
  EndIf
EndProcedure

Procedure.i isatom(P, x.i, *out.Integer)
  If (x & 3) = 2
    *out\i = x >> 2
    ProcedureReturn 1
  Else
    ProcedureReturn 0
  EndIf
EndProcedure

Procedure.i isfloat(P, x.i, *out.double)
  Define *float.datafloat
  If (x & 3) = 3
    *float = x & ~3
    If *float\typeatom = _type_float
      *out\d = *float\value
      ProcedureReturn 1
    EndIf
  EndIf
  ProcedureReturn 0
EndProcedure




Macro popint(x)
  x = pop(P)
  If *ThisProcess\errorstate
    ProcedureReturn
  EndIf
  If isint(P, x, @x)
  Else
    runtimefault(P, "Expected integer.")
    ProcedureReturn
  EndIf
EndMacro

Macro popatom(x)
  x = pop(P)
  If *ThisProcess\errorstate
    ProcedureReturn
  EndIf
  If isatom(P, x, @x)
  Else
    runtimefault(P, "Expected atom.")
    ProcedureReturn
  EndIf
EndMacro

Macro popfloat(x)
  Define y.i
  y = pop(P)
  If *ThisProcess\errorstate
    ProcedureReturn
  EndIf
  If isfloat(P, y, @x)
  Else
    runtimefault(P, "Expected float.")
    ProcedureReturn
  EndIf
EndMacro

Macro popstring(x)
  Define y.i
  y = pop(P)
  If checktype(y) = _type_string
    x = y & ~3
  Else
    runtimefault(P, "Expected string.")
    ProcedureReturn
  EndIf
EndMacro

Macro poptuple(x)
  Define y.i
  y = pop(P)
  If checktype(y) = _type_tuple
    x = y
  Else
    runtimefault(P, "Expected tuple.")
    ProcedureReturn
  EndIf
EndMacro


Macro getnumeric(input, targetvar)
  Define placehold.i
  If isint(P,input, @placehold)
    targetvar = placehold
  Else
    If isfloat(P, input, @targetvar)
    Else
      runtimefault(P, "Expected numeric.")
      ProcedureReturn
    EndIf
  EndIf
EndMacro


Procedure checktrue(P, x.i)
  Define result.i, float.d, *string.datastring
  If (x & 3) = 1
    result= x >> 2
  Else
    result = 0
    Select checktype(x)
      Case _type_atom
        If (x >> 2) = _true
          result = 1
          EndIf
      Case _type_float
        isfloat(P, x, @float)
        If float <> 0.0
          result= 1
        EndIf
      Case _type_string
        *string = x
        If (*string\value) <> ""
          result = 1
        EndIf
      Default
    EndSelect
  EndIf
  
  ProcedureReturn result
EndProcedure


Macro integers(first, second)
  ((second & 3) = 1) And ((first & 3) = 1)
EndMacro


Procedure.s formatobject(N, object.i)
  Define workingstring.s
  Select checktype(object)
    Case _type_empty
      workingstring = "(Empty)"
    Case _type_integer
      workingstring = Str(object >> 2)
    Case _type_float
      Define value.d, *float.datafloat
      *float = (object & ~3)
      value = *float\value
      workingstring= StrD(*float\value)
      workingstring= RTrim(workingstring, "0")
      If Right(workingstring, 1) = "."
        workingstring + "0"
      EndIf
    Case _type_atom
      workingstring = "'"+AtomToString(object >> 2)+"'"
    Case _type_string
      Define *string.datastring
      *string = (object & ~3)
      workingstring = #DQUOTE$+*string\value+#DQUOTE$
    Case _type_stackmark
      workingstring = "{"
    Case _type_indirect
      Define *indirect.dataindirect
      *indirect = (object & ~3)
      workingstring = formatobject(N, *indirect\target)
    Case _type_tuple
      Define *tuple.datatuple, i
      workingstring= "{ " 
      *tuple = (object & ~3)
      For i = 0 To *tuple\length - 1
        workingstring = workingstring + formatobject(N, *tuple\content\q[i]) + ", "
      Next i
      workingstring= Left(workingstring,Len(workingstring)-2) + " }"
    Default
      workingstring= "(UnknownObject)"
  EndSelect
  ProcedureReturn workingstring
EndProcedure

Procedure.s stacktrace(P)
  Define outputstring.s, i.i, object.i, tempstring.s
  If dtop > 0
    For i = 0 To dtop - 1
      object = dstack[i]
      tempstring = formatobject(P\node, object)
      If checktype(object) = _type_string And Len(tempstring) > 100
        tempstring = Left(tempstring,50) + #DQUOTE$ + "..." + #DQUOTE$ +Right(tempstring, 50)
      EndIf
      outputstring=outputstring + tempstring + "  "
    Next i
    outputstring = Left(outputstring,Len(outputstring)-2)
    ProcedureReturn outputstring
  EndIf
EndProcedure


; IDE Options = PureBasic 5.20 beta 16 LTS (Windows - x86)
; CursorPosition = 145
; FirstLine = 134
; Folding = -----
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory