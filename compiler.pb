; compiler.pb -- compilation routines for frf
; should be called by the including application

XIncludeFile "stack.pb"
XIncludeFile "prims.pb"
XIncludeFile "mem.pb"

Procedure.i appendcell(*cs.compilestate,Op.i)
  Define *new.codeset
  If *cs\codeword = 0
    ProcedureReturn
  EndIf
  
  *cs\codeword = growcodeword(*cs\Node, *cs\codeword)
  
  *cs\codeword\codestream[*cs\codeword\instructioncount] = Op
  *cs\codeword\InstructionCount + 1
EndProcedure

;
; Dirty hack!
; 
; Here we have to use PokeD because Purebasic 'helpfully' does a type cast from D to I if we simply do an assignment to the structure.
; So we peek/poke the value directly using manual manipulation of the structure.
; 'codeset' is the structure that holds the stream. It's always 64-bit cells even on 32-bit systems, to accomodate double-sized floats.
;
; a matching hack is found in stack.pb

Procedure.i appendcellD(*cs.compilestate, value.D)
  Define *thiscword, icount 
  Define *new.codeset
  If *cs\codeword = 0
    ProcedureReturn
  EndIf
  
  *cs\codeword = growcodeword(*cs\Node, *cs\codeword)
  
  *thiscword = *cs\codeword + SizeOf( codeset )
  icount = *cs\codeword\instructioncount
  
  PokeD(*thiscword + (icount * SizeOf(quad)), value)
  
  *cs\codeword\InstructionCount + 1 
EndProcedure


atom(squote)
atom(dquote)
atom(parens)
atom(unspecified)


Procedure.s nextitem(*cs.compilestate)
  
  Define Result.s, workingstring.s, thischar.s, nextchar.s, escapedchar.s, index.i, done.i
  Define BlockType
  
  workingstring = Trim(*cs\stream)
  
  ; First thing we do is make sure we're dealing with a non-whitespace first character.
  ; The trim will get rid of leading spaces, but it won't get rid of leading cr/lf/tab,
  ; and it's possible there's spaces after those. So, we iterate until we find a non-cr/lf/tab/space character.
  index = 0
  Repeat
    thischar = Mid(workingstring,index, 1)
    If (Not (thischar = #CR$ Or thischar = #LF$ Or thischar = " " Or thischar = #TAB$))
      done = 1
      index - 1
    Else
      index + 1
    EndIf
  Until done Or (Len(workingstring) <= index)
  
  workingstring = Right(workingstring, Len(workingstring) - index)
  index = 0
  done = 0
  
  ; Was there actually nothing useful left? If so, bail
  If Len(workingstring) = 0
    *cs\stream = ""
    ProcedureReturn ""
  EndIf
  
  thischar = Mid(workingstring,index, 1)
  result = thischar
  
  index = 1
  
  ; The first character of the string is what determines what kind of string we have.
  ; We deal in single and double quote delimited strings, as well as parentheses
  ; delimited comments.
  ; Everything else is a single 'word'.
  Select thischar
    Case #DQUOTE$
      BlockType = _dquote
    Case "("
      BlockType = _parens
    Case "'"
      BlockType = _squote
    Default
      BlockType = _unspecified
  EndSelect
  
  ; This is kind of ugly and has repetitive bits, but there isn't a good way to avoid that
  ; without ugly macro use, which sharply cuts into readability.
  Repeat
    index + 1
    thischar = Mid(workingstring,index, 1)
    Select thischar
      Case #DQUOTE$
        If BlockType = _dquote
          result = result + thischar
          done = 1
        Else
          result = result + thischar
        EndIf
      Case ")"
        If BlockType = _parens
          result = result + thischar
          done = 1
        Else
          result = result + thischar
        EndIf
      Case "'"
        If BlockType = _squote
          result = result + thischar
          done = 1
        Else
          result = result + thischar
        EndIf
      Case " ", #TAB$, #CR$, #LF$
        If blocktype = _unspecified
          done = 1
        Else
          result = result + thischar
        EndIf
      Default
        
        ; this is where we handle 'escape characters'
        ; These are only valid in double quote delimited strings
        ; In every other case, \ is treated like any other printable character
        If BlockType = _dquote And thischar = "\"
          index + 1
          escapedchar = Mid(workingstring,index,1)
          Select escapedchar
            Case "r", "R", "n", "N"
              Result = Result + #LF$
            Case "t", "T"
              result = result + #TAB$
            Case "["
              result = result + #ESC$
            Default
              result = result + escapedchar  
              ; We don't check for cr/lf here because a line ending inside a quoted string 
              ; has undefined behavior. It should be disallowed, but that's work to enforce. :(
          EndSelect
        Else
          result = result + thischar
        EndIf
        
    EndSelect
  Until done Or index > Len(workingstring)
  
  Debug "text (" + Str(Len(Trim(Result))) + " chars): " + result 
  
  *cs\stream = Right(workingstring, Len(workingstring) - index)
  *cs\result = Result
  ; This might be redundant, but I figure it's better to have it available
  ; Alternately, I might disregard the return value from this procedure
  *cs\resulttype = BlockType
  
  ProcedureReturn result
EndProcedure

atom(line)

Procedure.s nextline(*cs.compilestate)
  Define workingstring.s, index.i, thischar.s, done.i, result.s
  workingstring = Trim(*cs\stream)
  
  Repeat
    thischar = Mid(workingstring, index, 1)
    If thischar = #CR$ Or thischar = #LF$
      done = 1
      Break
    EndIf
    index + 1
  Until done Or index > Len(workingstring)
  
  result = Left(workingstring, index)
  *cs\stream = Right(workingstring, Len(workingstring) - index)
  *cs\result = result
  ; This might be redundant, but I figure it's better to have it available
  ; Alternately, I might disregard the return value from this procedure
  *cs\resulttype = _line
  
  ProcedureReturn result
EndProcedure


Procedure pushcompilestate(*cs.compilestate, itematom.i, from.i, target.i)
  
EndProcedure


Procedure popcompilestate(*cs.compilestate)
  
EndProcedure

;Procedure
  
;EndProcedure


Procedure.i checktopstate(*cs.compilestate)
  ProcedureReturn *cs\compilestack[*cs\compilelevel]
EndProcedure


atom(If)
atom(Else)
atom(then)
  
Procedure.i ifhandler(*cs.compilestate, itematom.i)
  Define self.i
  self = @ifhandler()
  Select itematom
    Case 0
      addflowterm(_if)
      addflowterm(_else)
      addflowterm(_then)
    Case _if
      pushcompilestate(*cs, _if)
    Case _else
      If checktopstate(*cs) = _if
        
      Else
        
      EndIf
    Case _then
      If checktopstate(*cs) = _if
        
      Else
        
      EndIf
    Default
      ProcedureReturn itematom
  EndSelect  
  
  ProcedureReturn 0
EndProcedure




Procedure parsestream(*cs.compilestate, inputstream.s)
  
  
EndProcedure





  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  ;-
  ;- Old crap
  ;-
  
CompilerIf #False
  
  
;Structure flowcontrolitem
;  flowatom.i
;  celltarget.i
;EndStructure

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
  addline("Error: unexpected '"+AtomToString(foundatom)+"' after '"+AtomToString(previousatom)+"'.")
EndProcedure


Procedure.i checkflowcontrol(P, maybeop.i)
  Define ifloc.i, thenloc.i, elseloc.i, loopstart.i, loopend.i
  Select MaybeOp
    Case _if
      addflowframe(P, _if)
      appendcell(P,0)
      appendcell(P,0)
    Case _else
      If flowtopatom(P) = _if
        addflowframe(P, _else)
        appendcell(P,0)
        appendcell(P,0)
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
        appendcell(P, @cjmp())
        appendcell(P, loopstart)
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
        appendcell(P, @jmp())
        appendcell(P, loopstart)
        updatecells(P, loopstart, loopend, @p_continue(), loopstart)
        updatecells(P, loopstart, loopend, @p_break(), loopend+2)
        updatecells(P, loopstart, loopend, @p_while(), loopend+2)
      Else
        unexpectedprim(P, _repeat, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _while
      If searchflowstack(P, _begin)
        appendcell(P, @p_while())
        appendcell(P, @p_while())
      Else
        unexpectedprim(P, _while, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _continue
      If searchflowstack(P, _begin)
        appendcell(P, @p_continue())
        appendcell(P, @p_continue())
      Else
        unexpectedprim(P, _continue, flowtopatom(P))
        ResetP(P, _error)
      EndIf
    Case _break
      If searchflowstack(P, _begin)
        appendcell(P, @p_break())
        appendcell(P, @p_break())
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
  Define *op, firstcharasc, MaybeOp.i 
  *op = 0
  firstcharasc.i = Asc(Left(inputstr,1))
  
  ; Are we dealing with a number?
  ; 48 to 57 is digit. 45 is -. 36 is $. 37 is %. 
  
  If (firstcharasc >= 48 And firstcharasc <= 57) Or (Len(inputstr) >=2 And (firstcharasc = 36 Or firstcharasc = 45 Or firstcharasc = 37))
    Select CountString(inputstr, ".")
      Case 0
        appendcell(P,@p_pushint())
        appendcell(P,Val(inputstr))
        ProcedureReturn 0
      Case 1
        
        appendcell(P,@p_pushfloat())
        appendcellD(P,ValD(inputstr))
        ProcedureReturn 0
    EndSelect
  EndIf
  
  MaybeOp.i = VerifyAtom(LCase(inputstr))
  
;  If checkflowcontrol(P, MaybeOp)
;  Else
;    *op = atomtoprim(P\Node, MaybeOp)
;    If *op
;      appendcell(P,*op)
;    Else
;      *op = atomtoword(P\Node, MaybeOp)
;      If *op
;        appendcell(P, @p_call())
;        appendcell(P, *op)
;      Else
;        AddLine("Error: unknown token '"+inputstr+"'.")
;        ResetP(P, _error)
;      EndIf
;    EndIf
;  EndIf
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
  appendcell(P,@p_pushatom())
  appendcell(P, stringatom)
EndProcedure


Procedure compilestring(P, inputstr.s)
  Define stringatom.i
  stringatom = StringtoAtom(inputstr)
  appendcell(P,@p_pushstring())
  appendcell(P, stringatom)
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
      AddLine("Error: unknown token '"+workingstring+";'.")
      ResetP(P, _error)
      ProcedureReturn inputstr
    EndIf
    tokenize(P, "exit")
    If (flowcontroltop > 0)
      AddLine("Error: Unexpected end of word.")
      flowcontroltop = 0
      ResetP(P, _error)
      ProcedureReturn
    Else
      addline("Word added: '"+AtomToString(cword\nameatom)+"' of length "+Str(cword\instructioncount)+".")
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
      AddLine("Error: invalid name.")
      pmode = 0
      ResetP(P, _error)
      ProcedureReturn
  EndSelect
  
  wordatom.i = newatom(inputstr)
;  If wordatom
;    If atomtoword(P\Node, wordatom)
;      pushstate(P)
;      cword = atomtoword(P\Node, wordatom)
;      cword\nameatom = wordatom
;      cword\instructioncount = 0
;      AddLine("Recompiling word '"+inputstr+"'.")
;    Else
;      pushstate(P)
;      cword = newcodeword(P\Node, 1024, wordatom)
;      AddLine("Compiling new word '"+inputstr+"'.")
;    EndIf
;  EndIf
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










CompilerEndIf



;
;- - asdf






Procedure loadfile(*cs.compilestate, filename.s)
  Define filenum.i
  filename = Trim(LCase(filename))
  filenum.i = ReadFile(#PB_Any, filename)
  Define *thisP.processstate
  
  
  If filenum
    While Eof(filenum) = 0
;      parseline(P, ReadString(filenum))
    Wend
    CloseFile(filenum)
  Else
    addline("Error: '"+filename+"' could not be accessed.")
  EndIf
  
EndProcedure



Procedure p_loadfile(P)
  Define *input.datastring
  needstack(1)
  popstring(*input)
  loadfile(P, *input\value)
EndProcedure
registerprim("core", "loadfile", @p_loadfile())




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



; IDE Options = PureBasic 5.21 LTS (Windows - x86)
; CursorPosition = 212
; FirstLine = 195
; Folding = -------
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory