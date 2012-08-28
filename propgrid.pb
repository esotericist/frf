;
; Simple PropertyGrid
;
; by Danilo, May 2012, PureBasic 4.60
;
#PropertyGrid_SectionFont     = "Arial"

CompilerSelect #PB_Compiler_OS
  CompilerCase                              #PB_OS_Windows
    #PropertyGrid_DefaultItemHeight = 20
    #PropertyGrid_SectionFontSize   = 10
  CompilerCase                              #PB_OS_Linux
    #PropertyGrid_DefaultItemHeight = 22
    #PropertyGrid_SectionFontSize   = 10
  CompilerCase                              #PB_OS_MacOS
    #PropertyGrid_DefaultItemHeight = 25
    #PropertyGrid_SectionFontSize   = 14
  CompilerDefault                           ; future
    #PropertyGrid_DefaultItemHeight = 20
    #PropertyGrid_SectionFontSize   = 12
CompilerEndSelect

Structure PropertyGridGadgetSubItem
  DescriptionGadget.i
  ItemGadget.i
EndStructure

Structure PropertyGridItem
  SectionDescriptionGadget.i
  SectionButtonGadget.i
  IsSection.i
  Opened.i
  List SubItems.PropertyGridGadgetSubItem()
EndStructure

Structure PropertyGridData
  ItemHeight.i
  Font.i
  backColor.q
  frontColor.q
  List Items.PropertyGridItem()
EndStructure

Procedure __GetPropertyGridImage(open.i)
  Static openImage.i, closeImage.i
  
  If Not openImage
    openImage = CreateImage(#PB_Any,11,11,24)
    If openImage And StartDrawing( ImageOutput(openImage) )
      Box(0,0,11,11,RGB($FF,$FF,$FF))
      DrawingMode(#PB_2DDrawing_Outlined)
      Box(0,0,11,11,RGB($00,$00,$00))
      LineXY(2,5,8,5,RGB($00,$00,$00))
      StopDrawing()
    EndIf
  EndIf
  If Not closeImage
    closeImage = CreateImage(#PB_Any,11,11,24)
    If closeImage And StartDrawing( ImageOutput(closeImage) )
      Box(0,0,11,11,RGB($FF,$FF,$FF))
      DrawingMode(#PB_2DDrawing_Outlined)
      Box(0,0,11,11,RGB($00,$00,$00))
      LineXY(2,5,8,5,RGB($00,$00,$00))
      LineXY(5,2,5,8,RGB($00,$00,$00))
      StopDrawing()
    EndIf
  EndIf
  
  
  If open
    ProcedureReturn openImage
  Else
    ProcedureReturn closeImage
  EndIf
EndProcedure

Procedure UpdatePropertyGrid(PropertyGridGadget)
  Define *memory.PropertyGridData, i.i, height.i, width.i
  
  *memory.PropertyGridData = GetGadgetData(PropertyGridGadget)
  If *memory
    i = 0
    height = *memory\ItemHeight
    width  = GadgetWidth(PropertyGridGadget) - 4
    ForEach *memory\Items()
      If *memory\Items()\IsSection
        ResizeGadget( *memory\Items()\SectionDescriptionGadget,20,i,width-20,height-1)
        ResizeGadget( *memory\Items()\SectionButtonGadget     , 4,i+height*0.5-6,11,11)
      Else
        ResizeGadget( *memory\Items()\SectionDescriptionGadget,20,i,width*0.5-40,height-1)
        ResizeGadget( *memory\Items()\SectionButtonGadget     ,width*0.5-19,i,width*0.5-4,height-1)
      EndIf
      i + height
      If *memory\Items()\Opened
        ForEach *memory\Items()\SubItems()
          ResizeGadget( *memory\Items()\SubItems()\DescriptionGadget,20,i,width*0.5-40,height-1)
          ResizeGadget( *memory\Items()\SubItems()\ItemGadget       ,width*0.5-19,i,width*0.5-4,height-1)
          i + height
        Next
      EndIf
    Next
    If GetGadgetAttribute(PropertyGridGadget,#PB_ScrollArea_InnerHeight) <> i
      SetGadgetAttribute(PropertyGridGadget,#PB_ScrollArea_InnerHeight,i)
    EndIf
    If GetGadgetAttribute(PropertyGridGadget,#PB_ScrollArea_InnerWidth) <> width-20
      SetGadgetAttribute(PropertyGridGadget,#PB_ScrollArea_InnerWidth,width-20)
    EndIf
    
  EndIf
EndProcedure


Procedure __SetSection(PropertyGridGadget,section)
  Define *memory.PropertyGridData
  
  *memory.PropertyGridData = GetGadgetData(PropertyGridGadget)
  If *memory
    If section = 0
      LastElement( *memory\Items() )
      OpenGadgetList(PropertyGridGadget)
      ProcedureReturn #True
    EndIf
    ForEach *memory\Items()
      If *memory\Items()\SectionButtonGadget = section
        OpenGadgetList(PropertyGridGadget)
        ProcedureReturn #True
      EndIf
    Next
  EndIf
EndProcedure

Procedure __AddSectionItem(PropertyGridGadget,section,Description.s,Gadget.i)
  Define *memory.PropertyGridData, descGadget.i
  
  *memory.PropertyGridData = GetGadgetData(PropertyGridGadget)
  If *memory
    
    descGadget = StringGadget(#PB_Any,0,0,0,0,Description,#PB_String_ReadOnly)
    ;descGadget = TextGadget(#PB_Any,0,0,0,0,Description);,#PB_Text_Border)
    ;DisableGadget( descGadget, 1 )
    
    If section = 0
      If AddElement( *memory\Items() )
        *memory\Items()\IsSection = 0
        *memory\Items()\SectionButtonGadget      = Gadget
        *memory\Items()\SectionDescriptionGadget = descGadget
        CloseGadgetList()
        UpdatePropertyGrid(PropertyGridGadget)
        ProcedureReturn #True
      EndIf
    Else
      LastElement( *memory\Items()\SubItems() )
      If AddElement( *memory\Items()\SubItems() )
        *memory\Items()\SubItems()\DescriptionGadget = descGadget
        *memory\Items()\SubItems()\ItemGadget        = Gadget
        CloseGadgetList()
        If *memory\Items()\Opened
          UpdatePropertyGrid(PropertyGridGadget)
        Else
          HideGadget(*memory\Items()\SubItems()\DescriptionGadget,1)
          HideGadget(*memory\Items()\SubItems()\ItemGadget       ,1)
        EndIf
        ProcedureReturn #True
      EndIf
    EndIf
  EndIf
  FreeGadget(Gadget)
EndProcedure

Procedure PropertyGridGadget(gadgetNr,x,y,width,height,backColor.q=-1,frontColor.q=-1,flags=#PB_ScrollArea_Single)
  Define *memory.PropertyGridData, gadget.i
  
  gadget = ScrollAreaGadget(gadgetNr,x,y,width,height,width-20,1,#PropertyGrid_DefaultItemHeight,flags)
  If gadget
    If gadgetNr = #PB_Any : gadgetNr = gadget : EndIf
    If backColor <> -1
      SetGadgetColor(gadgetNr,#PB_Gadget_BackColor,backColor)
    EndIf
    If frontColor <> -1
      SetGadgetColor(gadgetNr,#PB_Gadget_FrontColor,frontColor)
    EndIf
    *memory.PropertyGridData = AllocateMemory( SizeOf(PropertyGridData) )
    If Not *memory
      FreeGadget(gadgetNr)
      ProcedureReturn 0
    EndIf
    InitializeStructure(*memory,PropertyGridData)
    *memory\ItemHeight = #PropertyGrid_DefaultItemHeight
    *memory\Font       = LoadFont(#PB_Any,#PropertyGrid_SectionFont,#PropertyGrid_SectionFontSize,#PB_Font_Bold)
    *memory\backColor  = backColor
    *memory\frontColor = frontColor
    SetGadgetData(gadgetNr,*memory)
    CloseGadgetList()
  EndIf
  ProcedureReturn gadgetNr
EndProcedure

Procedure freepropertygridgadget(gadgetNr.i)
  FreeMemory(GetGadgetData(gadgetNr))
  FreeGadget(gadgetNr)
EndProcedure


Procedure AddSection(PropertyGridGadget,sectionName.s,open.i=1)
  Define *memory.PropertyGridData, width.i, height.i, gadget.i
  
  *memory.PropertyGridData = GetGadgetData(PropertyGridGadget)
  If *memory
    LastElement( *memory\Items() )
    If AddElement( *memory\Items() )
      OpenGadgetList(PropertyGridGadget)
      width  = GadgetWidth(PropertyGridGadget)
      height = *memory\ItemHeight
      gadget = TextGadget(#PB_Any,20,0,width-20,height,sectionName)
      *memory\Items()\SectionDescriptionGadget = gadget
      *memory\Items()\SectionButtonGadget      = ImageGadget(#PB_Any,0,0,0,0,ImageID(__GetPropertyGridImage(open)))
      *memory\Items()\Opened    = open
      *memory\Items()\IsSection = 1
      
      SetGadgetFont(gadget,FontID(*memory\Font))
      If *memory\backColor <> -1
        SetGadgetColor(gadget,#PB_Gadget_BackColor,*memory\backColor)
      EndIf
      If *memory\frontColor <> -1
        SetGadgetColor(gadget,#PB_Gadget_FrontColor,*memory\frontColor)
      EndIf
      CloseGadgetList()
      UpdatePropertyGrid(PropertyGridGadget)
      ProcedureReturn *memory\Items()\SectionButtonGadget
    EndIf
  EndIf
EndProcedure

Procedure RemovePropertyGridSection(PropertyGridGadget,SectionGadget)
  Define *memory.PropertyGridData, open, *pgriditem.PropertyGridItem, *pgsubitem.PropertyGridGadgetSubItem
  *memory.PropertyGridData = GetGadgetData(PropertyGridGadget)
  If *memory
    ForEach *memory\Items()
      If SectionGadget = *memory\Items()\SectionButtonGadget
        *pgriditem=*memory\items()
        
        *memory\Items()\Opened ! 1
        open = *memory\Items()\Opened
        SetGadgetState(*memory\Items()\SectionButtonGadget,ImageID(__GetPropertyGridImage(open)))
        
        ForEach *memory\items()\SubItems()
          FreeGadget(*memory\items()\SubItems()\DescriptionGadget)
          FreeGadget(*memory\items()\SubItems()\ItemGadget)
        Next
        
        FreeGadget(*memory\items()\SectionDescriptionGadget)
        FreeGadget(*memory\items()\SectionButtonGadget)
        DeleteElement(*memory\items())
      EndIf
    Next
  EndIf
  UpdatePropertyGrid(PropertyGridGadget)
EndProcedure


Procedure CheckPropertyGridSectionClick(PropertyGridGadget,EventGadget,EventType)
  Define *memory.PropertyGridData, open
  
  *memory.PropertyGridData = GetGadgetData(PropertyGridGadget)
  If *memory
    ForEach *memory\Items()
      If EventGadget = *memory\Items()\SectionButtonGadget And EventType = #PB_EventType_LeftClick
        *memory\Items()\Opened ! 1
        open = *memory\Items()\Opened
        SetGadgetState(*memory\Items()\SectionButtonGadget,ImageID(__GetPropertyGridImage(open)))
        ForEach *memory\Items()\SubItems()
          HideGadget( *memory\Items()\SubItems()\DescriptionGadget , 1-open )
          HideGadget( *memory\Items()\SubItems()\ItemGadget        , 1-open )
        Next
        UpdatePropertyGrid(PropertyGridGadget)
        Break
      EndIf
    Next
    UpdatePropertyGrid(PropertyGridGadget)
  EndIf
EndProcedure

;---[ PropertyGrid Gadgets ]---

Procedure AddStringGadget(PropertyGridGadget,section,Description.s,Content.s,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = StringGadget(#PB_Any,0,0,0,0,Content,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddCheckBoxGadget(PropertyGridGadget,section,Description.s,Text.s,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = CheckBoxGadget(#PB_Any,0,0,0,0,Text,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddButtonGadget(PropertyGridGadget,section,Description.s,Text.s,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = ButtonGadget(#PB_Any,0,0,0,0,Text,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddComboBoxGadget(PropertyGridGadget,section,Description.s,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = ComboBoxGadget(#PB_Any,0,0,0,0,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddProgressBarGadget(PropertyGridGadget,section,Description.s,Minimum,Maximum,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = ProgressBarGadget(#PB_Any,0,0,0,0,Minimum,Maximum,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddSpinGadget(PropertyGridGadget,section,Description.s,Minimum,Maximum,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = SpinGadget(#PB_Any,0,0,0,0,Minimum,Maximum,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddTrackBarGadget(PropertyGridGadget,section,Description.s,Minimum,Maximum,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = TrackBarGadget(#PB_Any,0,0,0,0,Minimum,Maximum,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure


Procedure AddSplitterGadget(PropertyGridGadget,section,Description.s,Gadget1,Gadget2,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = SplitterGadget(#PB_Any,0,0,0,0,Gadget1,Gadget2,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddIPAddressGadget(PropertyGridGadget,section,Description.s)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = IPAddressGadget(#PB_Any,0,0,120,#PropertyGrid_DefaultItemHeight)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddShortcutGadget(PropertyGridGadget,section,Description.s,Shortcut)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = ShortcutGadget(#PB_Any,0,0,0,0,Shortcut)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddDateGadget(PropertyGridGadget,section,Description.s,Mask$="",Date=0,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = DateGadget(#PB_Any,0,0,0,0,Mask$,Date,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

Procedure AddHyperLinkGadget(PropertyGridGadget,section,Description.s,Text$,Color,flags=0)
  Define gadget.i
  
  If __SetSection(PropertyGridGadget,section)
    gadget = HyperLinkGadget(#PB_Any,0,0,0,0,Text$,Color,flags)
    If __AddSectionItem(PropertyGridGadget,section,Description,gadget)
      ProcedureReturn gadget
    EndIf
  EndIf
EndProcedure

;---------------------------------------------------------------------

Procedure AddComboBox_FalseTrue(PropertyGridGadget,section,Description.s,FalseTrue.i)
  Define combo.i
  combo = AddComboBoxGadget(PropertyGridGadget,section,Description)
  AddGadgetItem(combo,-1,"False")
  AddGadgetItem(combo,-1,"True")
  SetGadgetState(combo,FalseTrue)
  ProcedureReturn combo
EndProcedure

CompilerIf #False
  
  OpenWindow(0,0,0,800,600,"PropertyGrid",#PB_Window_SystemMenu|#PB_Window_ScreenCentered|#PB_Window_Invisible)
  
  prop = PropertyGridGadget(#PB_Any,10,10,300,580);,RGB($80,$80,$80),RGB($00,$00,$00))
  
  section1 = AddSection(prop,"Section 1")
  
  string1  = AddStringGadget(prop,section1,"String","Item 1")
  string2  = AddStringGadget(prop,section1,"Number","123456",#PB_String_Numeric)
  
  spin1    = AddSpinGadget  (prop,section1,"SpinGadget",1,100,#PB_Spin_Numeric)
  SetGadgetText(spin1,"1")
  
  split = AddSplitterGadget(prop,section1,"Splitter",ButtonGadget(#PB_Any,0,0,50,0,"1"),ButtonGadget(#PB_Any,0,0,50,0,"2"),#PB_Splitter_Vertical)
  SetGadgetAttribute(split,#PB_Splitter_FirstMinimumSize,50)
  SetGadgetAttribute(split,#PB_Splitter_SecondMinimumSize,50)
  
  AddTrackBarGadget(prop,section1,"Trackbar",1,100)
  
  section2 = AddSection(prop,"Section 2")
  
  combo1 = AddComboBoxGadget(prop,section2,"Combobox 1")
  For i = 1 To 10 : AddGadgetItem(combo1,-1,"Item "+Str(i)) : Next
  
  check1 = AddCheckBoxGadget(prop,section2,"Checkbox 1","Check me!")
  btn1   = AddButtonGadget  (prop,section2,"Choose Directory","...")
  
  section3 = AddSection(prop,"Section 3")
  
  progress = AddProgressBarGadget(prop,section3,"Progress",0,100)
  SetGadgetState(progress,40)
  
  AddShortcutGadget(prop,section3,"Shortcut",0)
  
  AddDateGadget(prop,section3,"Date")
  
  AddHyperLinkGadget(prop,section3,"Hyperlink","Hyper Hyper!",RGB($00,$00,$FF))
  
  ip = AddIPAddressGadget(prop,section3,"IP Address")
  SetGadgetState(ip, MakeIPAddress(127, 0, 0, 1))
  
  For i = 4 To 20
    sec = AddSection(prop,"Section "+Str(i),0)
    For j = 1 To 10
      AddStringGadget(prop,sec,"Description "+Str(j),"Item "+Str(j))
    Next j
  Next i
  
  
  
  prop2 = PropertyGridGadget(#PB_Any,330,10,450,280,RGB($40,$40,$40),RGB($FF,$FF,$FF))
  For i = 1 To 20
    sec = AddSection(prop2,"Section "+Str(i),0)
    For j = 1 To 10
      AddStringGadget(prop2,sec,"Section "+Str(i)+", Field "+Str(j),"Item "+Str(j))
    Next j
  Next i
  
  prop3 = PropertyGridGadget(#PB_Any,330,300,300,280,-1,-1,#PB_ScrollArea_Flat)
  AddComboBox_FalseTrue(prop3,0,"AllowDrop"    ,#False)
  AddComboBox_FalseTrue(prop3,0,"Enabled"      ,#True)
  AddComboBox_FalseTrue(prop3,0,"FullRowSelect",#False)
  AddComboBox_FalseTrue(prop3,0,"HideSelection",#True)
  AddComboBox_FalseTrue(prop3,0,"HotTracking"  ,#False)
spinImgIdx    = AddSpinGadget  (prop3,0,"ImageIndex",1,100,#PB_Spin_Numeric) : SetGadgetText(spinImgIdx,"1")
  secImageSize  = AddSection(prop3,"ImageSize",0)
spinImgWidth  = AddSpinGadget  (prop3,secImageSize,"Width ",1,100,#PB_Spin_Numeric) : SetGadgetText(spinImgWidth ,"32")
spinImgHeight = AddSpinGadget  (prop3,secImageSize,"Height",1,100,#PB_Spin_Numeric) : SetGadgetText(spinImgHeight,"32")
  
  
  While WindowEvent():Wend
  HideWindow(0,0)
  
  Repeat
    Select WaitWindowEvent()
      Case #PB_Event_CloseWindow
        Break
      Case #PB_Event_Gadget
        EventGadget = EventGadget()
        EventType   = EventType()
        Select EventGadget
          Case btn1
            MessageRequester("INFO","Button pressed")
          Default
            ; check for PropertyGrid Events (section opened/closed)
            CheckPropertyGridSectionClick(prop ,EventGadget,EventType)
            CheckPropertyGridSectionClick(prop2,EventGadget,EventType)
            CheckPropertyGridSectionClick(prop3,EventGadget,EventType)
        EndSelect
    EndSelect
  ForEver
  
CompilerEndIf

; IDE Options = PureBasic 4.70 Beta 1 (Windows - x64)
; CursorPosition = 200
; FirstLine = 166
; Folding = ----
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory