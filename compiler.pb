; compiler.pb -- compilation routines for frf
; should be called by the including application

XIncludeFile "stack.pb"
XIncludeFile "prims.pb"
XIncludeFile "mem.pb"

Procedure.i appendinstruction(P,Op.i)
  Define *new.codeset
  If cword = 0
    ProcedureReturn
  EndIf
  
  cword = growcodeword(P\Node, cword)
  
  cword\codestream[cword\instructioncount] = Op
  cword\InstructionCount + 1
EndProcedure

;
; Dirty hack!
; 
; Here we have to use PokeD because Purebasic 'helpfully' does a type cast from D to I if we simply do an assignment to the structure.
; So we peek/poke the value directly using manual manipulation of the structure.
; 'codeset' is the structure that holds the stream. It's always 64-bit cells even on 32-bit systems, to accomodate double-sized floats.
;
; a matching hack is found in stack.pb
Procedure.i appendinstructionD(P, value.D)
  Define *thiscword, icount.i, *new.codeset
  If cword = 0
    ProcedureReturn
  EndIf
  
  cword = growcodeword(P\Node, cword)
  
  *thiscword = cword + SizeOf(codeset)
  icount = cword\instructioncount
  
  PokeD(*thiscword + (icount * SizeOf(quad)), value)
  
  cword\InstructionCount + 1
EndProcedure



; flow control

; we define 'pseudoprims' here. They don't have genuine procedures associated with them, so we just generate atoms.
; 

atom(If)
atom(Else)  
atom(then)
atom(begin)
atom(Until)
atom(Repeat)
atom(Continue)
atom(Break)
atom(While)
  
Structure flowcontrolitem
  flowatom.i
  celltarget.i
EndStructure

Threaded Dim flowcontrolstack.flowcontrolitem(1024)
Threaded flowcontroltop.i  


Procedure.i flowtopatom(P)
  ProcedureReturn flowcontrolstack(flowcontroltop)\flowatom
EndProcedure

Procedure.i flowtopcell(P)
  ProcedureReturn flowcontrolstack(flowcontroltop)\celltarget
EndProcedure


Procedure.i searchflowstack(P, searchatom)
  Define i.i, result.i
  For i = flowcontroltop To 0 Step -1
    result = flowcontrolstack(i)\flowatom
    If result = searchatom
      ProcedureReturn 1
    EndIf
  Next i
  ProcedureReturn 0
EndProcedure


Procedure addflowframe(P, thisatom.i)
  flowcontroltop + 1
  flowcontrolstack(flowcontroltop)\flowatom = thisatom
  flowcontrolstack(flowcontroltop)\celltarget = cword\InstructionCount
EndProcedure

Procedure popflowtop(P)
  flowcontrolstack(flowcontroltop)\flowatom = 0
  flowcontrolstack(flowcontroltop)\celltarget = 0
  flowcontroltop - 1
EndProcedure


Procedure dropflowframe(P, stackitem.i)
  Define i.i
  For i = stackitem.i To flowcontroltop
    flowcontrolstack(i)\flowatom = flowcontrolstack(i+1)\flowatom
    flowcontrolstack(i)\celltarget = flowcontrolstack(i+1)\celltarget
  Next i 
  flowcontroltop - 1
EndProcedure

Procedure updatecells(P, firstcell.i, lastcell.i, keyop.i, targetcell.i)
  Define i.i
  For i = firstcell To lastcell
    If (cword\codestream[i] = keyop.i) And (cword\codestream[i+1] = keyop.i)
      cword\codestream[i+1] = targetcell
    EndIf
  Next i
EndProcedure

Procedure unexpectedprim(P, foundatom, previousatom)
  addline(0, "Error: unexpected '"+AtomToString(foundatom)+"' after '"+AtomToString(previousatom)+"'.")
EndProcedure


Procedure.i checkflowcontrol(P, maybeop.i)
  Define ifloc.i, thenloc.i, elseloc.i, loopstart.i, loopend.i
  Select MaybeOp
    Case _if
      addflowframe(P, _if)
      appendinstruction(P,0)
      appendinstruction(P,0)
    Case _else
      If flowtopatom(P) = _if
        addflowframe(P, _else)
        appendinstruction(P,0)
        appendinstruction(P,0)
      Else
        unexpectedprim(P, _else, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _then
      Select flowtopatom(P)
        Case _if
          ifloc.i = flowtopcell(P)
          thenloc.i = cword\instructioncount
          popflowtop(P)
          cword\codestream[ifloc] = @cjmp()
          cword\codestream[ifloc+1] = thenloc
        Case _else
          elseloc.i = flowtopcell(P)
          popflowtop(P)
          ifloc.i = flowtopcell(P)
          popflowtop(P)
          thenloc.i = cword\instructioncount
          cword\codestream[ifloc] = @cjmp()
          cword\codestream[ifloc+1] = elseloc+2
          cword\codestream[elseloc] = @jmp()
          cword\codestream[elseloc+1] = thenloc
        Default
          unexpectedprim(P, _then, flowtopatom(P))
          ResetP(P, _error)
      EndSelect
    Case _begin
      addflowframe(P, _begin)
    Case _until
      If flowtopatom(P) = _begin
        loopstart.i = flowtopcell(P)
        loopend.i = cword\instructioncount
        popflowtop(P)
        appendinstruction(P, @cjmp())
        appendinstruction(P, loopstart)
        updatecells(P, loopstart, loopend, @p_continue(), loopstart)
        updatecells(P, loopstart, loopend, @p_break(), loopend+2)
        updatecells(P, loopstart, loopend, @p_while(), loopend+2)
      Else
        unexpectedprim(P, _until, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _repeat
      If flowtopatom(P) = _begin
        loopstart.i = flowtopcell(P)
        loopend.i = cword\instructioncount
        popflowtop(P)
        appendinstruction(P, @jmp())
        appendinstruction(P, loopstart)
        updatecells(P, loopstart, loopend, @p_continue(), loopstart)
        updatecells(P, loopstart, loopend, @p_break(), loopend+2)
        updatecells(P, loopstart, loopend, @p_while(), loopend+2)
      Else
        unexpectedprim(P, _repeat, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _while
      If searchflowstack(P, _begin)
        appendinstruction(P, @p_while())
        appendinstruction(P, @p_while())
      Else
        unexpectedprim(P, _while, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _continue
      If searchflowstack(P, _begin)
        appendinstruction(P, @p_continue())
        appendinstruction(P, @p_continue())
      Else
        unexpectedprim(P, _continue, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _break
      If searchflowstack(P, _begin)
        appendinstruction(P, @p_break())
        appendinstruction(P, @p_break())
      Else
        unexpectedprim(P, _break, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Default
      ProcedureReturn 0 ; returns 0 if this doesn't match a flow control construct
  EndSelect
  ProcedureReturn 1     ; returns 1 if this was successfully handled as flow control
EndProcedure





Procedure tokenize(P,inputstr.s)
  Define *op, firstcharasc.i, MaybeOp.i 
  *op = 0
  firstcharasc.i = Asc(Left(inputstr,1))
  
  ; Are we dealing with a number?
  ; 48 to 57 is digit. 45 is -. 36 is $. 37 is %. 
  
  If (firstcharasc >= 48 And firstcharasc <= 57) Or (Len(inputstr) >=2 And (firstcharasc = 36 Or firstcharasc = 45 Or firstcharasc = 37))
    Select CountString(inputstr, ".")
      Case 0
        appendinstruction(P,@p_pushint())
        appendinstruction(P,Val(inputstr))
        ProcedureReturn 0
      Case 1
        
        appendinstruction(P,@p_pushfloat())
        appendinstructionD(P,ValD(inputstr))
        ProcedureReturn 0
    EndSelect
  EndIf
  
  MaybeOp.i = VerifyAtom(LCase(inputstr))
  
  If checkflowcontrol(P, MaybeOp)
  Else
    *op = atomtoprim(P\Node, MaybeOp)
    If *op
      appendinstruction(P,*op)
    Else
      *op = atomtoword(P\Node, MaybeOp)
      If *op
        appendinstruction(P, @p_call())
        appendinstruction(P, *op)
      Else
        AddLine(0, "Error: unknown token '"+inputstr+"'.")
        ResetP(P, _error)
      EndIf
    EndIf
  EndIf
EndProcedure

Macro checktoken(this)
  If P\Node\defines(this)
    inputstr = P\Node\defines() + inputstr
  Else
    tokenize(P, this)
  EndIf
EndMacro


Procedure.s compilerdirective(P, directive.s, restofline.s)
  Define spaceposition.i, name.s
  Select directive
    Case "var"
      ProcedureReturn restofline
    Case "$def"
      spaceposition = FindString(restofline,Chr(32))
      If spaceposition
        name = Left(restofline,spaceposition-1)
        restofline = Right(restofline,Len(restofline)-spaceposition)
        P\node\defines(name) = restofline
      EndIf
      ProcedureReturn
    Default
      tokenize(P, directive)
      ProcedureReturn restofline
  EndSelect
EndProcedure


Procedure compileatom(P, inputstr.s)
  Define stringatom.i
  stringatom = StringtoAtom(inputstr)
  appendinstruction(P,@p_pushatom())
  appendinstruction(P, stringatom)
EndProcedure


Procedure compilestring(P, inputstr.s)
  Define stringatom.i
  stringatom = StringtoAtom(inputstr)
  appendinstruction(P,@p_pushstring())
  appendinstruction(P, stringatom)
EndProcedure



; stubs for later


 
; pmode (shorthand macro for parsemode in current P):
; with bit 0 (1) set, comment.     ( )
; with bit 1 (2) set, compile.     : ;
; with bit 2 (4) set, string.      " "
; with bit 3 (8) set, escape character.  \
; with bit 4 (16) set, compiler directive
; with bit 5 (32) set, atom.   ' '

; These macros save me some boilerplate.

Macro StartParsing
  Define workingstring.s, nextchar.s
  workingstring.s = ""
  While Len(inputstr)
    nextchar.s = Left(inputstr, 1)
    inputstr = Right(inputstr, Len(inputstr)-1)
    Select nextchar
EndMacro

Macro continueparsing
  workingstring = workingstring + nextchar
EndMacro

Macro StopParsing
    Default
      continueparsing
    EndSelect
  Wend
  ProcedureReturn inputstr
EndMacro


Procedure.s parsedirective(P, inputstr.s)
  startparsing
    Case " "
      inputstr = compilerdirective(P, workingstring, inputstr)
      workingstring = ""
      pmode = pmode & ~16
      ProcedureReturn
  StopParsing
EndProcedure


Procedure.s parseatom(P, inputstr.s)
  StartParsing
    Case "'"  ; Single Quote
      compileatom(P, workingstring)
      workingstring = ""
      pmode = pmode & ~32
      ProcedureReturn inputstr
  StopParsing
EndProcedure


Procedure.s parsestring(P, inputstr.s)
  StartParsing
    Case "r", "R"
      If (pmode & 8) = 8
        pmode = pmode & ~8
        nextchar = #CR$
        continueparsing
      Else
        continueparsing
      EndIf
    Case "\"
      If (pmode & 8) = 8
        pmode = pmode & ~8
        continueparsing
      Else
        nextchar = ""
        pmode = pmode | 8
      EndIf
    Case #DQUOTE$  ; Quotation mark
      If (pmode & 8) = 8
        pmode = pmode & ~8
        continueparsing
        nextchar = ""
      Else
        compilestring(P, workingstring)
        workingstring = ""
        pmode = pmode & ~4
        ProcedureReturn inputstr
      EndIf
  StopParsing
EndProcedure

Procedure.s parseimmed(P, inputstr.s)
  StartParsing
    Case " "
      If Len(workingstring) > 0
        checktoken(workingstring)
      EndIf
      workingstring.s = ""
      If *ThisProcess\errorstate
        ProcedureReturn
      EndIf
    Case "$"
      If Len(workingstring) = 0
        pmode = pmode | 16
        inputstr = nextchar + inputstr
        ProcedureReturn inputstr
      Else
        continueparsing
      EndIf
    Case #DQUOTE$ ; quotation mark
      pmode = pmode | 4
      ProcedureReturn inputstr
    Case "'" ; single quotes
      pmode = pmode | 32
      ProcedureReturn inputstr
    Case "("
      pmode = pmode | 1
      If Len(workingstring) > 0
        checktoken(workingstring)
      EndIf
      workingstring.s = ""
      ProcedureReturn inputstr
    Case ":"
      pmode = pmode | 2
      ProcedureReturn inputstr
  StopParsing
EndProcedure

Procedure.s parsecomment(P, inputstr.s)
  StartParsing
    Case ")"
      pmode = pmode & ~1
      ProcedureReturn inputstr
  StopParsing
EndProcedure

Procedure.s parsecompile(P, inputstr.s)
  StartParsing
    Case " "
      If Len(workingstring) > 0
        checktoken(workingstring)
      EndIf
      workingstring.s = ""
      If *ThisProcess\errorstate
        ProcedureReturn
      EndIf
    Case #DQUOTE$ ; quotation mark
      pmode = pmode | 4
      ProcedureReturn inputstr
    Case "'" ; single quotes
      pmode = pmode | 32
      ProcedureReturn inputstr
    Case "("
      If Len(workingstring) > 0
        checktoken(workingstring)
      EndIf
      workingstring.s = ""
      pmode = pmode | 1
      ProcedureReturn inputstr
    Case ";"
      If Len(workingstring) > 0
        AddLine(0, "Error: unknown token '"+workingstring+";'.")
        ResetP(P, _error)
        ProcedureReturn inputstr
      EndIf
      tokenize(P, "exit")
      If (flowcontroltop > 0)
        AddLine(0, "Error: Unexpected end of word.")
        flowcontroltop = 0
        ResetP(P, _error)
        ProcedureReturn
      Else
        addline(0, "Word added: '"+AtomToString(cword\nameatom)+"' of length "+Str(cword\instructioncount)+".")
        pmode = pmode & ~2
        popstate(P)
      EndIf
      ProcedureReturn inputstr
  StopParsing
EndProcedure

 
 
Procedure newword(P, inputstr.s)
  Define wordatom.i
  Select inputstr
    Case ":", ";", "@", "!", "var"
      AddLine(0, "Error: invalid name.")
      pmode = 0
      ResetP(P, _error)
      ProcedureReturn
  EndSelect
  
  wordatom.i = newatom(inputstr)
  If wordatom
    If atomtoword(P\Node, wordatom)
      pushstate(P)
      cword = atomtoword(P\Node, wordatom)
      cword\nameatom = wordatom
      cword\instructioncount = 0
      AddLine(0, "Recompiling word '"+inputstr+"'.")
    Else
      pushstate(P)
      cword = newcodeword(P\Node, 1024, wordatom)
      AddLine(0, "Compiling new word '"+inputstr+"'.")
    EndIf
  EndIf
EndProcedure


Procedure.s parsenewword(P, inputstr.s)
  StartParsing
      Case "("
        pmode = pmode | 1
        If (Len(workingstring) > 0) And (workingstring <> " ")
          newword(P, workingstring)
        EndIf
        ProcedureReturn inputstr
      Case  " "
        If (Len(workingstring) > 0) And (workingstring <> " ")
          newword(P, workingstring)
        EndIf
        
        ProcedureReturn inputstr
  StopParsing
EndProcedure


Procedure.i parseline(P,inputstr.s)  
  Define workingstring.s
  workingstring.s = Trim(inputstr)+" "
  
  While (Len(workingstring) > 0) And (*ThisProcess\errorstate = 0)
    If pmode & 1
      workingstring = parsecomment(P, workingstring)
      Continue
    EndIf
    If pmode & 32
      workingstring = parseatom(P, workingstring)
      Continue
    EndIf
    If pmode & 16
      workingstring = parsedirective(P, workingstring)
      Continue
    EndIf
    If pmode & 4
      workingstring = parsestring(P, workingstring)
      Continue
    EndIf
    If pmode & 2
      If cword\nameatom = 0
        workingstring = parsenewword(P, workingstring)
      Else
        workingstring = parsecompile(P, workingstring)
      EndIf
      Continue
    EndIf
    workingstring = parseimmed(P, workingstring)
  Wend
EndProcedure


Procedure loadfile(P, filename.s)
  Define filenum.i
  filename = Trim(LCase(filename))
  filenum.i = ReadFile(#PB_Any, filename)
  If filenum
    While Eof(filenum) = 0
      parseline(P, ReadString(filenum))
    Wend
    CloseFile(filenum)
  Else
    addline(0, "Error: '"+filename+"' could not be accessed.")
    ResetP(P, _error)
  EndIf
EndProcedure



Procedure p_loadfile(P)
  Define *input.datastring
  needstack(1)
  popstring(*input)
  loadfile(P, *input\value)
EndProcedure
registerprim(loadfile, @p_loadfile())




;
;
;-  Built-in defines
;
; .. yes, they need the space at the end. the parser is still kind of pants.
;
;

N\defines("case") = "begin dup "
N\defines("when") = "if pop "
N\defines("end") = "break then dup "
N\defines("default") = "pop 1 if "
N\defines("endcase") = "pop pop 1 until "
N\defines("}t") = "} tuple_make "
N\defines("t@") = "tuple_getelement "
N\defines("0@") = "0 tuple_getelement "
N\defines("1@") = "1 tuple_getelement "
N\defines("2@") = "2 tuple_getelement "
N\defines("3@") = "3 tuple_getelement "
N\defines("4@") = "4 tuple_getelement "
N\defines("t!") = "tuple_setelement "
N\defines("0!") = "0 tuple_setelement "
N\defines("1!") = "1 tuple_setelement "
N\defines("2!") = "2 tuple_setelement "
N\defines("3!") = "3 tuple_setelement "
N\defines("4!") = "4 tuple_setelement "

; IDE Options = PureBasic 5.71 LTS (Windows - x64)
; CursorPosition = 514
; FirstLine = 510
; Folding = ------
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory