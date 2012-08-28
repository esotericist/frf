
;
; Simple PropertyGrid
;
; by Danilo, May 2012, PureBasic 4.60
;
#PropertyGrid_SectionFont          = "Arial"
#PropertyGrid_ImageLineColor       = $0000ff
#PropertyGrid_ImageBackgroundColor = $ffffff
#PropertyGrid_SectionNamePreSpace  = #True

Macro OS_Select ( variable, windows, linux, macos, def )
 
  CompilerSelect #PB_Compiler_OS
    CompilerCase #PB_OS_Windows : variable = windows
    CompilerCase #PB_OS_Linux   : variable = linux
    CompilerCase #PB_OS_MacOS   : variable = macos
    CompilerDefault            : variable = def
  CompilerEndSelect
 
EndMacro

OS_Select ( #PropertyGrid_DefaultItemHeight, 20, 22, 25, 20 )
OS_Select ( #PropertyGrid_SectionFontSize,   10, 10, 14, 12 )

Structure PropertyGridGadgetSubItem
    DescriptionGadget.i
    ItemGadget.i
EndStructure

Structure PropertyGridItem
    SectionDescriptionGadget.i
    SectionButtonGadget.i
    IsSection.i
    IsLocked.b
    Opened.i
    PopupMenu.i
    List SubItems.PropertyGridGadgetSubItem()
EndStructure

Structure PropertyGridData
  GridID.l   
  ItemHeight.i
  Font.i
  backColor.q
  frontColor.q
  Window.i
  PopupMenu.i
  List Items.PropertyGridItem()
EndStructure

Structure PropertyGridInfo
  ID.l
  Parent.i
EndStructure

Structure PropertyGridPopup
  GridGadget.i
  MenuItem.i
  Parent.i
  Item.i
 
EndStructure

Global NewList PopupMenuItems.PropertyGridPopup()
Global LastPopupMenuItem.i = -1

Procedure __GetPropertyGridImage ( state.i )
 
  Static openImage.i, closeImage.i, lockImage.i, defaultImage.i
 
  If Not defaultImage
    defaultImage = CreateImage ( #PB_Any, 11, 11, 24 )
    If defaultImage And StartDrawing ( ImageOutput ( defaultImage ) )
      Box ( 0, 0, 11, 11, #PropertyGrid_ImageBackgroundColor )
      DrawingMode ( #PB_2DDrawing_Outlined )
      Box ( 0, 0, 11, 11, #PropertyGrid_ImageLineColor )
      StopDrawing()
    EndIf
  EndIf
 
  If Not openImage
    openImage = CopyImage ( defaultImage, #PB_Any )
    If openImage And StartDrawing ( ImageOutput ( openImage ) )
      LineXY ( 2, 5, 8, 5, #PropertyGrid_ImageLineColor )
      StopDrawing ( )
    EndIf
  EndIf
     
  If Not closeImage
    closeImage = CopyImage ( openImage, #PB_Any )
    If closeImage And StartDrawing( ImageOutput ( closeImage ) )
      LineXY ( 5, 2, 5, 8, #PropertyGrid_ImageLineColor )
      StopDrawing ( )
    EndIf
  EndIf
 
  If Not lockImage
    lockImage = CopyImage ( defaultImage, #PB_Any )
    If lockImage And StartDrawing( ImageOutput ( lockImage ) )
      Box ( 3, 3, 5, 5, #PropertyGrid_ImageLineColor )
      StopDrawing ( )
    EndIf
  EndIf

 
  Select state
    Case 0 : ProcedureReturn closeImage
    Case 1 : ProcedureReturn openImage
    Case 2 : ProcedureReturn lockImage
    Default : ProcedureReturn defaultImage
  EndSelect
     
EndProcedure

Procedure UpdatePropertyGrid ( PropertyGridGadget )
 
  *memory.PropertyGridData = GetGadgetData ( PropertyGridGadget )
 
  If *memory
    i = 0
    height = *memory\ItemHeight
    width  = GadgetWidth ( PropertyGridGadget )
   
    ForEach *memory\Items()
      If *memory\Items()\IsSection
        ResizeGadget( *memory\Items()\SectionDescriptionGadget, 20, i, width - 20, height - 1 )
        ResizeGadget( *memory\Items()\SectionButtonGadget     , 4,  i + height * 0.5 - 6, 11, 11 )
      Else
        ResizeGadget( *memory\Items()\SectionDescriptionGadget, 20, i, width * 0.5 - 40, height - 1 )
        ResizeGadget( *memory\Items()\SectionButtonGadget     , width * 0.5 - 19, i, width * 0.5 - 4, height - 1 )
      EndIf
     
      i + height
      If *memory\Items()\Opened
        ForEach *memory\Items()\SubItems()
          ResizeGadget( *memory\Items()\SubItems()\DescriptionGadget, 20, i, width * 0.5 - 40, height - 1)
          ResizeGadget( *memory\Items()\SubItems()\ItemGadget       , width * 0.5 - 19, i, width * 0.5 - 4, height - 1 )
          i + height
        Next
      EndIf
    Next
    SetGadgetAttribute ( PropertyGridGadget, #PB_ScrollArea_InnerHeight, i )
    SetGadgetAttribute ( PropertyGridGadget, #PB_ScrollArea_InnerWidth, width - 20 )
       
  EndIf
EndProcedure



Procedure __SetSection ( PropertyGridGadget, section )
 
  *memory.PropertyGridData = GetGadgetData ( PropertyGridGadget )
 
  If *memory
    If section = 0
        LastElement( *memory\Items() )
        OpenGadgetList ( PropertyGridGadget )
        ProcedureReturn #True
      EndIf
    ForEach *memory\Items()
      If *memory\Items()\SectionButtonGadget = section
        OpenGadgetList ( PropertyGridGadget )
        ProcedureReturn #True
      EndIf
    Next
  EndIf
   
EndProcedure

Procedure __AddSectionItem ( PropertyGridGadget, section, Description.s, Gadget.i )
 
  *memory.PropertyGridData = GetGadgetData ( PropertyGridGadget )
 
  If *memory

    descGadget = StringGadget ( #PB_Any, 0, 0, 0, 0, Description, #PB_String_ReadOnly )
    ;descGadget = TextGadget(#PB_Any,0,0,0,0,Description);,#PB_Text_Border)
    ;DisableGadget( descGadget, 1 )
   
    If section = 0
      If AddElement( *memory\Items() )
       
        With *memory\Items()
          \IsSection                = 0
          \IsLocked                 = #False
          \SectionButtonGadget      = Gadget
          \SectionDescriptionGadget = descGadget
        EndWith
         
        CloseGadgetList()
        UpdatePropertyGrid ( PropertyGridGadget )
        ProcedureReturn #True
      EndIf
     
    Else
      LastElement( *memory\Items()\SubItems() )
      If AddElement( *memory\Items()\SubItems() )
       
        With *memory\Items()\SubItems()
          \DescriptionGadget = descGadget
          \ItemGadget        = Gadget
        EndWith
       
        CloseGadgetList()
        If *memory\Items()\Opened
          UpdatePropertyGrid(PropertyGridGadget)
        Else
          HideGadget ( *memory\Items()\SubItems()\DescriptionGadget, 1 )
          HideGadget ( *memory\Items()\SubItems()\ItemGadget       , 1 )
        EndIf
       
        ProcedureReturn #True
      EndIf
    EndIf
  EndIf
  FreeGadget ( Gadget )
 
EndProcedure

Procedure __SetGadgetColor ( gadget, color_type, color )
 
  If color <> -1
    SetGadgetColor ( gadget, color_type, color )
  EndIf
 
EndProcedure

Procedure __AddPopupMenuItem ( PropertyGridGadget, button, menu_item, text.s )
 
  LastPopupMenuItem + 1
 
  AddElement ( PopupMenuItems() )
  PopupMenuItems()\GridGadget = PropertyGridGadget
  PopupMenuItems()\Item       = LastPopupMenuItem
  PopupMenuItems()\MenuItem   = menu_item
  PopupMenuItems()\Parent     = button
     
  MenuItem ( LastPopupMenuItem, text )
 
EndProcedure

Procedure PropertyGridGadget ( gadgetNr, window, x, y, width, height, backColor.q = -1, frontColor.q = -1, flags = #PB_ScrollArea_Single )
 
  gadget = ScrollAreaGadget ( gadgetNr, x, y, width, height, width - 20, 1, #PropertyGrid_DefaultItemHeight, flags )
 
  If gadget
    If gadgetNr = #PB_Any : gadgetNr = gadget : EndIf
     
    __SetGadgetColor ( gadgetNr, #PB_Gadget_BackColor,  backColor  )
    __SetGadgetColor ( gadgetNr, #PB_Gadget_FrontColor, frontColor )
     
    *memory.PropertyGridData = AllocateMemory( SizeOf ( PropertyGridData ) )
   
    If Not *memory
      FreeGadget(gadgetNr)
      ProcedureReturn 0
    EndIf
   
    InitializeStructure ( *memory,PropertyGridData )
       
    With *memory
      \ItemHeight = #PropertyGrid_DefaultItemHeight
      \Font       = LoadFont ( #PB_Any, #PropertyGrid_SectionFont, #PropertyGrid_SectionFontSize, #PB_Font_Bold )
      \backColor  = backColor
      \frontColor = frontColor
      \Window     = window
    EndWith
       
    SetGadgetData ( gadgetNr, *memory )
    CloseGadgetList()
  EndIf
   
  ProcedureReturn gadgetNr
   
EndProcedure

Procedure AddSection ( PropertyGridGadget, sectionName.s, open.i = 1, backColor.q = -1, frontColor.q = -1 )
 
  If #PropertyGrid_SectionNamePreSpace = #True
    sectionName = " " + sectionName
  EndIf
 
  *memory.PropertyGridData = GetGadgetData ( PropertyGridGadget )
 
  If *memory
    LastElement( *memory\Items() )
    If AddElement( *memory\Items() )
      OpenGadgetList ( PropertyGridGadget )
      width  = GadgetWidth ( PropertyGridGadget )
      height = *memory\ItemHeight
      gadget = TextGadget ( #PB_Any, 20, 0, width - 20, height, sectionName )
      With *memory\Items()
        \SectionDescriptionGadget = gadget
        \SectionButtonGadget      = ImageGadget ( #PB_Any, 0, 0, 0, 0, ImageID ( __GetPropertyGridImage ( open ) ) )
        \Opened                   = open
        \IsSection                = 1
        \IsLocked                 = #False
        If \SectionButtonGadget
          PopupMenu = CreatePopupMenu ( #PB_Any )
          If PopupMenu
            __AddPopupMenuItem ( PropertyGridGadget, \SectionButtonGadget, 1, "Expand All" )
            __AddPopupMenuItem ( PropertyGridGadget, \SectionButtonGadget, 2, "Colapse All" )
            __AddPopupMenuItem ( PropertyGridGadget, \SectionButtonGadget, 3, "Lock / Unlock" )
           
            \PopupMenu = PopupMenu

          EndIf
          *memory_button.PropertyGridInfo = AllocateMemory ( SizeOf ( PropertyGridInfo ) )
          If *memory_button
            *memory_button\ID     = $44495247
            *memory_button\Parent = PropertyGridGadget
          EndIf
          SetGadgetData ( \SectionButtonGadget, *memory_button )
        EndIf
      EndWith   
      If backColor  = -1 : backColor  = *memory\backColor  : EndIf
      If frontColor = -1 : frontColor = *memory\frontColor : EndIf
      SetGadgetFont    ( gadget, FontID ( *memory\Font ) )
      __SetGadgetColor ( gadget, #PB_Gadget_BackColor,  backColor  )
      __SetGadgetColor ( gadget, #PB_Gadget_FrontColor, frontColor )
      CloseGadgetList()
      UpdatePropertyGrid ( PropertyGridGadget )
      ProcedureReturn *memory\Items()\SectionButtonGadget
    EndIf
  EndIf
 
EndProcedure

Procedure LockSection ( PropertyGridGadget, Section, Lock )
 
  *memory.PropertyGridData = GetGadgetData ( PropertyGridGadget )
   
  If *memory
     
    ForEach *memory\Items()
     
      If *memory\Items()\SectionButtonGadget = Section
        *memory\Items()\IsLocked = Lock
        If Lock = #True
          image_number = 2
         
        Else
          image_number = *memory\Items()\Opened
         
        EndIf
        SetGadgetState ( *memory\Items()\SectionButtonGadget, ImageID ( __GetPropertyGridImage ( image_number ) ) )
        Break
      EndIf
     
    Next
     
  EndIf
 
EndProcedure


Procedure ChangeSectionState ( *memory.PropertyGridItem, state )
 
  If state = 2
    *memory\Opened ! 1
  Else
    *memory\Opened = state
  EndIf
 
  SetGadgetState ( *memory\SectionButtonGadget, ImageID ( __GetPropertyGridImage ( *memory\Opened ) ) )
           
  ForEach *memory\SubItems()
    HideGadget( *memory\SubItems()\DescriptionGadget , 1 - *memory\Opened )
    HideGadget( *memory\SubItems()\ItemGadget        , 1 - *memory\Opened )
  Next
 
EndProcedure


Procedure CheckPropertyGridSectionClick ( PropertyGridGadget, EventGadget, EventType )
  *memory.PropertyGridData = GetGadgetData ( PropertyGridGadget )
  If *memory
    ForEach *memory\Items()
      If EventGadget = *memory\Items()\SectionButtonGadget
        If EventType = #PB_EventType_LeftClick And *memory\Items()\IsLocked = #False
          ChangeSectionState ( *memory\Items(), 2 )
          UpdatePropertyGrid ( PropertyGridGadget )
          ProcedureReturn
        EndIf
        If EventType = #PB_EventType_RightClick And *memory\Items()\PopupMenu <> 0
          DisplayPopupMenu ( *memory\Items()\PopupMenu, WindowID ( *memory\Window ) )
        EndIf
      EndIf
    Next
  EndIf
EndProcedure


Procedure SectionActions ( PropertyGridGadget, Section, Action )
 
  *memory.PropertyGridData = GetGadgetData ( PropertyGridGadget )
   
  If *memory
     
    ForEach *memory\Items()
       
      Select Action
           
        Case 1
          If *memory\Items()\IsLocked = #False
            ChangeSectionState ( *memory\Items(), 1 )
          EndIf
           
        Case 2
          If *memory\Items()\IsLocked = #False
            ChangeSectionState ( *memory\Items(), 0 )
          EndIf
           
        Case 3
          If Section = *memory\Items()\SectionButtonGadget
            If *memory\Items()\IsLocked = #True
              LockSection ( PropertyGridGadget, Section, #False )
               
            Else
              LockSection ( PropertyGridGadget, Section, #True )
               
            EndIf
            ProcedureReturn
          EndIf
           
      EndSelect
       
    Next
    UpdatePropertyGrid ( PropertyGridGadget )
  EndIf
 
EndProcedure


;---[ PropertyGrid Gadgets ]---

Macro AddGridGadget ( PGG, Section, GadgetNr )
 
  If __SetSection ( PGG, Section )
    gadget = GadgetNr
    If __AddSectionItem ( PGG, Section, Description, gadget )
      ProcedureReturn gadget
    EndIf
  Else
    FreeGadget ( GadgetNr )
  EndIf
 
EndMacro

Procedure AddStringGadget ( PropertyGridGadget, section, Description.s, Content.s, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, StringGadget ( #PB_Any, 0, 0, 0, 0, Content, flags ) )
EndProcedure

Procedure AddCheckBoxGadget ( PropertyGridGadget, section, Description.s, Text.s, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, CheckBoxGadget ( #PB_Any, 0, 0, 0, 0, Text, flags ) )
EndProcedure

Procedure AddButtonGadget ( PropertyGridGadget, section, Description.s, Text.s, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, ButtonGadget ( #PB_Any, 0, 0, 0, 0, Text, flags ) )
EndProcedure

Procedure AddComboBoxGadget ( PropertyGridGadget, section, Description.s, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, ComboBoxGadget ( #PB_Any, 0, 0, 0, 0, flags ) )
EndProcedure

Procedure AddProgressBarGadget ( PropertyGridGadget, section, Description.s, Minimum, Maximum, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, ProgressBarGadget ( #PB_Any, 0, 0, 0, 0, Minimum, Maximum, flags ) )
EndProcedure

Procedure AddSpinGadget ( PropertyGridGadget, section, Description.s, Minimum, Maximum, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, SpinGadget ( #PB_Any, 0, 0, 0, 0, Minimum, Maximum, flags ) )
EndProcedure

Procedure AddTrackBarGadget ( PropertyGridGadget, section, Description.s, Minimum, Maximum, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, TrackBarGadget ( #PB_Any, 0, 0, 0, 0, Minimum, Maximum, flags ) )
EndProcedure

Procedure AddSplitterGadget ( PropertyGridGadget, section, Description.s, Gadget1, Gadget2, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, SplitterGadget ( #PB_Any, 0, 0, 0, 0, Gadget1, Gadget2, flags ) )
EndProcedure

Procedure AddIPAddressGadget ( PropertyGridGadget, section, Description.s )
  AddGridGadget ( PropertyGridGadget, section, IPAddressGadget ( #PB_Any, 0, 0, 120, #PropertyGrid_DefaultItemHeight ) )
EndProcedure

Procedure AddShortcutGadget ( PropertyGridGadget, section, Description.s, Shortcut )
  AddGridGadget ( PropertyGridGadget, section, ShortcutGadget ( #PB_Any, 0, 0, 0, 0, Shortcut ) )
EndProcedure

Procedure AddDateGadget ( PropertyGridGadget, section, Description.s, Mask$ = "", Date = 0, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, DateGadget ( #PB_Any, 0, 0, 0, 0, Mask$, Date, flags ) )
EndProcedure

Procedure AddHyperLinkGadget ( PropertyGridGadget, section, Description.s, Text$, Color, flags = 0 )
  AddGridGadget ( PropertyGridGadget, section, HyperLinkGadget ( #PB_Any, 0, 0, 0, 0, Text$, Color, flags ) )
EndProcedure

Procedure IsGridGadget ( Gadget )
 
  *memory.PropertyGridInfo = GetGadgetData ( Gadget )
 
  If *memory
   
    If *memory\ID = $44495247
      ProcedureReturn *memory\Parent
     
    EndIf
   
  EndIf
 
  ProcedureReturn 0
 
EndProcedure


;---------------------------------------------------------------------

Procedure AddComboBox_FalseTrue(PropertyGridGadget,section,Description.s,FalseTrue.i)
    combo = AddComboBoxGadget(PropertyGridGadget,section,Description)
    AddGadgetItem(combo,-1,"False")
    AddGadgetItem(combo,-1,"True")
    SetGadgetState(combo,FalseTrue)
    ProcedureReturn combo
EndProcedure

main_window = OpenWindow ( #PB_Any,0,0,800,600,"PropertyGrid",#PB_Window_SystemMenu|#PB_Window_ScreenCentered|#PB_Window_Invisible)

prop = PropertyGridGadget(#PB_Any, main_window, 10, 10, 300, 580);,RGB($80,$80,$80),RGB($00,$00,$00))

section1 = AddSection(prop,"Section 1", 1, $ffcc00 )

    string1  = AddStringGadget(prop,section1,"String","Item 1")
    string2  = AddStringGadget(prop,section1,"Number","123456",#PB_String_Numeric)
   
    spin1    = AddSpinGadget  (prop,section1,"SpinGadget",1,100,#PB_Spin_Numeric)
    SetGadgetText(spin1,"1")
   
    split = AddSplitterGadget(prop,section1,"Splitter",ButtonGadget(#PB_Any,0,0,50,0,"1"),ButtonGadget(#PB_Any,0,0,50,0,"2"),#PB_Splitter_Vertical)
    SetGadgetAttribute(split,#PB_Splitter_FirstMinimumSize,50)
    SetGadgetAttribute(split,#PB_Splitter_SecondMinimumSize,50)

    AddTrackBarGadget(prop,section1,"Trackbar",1,100)
   
    LockSection ( prop, section1, #True )
   
section2 = AddSection(prop,"Section 2", 1, $00ccff)

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



prop2 = PropertyGridGadget(#PB_Any, main_window, 330,10,450,280,RGB($40,$40,$40),RGB($FF,$FF,$FF))
For i = 1 To 20
    sec = AddSection(prop2,"Section "+Str(i),0)
    For j = 1 To 10
        AddStringGadget(prop2,sec,"Section "+Str(i)+", Field "+Str(j),"Item "+Str(j))
    Next j
Next i

prop3 = PropertyGridGadget(#PB_Any, main_window, 330,300,300,280,-1,-1,#PB_ScrollArea_Flat)
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
HideWindow(main_window,0)

Repeat
    Select WaitWindowEvent()
        Case #PB_Event_CloseWindow
          Break
         
        Case #PB_Event_Menu
          ResetList ( PopupMenuItems() )
          ForEach PopupMenuItems()
            If PopupMenuItems()\Item = EventMenu()
              SectionActions ( PopupMenuItems()\GridGadget, PopupMenuItems()\Parent, PopupMenuItems()\MenuItem )
             
            EndIf
          Next
         
        Case #PB_Event_Gadget
            EventGadget = EventGadget()
            EventType   = EventType()
           
            Select EventGadget
                Case btn1
                  MessageRequester("INFO","Button pressed")
                  LockSection ( prop, section1, #False )
                 
                Default
                 
                  GridGagdet = IsGridGadget ( EventGadget )
                 
                  If GridGagdet
                    CheckPropertyGridSectionClick ( GridGagdet, EventGadget, EventType )
                   
                  EndIf

            EndSelect
    EndSelect
ForEver
; IDE Options = PureBasic 4.61 (Windows - x64)
; CursorPosition = 375
; FirstLine = 375
; Folding = -----
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory