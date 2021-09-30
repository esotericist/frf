; frf.pb -- the frf shell.
; Attempts are being made to design this modularly, so that the frf P and its associated components can be used elsewhere without modification.


EnableExplicit

;- Declares, includes, and initialization
Declare AddLine(EditorGadgetID.i, inputstr.s)
Declare eventcheck(N,statusnote.s)
Declare WindowResize()
Declare populatenodestateview(N)
Declare createnodestateview(N)
XIncludeFile "mem.pb"
Define *thisNode.NodeState
N = initnode()
XIncludeFile "vm.pb"
XIncludeFile "propgrid.pb"

Structure ProcessListData
  header.i
  datatop.i
  calltop.i
  currop.i
  messagequeue.i
EndStructure

Global proplist.i, showprocesses
Global NewMap pmap.ProcessListData(100)
Global exit.i, newtext


; LocalPrims currently only has 'p_dot', but others could go there as well.
; I hope to move the whole 'interactive shell' stuff there eventually, since it doesn't need to be part of the compiler core,
; and a lot of it could theoretically be done in frf.
; That's when I know I'll have won.
Gosub LocalPrims

exit.i = 0

Gosub SetupWindow

AddLine(0, "frf shell by eso.")
addline(0, " ")
AddLine(0, " ready.")


Define title.s, activity.i
;- MainLoop:

; createnodestateview(N)
MainLoop:
Repeat
  
  activity = scheduler(N)
  
  
  eventcheck(N, "")
  
  If activity = 0
    Delay(10)
  EndIf
    If IsWindow(1)
      Define counter.i
      counter +1
      If counter > 10
        populatenodestateview(N)
        counter = 0
      EndIf
    EndIf
Until Exit
End



;- Events:

Procedure replloadfile(N, filename.s) 
  Define filenum.i
  filename = Trim(LCase(filename))
  filenum.i = ReadFile(#PB_Any, filename)
  If filenum
    While Eof(filenum) = 0
      interpret(N, ReadString(filenum))
    Wend
    CloseFile(filenum)
  Else
    addline(0, "Error: '"+filename+"' could not be accessed.")
  EndIf
EndProcedure



Procedure eventcheck(N, statusnote.s)
  Define event.i, inputline.s, title.s, z.i, messages.i, messageID.i, i.i
  Static oldtotal.i, oldactive.i, oldinactive.i
  Static newtotal.i, newactive.i, newinactive.i
  
  newtotal = MapSize(N\processpool())
  newactive = ListSize(N\activeprocesses())
  newinactive = ListSize(N\inactiveprocesses())
  If (newtotal <> oldtotal) Or (newactive <> oldactive) Or (newinactive <> oldinactive)
    title = "frf shell - "+Str(newtotal)+" processes, "+Str(newactive)+" active, "+Str(newinactive)+" inactive."
    If statusnote 
      title = title + " -- " + statusnote
    EndIf
    SetWindowTitle(0, title)
    oldtotal = newtotal
    oldactive = newactive
    oldinactive = newinactive
  EndIf
  
  messages = messagecount(N, 0)
  If messages > 0
    For i = 1 To messages
      messageID= recvmessage(N, 0)
      AddLine(0, formatobject(N, messageID))
    Next i
  EndIf
  
  Event = WindowEvent()
  
  Define EventType, EventGadget, GridGadget
  If Event
    Select Event
      Case #PB_Event_SizeWindow     
        WindowResize()
      Case #PB_Event_CloseWindow ; In case someone clicks the little X.
        Select GetActiveWindow()
          Case 0
            exit = 1
          Case 1
            If proplist <> 0
              freepropertygridgadget(proplist)
            EndIf
            ClearMap(pmap())
            proplist = 0
;            showprocesses = 0
            CloseWindow(1)
        EndSelect
      Case #PB_Event_Menu
        Select EventMenu()
          Case 1
            Define RequestedFile.s
            RequestedFile = OpenFileRequester("Load frf source:",GetCurrentDirectory(),"frf source|*.frf",0)
            AddLine(0, "Loading from source file: " + RequestedFile )
            replloadfile(N, RequestedFile)
          Case 2
            exit = 1
          Case 3
            createnodestateview(N)
          Case 100
            Select GetActiveGadget()
              Case 0
                SetActiveGadget(1)
              Case 1
                inputline.s = GetGadgetText(1)
                interpret(N, inputline)
                SetGadgetText(1,"")
              Default
                Debug "c"
            EndSelect
          Case 101
            exit = 1
          Case 111
            If proplist <> 0
              freepropertygridgadget(proplist)
            EndIf
            ClearMap(pmap())
;            showprocesses = 0
            proplist = 0
            CloseWindow(1)
        EndSelect
        
      Case #PB_Event_Gadget
        EventGadget = EventGadget()
        EventType = EventType()
        Select EventGadget
          Case 0                  ; Sadly, this will never happen. :(
            Select EventType
              Case 256
                
              Case #PB_EventType_Focus;, 256
;                Debug "Gadget 0, focus"
                SetActiveGadget(1)
              Case #PB_EventType_LostFocus
;                Debug "Gadget 0, lost focus"
              Default
                ;                Debug "Gadget 0, other" + Str(EventType)
                If newtext = 1
                  SetActiveGadget(0)
                  SetActiveGadget(1)
                  newtext = 0
                EndIf
            EndSelect
          Case 1
            Select EventType
              Case #PB_EventType_LostFocus
;                Debug "Gadget 1, lost focus"
              Case #PB_EventType_Change
;                Debug "Gadget 1, change"
              Case #PB_EventType_Focus, 256
;                Debug "Gadget 1, focus"
              Default
;                Debug "Gadget 1, other"
            EndSelect
          Case 12
            showprocesses = GetGadgetState(12)
          Default
            
            CheckPropertyGridSectionClick ( proplist, EventGadget, EventType )
        EndSelect
    EndSelect
  Else
  EndIf
EndProcedure



;- SetupWindow:
SetupWindow:
  Global Width.i, Height.i
  If Not OpenWindow(0, 0, 0, 640, 480, "EditorGadget", #PB_Window_SystemMenu | #PB_Window_MaximizeGadget | #PB_Window_SizeGadget | #PB_Window_ScreenCentered)
    Debug "Window open failed."
    End
  EndIf
  SetWindowTitle(0,"frf shell - 0 processes, 0 active, 0 inactive.")
  Width = WindowWidth(0, #PB_Window_InnerCoordinate)
  Height = WindowHeight(0, #PB_Window_InnerCoordinate)
  EditorGadget(0, 4, 4, width - 8, height - 30,#PB_Editor_ReadOnly)
  
  StringGadget(1, 4, height - 30, width - 8, 24, "")
  
  CreateMenu(0,WindowID(0))
  MenuTitle("VM")
  MenuItem(1,"Load Source")
  MenuBar()
  MenuItem(2,"Exit")
  MenuTitle("Debug")
  MenuItem(3,"NodeState")
  
  SetActiveGadget(1)
  AddKeyboardShortcut(0, #PB_Shortcut_Return, 100)
  AddKeyboardShortcut(0, #PB_Shortcut_Escape, 101)
  WindowResize()
Return

;- ResizeWindow:
  
Procedure WindowResize()
  Width = WindowWidth(0,  #PB_Window_InnerCoordinate)
  Height = WindowHeight(0,  #PB_Window_InnerCoordinate)
  ResizeGadget(0, 4, 4, width - 8, height - 60)
  ResizeGadget(1, 4, height - 54, width - 8, 24)
  SetActiveGadget(1)
EndProcedure


Procedure AddLine(EditorGadgetID.i, outputstr.s)
  If Len(outputstr) > 0
    AddGadgetItem(0, -1, outputstr)
    
    ; platform specific code for moving the editor to the bottom line:
    ; from: https://www.purebasic.fr/english/viewtopic.php?p=538446#p538446
    CompilerSelect #PB_Compiler_OS
      CompilerCase #PB_OS_Linux

      Protected EndMark.I
      Protected TextBuffer.I
      Protected EndIter.GtkTextIter

      TextBuffer = gtk_text_view_get_buffer_(GadgetID(EditorGadgetID))
      gtk_text_buffer_get_end_iter_(TextBuffer, @EndIter)
      EndMark = gtk_text_buffer_create_mark_(TextBuffer, "end_mark", @EndIter, #False)
      gtk_text_view_scroll_mark_onscreen_(GadgetID(EditorGadgetID), EndMark)
    CompilerCase #PB_OS_MacOS
        Protected Range.NSRange
  
        Range.NSRange\location = Len(GetGadgetText(EditorGadgetID))
        CocoaMessage(0, GadgetID(EditorGadgetID), "scrollRangeToVisible:@", @Range)
      CompilerCase #PB_OS_Windows
        SendMessage_(GadgetID(EditorGadgetID), #EM_SETSEL, -1, -1)
    CompilerEndSelect
  
  EndIf
  newtext = 1
EndProcedure


Procedure populatenodestateview(N)
  Static refreshtimer.i
  Define section.i, *thisProcess.ProcessState, pid$, pstruct.ProcessListData
  ClearGadgetItems(11)
  AddGadgetItem(11,-1,"Process Pool"+Chr(10)+Str(MapSize(N\processpool())))
  AddGadgetItem(11,-1,"Next Process ID"+Chr(10)+Str(N\nextprocessID))
  AddGadgetItem(11,-1,"Active Processes"+Chr(10)+Str(ListSize(N\activeprocesses())))
  AddGadgetItem(11,-1,"Inactive Processes"+Chr(10)+Str(ListSize(N\inactiveprocesses())))
  AddGadgetItem(11,-1,"Dead Processes"+Chr(10)+Str(ListSize(N\deadprocesses())))
  AddGadgetItem(11,-1,"Object Pool"+Chr(10)+Str(ListSize(N\objectpool())))
  
  
  If (refreshtimer > 2) And showprocesses
    If proplist = 0
      proplist = PropertyGridGadget(#PB_Any,4,220,394,380,RGB($40,$40,$40),RGB($FF,$FF,$FF))
      
      
    EndIf
    ForEach N\processpool()
      P = N\processpool()
      pid$ = Str(P\pid)
      pstruct = pmap(pid$)
      If pstruct\header
        SetGadgetText(pstruct\datatop, Str(*thisProcess\d\top))
        SetGadgetText(pstruct\calltop, Str(*thisProcess\c\top))
        SetGadgetText(pstruct\currop, Str(*thisProcess\currentop))
        SetGadgetText(pstruct\messagequeue, Str(ListSize(*thisProcess\MessageQueue())))
      Else
        pstruct\header = AddSection(proplist,"Process ID: "+Str(*thisProcess\pid))
        section = pstruct\header
        pstruct\datatop = AddStringGadget(proplist,section,"Data Stack Top",Str(*thisProcess\d\top),#PB_String_ReadOnly)
        pstruct\calltop = AddStringGadget(proplist,section,"Call Stack Top",Str(*thisProcess\c\top),#PB_String_ReadOnly)
        pstruct\currop = AddStringGadget(proplist,section,"Current Op",Str(*thisProcess\currentop),#PB_String_ReadOnly)
        pstruct\messagequeue = AddStringGadget(proplist,section,"Messages",Str(ListSize(*thisProcess\MessageQueue())),#PB_String_ReadOnly)
        pmap(pid$) = pstruct
      EndIf
    Next N\processpool()
    ForEach pmap()
      pstruct = pmap()
      pid$ = MapKey(pmap())
      If Not (FindMapElement(N\processpool(),pid$))
        RemovePropertyGridSection(proplist,pstruct\header)
        DeleteMapElement(pmap())
      EndIf
    Next pmap()
    
    refreshtimer = 0
  Else
    If (showprocesses = 0) And (proplist <> 0)
      freepropertygridgadget(proplist)
        ClearMap(pmap())
      proplist = 0
    EndIf
    
  EndIf
  
  refreshtimer + 1
EndProcedure


Procedure createnodestateview(N)
  Define i.i
  If IsWindow(1)
    populatenodestateview(N)
    SetActiveWindow(1)
  Else
    proplist = 0
    OpenWindow(1,50,50,400,600,"NodeState",#PB_Window_SystemMenu,WindowID(0))
    ListIconGadget(11,4,4,192,196,"Property",120,#PB_ListIcon_GridLines)
    AddGadgetColumn(11,1,"Value",48)
    ButtonGadget(12,4,200,150,20,"Enable Process View",#PB_Button_Toggle)
    SetGadgetState(12,showprocesses)
    populatenodestateview(N)
    AddKeyboardShortcut(1, #PB_Shortcut_Escape, 111)
  EndIf
EndProcedure



;- LocalPrims:
LocalPrims:
  
  ; We define and register the dot prim here because its behavior is specific to this shell.
Procedure p_dot(P)
  Define object.i, outstring.s, type.i
  needstack(1)
  object = pop(P)
  outstring.s = formatobject(P, object)
  If (checktype(object) = _type_string) Or (checktype(object) = _type_atom)
    outstring = Left(Right(outstring, Len(outstring)-1),Len(outstring)-2)
  EndIf
  If P\Node\interpreterprocess
    If *ThisProcess\pid <> P\Node\interpreterprocess.ProcessState\pid
      outstring = "[From process "+Str(*ThisProcess\pid)+": "+outstring+" ]"
    EndIf
  Else
    outstring = "[From process "+Str(*ThisProcess\pid)+": "+outstring+" ]"
  EndIf
  
  addline(0, outstring)
EndProcedure
registerprimunprintable(".", @p_dot())

Procedure p_clear(P)
  ClearGadgetItems(0)
  
EndProcedure
registerprim(clear,@p_clear())

Return
; IDE Options = PureBasic 5.71 LTS (Windows - x64)
; CursorPosition = 7
; Folding = --
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory