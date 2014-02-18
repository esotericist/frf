; atoms.pb   --  Provides a mechanism for registering atoms and converting atoms back to text.
;       void (Amy Lear), 2012
;
;       Purpose: Allows a unified means of working with named values without constantly passing around strings.
;       With this system you can use atoms in both code and in data files, without worrying about pre-defined enumerations.
;
;       Inspired by erlang atoms, but obviously significantly less awesome without compiler support (which is in progress).



; AtomMax starts at 4K. Each time it hits this limit it re-sizes the table (doubling each time).
; If we excessively exceed this count string to atom lookups could eventually slow significantly.
; This bears examining.
Global AtomMax.i = 4096
Global NewMap AtomTable.i(AtomMax * 4)    ; 16k entries, just in case.
Global Dim AtomStrings.s(AtomMax)
Global AtomCount.i = 0


; null string is always the 0th atom and doesn't count against the total # of atoms in use.
AtomStrings(0) = ""
AtomTable("") = 0


; Ensures valid string input only.
; TODO: Possibly this should error out on invalid input?
; Currently skipping a lot of sanitization because the VM asks weird things.
; Will clean up later
Procedure.s SanitizeAtomString(InputString.s) 
  Define OutputString.s, ThisChar.i
  InputString = LCase(Trim(InputString))
;  For i = 1 To Len(InputString)
;    ThisChar = Asc(Mid(InputString,i,1))
;    Select ThisChar
;      Case 46, 47, 48 To 57, 95, 141 To 172 ; Allowed characters: period, slash, numbers, underscore, lowercase letters
;        OutputString = OutputString + Chr(ThisChar)
;    EndSelect
;  Next i
  ProcedureReturn InputString
EndProcedure

; For when you simply need to know what atom a string matches to ONLY if it already exists as an atom.
; Remember to convert to lowercase if you're dealing with case insensitive strings.
Procedure.i VerifyAtom(InputString.s)
  Define *Atomptr, AtomNum
  If InputString = ""
    ProcedureReturn 0
  EndIf
  
  *Atomptr = FindMapElement(AtomTable(), InputString)
  If *Atomptr
    AtomNum = PeekI(*Atomptr)
  EndIf
  ProcedureReturn AtomNum
EndProcedure

; For if you absolutely must have an atom result from your exact string. Use with care.
; Strings for case insensitive contexts need to be converted to lowercase first. newatom() below will do this.
Procedure.i StringtoAtom(InputString.s)
  Define AtomNum.i
  If InputString = ""
    ProcedureReturn 0
  EndIf 
  AtomNum.i = VerifyAtom(InputString)
  If Not AtomNum
    If AtomCount = AtomMax 
      AtomMax * 2
      ReDim AtomStrings.s(AtomMax)
    EndIf
    AtomCount + 1
    AtomNum = AtomCount
    AtomTable(InputString) = AtomNum
    AtomStrings(AtomNum) = InputString
  EndIf
  ProcedureReturn AtomNum
EndProcedure

Procedure.i newatom(InputString.s)
  ProcedureReturn StringtoAtom(SanitizeAtomString(InputString))
EndProcedure


Procedure.s AtomToString(InputAtom.i)
  Define OutputString.s
  OutputString.s = AtomStrings(InputAtom)
  ProcedureReturn OutputString
EndProcedure

;
;     The following macros allow us to declare atoms and use them more easily within PB code.
;     Use atom(thisatom) to declare an atom, which in turn declares and initializes a global variable named _thisatom
;     Any code that's going to use the _thisatom syntax needs to declare with atom() before the first appearance
;     of _thisatom. Never assign to the resultant variable: pretend it's a constant.
;
;     Never use quotes to delineate an atom, either for declaration or actual usage.
;
;     Leading and trailing spaces are stripped, and the atom is stored in lowercase. Do not use atoms for case sensitive contexts.
;
;     While legal characters are alphanumeric, underscores, periods, and forward slash, only use alphanumeric and underscores for
;     atoms declared in code. Periods and slashes will result in a compilation error, and thus should only be used in data-based
;     or run-time generated atoms.
;


Macro AtomQuote     ; Necessary for putting quotes around the atom name in the next macro
  "
EndMacro

Macro atom(atomtext)
  Global _#atomtext.i = newatom(AtomQuote#atomtext#AtomQuote) ; effectively stringtoatom("atomtext")
EndMacro

; IDE Options = PureBasic 5.20 beta 16 LTS (Windows - x86)
; CursorPosition = 27
; FirstLine = 20
; Folding = --
; EnableXP
; Executable = ..\pref.exe
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory