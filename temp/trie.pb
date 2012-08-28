

Structure edge
   edge.i[2]
EndStructure

Structure TrieNode;
   Value.i;
   Vertex.edge
EndStructure;

#TrieIntSize = SizeOf(Integer)*8
#TrieNodeSize = SizeOf(TrieNode)

Structure Trie
  *vt.i
  *root.TrieNode;
 EndStructure;
 
 
Global NewList TrieRoots.trie()
Global NewList TrieNodes.TrieNode()

Procedure.i addroot()
  ProcedureReturn AddElement(TrieRoots())
EndProcedure

Procedure.i addnode()
  ProcedureReturn AddElement(TrieNodes())
EndProcedure

Procedure freeroot(*root)
  SelectElement(TrieRoots(),*root)
  DeleteElement(TrieRoots())
EndProcedure

Procedure FreeNod(*node)
  SelectElement(TrieNodes(),*node)
  DeleteElement(TrieNodes())
EndProcedure


Declare NewTrie(*obj.Trie)
Declare Free_Nodes(*node.TrieNode)
Declare Free_Trie(*this.Trie)
Declare Add_Trie(*this.Trie,key.i,value.i=0)
Declare lookup_Trie(*this.Trie,key.i)
Declare Remove_Trie(*this.Trie,key.i)

Interface clsTrie
  Free()
  Add(key.i,item.i=0)
  LookUp(key.i)
  Remove(key.i)
EndInterface   

DataSection: vt_Trie:
  Data.i @Free_Trie()
  Data.i @Add_Trie()
  Data.i @lookup_Trie()
  Data.i @Remove_Trie()
EndDataSection   

Procedure Free_Nodes(*node.TrieNode)
  ;Free all Trie Nodes
  Protected a.i
   If Not *node
      ProcedureReturn;
   EndIf
   Free_Nodes(*node\Vertex\edge[0]);
   Free_Nodes(*node\Vertex\edge[1]);
   If *node
     ;FreeNod(*node)
     FreeMemory(*node)
   EndIf
   
EndProcedure

Procedure NewTrie(*obj.Trie)
  *obj = addroot()
  *obj = AllocateMemory(SizeOf(Trie))
  If *obj
   *obj\vt=?vt_Trie
  EndIf
  ;*obj\root = addnode()
  *obj\root = AllocateMemory(#TrieNodeSize)
  ProcedureReturn *obj
EndProcedure

Procedure Free_Trie(*this.Trie)
  Free_Nodes(*this\root)
  ;freeroot(*this)
  FreeMemory(*this)
EndProcedure

Procedure Add_Trie(*this.Trie,key.i,value.i=0)
  ;Add item to the Trie, value is optional
  Protected *tnode.TrieNode,*node.TrieNode,len.i,tkey.i,lkey.i,pos.i
 
   *node = *this\root;
   
   While pos < #TrieIntSize
     
     If Not *node         
      ;*node = addnode()
      *node = AllocateMemory(#TrieNodeSize)
      *tnode\Vertex\edge[lkey] = *node
     EndIf
     
     If pos =#TrieIntSize-1
       *node\value = value
     EndIf
     
     *tnode = *node
      If key & (1<<pos)
       *node = *node\Vertex\edge[1]
       lkey=1
     Else
       *node = *node\Vertex\edge[0]
       lkey=0 
     EndIf   
     
      pos+1
     
   Wend
   
EndProcedure

Procedure lookup_Trie(*this.Trie,key.i)
  ;Find the value of the key returns the value or 0
  Protected *node.TrieNode,result.i,pos.i   
 
  *node = *this\root
 
  While pos < #TrieIntSize
   
     If key & (1<<pos)
       *node = *node\Vertex\edge[1]
     Else
       *node = *node\Vertex\edge[0]
     EndIf   
     If Not *node
       Break
     EndIf
     pos+1
     result = *node\value
   Wend
   
   ProcedureReturn result
   
EndProcedure

Procedure Remove_Trie(*this.Trie,key.i)
  ;Find the value of the key returns the value or 0
  Protected *node.TrieNode,*result.TrieNode,pos   
 
  *node = *this\root
 
  While pos < #TrieIntSize
   
     If key & (1<<pos)
       *node = *node\Vertex\edge[1]
     Else
       *node = *node\Vertex\edge[0]
     EndIf   
     If Not *node
       Break
     EndIf
     pos+1
     *result  = *node
   Wend
   If *result
     *result\Value = 0
   EndIf   
     
EndProcedure

;Test code_____________________________________________________________________


Global mt.clsTrie = NewTrie(@mt)

;DisableDebugger

Structure pair
   a.i
   b.i
EndStructure

size = 1024*1024

NewMap a.i(size)


b=46589
For a=1 To size
   b+1
   a(Str(a)) = b
   a(Str(b)) = a
   
   mt\Add(a,b)
   mt\Add(b,a)
   
Next

MessageRequester("INFO", "List and Maps filled. Start test.")

start = ElapsedMilliseconds()
For b = 1 To 10

  For n = 10 To size
     temp = mt\LookUp(n)
  Next
 
  For n = 46599 To 97799
     temp = mt\LookUp(n)
   Next
 
Next 
stop = ElapsedMilliseconds()

result_trie = stop-start

start = ElapsedMilliseconds()

For b = 1 To 10

For n = 10 To size
   temp = a(Str(n))
Next

For n = 46599 To 97799
   temp = a(Str(n))
 Next
 
Next 
 
stop = ElapsedMilliseconds()

result_map = stop-start

MessageRequester("", "Map: "+Str(result_map)+#CRLF$+"Trie: "+Str(result_Trie))

mt\Free()
; IDE Options = PureBasic 4.61 (Windows - x86)
; CursorPosition = 189
; FirstLine = 169
; Folding = --
; EnableXP
; CurrentDirectory = C:\Users\void\Dropbox\
; CompileSourceDirectory