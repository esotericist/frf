; prims.pb -- frf primitives
; contains everything the P must have to operate
;
; including application can add additional primitives if need be.
; the mechanism for this is defined in mem.pb
;

XIncludeFile "stack.pb"
XIncludeFile "mem.pb"
XIncludeFile "structures.pb"

;-----Stack management prims

Procedure displaytop(P)
  needstack(1)
  Debug "Top:"+Str(dstack[dtop-1])
EndProcedure
registerprim(displaytop,@displaytop())

Procedure p_dup(P) ; ( x -- x x )
  Define Input.i
  needstack(1)
  input = pop(P)
  push(P,Input)
  push(P,Input)
EndProcedure
registerprim(dup, @p_dup())

Procedure p_over(P) ; ( x y -- x y x )
  Define firstinput.i, secondinput.i
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  push(P,firstinput)
  push(P,secondinput)
  push(P,firstinput)
EndProcedure
registerprim(over, @p_over())

Procedure p_swap(P) ; ( x y -- y x )
  Define first.i, second.i
  needstack(2)
  second = pop(P)
  first = pop(P)
  push(P,second)
  push(P,first)
EndProcedure
registerprim(Swap, @p_swap())

Procedure p_rot(P)  ; ( x y z -- y z x )
  Define first.i, second.i, third.i
  needstack(3)
  third = pop(P)
  second = pop(P)
  first = pop(P)
  push(P,second)
  push(P,third)
  push(P,first)
EndProcedure
registerprim(rot, @p_rot())

Procedure p_pick(P) ; ( xi ... x1 i -- xi ... x1 xi ) // 1 2 3 4 4 pick -> 1 2 3 4 1
  Define argument.i, object.i
  needstack(1)
  popint(argument)
  If argument <= 0
    runtimefault(P, "Error: pick expects a positive integer.")
    ProcedureReturn
  EndIf
  needstack(argument)
  object = dstack[dtop - argument]
  push(P, object)
EndProcedure
registerprim(pick, @p_pick())


Procedure p_rotate(P)  ; ( xi ... x1 i -- x(i-1) ... x1 xi ) // '"a" "b" "c" "d" 4 rotate' would leave "b" "c" "d" "a" on the stack.  
  Define argument.i, stackrange.i, rangetop.i, rangebottom.i, i.i
  needstack(1)
  popint(argument)
  
  stackrange = Abs(argument)
  needstack(stackrange)
  rangetop.i = dstack[dtop -1]
  If Sign(argument) > 0
    rangebottom = dstack[dtop - stackrange]
    For i = dtop - stackrange To dtop - 2 
      dstack[i] = dstack[i+1]
    Next i
    dstack[dtop -1] = rangebottom
  ElseIf Sign(argument) < 0
    rangetop.i = dstack[dtop - 1]
    
    For i = dtop -1 To dtop - stackrange + 1 Step -1
      dstack[i] = dstack[i-1]
    Next i
    dstack[dtop - stackrange] = rangetop
  EndIf
EndProcedure
registerprim(rotate, @p_rotate())

Procedure p_popn(P) ; ( xi ... x1 i -- )
  Define argument.i
  needstack(1)
  popint(argument)
  
  If argument < 0
    runtimefault(P, "Error: popn expects a non-negative integer.")
    ProcedureReturn
  EndIf
  needstack(argument)
  While argument > 0
    pop(P)
    argument - 1
  Wend
EndProcedure
registerprim(popn, @p_popn())

Procedure p_depth(P) ; (xn ... x0 -- n )
  pushint(P,dtop)
EndProcedure
registerprim(depth, @p_depth())


; special: searches stack for a stack marker ( { ), removes it, and pushes an integer
; denoting how many stack items were between the marker and top of stack.
Procedure stackrange(P) 
  Define stackrange.i, i.i, found
  For i = dtop -1 To 0 Step -1
    found = checktype(dstack[i])
    If found = _type_stackmark
      Break 
    EndIf
  Next i
  If found = _type_stackmark
    unuseobject(P\node, dstack[i])
    found = i
    For i = found To dtop - 1
      dstack[i] = dstack[i+1]
    Next i
    dtop - 1
    stackrange = dtop - found
  EndIf
  pushint(P, stackrange)
 EndProcedure
registerprimunprintable("}", @stackrange())
  
;
;-----Data conversion
;

Procedure p_isint(P) ; ( x -- b )
  needstack(1)
  If checktype(pop(P)) = _type_integer
    pushint(P,1)
  Else
    pushint(P,0)
  EndIf
EndProcedure
registerprimunprintable("int?", @p_isint())

Procedure p_isfloat(P) ; ( x -- b )
  needstack(1)
  If checktype(pop(P)) = _type_float
    pushint(P,1)
  Else
    pushint(P,0)
  EndIf
EndProcedure
registerprimunprintable("float?", @p_isfloat())

Procedure p_isstring(P) ; ( x -- b )
  needstack(1)
  If checktype(pop(P)) = _type_string
    pushint(P,1)
  Else
    pushint(P,0)
  EndIf
EndProcedure
registerprimunprintable("string?", @p_isstring())

Procedure p_isatom(P) ; ( x -- b )
  needstack(1)
  If checktype(pop(P)) = _type_atom
    pushint(P,1)
  Else
    pushint(P,0)
  EndIf
EndProcedure
registerprimunprintable("atom?", @p_isatom())

Procedure p_ismark(P) ; ( x -- b )
  needstack(1)
  If checktype(pop(P)) = _type_stackmark
    pushint(P, 1)
  Else
    pushint(P, 0)
  EndIf
EndProcedure
registerprimunprintable("mark?", @p_ismark())

Procedure p_intostr(P) ; ( i -- s )
  Define i.i, output.s
  needstack(1)
  popint(i)
  output = Str(i)
  pushstring(P, output)
EndProcedure
registerprim(intostr, @p_intostr())

Procedure p_ftostr(P) ; ( f -- s )
  Define float.d, output.s
  needstack(1)
  popfloat(float)
  output = RTrim(StrD(float),"0")
  If Right(output, 1) = "."
    output + "0"
  EndIf
  pushstring(P, output)
EndProcedure
registerprim(ftostr, @p_ftostr())


Procedure p_atoi(P) ; ( s -- i )
  Define output.i, *string.datastring
  needstack(1)
  popstring(*string)
  output = Val(*string\value)
  pushint(P, output)
EndProcedure
registerprim(atoi, @p_atoi())
  
Procedure p_strtof(P) ; ( s -- f )
  Define output.D, *string.datastring
  needstack(1)
  popstring(*string)
  output = ValD(*string\value)
  pushfloat(P, output)
EndProcedure
registerprim(strtof, @p_strtof())

;
;
;-----Math prims:
;



Procedure p_maxint(P) ; ( -- i )
  Define value.i
  value = 0 ;(2 << 60) -1
  pushint(P, value)
EndProcedure
registerprim(maxint, @p_maxint())


Procedure p_random(P) ; ( -- i )
  Define value.i
  value = Random((2<<60)-1)
  pushint(P, value)
EndProcedure
registerprim(random, @p_random())


;Procedure 
  
;EndProcedure



Procedure p_add(P) ; ( n1 n2 -- n3)
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d, outputfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    output.i  = (firstinput >> 2) + (secondinput >> 2)
    pushint(P,output)
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    outputfloat = firstfloat + secondfloat
    pushfloat(P, outputfloat)
  EndIf
EndProcedure
registerprimunprintable("+", @p_add())


Procedure p_sub(P) ; ( n1 n2 -- n3)
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d, outputfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    output.i  = (firstinput >> 2) - (secondinput >> 2)
    pushint(P,output)
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    outputfloat = firstfloat - secondfloat
    pushfloat(P, outputfloat)
  EndIf
EndProcedure
registerprimunprintable("-", @p_sub())

Procedure p_mult(P) ; ( n1 n2 -- n3)
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d, outputfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    output.i  = (firstinput >> 2) * (secondinput >> 2)
    pushint(P,output)
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    outputfloat = firstfloat * secondfloat
    pushfloat(P, outputfloat)
  EndIf
EndProcedure
registerprimunprintable("*", @p_mult())

Procedure p_div(P) ; ( n1 n2 -- n3)
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d, outputfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    output.i  = (firstinput >> 2) / (secondinput >> 2)
    pushint(P,output)
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    outputfloat = firstfloat / secondfloat
    pushfloat(P, outputfloat)
  EndIf
EndProcedure
registerprimunprintable("/", @p_div())

Procedure p_mod(P) ; ( n1 n2 -- n3)
  Define firstinput.i, secondinput.i, output.i
  needstack(2)
  popint(secondinput)
  popint(firstinput)
  output.i = Mod(firstinput, secondinput)
  pushint(P,output)
EndProcedure
registerprimunprintable("%", @p_mod())


;-----logic


Procedure p_gt(P) ; ( x1 x2 -- b )
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    If (firstinput >> 2) > (secondinput >> 2)
      output.i = 1
    Else
      output.i = 0
    EndIf
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    If firstfloat > secondfloat
      output.i = 1
    Else
      output.i = 0
    EndIf
  EndIf
  pushint(P,output)
EndProcedure
registerprimunprintable(">", @p_gt())

Procedure p_lt(P) ; ( x1 x2 -- b )
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    If (firstinput >> 2) < (secondinput >> 2)
      output.i = 1
    Else
      output.i = 0
    EndIf
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    If firstfloat < secondfloat
      output.i = 1
    Else
      output.i = 0
    EndIf
  EndIf
  pushint(P,output)
EndProcedure
registerprimunprintable("<", @p_lt())

Procedure p_gte(P) ; ( x1 x2 -- b )
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    If (firstinput >> 2) >= (secondinput >> 2)
      output.i = 1
    Else
      output.i = 0
    EndIf
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    If firstfloat >= secondfloat
      output.i = 1
    Else
      output.i = 0
    EndIf
  EndIf
  pushint(P,output)
EndProcedure
registerprimunprintable(">=", @p_gte())

Procedure p_lte(P) ; ( x1 x2 -- b )
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    If (firstinput >> 2) <= (secondinput >> 2)
      output.i = 1
    Else
      output.i = 0
    EndIf
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    If firstfloat <= secondfloat
      output.i = 1
    Else
      output.i = 0
    EndIf
  EndIf
  pushint(P,output)
EndProcedure
registerprimunprintable("<=", @p_lte())

Procedure p_eq(P) ; ( x1 x2 -- b )
  Define firstinput.i, secondinput.i, output.i, firstfloat.d, secondfloat.d
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  If integers(firstinput, secondinput)
    If (firstinput >> 2) = (secondinput >> 2)
      output.i = 1
    Else
      output.i = 0
    EndIf
  Else
    getnumeric(firstinput, firstfloat)
    getnumeric(secondinput, secondfloat)
    If firstfloat = secondfloat
      output.i = 1
    Else
      output.i = 0
    EndIf
  EndIf
  pushint(P,output)
EndProcedure
registerprimunprintable("=", @p_eq())


Procedure p_or(P) ; ( x1 x2 -- b )
  Define firstinput.i, secondinput.i, firsttruth.i, secondtruth.i, output.i
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  
  secondtruth = checktrue(P, secondinput)
  firsttruth = checktrue(P, firstinput)
  If firsttruth Or secondtruth
    output.i = 1
  Else
    output.i = 0
  EndIf
  pushint(P,output)
EndProcedure
registerprim(Or, @p_or())

Procedure p_and(P) ; ( x1 x2 -- b )
  Define firstinput.i, secondinput.i, firsttruth.i, secondtruth.i, output.i
  needstack(2)
  secondinput = pop(P)
  firstinput = pop(P)
  
  secondtruth = checktrue(P, secondinput)
  firsttruth = checktrue(P, firstinput)
  If firsttruth And secondtruth
    output.i = 1
  Else
    output.i = 0
  EndIf
  pushint(P,output)
EndProcedure
registerprim(And, @p_and())

Procedure p_not(P) ; ( x -- b )
  Define firstinput.i, firsttruth.i, output.i
  needstack(1)
  firstinput.i = pop(P)
  
  firsttruth = checktrue(P, firstinput)
  If firsttruth
    output.i = 0
  Else
    output.i = 1
  EndIf
  pushint(P,output)
EndProcedure
registerprim(Not, @p_not())

;
;
;-----string prims
;
;

Procedure p_instr(P) ; ( s1 s2 -- i )
  Define *firststring.datastring, *secondstring.datastring, outputposition.i
  popstring(*secondstring)
  popstring(*firststring)
  outputposition = FindString(*firststring\value, *secondstring\value)
  pushint(P, outputposition)
EndProcedure
registerprim(instr, @p_instr())


Procedure p_instring(P) ; ( s1 s2 -- i )
  Define *firststring.datastring, *secondstring.datastring, outputposition.i
  popstring(*secondstring)
  popstring(*firststring)
  outputposition = FindString(LCase(*firststring\value), LCase(*secondstring\value))
  pushint(P, outputposition)
EndProcedure
registerprim(instring, @p_instring())



Procedure p_strcut(P) ; ( s1 i -- s2 s3 )
  Define *inputstring.datastring, inputindex.i, thislen.i, firstoutput.s, secondoutput.s
  needstack(2)
  popint(inputindex)
  popstring(*inputstring)
  thislen = Len(*inputstring\value)
  If inputindex < 0
    inputindex = 0
  EndIf
  If inputindex > thislen
    inputindex = thislen
  EndIf
  firstoutput = Left(*inputstring\value, inputindex)
  secondoutput = Right(*inputstring\value, thislen-inputindex)
  pushstring(P, firstoutput)
  pushstring(P, secondoutput)
EndProcedure
registerprim(strcut, @p_strcut())

Procedure p_strlen(P) ; ( s -- i )
  Define *inputstring.datastring, outputlength.i
  needstack(1)
  popstring(*inputstring)
  outputlength = Len(*inputstring\value)
  pushint(P, outputlength)
EndProcedure
registerprim(strlen, @p_strlen())


Procedure p_strcat(P) ; ( s1 s2 -- s3 )
  Define *firststring.datastring, *secondstring.datastring, outputstring.s
  needstack(2)
  popstring(*secondstring)
  popstring(*firststring)
  outputstring = *firststring\value + *secondstring\value
  pushstring(P, outputstring)
EndProcedure
registerprim(strcat, @p_strcat())


;
;
;-----File Prims
;
;

Procedure p_fwrite(P) ; ( filename:s, writestring:s, offset:i -- success:i)
  Define *firststring.datastring, *secondstring.datastring, offset.i, success.i, filenum.i
  needstack(3)
  popint(offset)
  popstring(*secondstring)
  popstring(*firststring)
  filenum = OpenFile(#PB_Any, *firststring\value)
  If filenum
    FileSeek(filenum,offset)
    WriteString(filenum,*secondstring\value)
    CloseFile(filenum)
    success=1
  EndIf
  pushint(P, success)
EndProcedure
registerprim(fwrite, @p_fwrite())


Procedure p_fappend(P)  ; ( filename:s, appendstring:s -- success:i )
  Define *firststring.datastring, *secondstring.datastring, thissize.i, success.i, filenum.i
  needstack(2)
  popstring(*secondstring)
  popstring(*firststring)
  thissize= FileSize(*firststring\value)
  Select thissize
    Case -2, -1
      success = 0
    Default
      filenum = OpenFile(#PB_Any, *firststring\value)
      If filenum
        FileSeek(filenum,Lof(filenum))
        WriteString(filenum,*secondstring\value)
        CloseFile(filenum)
        success = 1
      Else
        success = 0
      EndIf
  EndSelect
  pushint(P, success)
EndProcedure
registerprim(fappend, @p_fappend())


Procedure p_freadto(P)  ; ( filename:s, initialoffset:i, delimiter:s -- finaloffset:i outputstring:s )
  Define *inputstring.datastring, filenum.i, initialoffset.i, *inputdelim.datastring, delimiter.s, finaloffset.i, outputstring.s, done.i, thischar.c, thisstring.s
  Define filelength.i, charcount.i, statusstring.s
  needstack(3)
  popstring(*inputdelim)
  popint(initialoffset)
  popstring(*inputstring)
  delimiter = *inputdelim\value
  If delimiter = "$EOF$"
    delimiter = ""
  EndIf
  delimiter = Left(delimiter, 1)
  filenum = ReadFile(#PB_Any, *inputstring\value)
  If filenum
    filelength = Lof(filenum)
    If initialoffset >= filelength
      outputstring = ""
      finaloffset = -1
    Else
      FileSeek(filenum, initialoffset)
      done = 0
      outputstring = ""
      If delimiter = #CR$
        outputstring = ReadString(filenum)
      Else
        If delimiter = ""
          Repeat  
            thisstring = ReadString(filenum)
            outputstring = outputstring + thisstring + #CR$
            statusstring.s = "Reading: " + *inputstring\value + ": " + Str(Len(outputstring)) + " of " + Str(filelength) + " (" + Str(100 * Len(outputstring) / filelength) +"%)"
            eventcheck(P\Node, statusstring)
          Until Eof(filenum)
        Else
          Repeat
            thischar = ReadCharacter(filenum)
            thisstring = Chr(thischar)
            outputstring = outputstring + thisstring
            statusstring.s = "Reading: " + *inputstring\value + ": " + Str(Len(outputstring)) + " of " + Str(filelength) + " (" + Str(100 * Len(outputstring) / filelength) +"%)"
            eventcheck(P\Node, statusstring)
          Until (thisstring = delimiter) Or (Eof(filenum))
        EndIf
      EndIf
      If Eof(filenum)
        finaloffset = -1
      Else
        finaloffset = Loc(filenum)
      EndIf
    EndIf
    pushstring(P,outputstring)
    pushint(P, finaloffset)
    CloseFile(filenum)
  Else
    runtimefault(P, "Error: Could not open file '"+*inputstring\value+"'.")
  EndIf
EndProcedure
registerprim(freadto, @p_freadto())


;
;
;-----Flow Control Prims
;
;


Procedure cjmp(P) ; This is only used internally by the compiler.
  Define targetop.i, input.i, truth.i
  needstack(1)
  targetop.i = cword\codestream[*ThisProcess\currentop]
  input.i = pop(P)
  truth = checktrue(P, input)
  If truth
    *ThisProcess\currentop + 1
  Else
    *ThisProcess\currentop = targetop
  EndIf
EndProcedure
registerprim(cjmp, @cjmp())

Procedure jmp(P) ; This is only used internally by the compiler.
  Define targetop.i
  targetop.i = cword\codestream[*ThisProcess\currentop]
  *ThisProcess\currentop = targetop
EndProcedure
registerprim(jmp, @jmp())


Procedure p_continue(P) ; ( -- )
  Define targetop.i
  targetop.i = cword\codestream[*ThisProcess\currentop]
  *ThisProcess\currentop = targetop
EndProcedure
registerprim(Continue, @p_continue())

Procedure p_break(P) ; ( -- )
  Define targetop.i
  targetop.i = cword\codestream[*ThisProcess\currentop]
  *ThisProcess\currentop = targetop
EndProcedure
registerprim(Break, @p_break())


Procedure p_while(P) ; ( x -- )
  Define targetop.i, input.i, truth.i
  needstack(1)
  targetop.i = cword\codestream[*ThisProcess\currentop]
  input = pop(P)
  truth = checktrue(P, Input)
  If truth
    *ThisProcess\currentop + 1
  Else
    *ThisProcess\currentop = targetop
  EndIf
EndProcedure
registerprim(While, @p_while())
  
Procedure p_call(P) ; ( -- )
  Define targetword.i
  targetword.i = cword\codestream[*ThisProcess\currentop]
  *ThisProcess\currentop + 1
  pushstate(P)
  If Not *ThisProcess\errorstate
    cword=targetword
    *ThisProcess\currentop = 0
  EndIf
EndProcedure
registerprim(call, @p_call())  
  
Procedure p_exit(P) ; ( -- )
  popstate(P)
EndProcedure
registerprim(exit, @p_exit())
  
;
;
;-----Multitasking Prims
;
;

Procedure p_now(P) ; ( -- i )
  pushint(P, gettime())
EndProcedure
registerprim(now, @p_now())


Procedure p_pid(P) ; ( -- i )
  pushint(P, *ThisProcess\pid)
EndProcedure
registerprim(pid,@p_pid())

Procedure p_ispid(P) ; ( i -- b )
  Define input.i
  needstack(1)
  popint(input)
  If FindMapElement(P\Node\processpool(),Str(Input))
    pushint(P, 1)
  Else
    pushint(P, 0)
  EndIf
EndProcedure
registerprimunprintable("ispid?",@p_ispid())

Procedure p_kill(P) ; ( i -- b )
  Define *thatP.ProcessState, pid.i
  needstack(1)
  popint(pid)
  If pid > 0
    If FindMapElement(P\Node\processpool(),Str(pid))
      pushint(P, 1)
      *thatP = @P\Node\processpool()
      killproc(*thatP, _killed, "")
    Else
      pushint(P, 0)
    EndIf
  Else
    runtimefault(P, "kill expects integer greater than zero")
  EndIf
EndProcedure
registerprim(kill, @p_kill())

; This structure is only used to help with copying flow control frames from an old process to a new process during a fork.
Structure workstackframe
  work.flowcontrolframe[0]
EndStructure

Procedure copyframe(*input.workstackframe,*output.workstackframe)
  *output\work = *input\work
EndProcedure

Procedure p_fork(*ThisProcess.ProcessState) ; ( -- i )
  Define *NewProcess.ProcessState, i.i, *temp.codeset
  *NewProcess = newprocess(P\Node,*ThisProcess\d\max,*ThisProcess\c\max)
  
  *NewProcess\codeword = *ThisProcess\codeword
  *NewProcess\currentop = *ThisProcess\currentop
  *NewProcess\debugmode = *ThisProcess\debugmode
  ;*NewP\instructions = *ThisProcess\instructions
  *NewProcess\opmax = *ThisProcess\opmax
  For i = 0 To dtop - 1
    push(*NewProcess, dstack[i])
  Next i
  *NewProcess\c\top = ctop
  For i = 0 To ctop - 1
    copyframe(cstack[i],*NewProcess\c\stack[i])
  Next i
  If ctop > 0
    *temp = cstack[0]\codeword
    EndIf
  pushint(*NewProcess, 0)
  pushint(*ThisProcess, *NewProcess\pid)
EndProcedure
registerprim(fork, @p_fork())

;
;
;----- IPC prims
;
;

; these are found in vm.pb


;
;
;-----Tuple Prims
;
;

; these are found in structures.pb


;
;
;-----Debug Prims
;
;

Procedure p_debugon(P) ; ( -- )
  debugstate = 1
EndProcedure
registerprim(debugon, @p_debugon())

Procedure p_debugoff(P) ; ( -- )
  debugstate = 0
EndProcedure
registerprim(debugoff, @p_debugoff())

Procedure p_setmaxops(P) ; ( i -- )
  Define input.i
  needstack(1)
  popint(input)
  If input >= 10
    *ThisProcess\opmax = input
  EndIf
EndProcedure
registerprim(setmaxops, @p_setmaxops())

Procedure p_stacktrace(P) ; ( -- )
  AddLine(0, "Stack: ( "+stacktrace(P)+" )")
EndProcedure
registerprim(stacktrace, @p_stacktrace())
; IDE Options = PureBasic 5.71 LTS (Linux - x64)
; CursorPosition = 881
; FirstLine = 872
; Folding = ----------
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory