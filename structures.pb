; structures.pb -- routines for handling tuples, arrays, etc
;

XIncludeFile "stack.pb"
XIncludeFile "mem.pb"

Procedure.i struct_getelement(N, tupleID.i, element.i)
  Define *tuple.datatuple, resultID.i
  *tuple=tupleID & ~3
  If element > *tuple\length
    AddLine("Error: element out of bounds.")
    ProcedureReturn 0
  EndIf
  resultID = *tuple\content\q[element]
  ProcedureReturn resultID
EndProcedure

Procedure.i struct_setelement(N, tupleID, element.i, input.i)
  Define *tuple.datatuple, newID.i, *newtuple.datatuple, i.i
  *tuple = tupleID & ~3
  
  If element > *tuple\length - 1
    AddLine("Error: element out of bounds.")
    ProcedureReturn 0
  EndIf
  If *tuple\refcount > 0
    newID = newtuple(N,*tuple\length)
    *newtuple = newID & ~3
    For i = 0 To *tuple\length -1
      If i = element
        *newtuple\content\q[i] = input.i
      Else
        *newtuple\content\q[i] = *tuple\content\q[i]
      EndIf
      useobject(N, *newtuple\content\q[i])
    Next i
    tupleid = newID
  Else
    useobject(N, input)
    *tuple\content\q[element] = input
  EndIf
  ProcedureReturn tupleID
EndProcedure

Procedure p_tuple_getelement(P)
  Define element.i, tupleID.i, *tuple.datatuple, resultID.i
  needstack(2)
  popint(element)
  poptuple(tupleID)
  resultID = struct_getelement(P\node, tupleID, element)
  If resultID
    push(P, resultID)
  Else
    AddLine("Error: element out of bounds.")
    resetP(P, _error)
    ProcedureReturn
  EndIf
EndProcedure
registerprim(tuple_getelement,@p_tuple_getelement())

Procedure p_tuple_explode(P)
  Define tuplesize.i, tupleID.i, *tuple.datatuple, i.i, x.i
  needstack(1)
  poptuple(tupleID)
  *tuple = tupleID & ~3
  tuplesize = *tuple\length
  For i = 0 To tuplesize -1
    x=*tuple\content\q[i]
    push(P, x)
  Next i
  pushint(P, tuplesize)
EndProcedure
registerprim(tuple_explode,@p_tuple_explode())

Procedure p_tuple_setelement(P)  ; data, tuple, index
  Define tupleID, *tuple.datatuple, input.i, element.i, resultID
  needstack(3)
  
  popint(element)
  poptuple(tupleID)
  input = pop(P)
  
  resultID = struct_setelement(P\node, tupleID, element, input)
  If resultID
    push(P,resultID)
  Else
    AddLine("Error: element out of bounds.")
    resetP(P, _error)
    ProcedureReturn
  EndIf
EndProcedure
registerprim(tuple_setelement,@p_tuple_setelement())

Procedure p_tuple_make(P)
  Define tuplesize.i, tupleID.i, *tuple.datatuple, i.i, x.i
  needstack(1)
  popint(tuplesize)
  
  If tuplesize < 1
    addline("Error: tuple_make expects a positive integer.")
    ResetP(P, _error)
    ProcedureReturn
  EndIf
  
  needstack(tuplesize)
  tupleID = newtuple(P\node, tuplesize)
  *tuple = tupleID & ~3
  
  For i = tuplesize - 1 To 0 Step -1
    x = pop(P)
    *tuple\content\q[i] = x
    useobject(P\node, x)
  Next i
  
  pushtuple(P, tupleID)
EndProcedure
registerprim(tuple_make,@p_tuple_make())




; IDE Options = PureBasic 5.71 LTS (Windows - x64)
; CursorPosition = 65
; FirstLine = 41
; Folding = --
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory