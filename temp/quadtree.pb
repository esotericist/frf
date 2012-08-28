;21/12/08 - 4.30
;Example of constructing a quadtree with distribution of objects
;You can change the value of #Quadobject to see the change in distribution
;

Structure s_object
  x.l
  y.l
  radius.l
EndStructure

Structure s_box
  Xmin.l
  Ymin.l
  Xmax.l
  Ymax.l
EndStructure

Structure s_QuadTree
  Depth.l
  box.s_box
  numobjects.l
  *list.s_object 
  *child.s_QuadTree[4]
EndStructure

;-Declaration des procédures
Declare ConstructionQuadTree(*node.s_QuadTree, *box.s_box, *list.s_object, numobjects)
Declare RenderQuadtree(*this.s_QuadTree, counter.i)

;-Variables de configuration
#numobjects  = 5000           ; Nombre d'objects dans la scène
#QuadSize  = 768         ; Taille initiale du quadtree
#QuadDepth = 6           ; Profondeur du quadtree (nb de fois qu'on decoupe le plan)
#Quadobject = 20            ; Nombre d'objects max par node
#radius     = 2            ; radius d'un object

Dim listInitiale.s_object(#numobjects-1)   
Define.s_QuadTree nodeInitial
Define.s_box boxInitiale

;-Initialise une box
Procedure Initbox(*this.s_box, Xmin, Xmax, Ymin, Ymax)
  With *this
    \Xmin = Xmin 
    \Xmax = Xmax
    \Ymin = Ymin
    \Ymax = Ymax
  EndWith
EndProcedure

;Creating a list of objects (spheres in this example)
Procedure Creationlist(Array this.s_object(1))
  For i=0 To #numobjects-1
    this(i)\x = Random(#QuadSize-#radius)
    this(i)\y = Random(#QuadSize-#radius)
    this(i)\radius = #radius   
  Next i
EndProcedure

;Construction of the quadtree with distribution of objects
Procedure ConstructionQuadTree(*node.s_QuadTree, *box.s_box, *list.s_object, numobjects)
  Define.s_box boxchild
  Define.s_object *listchild, *Ptr
  ;Define.s_Vecteur Centrebox, DemiDimensionbox
  Define.s_QuadTree    *PtrF
  Define.l x, y, z, i, t, numobjectschild
   
  NewList List.s_object()
 
  *node\box\Xmin = *box\Xmin
  *node\box\Xmax = *box\Xmax
  *node\box\Ymin = *box\Ymin
  *node\box\Ymax = *box\Ymax

  ; The node can be shared?
  If numobjects > #Quadobject And *node\Depth < #QuadDepth

    ;On répartit les objects dans les nodes child  -- Breaking objects in the child nodes?

    For y = 0 To 1
      For x = 0 To 1
       
        ;Number of children
        i = (y << 1) | x
       
        ;bounding box of the child
        boxchild\Xmin = (1.0 - x / 2.0) * *box\Xmin  + x / 2.0 * *box\Xmax
        boxchild\Xmax = boxchild\Xmin + (*box\Xmax - *box\Xmin) / 2.0
        boxchild\Ymin = (1.0 - y / 2.0) * *box\Ymin  + y / 2.0 * *box\Ymax
        boxchild\Ymax = boxchild\Ymin + (*box\Ymax - *box\Ymin) / 2.0 
         
        *Ptr = *list
       
        ClearList(List())
         
        For t = 1 To numobjects

          ; Calculating objects collision with the box's child 
          If *Ptr\x > boxchild\Xmin And *Ptr\x < boxchild\Xmax And *Ptr\y > boxchild\Ymin And *Ptr\y < boxchild\Ymax
             
            AddElement(List())

            CopyMemory(*Ptr, List(), SizeOf(s_object))
           
          EndIf
         
          *Ptr + SizeOf(s_object)
         
        Next t

        numobjectschild = ListSize(List())
       
        *listchild = #Null
       
        If numobjectschild 
         
          *listchild = AllocateMemory(SizeOf(s_object) * numobjectschild)
          *Ptr = *listchild
         
          ForEach List()
            CopyMemory(List(), *Ptr, SizeOf(s_object))
            *Ptr + SizeOf(s_object)
          Next
         
        EndIf

        ;adds a node
        *node\child[i]=AllocateMemory(SizeOf(s_QuadTree))
     
        *PtrF = *node\child[i]
        *PtrF\Depth = *node\Depth + 1
     
        ConstructionQuadTree(*node\child[i], @boxchild, *listchild, numobjectschild)
       
      Next x
    Next y

  Else
 
    ; Affects the current note in the list
    *node\list = *list
    *node\numobjects = numobjects

  EndIf
 
EndProcedure

; Rendu du Quadtree
Procedure RenderQuadtree(*this.s_QuadTree, counter.i)
  counter + 1
  DrawingMode(#PB_2DDrawing_Outlined)
  Box(*this\box\xmin,*this\box\ymin,*this\box\xmax-*this\box\xmin,*this\box\ymax-*this\box\ymin,#White)
  If *this\list
    *Ptr.s_object = *this\list
    For i = 0 To *this\numobjects-1
      ;Circle(*Ptr\x,*Ptr\y, *Ptr\radius,#Red)
      Box((*ptr\x) -1,(*ptr\y) -1, 2, 2,#Red)
      *Ptr + SizeOf(s_object)
    Next i
  EndIf 
  For y = 0 To 1
    For x = 0 To 1
      i = (y << 1) | x
      If *this\child[i]         
        counter = RenderQuadtree(*this\child[i], counter)
      EndIf
    Next x
  Next y
  ProcedureReturn counter
EndProcedure 

;*******************
;- Exemple         *
;*******************
If InitSprite()=0 Or InitMouse()=0 Or InitKeyboard()=0
  End
EndIf

OpenWindow(0,0,0,1024,768,"Quadtree Demo")
OpenWindowedScreen(WindowID(0),0,0,1024,768,0,0,0)
;OpenScreen(1280,1024,32,"Quadtree Demo")

Repeat

Initbox(@boxInitiale, 0, #QuadSize, 0, #QuadSize)
Creationlist(listInitiale())
ConstructionQuadTree(@nodeInitial, @boxInitiale, listInitiale(), #numobjects)

Repeat
  ClearScreen(#Black)
 
  ; Rendu du quadtree
  StartDrawing(ScreenOutput())
  counter.i = RenderQuadtree(@nodeInitial, 0)
  DrawText(1,1,Str(counter))
  StopDrawing()
  FlipBuffers()

  ExamineKeyboard()
Until KeyboardPushed(#PB_Key_Space) Or KeyboardPushed(#PB_Key_Escape)

Until KeyboardPushed(#PB_Key_Escape)

; IDE Options = PureBasic 4.60 (Windows - x64)
; CursorPosition = 30
; FirstLine = 30
; Folding = -
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\