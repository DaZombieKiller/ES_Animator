#include <windows.h>    
#include "animator.h"
#include <stdio.h>
#include <math.h>

void CheckMouseRange(POINT& p)
{
	if (p.x<10 || p.x>790 || p.y<30 || p.y>550)
	{
		SetCursorPos(400,300);
		GetCursorPos(&p);
	}
}

void ProcessCapturedMouse()
{
	POINT p;
    char t[64];
	GetCursorPos(&p);

    int dx = p.x - lastmpos.x;
    int dy = p.y - lastmpos.y;

	switch( CurCommand ) {
      case IDM_ANI:
	      if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		      g_3dwininfo[3]._xc0 -= (dx) / g_3dwininfo[3].Scaler;
		      g_3dwininfo[3]._yc0 += (dy) / g_3dwininfo[3].Scaler; 		 
		   } else if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { 
			  g_3dwininfo[3].Scaler*=(float)(1.0+float(dy)/100);
		      if (g_3dwininfo[3].Scaler>100) g_3dwininfo[3].Scaler=(float)100;
		      if (g_3dwininfo[3].Scaler<0.1) g_3dwininfo[3].Scaler=(float)0.1; 
           } else  {         
	         g_alpha-=(float)(dx)/100;
	         g_beta-=(float)(dy)/100;	
           }
   	   UpdateEditorWindow(3);          
       break;

      case idt_mov : {
       if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { 
		  g_3dwininfo[CurEditor].Scaler*=(float)(1.0+float(dy)/100);
		  if (g_3dwininfo[CurEditor].Scaler>100) g_3dwininfo[CurEditor].Scaler=(float)100;
		  if (g_3dwininfo[CurEditor].Scaler<0.1) g_3dwininfo[CurEditor].Scaler=(float)0.1; 
        } else  {         
          g_3dwininfo[CurEditor]._xc0 -= (dx) / g_3dwininfo[CurEditor].Scaler;
          g_3dwininfo[CurEditor]._yc0 += (dy) / g_3dwininfo[CurEditor].Scaler;   
        }
       UpdateEditorWindow(CurEditor);          
       break;
      }

	  case idt_setsphere : {
			SphereR[CurObject] += (float)dx;
            sprintf(t,"Sphere R = %3.1f",SphereR[CurObject]);
            SetWindowTitle(t);
            UpdateEditorWindow(-1);
		    break;
		}
 
	  case idt_ikmove: {  
          float scale = g_3dwininfo[CurEditor].Scaler;
		  Vector3d v;
		  int RevView = 0;
          switch (CurEditor) {
           case 0:
	 		  gDX = ((float)(dx))/scale;
			  gDY = ((float)(dy))/scale;
			  gDZ = 0;
			  if (RevView) gDX *= -1;
			  if ((iMoveMode & 1)==0) gDX=0;
			  if ((iMoveMode & 2)==0) gDY=0;
			  v.x = gDX;
			  v.y = gDY;
			  v.z = gDZ;
			  if (mode3d) DragLinks3DR(CurObject, v); else DragLinks2DR(CurObject, v);
			  //DragBase(v);
			  //MoveJoint(CurObject, v);
             break;
           case 1:
	 		  gDZ = ((float)(dx))/scale;
			  gDY = ((float)(dy))/scale;
			  gDX = 0;
			  
			  if (RevView) gDZ *= -1;
			  if ((iMoveMode & 1)==0) gDZ=0;
			  if ((iMoveMode & 2)==0) gDY=0;

			  v.x = gDX;
			  v.y = gDY;
			  v.z = gDZ;
			  if (mode3d) DragLinks3DR(CurObject, v); else DragLinks2DR(CurObject, v);
			  //DragBase(v);
			  //MoveJoint(CurObject, v);
             break;
           case 2:
	 		  gDX = ((float)(dx))/scale;
			  gDZ = ((float)(dy))/scale;/// scale;
			  gDY = 0;
			  
			  if (RevView) gDX *= -1;
			  if (RevView) gDZ *= -1;

			  if ((iMoveMode & 1)==0) gDX=0;
			  if ((iMoveMode & 2)==0) gDZ=0;

			  v.x = gDX;
			  v.y = gDY;
			  v.z = gDZ;
			  if (mode3d) DragLinks3DR(CurObject, v); else DragLinks2DR(CurObject, v);
			  //DragBase(v);
			  //MoveJoint(CurObject, v);
             break;
		  }

          sprintf(t,"DX = %3.1f    DY = %3.1f   DZ = %3.1f",
           Track[CurFrame][CurObject].ddx,
           Track[CurFrame][CurObject].ddy,
           Track[CurFrame][CurObject].ddz);
//          SetWindowTitle(t);
//          sprintf(t,"gDX = %3.1f    gDY = %3.1f   gDZ = %3.1f", gDX,gDY,gDZ);
          SetWindowTitle(t);
 
          RefreshFrameAni();
          RefreshModel();

		  IK_RushObjects();

          UpdateEditorWindow(-1);
		  break;
		}

      case idt_move: {
          float scale = g_3dwininfo[CurEditor].Scaler;
          switch (CurEditor) {
           case 0:
             if (iMoveMode & 1) 
                 if (RevView)  Track[CurFrame][CurObject].ddx-=(float)dx / scale;
                          else Track[CurFrame][CurObject].ddx+=(float)dx / scale;
             if (iMoveMode & 2) Track[CurFrame][CurObject].ddy-=(float)dy / scale; 
             break;
           case 1:
             if (iMoveMode & 1) 
                 if (RevView) Track[CurFrame][CurObject].ddz-=(float)dx / scale;
                         else Track[CurFrame][CurObject].ddz+=(float)dx / scale;
             if (iMoveMode & 2) Track[CurFrame][CurObject].ddy-=(float)dy / scale;
             break;
           case 2:
             if (iMoveMode & 1) 
                 if (RevView) Track[CurFrame][CurObject].ddx-=(float)dx / scale;
                         else Track[CurFrame][CurObject].ddx+=(float)dx / scale;
             if (iMoveMode & 2) 
                 if (RevView) Track[CurFrame][CurObject].ddz-=(float)dy / scale;
                         else Track[CurFrame][CurObject].ddz+=(float)dy / scale;
             break;
          }

          sprintf(t,"DX = %3.1f    DY = %3.1f   DZ = %3.1f", 
           Track[CurFrame][CurObject].ddx,
           Track[CurFrame][CurObject].ddy,
           Track[CurFrame][CurObject].ddz);

          SetWindowTitle(t);
         
          RefreshFrameAni();
          RefreshModel();
          UpdateEditorWindow(-1);
          break; }
      
	  case idt_ikrot: {
		  float Xfi, Yfi, Zfi;

		  Xfi = Track[CurFrame][CurObject].Xfi;
		  Yfi = Track[CurFrame][CurObject].Yfi;
		  Zfi = Track[CurFrame][CurObject].Zfi;

          if (iRotMode == 0) 
              if (RevView) Xfi-=(float)dx / 2;
                      else Xfi+=(float)dx / 2;
          if (iRotMode == 1) Yfi+=(float)dx / 2;
          if (iRotMode == 2) 
              if (RevView) Zfi+=(float)dx / 2;
                      else Zfi-=(float)dx / 2;

		  SetIKBasisAngels(CurObject, Xfi*pi/180.f, Yfi*pi/180.f, Zfi*pi/180.f);
				  
          if (iRotMode == 0) sprintf(t,"X = %3.1f", Xfi);
          if (iRotMode == 1) sprintf(t,"Y = %3.1f", Yfi);
          if (iRotMode == 2) sprintf(t,"Z = %3.1f", Zfi);
          SetWindowTitle(t);

          RefreshFrameAni();
          RefreshModel();
          UpdateEditorWindow(-1);
		  break;}

      case idt_rotate:
		  
		  float Xfi, Yfi, Zfi;

		  Xfi = Track[CurFrame][CurObject].Xfi;
		  Yfi = Track[CurFrame][CurObject].Yfi;
		  Zfi = Track[CurFrame][CurObject].Zfi;

          if (iRotMode == 0) 
              if (RevView) Xfi-=(float)dx / 2;
                      else Xfi+=(float)dx / 2;
          if (iRotMode == 1) Yfi+=(float)dx / 2;
          if (iRotMode == 2) 
              if (RevView) Zfi+=(float)dx / 2;
                      else Zfi-=(float)dx / 2;

		  if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			  SetIKBasisAngels(CurObject, Xfi*pi/180.f, Yfi*pi/180.f, Zfi*pi/180.f);
		  } else {
			  Track[CurFrame][CurObject].Xfi = Xfi;
			  Track[CurFrame][CurObject].Yfi = Yfi;
			  Track[CurFrame][CurObject].Zfi = Zfi;
		  }
					  
          if (iRotMode == 0) sprintf(t,"X = %3.1f", Xfi);
          if (iRotMode == 1) sprintf(t,"Y = %3.1f", Yfi);
          if (iRotMode == 2) sprintf(t,"Z = %3.1f", Zfi);
          SetWindowTitle(t);

          RefreshFrameAni();
          RefreshModel();
          UpdateEditorWindow(-1);
          break;

      case idt_rotatel: {
          
          int CurO = CurObject;
          for (int o=0; o<4; o++) {
           if (CurO==-1) break;
           if (iRotMode == 0) Track[CurFrame][CurO].Xfi+=(float)dx / 2 * ObjRLkoeff[CurObject][o] / 100.f;
           if (iRotMode == 1) Track[CurFrame][CurO].Yfi+=(float)dx / 2 * ObjRLkoeff[CurObject][o] / 100.f;
           if (iRotMode == 2) Track[CurFrame][CurO].Zfi+=(float)dx / 2 * ObjRLkoeff[CurObject][o] / 100.f;
           CurO = gObj[CurO].owner;
          }

          
          if (iRotMode == 0) sprintf(t,"X = %3.1f", Track[CurFrame][CurObject].Xfi);
          if (iRotMode == 1) sprintf(t,"Y = %3.1f", Track[CurFrame][CurObject].Yfi);
          if (iRotMode == 2) sprintf(t,"Z = %3.1f", Track[CurFrame][CurObject].Zfi);
          SetWindowTitle(t);

          RefreshFrameAni();
          RefreshModel();
          UpdateEditorWindow(-1);
          break;
                        }

      case idt_3dscale :
          Track[CurFrame][CurObject].scx+=(float)dx / 200;
          Track[CurFrame][CurObject].scy+=(float)dx / 200;
          Track[CurFrame][CurObject].scz+=(float)dx / 200;
          
          sprintf(t,"SX = %3.1f    SY = %3.1f   SZ = %3.1f", 
           Track[CurFrame][CurObject].scx,
           Track[CurFrame][CurObject].scy,
           Track[CurFrame][CurObject].scz);
          SetWindowTitle(t);

          RefreshFrameAni();
          RefreshModel();
          UpdateEditorWindow(-1);
          break;

      case idt_2dscale :
          switch (CurEditor) {
           case 0:
             if (iMoveMode & 1) Track[CurFrame][CurObject].scx+=(float)dx / 200;
             if (iMoveMode & 2) Track[CurFrame][CurObject].scy+=(float)dx / 200;
             break;
           case 1:
             if (iMoveMode & 1) Track[CurFrame][CurObject].scz+=(float)dx / 200;
             if (iMoveMode & 2) Track[CurFrame][CurObject].scy+=(float)dx / 200;
             break;
           case 2:
             if (iMoveMode & 1) Track[CurFrame][CurObject].scx+=(float)dx / 200;
             if (iMoveMode & 2) Track[CurFrame][CurObject].scz+=(float)dx / 200;
             break;
          }          

          sprintf(t,"SX = %3.1f    SY = %3.1f   SZ = %3.1f", 
           Track[CurFrame][CurObject].scx,
           Track[CurFrame][CurObject].scy,
           Track[CurFrame][CurObject].scz);
          SetWindowTitle(t);

          RefreshFrameAni();
          RefreshModel();
          UpdateEditorWindow(-1);
          break;
   }
   CheckMouseRange(p);
   if (CurCommand==idt_ikmove) {
	   lastmpos=p;
   }
   else lastmpos=p;

}


void PutMouseInWindow( POINT& p )
{
   RECT* rc = &g_3dwininfo[CurEditor].DefPos;

   if( p.x < rc->left )
      p.x = rc->left-1;
   else if( p.x > rc->right )
      p.x = rc->right-1;

   if( p.y < rc->top )
      p.y = rc->top-1;
   else if( p.y > rc->bottom )
      p.y = rc->bottom-1;

   SetCursorPos( p.x, p.y );
}


void WrapCoords( DWORD& l, int& x, int& y )
{
   int _x = LOWORD( l );
   int _y = HIWORD( l );

   if( _x < g_3dwininfo[CurEditor].DefPos.left )
      _x = g_3dwininfo[CurEditor].DefPos.left;
   else if( _x > g_3dwininfo[CurEditor].DefPos.right )
      _x = g_3dwininfo[CurEditor].DefPos.right;

   if( _y < g_3dwininfo[CurEditor].DefPos.top )
      _y = g_3dwininfo[CurEditor].DefPos.top;
   else if( _y > g_3dwininfo[CurEditor].DefPos.bottom )
      _y = g_3dwininfo[CurEditor].DefPos.bottom;

   x = _x; y = _y;
}





void StopCapture( int OK )
{
	RECT rc;
    int o;
	ReleaseCapture();
	MouseCaptured=0;
	RestoreCursorPos();
	if (CurCommand==idt_ikmove) {
		int x,y;
	    RECT rc;
		GetWindowRect( g_3dwin[ CurEditor ], &rc );
		ConvertPointForCurrent( gObj[CurObject].ox, gObj[CurObject].oy, gObj[CurObject].oz, x, y );
		SetCursorPos(x+rc.left,y+rc.top);
	}

    Delay(10);
	ShowCursor( TRUE );

    switch (CurCommand) {
      
      case IDM_ANI:
        if (!OK) 
         { g_alpha = alpha;  g_beta = betha; }
        break;
      case idt_move    :
	  case idt_ikmove  :		  
      case idt_rotate  :
	  case idt_ikrot   :
      case idt_rotatel :
      case idt_3dscale :
      case idt_2dscale :
        if (!OK) for (o=0; o<128; o++) Track[CurFrame][o] = FrameTmp[o];
        RefreshFrame();
        break;
    }

	int i=GetKeyOnCommand(CurCommand);
	CurCommand=-1;


	if (i!=-1) {
	   rc.left=ToolKey[i].x;  rc.top=ToolKey[i].y;
	   rc.right=rc.left+31;   rc.bottom=rc.top+31;
	   InvalidateRect(g_ToolWin,&rc,FALSE); 
    }
    
    RefreshFrame();    
}

void StartCapture( void )
{
   ReActivateIK = 1;
   int o;
   SetCapture(g_MWin);
   SaveCursorPos();
   ShowCursor( FALSE );
   SetCursorPos(400,300);
   GetCursorPos(&lastmpos);
   MouseCaptured = 1; 

   gDX = 0;
   gDY = 0;
   gDZ = 0;

   switch (CurCommand) {
    case IDM_ANI: 
     alpha = g_alpha;
     betha = g_beta;
     break;
	case idt_ikmove  :
		SET_VECTOR(VCash,0,0,0);
		for (o=0; o<OCount; o++) FrameTmp[o] = Track[CurFrame][o];
		for (o=0; o<OCount; o++) {
			//if (Links[o].chCnt>0) 
			//MessageBeep(-1);
               //Track[CurFrame][o].active = TRUE;
		}
		//for (o=0; o<OCount; o++) FrameTmp[o] = Track[CurFrame][o];
	break;

	case idt_move    :
    case idt_rotate  :
    case idt_rotatel :
	case idt_ikrot   :
    case idt_3dscale :
    case idt_2dscale :
     for (o=0; o<OCount; o++) FrameTmp[o] = Track[CurFrame][o];
     if (!Track[CurFrame][CurObject].active) {
       Track[CurFrame][CurObject] = FrameAni[CurObject];
       Track[CurFrame][CurObject].active = TRUE; }

     if (CurCommand == idt_rotatel) 
      if (gObj[CurObject].owner!=-1)
       if (!Track[CurFrame][gObj[CurObject].owner].active) {
         Track[CurFrame][gObj[CurObject].owner] = FrameAni[gObj[CurObject].owner];
         Track[CurFrame][gObj[CurObject].owner].active = TRUE;
       }
       
     RefreshFrame();     
     break;
   }
}
