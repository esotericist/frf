; vm.pb -- the core virtual machine of frf
; when complete can be included in other projects



XIncludeFile "defines.pb"
XIncludeFile "mem.pb"

XIncludeFile "stack.pb"
XIncludeFile "prims.pb"
XIncludeFile "compiler.pb"


Procedure processexists(N, PID)
  Define result.i
  result = FindMapElement(N\Processpool(), Str(PID))
  
  ResetMap(N\processpool())
  
  If result
    ProcedureReturn 1
  Else
    ProcedureReturn 0
  EndIf
  
EndProcedure

Procedure addmessage(N, targetpid.i, messageID.i)
  Define P
  P = N\processpool(Str(targetpid))
  
  AddElement(P\MessageQueue())
  P\MessageQueue() = messageID
  useobject(N, messageID)
  
EndProcedure 

Procedure sendmessage(N, originPID.i, targetPID.i, messageID.i)
  Define tupleID.i, *tuple.datatuple
  
  tupleID = newtuple(N, 2)
  *tuple = ptr(tupleID)
  
  struct_setelement(N, tupleID, 0, (originPID << 2) | 1)
  struct_setelement(N, tupleID, 1, messageID)
  
  addmessage(N, targetPID, tupleID)
EndProcedure


Procedure.i messagecount(N, PID.i)
  Define P
  
  P = N\processpool(Str(PID))
  
  ProcedureReturn ListSize(P\MessageQueue())
EndProcedure

Procedure.i recvmessage(N, PID.i)
  Define tupleID.i, *tuple.datatuple, P
  P = N\processpool(Str(PID))
  
  If messagecount(N, PID)
    FirstElement(P\MessageQueue())
  
    tupleID = P\MessageQueue()
  
    unuseobject(N, tupleID)
    DeleteElement(P\MessageQueue())
  
    ProcedureReturn tupleID
  Else
    ProcedureReturn 0
    
  EndIf
EndProcedure


Procedure p_send(P)
  Define targetPID.i, messageID.i
  
  needstack(2)
  messageID = pop(P)
  popint(targetPID)
  
  If processexists(P\Node, targetPID)
    sendmessage(P\node, P\pid, targetPID, messageID)
  Else
    AddLine("Error: invalid process ID")
    ResetP(P, _error)
    ProcedureReturn
  EndIf
  
EndProcedure  
registerprim(send,@p_send())

Procedure p_receive(P)
  Define messageID.i
  
  If messagecount(P\node, P\PID) > 0
    messageID = recvmessage(P\Node, P\PID)
    push(P, messageID)
  Else
    pushint(P, 0)
  EndIf
EndProcedure  
registerprim(receive, @p_receive())

Procedure p_messages(P)
  pushint(P, messagecount(P\node, P\pid))
EndProcedure
registerprim(messages, @p_messages())


Procedure setactive(P)
  If *ThisProcess\executestate = _inactive
    ChangeCurrentElement(P\Node\inactiveprocesses(),*ThisProcess\processlistptr)
    DeleteElement(P\Node\inactiveprocesses())
    *ThisProcess\processlistptr = AddElement(P\Node\activeprocesses())
    P\Node\activeprocesses() = P
  EndIf
EndProcedure

Procedure setinactive(P)
  If *ThisProcess\executestate = _active
    ChangeCurrentElement(P\Node\activeprocesses(),*ThisProcess\processlistptr)
    DeleteElement(P\Node\activeprocesses())
    *ThisProcess\processlistptr = AddElement(P\Node\inactiveprocesses())
    P\Node\inactiveprocesses() = P
  EndIf
EndProcedure

Procedure setdead(P)
  If *ThisProcess\executestate = _active
    ChangeCurrentElement(P\Node\activeprocesses(),*ThisProcess\processlistptr)
    DeleteElement(P\Node\activeprocesses())
    *ThisProcess\executestate = 0
    *ThisProcess\processlistptr = 0
  EndIf
  If *ThisProcess\executestate = _inactive
    ChangeCurrentElement(P\Node\inactiveprocesses(),*ThisProcess\processlistptr)
    DeleteElement(P\Node\inactiveprocesses())
    *ThisProcess\executestate = 0
    *ThisProcess\processlistptr = 0
  EndIf
  If *ThisProcess\processlistptr = 0
    P\executestate = _killed
    *ThisProcess\processlistptr = AddElement(P\Node\deadprocesses())
    P\Node\deadprocesses() = P
  EndIf
  
  
EndProcedure


Procedure ResetP(P, reasonatom.i)
  ;Define *object.dataobject, *test.datastack
  If pmode & 2 And cword\nameatom > 0
    AddLine("Freeing memory for word: "+AtomToString(cword\nameatom))
    DeleteMapElement(P\Node\atomtowordtable(),Str(cword\nameatom))
    DeleteMapElement(P\Node\wordtoatomtable(),Str(cword))
    cword\nameatom = 0
  EndIf
  
  setdead(P)
  
  ctop = 0
  dtop = 0
  pmode = 0
  *ThisProcess\debugmode = 0
  *ThisProcess\currentop = 0
  *ThisProcess\codeword = 0
  *ThisProcess\errorstate = reasonatom
EndProcedure


Procedure killproc(P, reason.i, resulttext.s)
  Define stringid.i, tupleID.i, atomID.i, PID.i, N
  N = P\Node
  stringid = newstring(N, resulttext)
  tupleID = newtuple(N, 3)
  atomID = (reason << 2) | 2
  PID = (P\pid << 2) | 1
  struct_setelement(N, tupleID, 0, atomID)
  struct_setelement(N, tupleID, 1, PID)
  struct_setelement(N, tupleID, 2, stringid)
  
  sendmessage(N, 0, 0, tupleID)
  
  setdead(P)
  
  ctop = 0
  dtop = 0
  pmode = 0
  *ThisProcess\debugmode = 0
  *ThisProcess\currentop = 0
  *ThisProcess\codeword = 0
  *ThisProcess\errorstate = reason
EndProcedure

Procedure runtimefault(P, errortext.s)
  killproc(P, _error, errortext)
EndProcedure





Procedure pushstate(P)
  Define *test.flowcontrolframe, test2
  test2 = *ThisProcess\instructions
  *test = cstack[ctop]
  If ctop < *ThisProcess\c\max
    cstack[ctop]\codeword = cword
    cstack[ctop]\currentop = *ThisProcess\currentop
    cstack[ctop]\context = *ThisProcess\context
    ctop + 1
    *ThisProcess\context = newcontext(P)
  Else
    AddLine("Error: Callstack overflow.")
    ResetP(P, _error)
  EndIf
EndProcedure


Procedure popstate(P)
  If ctop > 0
    ctop - 1
    cword = cstack[ctop]\codeword
    *ThisProcess\currentop = cstack[ctop]\currentop
    *ThisProcess\context = cstack[ctop]\context
    cstack[ctop]\codeword = 0
    cstack[ctop]\currentop = 0
    cstack[ctop]\context = 0
  Else
    killproc(P, _exit, "")
  EndIf
EndProcedure




; Runtime

;
;


Procedure executetimeslice(P, timemax.i)
  Define q.i , thisop.i, r.i, tstart.i, tnow.i, debugstring.s
;  r = 0
  q = 0
  tstart = gettime()
  Repeat
    ;Debug cword\codestream
    ThisOp.i = cword\Codestream[*ThisProcess\currentop]
    If debugstate And thisop
      ;
      ; I cannot for the life of me figure out a better way of doing this than hand-adding special case debug entries.
      debugstring = "PID "+Str(*ThisProcess\pid)+": "+Str(*ThisProcess\currentop)+" : "+stacktrace(P)+ " > "
      Select thisop
        Case @p_pushint()
          debugstring+Str(cword\Codestream[*ThisProcess\currentop+1])
        Case @p_pushfloat()
          debugstring+StrD(PeekD(cword + 24 + (*ThisProcess\currentop +1) * 8))
        Case @p_pushstring()
          debugstring+#DQUOTE$+AtomToString(cword\Codestream[*ThisProcess\currentop+1])+#DQUOTE$
        Case @p_call()
          debugstring+"call: "+AtomToString(wordtoatom(P\node, cword\Codestream[*ThisProcess\currentop+1]))
        Case @cjmp()
          debugstring+"cjmp: "+Str(cword\Codestream[*ThisProcess\currentop+1])
        Case @jmp()
          debugstring+"jmp: "+Str(cword\Codestream[*ThisProcess\currentop+1])
        Case @p_while()
          debugstring+"while: "+Str(cword\Codestream[*ThisProcess\currentop+1])
        Case @p_break()
          debugstring+"break: "+Str(cword\Codestream[*ThisProcess\currentop+1])
        Case @p_continue()
          debugstring+"continue: "+Str(cword\Codestream[*ThisProcess\currentop+1])
        Default
          debugstring+AtomToString(primtoatom(P\Node,thisop))
      EndSelect
      AddLine(debugstring)
    EndIf
    q + 1
    *ThisProcess\currentop + 1
    Select ThisOp
      Case 0   ; Zero means we're done executing the current word but haven't closed out the P.
        If pmode = 0
          cword = 0
        Else
          Break
        EndIf
        
      Default
        CallFunctionFast(ThisOp, P)
    EndSelect
    tnow = gettime()
    If *ThisProcess\errorstate
      Break
    EndIf
;    r + 1
;    If r > 500
;      releaseobjects(P)
;      r = 0
;    EndIf
  ;
  ;  This is our runaway P escape valve. It defaults to 1,000 but can be changed with the setmaxops debug command.
  Until (cword = 0) Or (q >= *ThisProcess\opmax) Or (tnow - tstart) > timemax
;  If tnow - tstart = 0
;    *ThisProcess\runtime + 1
;  Else
    *ThisProcess\runtime + (tnow - tstart)
;  EndIf
;  Debug Str((tnow - tstart)) + " " + Str(q)
  *ThisProcess\instructions + q
EndProcedure

Procedure procreport(P)
  Define resultstring.s
  resultstring =Str(*ThisProcess\runtime - *ThisProcess\prevtime) +"ms of "+Str(*ThisProcess\runtime)+"ms, "+Str(*ThisProcess\instructions)+" ops, Stack: ( "
  resultstring + stacktrace(P)+" )" 
  If P\Node\interpreterprocess
    If P\pid = P\Node\interpreterprocess\pid
    Else
      resultstring = "[PID "+Str(P\pid)+": "+resultstring+"]"
    EndIf
  Else
    resultstring = "[PID "+Str(P\pid)+": "+resultstring+"]"
  EndIf
  If *ThisProcess\errorstate
    resultstring + " " + UCase(AtomToString(*ThisProcess\errorstate))
  Else
    resultstring + " OK"
  EndIf
  AddLine(resultstring)
EndProcedure

Procedure scheduler(N)
  Define *ThisProcess.ProcessState, tstart.i, tnow.i
  
  ; Commented out: instrumentation toys.
  ;tstart = gettime()    
  ForEach N\activeprocesses()
    P = N\activeprocesses()
    If cword And pmode = 0
      executetimeslice(P,20)
;      releaseobjects(N)
      eventcheck(N,"")
    Else
      procreport(P)
      If *ThisProcess\errorstate
        freeprocess(N, P)
      Else
        DeleteElement(N\activeprocesses())
        *ThisProcess\executestate = _inactive
        *ThisProcess\processlistptr = AddElement(N\inactiveprocesses())
        N\inactiveprocesses() = *ThisProcess
      EndIf
      DisableGadget(1,0)
      SetActiveGadget(1)
    EndIf
  Next N\activeprocesses()
  ForEach N\deadprocesses()
    P = N\deadprocesses()
    procreport(P)
    freeprocess(N,P)
  Next N\deadprocesses()
  releaseobjects(N)
  ;tnow = gettime()
  ;If tnow - tstart> 1
  ;  Debug tnow - tstart
  ;EndIf
  ProcedureReturn ListSize(N\activeprocesses())
EndProcedure


Procedure interpret(N, inputline.s)
  If Len(Trim(inputline)) = 0
    ProcedureReturn
  EndIf
  AddLine(" > "+inputline)
  DisableGadget(1,1)
  If N\interpreterprocess = 0
    Define P
    P = newprocess(N, 16, 16)
    N\interpreterprocess = P
  Else
    P = N\interpreterprocess
    If *ThisProcess\executestate = _inactive
      ChangeCurrentElement(N\inactiveprocesses(),*ThisProcess\processlistptr)
      DeleteElement(N\inactiveprocesses())
      *ThisProcess\executestate = _active
      *ThisProcess\processlistptr = AddElement(N\activeprocesses())
      N\activeprocesses() = *ThisProcess
    Else
    EndIf
    
  EndIf
  If cword = 0
    *ThisProcess\instructions = 0
    *ThisProcess\currentop = 0
    Define i.i
    *ThisProcess\errorstate = 0
    cword  = newcodeword(N, 512, 0)
    *ThisProcess\currentop = 0
    cword\InstructionCount = 0
  EndIf
  
  
  parseline(P,inputline)
  ;  appendinstruction(P, @p_exit())
  If (flowcontroltop > 0) And Not (pmode & 2)
    AddLine("Error: Unexpected end of stream.")
    flowcontroltop = 0
    ResetP(P, _error)
  EndIf
  If *ThisProcess\errorstate
  EndIf
  *ThisProcess\prevtime = *ThisProcess\runtime
EndProcedure

; IDE Options = PureBasic 5.71 LTS (Windows - x64)
; CursorPosition = 380
; FirstLine = 365
; Folding = ----
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory