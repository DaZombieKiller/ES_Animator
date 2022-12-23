#include "animator.h"   // includes application-specific information

int WinSX = 0;
int WinSY = 0;
int WinEX = 940;
int WinEY = 740;


int xa; int xb; int xa16; int xb16;
int v1, v2, v3, tx1, ty1, tx2, ty2, tx3, ty3, x1, Y1, x2, y2, x3, y3, z1, z2, z3, l1, l2, l3;
int dx1,  dx2,  dx3, 
     dtx1, dty1, dz1, 
     dtx2, dty2, dz2, 
     dtx3, dty3, dz3,
     tyb, tya, txa, txb, za, zb;

int ctdy, ctdx, cdz;
int _sp;








void HLineTx( void )
{
   if ( xa < xb ) {
   _asm {
      mov      edi,xa
      sar         edi, 16    
      mov      xa16,edi
      mov      eax,xb; 
      sar         eax, 16
      mov      xb16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  
      mov      ebx,WinSX; 
      cmp      xb16,ebx;  
      jl          END_PAINT;

      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L01
      shl         ebx,2;   
      add         ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txb; 
      sub      eax,txa; 
      imul     ebx; 
      mov      ctdx,edx
      mov      eax,tyb; 
      sub      eax,tya; 
      imul     ebx; 
      mov      ctdy,edx
}L01:     __asm {
      mov      ebx,tya;
      mov      edx,txa; 
	  cmp      xa16,0; 
	  jge L011 
	  mov eax,ctdy;
	  imul xa16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xa16
	  mov      edx,txa;
	  sub       edx,eax
	  add ecx,xa16
	  jz END_PAINT
	  mov      xa16,0    
} L011: 	  __asm {
	  cmp xb16,940
	  jl L012
      sub xb16,940
	  sub ecx,xb16
      jz END_PAINT
} L012:  	  __asm { 
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xa16;
	  shl        edi,1;         
      add      edi, lpVideoBuf
      
      push     ebp
      mov ebp,ctdx
      mov _sp,esp
      mov esp,ctdy
      mov      eax,edx                               
     
   } }  else  { 
   __asm {
      mov      edi,xb
      sar         edi, 16    
      mov      xb16,edi
      mov      eax,xa; 
      sar         eax, 16
      mov      xa16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  { Upper 0, Downer 199 => EXIT }
      mov      ebx,WinSX; 
      cmp      xa16,ebx;  
      jl          END_PAINT;
      
      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L02
      shl         ebx,2;   
      add      ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txa; 
      sub      eax,txb; 
      imul     ebx; 
      mov      ctdx,edx
      mov      eax,tya; 
      sub       eax,tyb; 
      imul     ebx; 
      mov      ctdy,edx
} L02:    __asm{      
	  mov      ebx,tyb;
      mov      edx,txb; 
      cmp      xb16,0; 
	  jge L021 
	  mov eax,ctdy;
	  imul xb16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xb16
	  mov      edx,txb;
	  sub       edx,eax
	  add ecx,xb16
	  jz END_PAINT
	  mov      xb16,0  
}L021: 	  __asm{
	  cmp xa16,940
	  jl L022
      sub xa16,940
	  sub ecx,xa16
      jz END_PAINT
}L022: 	  __asm{
     mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xb16;
	  shl        edi,1;         
      add      edi, lpVideoBuf
      
      push     ebp
      mov ebp,ctdx
     mov _sp,esp
     mov esp,ctdy 
      mov      eax,edx                               

   } }

 L9: 
   __asm {	  
      mov      eax,edx                               
      mov      esi,ebx                          
      shr        eax,16	                          
      shr        esi,16                            
      shl        esi,9                               
      add       esi,lpOrgText
      mov      ax,[esi+eax*2]                       
	  stosw
      
      add      edx,ebp                        
      add      ebx,esp
           
      dec      ecx;       
      jnz     L9

      mov esp,_sp
	  pop ebp

   }
END_PAINT:
   ;
}





void HLineOp( void )
{
   if ( xa < xb ) {
   _asm {
      mov      edi,xa
      sar         edi, 16    
      mov      xa16,edi
      mov      eax,xb; 
      sar         eax, 16
      mov      xb16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  
      mov      ebx,WinSX; 
      cmp      xb16,ebx;  
      jl          END_PAINT;

      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L01
      shl         ebx,2;   
      add         ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txb; 
      sub      eax,txa; 
      imul     ebx; 
      mov      ctdx,edx
      mov      eax,tyb; 
      sub      eax,tya; 
      imul     ebx; 
      mov      ctdy,edx
}L01:     __asm {
      mov      ebx,tya;
      mov      edx,txa; 
	  cmp      xa16,0; 
	  jge L011 
	  mov eax,ctdy;
	  imul xa16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xa16
	  mov      edx,txa;
	  sub       edx,eax
	  add ecx,xa16
	  jz END_PAINT
	  mov      xa16,0    
} L011: 	  __asm {
	  cmp xb16,940;
	  jl L012
      sub xb16,940;
	  sub ecx,xb16
      jz END_PAINT
} L012:  	  __asm { 
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xa16;
	  shl        edi,1;         
      add      edi, lpVideoBuf
      
      push     ebp
      mov ebp,ctdx
      mov _sp,esp
      mov esp,ctdy
      mov      eax,edx                               
     
   } }  else  { 
   __asm {
      mov      edi,xb
      sar         edi, 16    
      mov      xb16,edi
      mov      eax,xa; 
      sar         eax, 16
      mov      xa16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  { Upper 0, Downer 199 => EXIT }
      mov      ebx,WinSX; 
      cmp      xa16,ebx;  
      jl          END_PAINT;
      
      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L02
      shl         ebx,2;   
      add      ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txa; 
      sub      eax,txb; 
      imul     ebx; 
      mov      ctdx,edx
      mov      eax,tya; 
      sub       eax,tyb; 
      imul     ebx; 
      mov      ctdy,edx
} L02:    __asm{      
	  mov      ebx,tyb;
      mov      edx,txb; 
      cmp      xb16,0; 
	  jge L021 
	  mov eax,ctdy;
	  imul xb16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xb16
	  mov      edx,txb;
	  sub       edx,eax
	  add ecx,xb16
	  jz END_PAINT
	  mov      xb16,0  
}L021: 	  __asm{
	  cmp xa16,940
	  jl L022
      sub xa16,940
	  sub ecx,xa16
      jz END_PAINT
}L022: 	  __asm{
     mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xb16;
	  shl        edi,1;         
      add      edi, lpVideoBuf
      
      push     ebp
      mov ebp,ctdx
     mov _sp,esp
     mov esp,ctdy 
      mov      eax,edx                               

   } }

 L9: 
   __asm {	  
      mov      eax,edx                               
      mov      esi,ebx                          
      shr        eax,16	                          
      shr        esi,16                            
      shl        esi,9                               
      add       esi,lpOrgText
      mov      ax,[esi+eax*2]                       
      cmp      eax,0
      jz L10
      mov word ptr[edi],ax
   } L10: __asm {      
      add      edx,ebp                        
      add      ebx,esp
      add      edi,2
           
      dec      ecx;       
      jnz     L9

      mov esp,_sp
	  pop ebp

   }
END_PAINT:
   ;
}





void HLineTr( void )
{
   if ( xa < xb ) {
   _asm {
      mov      edi,xa
      sar         edi, 16    
      mov      xa16,edi
      mov      eax,xb; 
      sar         eax, 16
      mov      xb16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX
      cmp      edi,ebx;     
      jg          END_PAINT;  
      mov      ebx,WinSX; 
      cmp      xb16,ebx;  
      jl          END_PAINT;

      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L01
      shl         ebx,2;   
      add         ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txb; 
      sub      eax,txa; 
      imul     ebx; 
      mov      ctdx,edx
      mov      eax,tyb; 
      sub      eax,tya; 
      imul     ebx; 
      mov      ctdy,edx
}L01:     __asm {
      mov      ebx,tya;
      mov      edx,txa; 
	  cmp      xa16,0; 
	  jge L011 
	  mov eax,ctdy;
	  imul xa16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xa16
	  mov      edx,txa;
	  sub       edx,eax
	  add ecx,xa16
	  jz END_PAINT
	  mov      xa16,0    
} L011: 	  __asm {
	  cmp xb16,940;
	  jl L012
      sub xb16,940;
	  sub ecx,xb16
      jz END_PAINT
} L012:  	  __asm { 
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xa16;
	  shl        edi,1;         
      add      edi, lpVideoBuf
      
      push     ebp
      mov ebp,ctdx
      mov _sp,esp
      mov esp,ctdy
      mov      eax,edx                               
     
   } }  else  { 
   __asm {
      mov      edi,xb
      sar         edi, 16    
      mov      xb16,edi
      mov      eax,xa; 
      sar         eax, 16
      mov      xa16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  { Upper 0, Downer 199 => EXIT }
      mov      ebx,WinSX; 
      cmp      xa16,ebx;  
      jl          END_PAINT;
      
      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L02
      shl         ebx,2;   
      add      ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txa; 
      sub      eax,txb; 
      imul     ebx; 
      mov      ctdx,edx
      mov      eax,tya; 
      sub       eax,tyb; 
      imul     ebx; 
      mov      ctdy,edx
} L02:    __asm{      
	  mov      ebx,tyb;
      mov      edx,txb; 
      cmp      xb16,0; 
	  jge L021 
	  mov eax,ctdy;
	  imul xb16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xb16
	  mov      edx,txb;
	  sub       edx,eax
	  add ecx,xb16
	  jz END_PAINT
	  mov      xb16,0  
}L021: 	  __asm{
	  cmp xa16,940
	  jl L022
      sub xa16,940
	  sub ecx,xa16
      jz END_PAINT
}L022: 	  __asm{
     mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xb16;
	  shl        edi,1;         
      add      edi, lpVideoBuf
      
      push     ebp
      mov ebp,ctdx
      mov _sp,esp
      mov esp,ctdy 
      mov      eax,edx                               

   } }

 L9: 
   __asm {	  
      mov      eax,edx                               
      mov      esi,ebx                          
      shr        eax,16	                          
      shr        esi,16                            
      shl        esi,9                               
      add       esi,lpOrgText

      mov      si, [esi+eax*2]
	  cmp      si,0
      jz L10

	  and       esi,0x7bde	  
	  mov      ax,[edi]	  
	  and       eax,0x7bde	  
	  add       eax,esi
	  shr        eax,1      
	  mov      word ptr[edi],ax
L10:  
      add      edi,2
      add      edx,ebp                        
      add      ebx,esp      
           
      dec      ecx;       
      jnz     L9

      mov esp,_sp
	  pop ebp

   }
END_PAINT:
   ;
}













void DrawTexturedFace( int num )
{
   
   __asm {
      mov      edi, offset gFace
      mov      esi, num
      shl      esi, 6
      add      esi, edi

      mov      eax,offset scrc+4
      mov      edi, [esi+0 ]
      mov      ebx, [eax+edi*8]

      mov      edi, [esi+4 ]
      mov      ecx, [eax+edi*8]

      mov      edi, [esi+8 ]
      mov      edx,[eax+edi*8]

      cmp      ebx,ecx; 
      jge L20 ;  {== if BX<CX then ==}
              ;  {====  if ebx<edx then v1:=0 else v1:=2;  ====}
      cmp      ebx,edx; 
      jge      L11; 
      mov      v1,0; 
      jmp      L12
   }
L11:    
      v1 = 2; 
L12:
//            ;{====  if ecx>=edx then v3:=1 else v3:=2;  ====}

   __asm {
      cmp      ecx,edx; 
      jl       L13;  
      mov      v3,1; 
      jmp      L14
   }

L13:      
      v3 = 2;

L14:
   __asm {
      jmp      L30
   }
L20:
//    ;{==============================================================}
//            ; {====  if ecx<edx then v1:=1 else v1:=2;  ====}
   __asm {
      cmp      ecx,edx; 
      jge      L21; 
      mov      v1,1; 
      jmp      L22
   }
L21:      
   v1 = 2;
L22:
//             ;{====  if ebx>=edx then v3:=0 else v3:=2;  ====}
   __asm {
      cmp      ebx,edx; 
      jl       L23;  
      mov      v3,0; 
      jmp      L24
   }
L23:    
   v3 = 2;
L24:


//;{==== End find v1,v3 ==================================================}
//        ;{=== find v2 ====}
L30:      
   __asm {
      mov      eax,0
      cmp      eax,v1; 
      je       L31; 
      cmp      eax,v3; 
      je       L31; 
      jmp      L39; 
   }
      
L31: 
   __asm {
      inc         eax
      cmp      eax,v1; 
      je          L32; 
      cmp      eax,v3; 
      je          L32; 
      jmp      L39; 
   }
      
L32:  
   __asm {
      inc      eax
   }
L39:      
   __asm {
      mov      v2,eax
   
;{================== fill x,y,tx,ty ====================}
      mov      ebx,v1; 
      mov       eax,[esi+ebx*4+12]; 
	  shl         eax,16
	  add        eax,0x8000
            mov    tx1,eax      
      mov       eax,[esi+ebx*4+24]; 
      shl         eax,16
	  add        eax,0x8000
        mov       ty1,eax
      mov      edi,  [esi+ebx*4];               {// di:=1vNumber //}
      shl         edi,3; 
      add         edi,offset scrc;
      mov         eax,[edi]   
            mov          x1,eax;
      mov      eax,[edi+4]; 
            mov       Y1,eax

      
      mov      ebx,v2; 
      mov       eax,[esi+ebx*4+12]; 
      shl         eax,16
	  add        eax,0x8000
            mov    tx2,eax      
      mov       eax,[esi+ebx*4+24]; 
      shl         eax,16
	  add        eax,0x8000
        mov       ty2,eax
      mov      edi,  [esi+ebx*4];               {// di:=1vNumber //}
      shl         edi,3; 
      add         edi,offset scrc;
      mov         eax,[edi]   
            mov          x2, eax;
      mov      eax,[edi+4]; 
            mov       y2,eax


      mov      ebx,v3; 
      mov       eax,[esi+ebx*4+12]; 
      shl         eax,16
	  add        eax,0x8000
            mov    tx3,eax      
      mov       eax,[esi+ebx*4+24]; 
      shl         eax,16
	  add        eax,0x8000
        mov       ty3,eax
      mov      edi,  [esi+ebx*4];               {// di:=1vNumber //}
      shl         edi,3; 
      add         edi,offset scrc;
      mov         eax,[edi]   
            mov          x3,eax;
      mov      eax,[edi+4]; 
            mov       y3,eax

;{======================================== calc l1,l2,l3 =============}
      mov eax,y2; 
      sub eax,Y1; 
      mov l1,eax;
      mov eax,y3; 
      sub eax,y2; 
      mov l2,eax;
      mov eax,y3; 
      sub eax,Y1; 
      mov l3,eax;

;{======================================== calc dx,dtx,dty ===========}
;{============================================= 1->2 ===========}
      mov eax,tx1; 
      mov txa,eax; 
      mov txb,eax;
      mov eax,ty1; 
      mov tya,eax; 
      mov tyb,eax;

      mov eax,x1; 
      shl eax,16
      add eax,8000h
      mov xa,eax
      mov xb,eax

      mov ebx,l1;  
      cmp ebx,0;  
      je L101
      shl ebx,2;   
      add      ebx,offset DivTbl
      mov      ebx, [ebx]
      mov eax,tx2; 
      sub eax,tx1; 
      imul ebx; 
      mov dtx1,edx
      mov eax,ty2; 
      sub eax,ty1; 
      imul ebx; 
      mov dty1,edx
      
      mov eax,x2;  
      sub eax,x1;
      sal eax,16
      imul ebx;
      mov dx1,edx; 
   }

//;{============================================= 2->3 ===========}
L101:
   __asm {
      mov ebx,l2;  
      cmp ebx,0;  
      je L102
      shl ebx,2;  
      add      ebx, offset DivTbl
      mov ebx,[ebx]
      mov eax,tx3; 
      sub eax,tx2; 
      imul ebx; 
      mov dtx2,edx
      mov eax,ty3; 
      sub eax,ty2; 
      imul ebx; 
      mov dty2,edx
      mov eax,x3;  
      sub eax,x2;  
      sal eax,16
      imul ebx;
      mov dx2,edx; 
      
   }

//;{============================================= 1->3 ===========}

L102:   
   __asm {
      mov ebx,l3;  
      cmp ebx,0;  
      je L103
      shl ebx,2;   
      add      ebx, offset DivTbl
      mov ebx,[ebx]
      mov eax,tx3; 
      sub eax,tx1; 
      imul ebx; 
      mov dtx3,edx
      mov eax,ty3; 
      sub eax,ty1; 
      imul ebx; 
      mov dty3,edx
      mov eax,x3;  
      sub eax,x1;  
      sal eax,16
      imul ebx;
      mov dx3,edx; 
  }

L103:

L201:    
   _asm {
      mov eax,Y1; 
      cmp eax,y2; 
      jge L205
           
      cmp eax,WinSY; 
      jl L203;
      cmp eax,WinEY; 
      jg L400
      call HLineT
   }
L203:
   __asm {
      mov eax,dtx3; 
      add txa,eax
      mov eax,dty3; 
      add tya,eax
      mov eax,dx3; 
      add xa,eax
      
;{====}
      mov eax,dtx1; 
      add txb,eax
      mov eax,dty1; 
      add tyb,eax
      mov eax,dx1; 
      add xb,eax
      inc Y1; 
      jmp L201
   }

//;{=============== 2->3 =================}
L205:
   __asm {
      mov eax,tx2; 
      mov txb,eax;
      mov eax,ty2; 
      mov tyb,eax;
      mov eax,x2;  
      sal eax,16
      add eax,8000h
      mov xb,eax; 
     }

L301:    
   __asm {
      mov eax,Y1; 
      cmp eax,y3;  
      jge L400
      cmp eax,WinSY; 
      jl L303; 
      cmp eax,WinEY; 
      jg L400
      call HLineT
   }
L303:
   __asm {
      mov eax,dtx3; 
      add txa,eax
      mov eax,dty3; 
      add tya,eax
      mov eax,dx3; 
      add xa,eax
   ;{=====}
      mov eax,dtx2; 
      add txb,eax
      mov eax,dty2; 
      add tyb,eax
      mov eax,dx2; 
      add xb,eax
      
      inc Y1;
      jmp L301;
   }
L400: ;

}






























//============= Z BUFFER ===================

void Z_HLineTx( void )
{
   if ( xa < xb ) {
   _asm {
      mov      edi,xa
      sar         edi, 16    
      mov      xa16,edi
      mov      eax,xb; 
      sar         eax, 16
      mov      xb16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  
      mov      ebx,WinSX; 
      cmp      xb16,ebx;  
      jl          END_PAINT;

      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L01
      shl         ebx,2;   
      add         ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txb; 
      sub      eax,txa; 
      imul     ebx; 
          mov      ctdx,edx
      mov      eax,tyb; 
      sub      eax,tya; 
      imul     ebx; 
           mov      ctdy,edx
      mov      eax,zb; 
      sub      eax,za; 
      imul     ebx; 
           mov      cdz,edx
}L01:     __asm {
      mov      ebx,tya;
      mov      edx,txa; 
	  cmp      xa16,0; 
	  jge L011 
	  mov eax,ctdy;
	  imul xa16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xa16
	  mov      edx,txa;
	  sub       edx,eax
	  add ecx,xa16
	  jz END_PAINT
	  mov      xa16,0    
} L011: 	  __asm {
	  cmp xb16,940;
	  jl L012
      sub xb16,940;
	  sub ecx,xb16
      jz END_PAINT
} L012:  	  __asm { 
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xa16;
	  shl        edi,1;      
      
      push ebp
      mov     ebp,ecx
      mov     ecx,za      
      mov      _sp,esp
      mov      esp,lpVideoBuf

   } }  else  { 

   __asm {
      mov      edi,xb
      sar         edi, 16    
      mov      xb16,edi
      mov      eax,xa; 
      sar         eax, 16
      mov      xa16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  { Upper 0, Downer 199 => EXIT }
      mov      ebx,WinSX; 
      cmp      xa16,ebx;  
      jl          END_PAINT;
      
      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L02
      shl         ebx,2;   
      add      ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txa; 
      sub      eax,txb; 
      imul     ebx; 
        mov      ctdx,edx
      mov      eax,tya; 
      sub       eax,tyb; 
      imul     ebx; 
        mov      ctdy,edx
      mov      eax,za; 
      sub      eax,zb; 
      imul     ebx; 
           mov      cdz,edx
} L02:    __asm{      
	  mov      ebx,tyb;
      mov      edx,txb; 
      cmp      xb16,0; 
	  jge L021 
	  mov eax,ctdy;
	  imul xb16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xb16
	  mov      edx,txb;
	  sub       edx,eax
	  add ecx,xb16
	  jz END_PAINT
	  mov      xb16,0  
}L021: 	  __asm{
	  cmp xa16,940
	  jl L022
      sub xa16,940
	  sub ecx,xa16
      jz END_PAINT
}L022: 	  __asm{
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xb16;
	  shl        edi,1;   
      
      push     ebp
      mov      _sp,esp
      mov      esp,lpVideoBuf
      mov      ebp,ecx
      mov      ecx,zb
            
   } }

 L9: 
   __asm {      
      cmp     [offset zbufferdata + edi*2],ecx           
      ja          L10      
      mov      [offset zbufferdata + edi*2],ecx               
      
      mov      esi,ebx                         
      mov      eax,edx            
      shr        esi,16
      shr        eax,16	                   
      shl        esi,9              
      add       esi,lpOrgText
      
      mov      ax,[esi+eax*2]
	  mov      word ptr[esp + edi ],ax
} L10: __asm {       
      add      ebx,ctdy	  
      add      edx,ctdx	 
      add      ecx,cdz      
      add      edi,2
      dec       ebp
      jnz     L9

      mov esp,_sp
      pop ebp
      
      
	
   }
END_PAINT:
   ;
}




void Z_HLineOp( void )
{
   if ( xa < xb ) {
   _asm {
      mov      edi,xa
      sar         edi, 16    
      mov      xa16,edi
      mov      eax,xb; 
      sar         eax, 16
      mov      xb16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  
      mov      ebx,WinSX; 
      cmp      xb16,ebx;  
      jl          END_PAINT;

      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L01
      shl         ebx,2;   
      add         ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txb; 
      sub      eax,txa; 
      imul     ebx; 
          mov      ctdx,edx
      mov      eax,tyb; 
      sub      eax,tya; 
      imul     ebx; 
           mov      ctdy,edx
      mov      eax,zb; 
      sub      eax,za; 
      imul     ebx; 
           mov      cdz,edx
}L01:     __asm {
      mov      ebx,tya;
      mov      edx,txa; 
	  cmp      xa16,0; 
	  jge L011 
	  mov eax,ctdy;
	  imul xa16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xa16
	  mov      edx,txa;
	  sub       edx,eax
	  add ecx,xa16
	  jz END_PAINT
	  mov      xa16,0    
} L011: 	  __asm {
	  cmp xb16,940;
	  jl L012
      sub xb16,940;
	  sub ecx,xb16
      jz END_PAINT
} L012:  	  __asm { 
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xa16;
	  shl        edi,1;      
      
      push ebp
      mov     ebp,ecx
      mov     ecx,za      
      mov      _sp,esp
      mov      esp,lpVideoBuf

   } }  else  { 

   __asm {
      mov      edi,xb
      sar         edi, 16    
      mov      xb16,edi
      mov      eax,xa; 
      sar         eax, 16
      mov      xa16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  { Upper 0, Downer 199 => EXIT }
      mov      ebx,WinSX; 
      cmp      xa16,ebx;  
      jl          END_PAINT;
      
      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L02
      shl         ebx,2;   
      add      ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txa; 
      sub      eax,txb; 
      imul     ebx; 
        mov      ctdx,edx
      mov      eax,tya; 
      sub       eax,tyb; 
      imul     ebx; 
        mov      ctdy,edx
      mov      eax,za; 
      sub      eax,zb; 
      imul     ebx; 
           mov      cdz,edx
} L02:    __asm{      
	  mov      ebx,tyb;
      mov      edx,txb; 
      cmp      xb16,0; 
	  jge L021 
	  mov eax,ctdy;
	  imul xb16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xb16
	  mov      edx,txb;
	  sub       edx,eax
	  add ecx,xb16
	  jz END_PAINT
	  mov      xb16,0  
}L021: 	  __asm{
	  cmp xa16,940
	  jl L022
      sub xa16,940
	  sub ecx,xa16
      jz END_PAINT
}L022: 	  __asm{
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xb16;
	  shl        edi,1;   
      
      push     ebp
      mov      _sp,esp
      mov      esp,lpVideoBuf
      mov      ebp,ecx
      mov      ecx,zb
            
   } }

 L9: 
   __asm {      
      cmp     [offset zbufferdata + edi*2],ecx           
      ja          L10      
      mov      [offset zbufferdata + edi*2],ecx               
      
      mov      esi,ebx                         
      mov      eax,edx            
      shr        esi,16
      shr        eax,16	                   
      shl        esi,9              
      add       esi,lpOrgText
      
      mov      ax,[esi+eax*2]
      cmp      eax,0
      jz       L10
	  mov      word ptr[esp + edi ],ax
} L10: __asm {       
      add      ebx,ctdy	  
      add      edx,ctdx	 
      add      ecx,cdz      
      add      edi,2
      dec       ebp
      jnz     L9

      mov esp,_sp
      pop ebp
      
      
	
   }
END_PAINT:
   ;
}





void Z_HLineTr( void )
{
   if ( xa < xb ) {
   _asm {
      mov      edi,xa
      sar         edi, 16    
      mov      xa16,edi
      mov      eax,xb; 
      sar         eax, 16
      mov      xb16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  
      mov      ebx,WinSX; 
      cmp      xb16,ebx;  
      jl          END_PAINT;

      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L01
      shl         ebx,2;   
      add         ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txb; 
      sub      eax,txa; 
      imul     ebx; 
          mov      ctdx,edx
      mov      eax,tyb; 
      sub      eax,tya; 
      imul     ebx; 
           mov      ctdy,edx
      mov      eax,zb; 
      sub      eax,za; 
      imul     ebx; 
           mov      cdz,edx
}L01:     __asm {
      mov      ebx,tya;
      mov      edx,txa; 
	  cmp      xa16,0; 
	  jge L011 
	  mov eax,ctdy;
	  imul xa16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xa16
	  mov      edx,txa;
	  sub       edx,eax
	  add ecx,xa16
	  jz END_PAINT
	  mov      xa16,0    
} L011: 	  __asm {
	  cmp xb16,940;
	  jl L012
      sub xb16,940;
	  sub ecx,xb16
      jz END_PAINT
} L012:  	  __asm { 
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xa16;
	  shl        edi,1;      
      
      push ebp
      mov     ebp,ecx
      mov     ecx,za      
      mov      _sp,esp
      mov      esp,lpVideoBuf

   } }  else  { 

   __asm {
      mov      edi,xb
      sar         edi, 16    
      mov      xb16,edi
      mov      eax,xa; 
      sar         eax, 16
      mov      xa16,eax
      sub      eax,edi; 
      jz  END_PAINT
      mov      ecx,eax

      mov      ebx,WinEX; 
      cmp      edi,ebx;     
      jg          END_PAINT;  { Upper 0, Downer 199 => EXIT }
      mov      ebx,WinSX; 
      cmp      xa16,ebx;  
      jl          END_PAINT;
      
      mov      ebx,ecx; 
      cmp      ebx,1; 
      jle         L02
      shl         ebx,2;   
      add      ebx, offset DivTbl
      mov      ebx,[ebx]
      mov      eax,txa; 
      sub      eax,txb; 
      imul     ebx; 
        mov      ctdx,edx
      mov      eax,tya; 
      sub       eax,tyb; 
      imul     ebx; 
        mov      ctdy,edx
      mov      eax,za; 
      sub      eax,zb; 
      imul     ebx; 
           mov      cdz,edx
} L02:    __asm{      
	  mov      ebx,tyb;
      mov      edx,txb; 
      cmp      xb16,0; 
	  jge L021 
	  mov eax,ctdy;
	  imul xb16
	  sub ebx,eax
	  mov eax,ctdx
	  imul xb16
	  mov      edx,txb;
	  sub       edx,eax
	  add ecx,xb16
	  jz END_PAINT
	  mov      xb16,0  
}L021: 	  __asm{
	  cmp xa16,940
	  jl L022
      sub xa16,940
	  sub ecx,xa16
      jz END_PAINT
}L022: 	  __asm{
      mov      edi, Y1      
      mov      edi, [offset VideoOffset + edi*4]
      add      edi,xb16;
	  shl        edi,1;   
      
      push     ebp
      mov      _sp,esp
      mov      esp,lpVideoBuf
      mov      ebp,ecx
      mov      ecx,zb
            
   } }

 L9: 
   __asm {      
      cmp     [offset zbufferdata + edi*2],ecx           
      ja          L10            
      
      mov      esi,ebx                         
      mov      eax,edx            
      shr        esi,16
      shr        eax,16	                   
      shl        esi,9              
      add       esi,lpOrgText

      mov      si, [esi+eax*2]
	  and       esi,0x7bde	  
	  mov      ax,[esp + edi]
	  and       eax,0x7bde	  
	  add       eax,esi
	  shr        eax,1      
	  mov      word ptr[esp + edi ],ax

      

} L10: __asm {       
      add      ebx,ctdy	  
      add      edx,ctdx	 
      add      ecx,cdz      
      add      edi,2
      dec       ebp
      jnz     L9

      mov esp,_sp
      pop ebp
      
      
	
   }
END_PAINT:
   ;
}











void DrawZTexturedFace( int num )
{
   
   __asm {
      mov      edi, offset gFace
      mov      esi, num
      shl        esi, 6
      add       esi, edi

      mov      eax,offset scrc+4
      mov      edi, [esi+0 ]
      mov      ebx, [eax+edi*8]

      mov      edi, [esi+4 ]
      mov      ecx, [eax+edi*8]

      mov      edi, [esi+8 ]
      mov      edx,[eax+edi*8]

      cmp      ebx,ecx; 
      jge L20 ;  {== if BX<CX then ==}
             ;{====  if ebx<edx then v1:=0 else v1:=2;  ====}
      cmp      ebx,edx; 
      jge      L11; 
      mov      v1,0; 
      jmp      L12
   }
L11:    
      v1 = 2; 
L12:
//            ;{====  if ecx>=edx then v3:=1 else v3:=2;  ====}

   __asm {
      cmp      ecx,edx; 
      jl          L13;  
      mov      v3,1; 
      jmp      L14
   }

L13:      
      v3 = 2;

L14:
   __asm {
      jmp      L30
   }
L20:
//    ;{==============================================================}
//            ; {====  if ecx<edx then v1:=1 else v1:=2;  ====}
   __asm {
      cmp      ecx,edx; 
      jge      L21; 
      mov      v1,1; 
      jmp      L22
   }
L21:      
   v1 = 2;
L22:
//             ;{====  if ebx>=edx then v3:=0 else v3:=2;  ====}
   __asm {
      cmp      ebx,edx; 
      jl          L23;  
      mov      v3,0; 
      jmp      L24
   }
L23:    
   v3 = 2;
L24:


//;{==== End find v1,v3 ==================================================}
//        ;{=== find v2 ====}
L30:      
   __asm {
      mov      eax,0
      cmp      eax,v1; 
      je          L31; 
      cmp      eax,v3; 
      je          L31; 
      jmp      L39; 
   }
      
L31: 
   __asm {
      inc         eax
      cmp      eax,v1; 
      je          L32; 
      cmp      eax,v3; 
      je          L32; 
      jmp      L39; 
   }
      
L32:  
   __asm {
      inc      eax
   }
L39:      
   __asm {
      mov      v2,eax
   
;{================== fill x,y,tx,ty ====================}
      mov      ebx,v1; 
      mov       eax,[esi+ebx*4+12]; 
	  shl         eax,16
	  add        eax,0x8000
            mov    tx1,eax      
      mov       eax,[esi+ebx*4+24]; 
      shl         eax,16
	  add        eax,0x8000
        mov       ty1,eax
      mov      edi,  [esi+ebx*4];               {// di:=1vNumber //}
      //shl         edi,3; 
      //add         edi,offset scrc;
      mov         eax,[offset scrc + edi*8]   
            mov          x1,eax;
      mov      eax,[offset scrc + 4 + edi*8]; 
            mov       Y1,eax
      mov      eax,[offset ZSize + edi*4]
            mov       z1,eax

      mov      ebx,v2; 
      mov       eax,[esi+ebx*4+12]; 
      shl         eax,16
	  add        eax,0x8000
            mov    tx2,eax      
      mov       eax,[esi+ebx*4+24]; 
      shl         eax,16
	  add        eax,0x8000
        mov       ty2,eax
      mov      edi,  [esi+ebx*4];               {// di:=1vNumber //}
      //shl         edi,3; 
      mov         eax,[offset scrc + edi*8]   
            mov          x2,eax;
      mov      eax,[offset scrc + 4 + edi*8]; 
            mov       y2,eax
      mov      eax,[offset ZSize + edi*4]
            mov       z2,eax
      


      mov      ebx,v3; 
      mov       eax,[esi+ebx*4+12]; 
      shl         eax,16
	  add        eax,0x8000
            mov    tx3,eax      
      mov       eax,[esi+ebx*4+24]; 
      shl         eax,16
	  add        eax,0x8000
        mov       ty3,eax
      mov      edi,  [esi+ebx*4];               {// di:=1vNumber //}
      //shl         edi,3; 
      mov         eax,[offset scrc + edi*8]   
            mov          x3,eax;
      mov      eax,[offset scrc + 4 + edi*8]; 
            mov       y3,eax
      mov      eax,[offset ZSize + edi*4]
            mov       z3,eax
      

;{======================================== calc l1,l2,l3 =============}
      mov eax,y2; 
      sub eax,Y1; 
      mov l1,eax;
      mov eax,y3; 
      sub eax,y2; 
      mov l2,eax;
      mov eax,y3; 
      sub eax,Y1; 
      mov l3,eax;

;{======================================== calc dx,dtx,dty ===========}
;{============================================= 1->2 ===========}
      mov eax,tx1; 
      mov txa,eax; 
      mov txb,eax;
      mov eax,ty1; 
      mov tya,eax; 
      mov tyb,eax;
      mov eax,z1; 
      mov za,eax; 
      mov zb,eax;

      mov eax,x1; 
      shl eax,16
      add eax,8000h
      mov xa,eax
      mov xb,eax

      mov ebx,l1;  
      cmp ebx,0;  
      je L101
      shl ebx,2;   
      add      ebx,offset DivTbl
      mov      ebx, [ebx]
      mov eax,tx2; 
      sub eax,tx1; 
      imul ebx; 
        mov dtx1,edx
      mov eax,ty2; 
      sub eax,ty1; 
      imul ebx; 
        mov dty1,edx
      mov eax,z2; 
      sub eax,z1; 
      imul ebx; 
        mov dz1,edx
      
      mov eax,x2;  
      sub eax,x1;
      sal eax,16
      imul ebx;
        mov dx1,edx; 
   }

//;{============================================= 2->3 ===========}
L101:
   __asm {
      mov ebx,l2;  
      cmp ebx,0;  
      je L102
      shl ebx,2;  
      add      ebx, offset DivTbl
      mov ebx,[ebx]
      mov eax,tx3; 
      sub eax,tx2; 
      imul ebx; 
        mov dtx2,edx
      mov eax,ty3; 
      sub eax,ty2; 
      imul ebx; 
        mov dty2,edx
      mov eax,z3; 
      sub eax,z2; 
      imul ebx; 
        mov dz2,edx
      mov eax,x3;  
      sub eax,x2;  
      sal eax,16
      imul ebx;
        mov dx2,edx; 
      
   }

//;{============================================= 1->3 ===========}

L102:   
   __asm {
      mov ebx,l3;  
      cmp ebx,0;  
      je L103
      shl ebx,2;   
      add      ebx, offset DivTbl
      mov ebx,[ebx]
      mov eax,tx3; 
      sub eax,tx1; 
      imul ebx; 
        mov dtx3,edx
      mov eax,ty3; 
      sub eax,ty1; 
      imul ebx; 
        mov dty3,edx
      mov eax,z3; 
      sub eax,z1; 
      imul ebx; 
        mov dz3,edx
      mov eax,x3;  
      sub eax,x1;  
      sal eax,16
      imul ebx;
        mov dx3,edx; 
  }

L103:

L201:    
   _asm {
      mov eax,Y1; 
      cmp eax,y2; 
      jge L205
           
      cmp eax,WinSY; 
      jl L203;
      cmp eax,WinEY; 
      jg L400
      call HLineT
   }
L203:
   __asm {
      mov eax,dtx3; 
      add txa,eax
      
      mov eax,dty3; 
      add tya,eax

      mov eax,dz3; 
      add za,eax

      mov eax,dx3; 
      add xa,eax
      
;{====}
      mov eax,dtx1; 
      add txb,eax
      mov eax,dty1; 
      add tyb,eax
      mov eax,dz1; 
      add zb,eax
      mov eax,dx1; 
      add xb,eax
      inc Y1; 
      jmp L201
   }

//;{=============== 2->3 =================}
L205:
   __asm {
      mov eax,tx2; 
      mov txb,eax;
      mov eax,ty2; 
      mov tyb,eax;
      mov eax,z2; 
      mov zb,eax;
      mov eax,x2;  
      sal eax,16
      add eax,8000h
      mov xb,eax; 
     }

L301:    
   __asm {
      mov eax,Y1; 
      cmp eax,y3;  
      jge L400
      cmp eax,WinSY; 
      jl L303; 
      cmp eax,WinEY; 
      jg L400
      call HLineT
   }
L303:
   __asm {
      mov eax,dtx3; 
      add txa,eax
      mov eax,dty3; 
      add tya,eax
      mov eax,dz3; 
      add za,eax
      mov eax,dx3; 
      add xa,eax
   ;{=====}
      mov eax,dtx2; 
      add txb,eax
      mov eax,dty2; 
      add tyb,eax
      mov eax,dz2; 
      add zb,eax
      mov eax,dx2; 
      add xb,eax
      
      inc Y1;
      jmp L301;
   }
L400: ;
}






