#include <windows.h>    // includes basic windows functionality
#include <commctrl.h>
#include <stdio.h>
#include <math.h>
#include "animator.h"   // includes application-specific information

char FPUZERO[8];

void DrawToolKey(HDC hdc, int i, BOOL press)
{
   int x1=ToolKey[i].x; int y1=ToolKey[i].y; 

   HPEN hpenOld;

   if( press ) { 
      hpenOld = (HPEN)SelectObject(hdc,g_hpen7);
	   DrawBar(hdc,x1+1,y1+1,x1+31,y1+31);
       DrawKey(hdc, x1+1,y1+1,x1+31,y1+31,g_hpen2,g_hpenA); 
	   DrawIcon(hdc, x1+1,y1+1,ToolKey[i].bmp);
   } else {
     hpenOld = (HPEN)SelectObject(hdc,g_hpen9);
	  DrawBar(hdc,x1+1,y1+1,x1+31,y1+31);
	  DrawKey(hdc, x1+1,y1+1,x1+31,y1+31,g_hpen3dUp,g_hpen3dDn);
	  DrawIcon(hdc, x1,y1,ToolKey[i].bmp);
	}

   SelectObject(hdc,g_hpen1);
   DrawRect(hdc, x1,y1,x1+31,y1+31);

   SelectObject( hdc, hpenOld );
}

void DrawTollBox (HWND hWnd, HDC hdc)
{ 
        RECT rc;
        int x2;	int y2; 
	   
        GetClientRect    (hWnd, &rc);
	    x2=rc.right-rc.left-1;
	    y2=rc.bottom-rc.top-1;
	    
        HPEN hpenOld = (HPEN)SelectObject(hdc,g_hpen8);		

        Draw3dKey(hdc,0,0,x2-1,284,2,g_hpen3dDn,0,g_hpen3dUp);
        //Draw3dKey(hdc,0,208,x2-1,284,2,g_hpen3dDn,0,g_hpen3dUp);
        Draw3dKey(hdc,0,286,x2-1,356,2,g_hpen3dDn,0,g_hpen3dUp);
        Draw3dKey(hdc,0,358,x2-1,y2-1,2,g_hpen3dDn,0,g_hpen3dUp);
		
		for (int i=0; i<ToolKeyCnt; i++)
			DrawToolKey(hdc, i, (ToolKey[i].cmd==CurTool) ||  (ToolKey[i].cmd==CurCommand) );
        
		SelectObject(hdc,hpenOld);
}


int GetKeyOnCommand(int cmd)
{
	for (int i=0; i<ToolKeyCnt; i++)
		if (ToolKey[i].cmd==cmd) return i;
	return -1;
}

void ProcessAcceleration()
{   
  int laf = CurFrame;
  while (!Track[laf][CurObject].active) laf--;
  if (Track[laf][CurObject].acceleration<-32) Track[laf][CurObject].acceleration = -32;
  if (Track[laf][CurObject].acceleration> 32) Track[laf][CurObject].acceleration =  32;
  int a = Track[laf][CurObject].acceleration;
  char buf[12];
  wsprintf(buf," %d",a);
  SetWindowText(hwndAText,buf);
  SetScrollPos(hwndAcceleration, SB_CTL, a, TRUE);
}


void PasteChild(int papa)
{  
  for (int n=0; n<OCount; n++) 
   if (gObj[n].owner==papa) {
    if (ClipboardMode==cbmKey) Track[CurFrame][n] = ClipboardFrame[n]; 
	else { Track[CurFrame][n] = ClipboardFrame[CopyLinksCount]; CopyLinksCount++;}
    PasteChild(n);
  }
}


void CopyChild(int papa)
{
  for (int n=0; n<OCount; n++) 
   if (gObj[n].owner==papa) {
    ClipboardFrame[CopyLinksCount] = Track[CurFrame][n];
	CopyLinksCount++;
    CopyChild(n);
  }
}

char SwapName[32];
TKey SwapKey;

void LRSwapChild(int papa)
{  
  for (int n=0; n<OCount; n++) 
   if (gObj[n].owner==papa) {
    strcpy(SwapName, gObj[n].OName);
	
	if (SwapName[0] == 'l' || SwapName[0] == 'r') {
	 
    if (SwapName[0] == 'l') 
		   SwapName[0] = 'r'; 
	  else SwapName[0] = 'l';
     for (int i=0; i<OCount; i++) 
	  if (!strcmp(SwapName, gObj[i].OName)) {
        SwapKey = Track[CurFrame][n];
        Track[CurFrame][n] = Track[CurFrame][i];
		Track[CurFrame][i] = SwapKey;

		Track[CurFrame][n].Yfi*=-1;
		Track[CurFrame][n].Zfi*=-1;
		Track[CurFrame][i].Yfi*=-1;
		Track[CurFrame][i].Zfi*=-1;
		break;
	  }
    }
       
    LRSwapChild(n);
  }
}




void SetClipboardMode(int mode)
{
  ClipboardMode = mode;
  switch (mode) {
   case cbmKey:
    EnableWindow(hwndPasteKey,TRUE);
	EnableWindow(hwndPasteU,TRUE);
	EnableWindow(hwndPasteL,TRUE);
	EnableWindow(hwndPasteO,TRUE);
	break;
   case cbmLinks:
	EnableWindow(hwndPasteKey,FALSE);
	EnableWindow(hwndPasteU,FALSE);
	EnableWindow(hwndPasteL,TRUE);
	EnableWindow(hwndPasteO,FALSE);
	break;
   case cbmObject   :
	EnableWindow(hwndPasteKey,FALSE);
	EnableWindow(hwndPasteU,FALSE);
	EnableWindow(hwndPasteL,FALSE);
	EnableWindow(hwndPasteO,TRUE);
    break;
  }
}

void SwitchDamage()
{  
  DamageMode = 0;
  if (SendMessage(hwndDamage1,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |= 1;
  if (SendMessage(hwndDamage2,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |= 2;
  if (SendMessage(hwndDamage3,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |= 4;
  if (SendMessage(hwndDamage4,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |= 8;

  if (SendMessage(hwndDamage5,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |=16;
  if (SendMessage(hwndDamage6,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |=32;
  if (SendMessage(hwndDamage7,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |=64;
  if (SendMessage(hwndDamage8,  BM_GETSTATE, 0,0) & BST_CHECKED) DamageMode |=128;

  DamageMask = 0xAAAA0000;

  for (int d=0; d<8; d++)
      if (DamageMode & (1<<d)) {
          DamageMask = DamageMask ^ (3<<(16+d*2));
	  }      
}

LRESULT APIENTRY ToolWndProc( HWND hWnd, UINT message, UINT wParam, LONG lParam)
{	
	HDC hdc;
	PAINTSTRUCT ps;

    switch (message) {               
	    case WM_LBUTTONDOWN : {
			int mx = LOWORD(lParam);
			int my = HIWORD(lParam);
            int ot = CurTool;

			for (int i=0; i<ToolKeyCnt; i++) {
				if( mx >= ToolKey[i].x && mx <= ToolKey[i].x+31 &&
                my >= ToolKey[i].y && my <= ToolKey[i].y+31 )
				   if( ToolKey[i].IsTool ) 
                  CurTool=ToolKey[i].cmd;
				   else if (CurCommand!=ToolKey[i].cmd) { 
					  CurCommand=ToolKey[i].cmd;
					  PostMessage(g_MWin,WM_USER,0,0);
				   }
			}

			if (ot!=CurTool) {
				 RECT rc1; 
				 int i=GetKeyOnCommand(ot);
				 rc1.left=ToolKey[i].x;  rc1.top=ToolKey[i].y;
				 rc1.right=rc1.left+31;  rc1.bottom=rc1.top+31;
				 InvalidateRect(hWnd,&rc1,FALSE);
      
                 i=GetKeyOnCommand(CurTool);
				 rc1.left=ToolKey[i].x;  rc1.top=ToolKey[i].y;
				 rc1.right=rc1.left+31;  rc1.bottom=rc1.top+31;
				 InvalidateRect(hWnd,&rc1,FALSE);				 
			 }
         
			return 0;
			}

        case WM_PAINT:             // message: command from application menu 
	    	hdc=BeginPaint(hWnd, &ps );
            DrawTollBox(hWnd, hdc);
			EndPaint( hWnd, &ps );
			return 0;

        case WM_COMMAND: 
            if ((HWND)lParam == hwnd3dMode) {
				int state = SendMessage(hwnd3dMode,BM_GETCHECK, 0, 0);
				mode3d = (state==BST_CHECKED) ? 1:0;
			}

            if ((HWND)lParam == hwndM1) {
				int state = SendMessage(hwndM1,BM_GETCHECK, 0, 0);
				ShiftMode = (state==BST_CHECKED) ? 1:0;
			}
            if ((HWND)lParam == hwndM2) {
				int state = SendMessage(hwndM2,BM_GETCHECK, 0, 0);
				CtrlMode = (state==BST_CHECKED) ? 1:0;
			}


			
			if (HIWORD(wParam) == BN_CLICKED) {
            int o;
            if ((HWND)lParam == hwndLoopPlay)             
              LoopPlay  = SendMessage(hwndLoopPlay, BM_GETCHECK, 0, 0);
            if ((HWND)lParam == hwndWorld)
              WorldMode = SendMessage(hwndWorld   , BM_GETCHECK, 0, 0);

	        if ((HWND)lParam == hwndLRSwap) {			   
               int s = gObj[CurObject].owner;
               gObj[CurObject].owner = 255;
               LRSwapChild(255);
               gObj[CurObject].owner = s;			   
            }

            if ((HWND)lParam == hwndCopyKey) {
               for (o=0; o<OCount; o++) {
                 ClipboardFrame[o] = FrameAni[o];
                 ClipboardFrame[o].active = TRUE;				 
               }
			   SetClipboardMode(cbmKey);
			}

			if ((HWND)lParam == hwndCopyKey)
               for (o=0; o<OCount; o++) {
                 ClipboardFrame[o] = FrameAni[o];
                 ClipboardFrame[o].active = TRUE;
				 SetClipboardMode(cbmKey);
               }
			
			if ((HWND)lParam == hwndCopyL) {
			   CopyLinksCount = 0;
               SetClipboardMode(cbmLinks);
               int s = gObj[CurObject].owner;
               gObj[CurObject].owner = 255;
               CopyChild(255);
               gObj[CurObject].owner = s;
            }


			if ((HWND)lParam == hwndCopyO) {
                 ClipboardFrame[0] = FrameAni[CurObject];
                 ClipboardFrame[0].active = TRUE;
				 SetClipboardMode(cbmObject);
               }			

            if ((HWND)lParam == hwndLod1)  SetLod(0);
            if ((HWND)lParam == hwndLod2)  SetLod(1);
            if ((HWND)lParam == hwndLod3)  SetLod(2);
            if ((HWND)lParam == hwndLod4)  SetLod(3);

            if ((HWND)lParam == hwndDamage1)  SwitchDamage();
            if ((HWND)lParam == hwndDamage2)  SwitchDamage();
            if ((HWND)lParam == hwndDamage3)  SwitchDamage();
            if ((HWND)lParam == hwndDamage4)  SwitchDamage();
            if ((HWND)lParam == hwndDamage5)  SwitchDamage();
            if ((HWND)lParam == hwndDamage6)  SwitchDamage();
            if ((HWND)lParam == hwndDamage7)  SwitchDamage();
            if ((HWND)lParam == hwndDamage8)  SwitchDamage();
                
			if ((HWND)lParam == hwndPasteKey)
               for (o=0; o<OCount; o++) 
                Track[CurFrame][o] = ClipboardFrame[o];

            if ((HWND)lParam == hwndPasteU)
               for (o=0; o<OCount; o++) 
                if (Track[CurFrame][o].active)
                 Track[CurFrame][o] = ClipboardFrame[o];

            if ((HWND)lParam == hwndPasteL) {
			   CopyLinksCount = 0;
               int s = gObj[CurObject].owner;
               gObj[CurObject].owner = 255;
               PasteChild(255);
               gObj[CurObject].owner = s;
            }

            if ((HWND)lParam == hwndPasteO)                             
			  if (ClipboardMode==cbmKey)
               Track[CurFrame][CurObject] = ClipboardFrame[CurObject];
			  else
			   Track[CurFrame][CurObject] = ClipboardFrame[0];

            SetFocus(g_MWin);                            
            RefreshFrame();              
            }
            return 0;

        case WM_HSCROLL:
         if (lParam == (int)hwndAcceleration) {
            int laf = CurFrame;
            int d = 0;
            while (!Track[laf][CurObject].active) laf--;
            switch ( (int) LOWORD(wParam) ) {        
              case SB_PAGERIGHT: d+=3;
              case SB_LINERIGHT: d+=1;                
                Track[laf][CurObject].acceleration+=d;
                ProcessAcceleration();

                if (WorldMode) 
                 for (int oo=0; oo<OCount; oo++) 
                  if (Track[laf][oo].active) 
                   Track[laf][oo].acceleration=Track[laf][CurObject].acceleration;

                RefreshFrame();
                break;
         
              case SB_PAGELEFT : d-=3;
              case SB_LINELEFT : d-=1;                                  
                Track[laf][CurObject].acceleration+=d;
                ProcessAcceleration();
                
                if (WorldMode) 
                 for (int oo=0; oo<OCount; oo++) 
                  if (Track[laf][oo].active) 
                   Track[laf][oo].acceleration=Track[laf][CurObject].acceleration;

                RefreshFrame();
                break;
         
              case SB_THUMBPOSITION:
              case SB_THUMBTRACK   :                               
                Track[laf][CurObject].acceleration = (short int) HIWORD(wParam);                

                if (WorldMode) 
                 for (int oo=0; oo<OCount; oo++) 
                  if (Track[laf][oo].active) 
                   Track[laf][oo].acceleration=Track[laf][CurObject].acceleration;

                ProcessAcceleration();
                RefreshFrame();
               break;  
             }             
         }
        return 0;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}

void FPUstosq(LPVOID lpDest, int Count, WORD W)
{
     __asm {
     mov    edi,lpDest
     mov    ax,W
     shl      eax,16
     mov    ax,W
     mov    dword ptr [offset FPUZERO],eax
     mov    dword ptr [offset FPUZERO+4],eax
     fld       qword ptr [offset FPUZERO]
     mov    ecx,Count     
     shr      ecx,3 }
LFPU: __asm   {
     fst       qword ptr [edi]
     add     edi,8
     dec     ecx
      jnz      LFPU
      finit      
      mov    ecx,Count
      and     ecx,7
      shr      ecx,1
      jz LEND
      rep stosw }

LEND: ;
}

void FPUmovsq(LPVOID lpDest, LPVOID lpSrc, int Count)
{
     __asm {
     mov    edi,lpDest
     mov    esi,lpSrc          
     mov    ecx,Count     
     shr      ecx,3 }
LFPU: __asm   {
     fld       qword ptr [esi]               
     fstp     qword ptr [edi]     
     add     esi,8
     add     edi,8
     dec      ecx
      jnz      LFPU

      finit      
      mov    ecx,Count
      and     ecx,7
      jz LEND
      rep movsb }
LEND: ;
}

void AddToolKey(int x, int y, HICON bmp, int cmd, int IsTool)
{
	ToolKey[ToolKeyCnt].bmp=bmp;
	ToolKey[ToolKeyCnt].x=x+4;
	ToolKey[ToolKeyCnt].y=y+4;
	ToolKey[ToolKeyCnt].cmd=cmd;
	ToolKey[ToolKeyCnt].IsTool=IsTool;
	ToolKeyCnt++;
}

void ConvertPointBack(int sx, int sy, float& x, float& y, float& z )
{
	x = 0.0f; y = 0.0f; z = 0.0f;
	float xc     = (float)g_3dwininfo[ g_ew ]._xc;
	float yc     = (float)g_3dwininfo[ g_ew ]._yc;
    float xc0    = g_3dwininfo[ g_ew ]._xc0;
	float yc0    = g_3dwininfo[ g_ew ]._yc0;
	float scaler = g_3dwininfo[ g_ew ].Scaler;
 	float grider = (g_3dwininfo[ g_ew ].GridStep>0 && g_3dwininfo[ g_ew ].MouseAlign) ? g_3dwininfo[ g_ew ].GridStep : (float)0.01;

	switch( g_ew ) {
	case 0:
		if (RevView) x = +(xc - sx)/scaler - xc0;
                else x = -(xc - sx)/scaler + xc0;
        y =  (yc - sy)/ scaler + yc0;
        z = bp.z;
		break;    
    case 1:
        x = bp.x;
        if (RevView) z = +(xc - sx)/scaler - xc0;
                else z = -(xc - sx)/scaler + xc0;
        y = (yc - sy)/ scaler + yc0;        
		break;    
    case 2:
        if (RevView) x = +(xc - sx)/scaler - xc0;              
                else x = -(xc - sx)/scaler + xc0;              
        y = bp.y;
        if (RevView) z = +(yc - sy)/ scaler + yc0;
                else z = -(yc - sy)/ scaler - yc0;
		break;    
	}    

	x =  (float)floor (x / grider + 0.5f ) * grider;
	y =  (float)floor (y / grider + 0.5f ) * grider;
	z =  (float)floor (z / grider + 0.5f ) * grider;
}


void SetWindowTitle(LPSTR s)
{
/*   
   sprintf(szWinTitle,"IKAnimator    %s - %s - %s  OLD(%4d,%4d,%4d),DX = %f Ang(%4d,%4d,%4d),  Ang1(%4d,%4d,%4d), Ang2(%4d,%4d,%4d),  a = %d  , absWa %1.3f  %s", ModelName, TrackName, gObj[CurObject].OName,
			(int)(Fi0[0]*180.f/pi), (int)(Fi0[1]*180.f/pi), (int)(Fi0[2]*180.f/pi), dXfi[0]*180.0/pi,
			(int)Track[CurFrame][33].Xfi,
			(int)Track[CurFrame][33].Yfi, //CurObject
			(int)Track[CurFrame][33].Zfi,
			(int)(Fi1[0]*180.f/pi), (int)(Fi1[1]*180.f/pi), (int)(Fi1[2]*180.f/pi),
			(int)(Fi2[0]*180.f/pi), (int)(Fi2[1]*180.f/pi), (int)(Fi2[2]*180.f/pi),
			algoritm,
			VectAbs(LInfo[0].W), 
			s);
*/
//   sprintf(szWinTitle,"Animator  ( a1=%1.5f,   a2=%1.5f algoritm = %d)  %s - %s - %s     %s  ",a1, a2,algoritm, ModelName, TrackName, gObj[CurObject].OName,  s);

   wsprintf(szWinTitle,"Animator    %s - %s - %s - %s     %s", ModelName, TrackName, sfxName, gObj[CurObject].OName, s);
   SetWindowText(g_MWin, szWinTitle);

	//if (CurObject>=0 && n==0) 
	//if (GetAsyncKeyState(VK_SPACE ) & 0x8000) 
	{
	 //MessageBeep(-1);
	}

}

void SetCurPosInfo(int x, int y)
{
   char t[128];
	float xf; float yf; float zf;
	ConvertPointBack(x,y, xf, yf, zf);

	switch (g_ew) {
      case 0: case 1 : case 2 :
         sprintf(t,"X=%.2f Y=%.2f Z=%.2f", xf, yf, zf); break;
	   case 3:  
         sprintf( t,"[camera]" );break;
	}

   SetWindowTitle( t );   
}


void ConvertPoint(float x, float y, float z, int& sx, int& sy )
{
	switch( g_ew ) {
	   case 0:
		   if (RevView) sx = g_xc - (int) ( (x + g_3dwininfo[0]._xc0) * g_scaler);
                   else sx = g_xc + (int) ( (x - g_3dwininfo[0]._xc0) * g_scaler);
           sy = g_yc - (int) ( (y - g_3dwininfo[0]._yc0) * g_scaler);
		   return;
      case 1:
		   if (RevView) sx = g_xc - (int) ( (z + g_3dwininfo[1]._xc0) * g_scaler);
           else         sx = g_xc + (int) ( (z - g_3dwininfo[1]._xc0) * g_scaler);
           sy = g_yc - (int) ( (y - g_3dwininfo[1]._yc0) * g_scaler);
		   return;
      case 2:
           if (RevView) sx = g_xc - (int) ( (x + g_3dwininfo[2]._xc0) * g_scaler);
                   else sx = g_xc + (int) ( (x - g_3dwininfo[2]._xc0) * g_scaler);
           if (RevView) sy = g_yc - (int) ( (z - g_3dwininfo[2]._yc0) * g_scaler);
                   else sy = g_yc + (int) ( (z + g_3dwininfo[2]._yc0) * g_scaler);
		   return;
      case 3: {
          float x1; float y1; float z1;
	      float x2; float y2; float z2;          

		   x1 = x * g_cosa - z * g_sina;
		   y1 = y;
           z1 = z * g_cosa + x * g_sina;

           x2 = x1;     
	 	   y2 = y1 * g_cosb + z1 * g_sinb;
           z2 = z1 * g_cosb - y1 * g_sinb;
                
		   sx = g_xc + (int) ( (x2 - g_3dwininfo[3]._xc0) * g_scaler);
           sy = g_yc - (int) ( (y2 - g_3dwininfo[3]._yc0) * g_scaler);
		   return;
		}
	}
}

void ConvertPointForCurrent( float x, float y, float z, int& sx, int& sy )
{
   int G_XC = (g_3dwininfo[CurEditor].DefPos.right - g_3dwininfo[CurEditor].DefPos.left) >> 1;
   int G_YC = (g_3dwininfo[CurEditor].DefPos.bottom - g_3dwininfo[CurEditor].DefPos.top) >> 1;
   float G_SCALER = g_3dwininfo[CurEditor].Scaler;

	switch( CurEditor ) {
	   case 0:
		   if (RevView) sx = G_XC - (int) ( (x + g_3dwininfo[0]._xc0) * G_SCALER );
                   else sx = G_XC + (int) ( (x - g_3dwininfo[0]._xc0) * G_SCALER );
           sy = G_YC - (int) ( (y - g_3dwininfo[0]._yc0) * G_SCALER );
		   return;

      case 1:
           if (RevView) sx = G_XC - (int) ( (z + g_3dwininfo[1]._xc0) * G_SCALER );
                   else sx = G_XC + (int) ( (z - g_3dwininfo[1]._xc0) * G_SCALER );
           sy = G_YC - (int) ( (y - g_3dwininfo[1]._yc0) * G_SCALER );
		   return;

      case 2:
           if (RevView) sx = G_XC - (int) ( (x + g_3dwininfo[2]._xc0) * G_SCALER );
                   else sx = G_XC + (int) ( (x - g_3dwininfo[2]._xc0) * G_SCALER );
           if (RevView) sy = G_YC - (int) ( (z - g_3dwininfo[2]._yc0) * G_SCALER );
                   else sy = G_YC + (int) ( (z + g_3dwininfo[2]._yc0) * G_SCALER );
	 	   return;

      case 3: {
          float x1; float y1; float z1;
	      float x2; float y2; float z2;
          

		   x1 = x * g_cosa - z * g_sina;
           z1 = z * g_cosa + x * g_sina;
		   y1 = y;

           x2 = x1;     
           z2 = z1 * g_cosb - y1 * g_sinb;
		   y2 = y1 * g_cosb + z1 * g_sinb;

   	       sx = G_XC + (int) ( (x2 - g_3dwininfo[3]._xc0) * G_SCALER );
           sy = G_YC - (int) ( (y2 - g_3dwininfo[3]._yc0) * G_SCALER );
		   return;
		}
	}
}

void CalcRotatedModel()
{
   float x1, y1, z1;   
   int xx, yy;
		
	for( int f=0; f<VCount; f++ ) {
      float z = gVertex[f].z;
      
      x1 = gVertex[f].x * g_cosa - z * g_sina;
      z1 = z * g_cosa + gVertex[f].x * g_sina;
      y1 = gVertex[f].y;

      rVertex[f].x = x1;
      rVertex[f].z = z1 * g_cosb - y1 * g_sinb;
	  rVertex[f].y = y1 * g_cosb + z1 * g_sinb;
        
      xx = g_xc + (int) ( (rVertex[f].x - g_3dwininfo[3]._xc0) * g_scaler);
      yy = g_yc - (int) ( (rVertex[f].y - g_3dwininfo[3]._yc0) * g_scaler);        
		
      scrc[f].x = xx; scrc[f].y = yy;      
	}   
}

void BuildTree( )
{
   POINT v[3];
	Current = -1;
	for (int f=0; f<FCount; f++) {
      int v1 = gFace[f].v1;
      int v2 = gFace[f].v2;
      int v3 = gFace[f].v3;

      if( gVertex[v1].hide || gVertex[v2].hide || gVertex[v3].hide )
         continue;

		v[0] = scrc[v1]; 
      v[1] = scrc[v2]; 
      v[2] = scrc[v3];

      if( (gFace[f].Flags & 0x0001) ||
         (v[1].x-v[0].x)*(v[2].y-v[1].y) - (v[1].y-v[0].y)*(v[2].x-v[1].x)>=0 ) {
			
         gFace[f].Distant = -(int)(rVertex[v1].z + rVertex[v2].z + rVertex[v3].z);
	     gFace[f].Next=-1;
		 if( Current == -1 )
            Current=f;
         else if( gFace[Current].Distant<gFace[f].Distant ) { 
            gFace[f].Next=Current; Current=f; 
         } else {
				int n=Current;
			   while( gFace[n].Next!=-1 && gFace[gFace[n].Next].Distant>gFace[f].Distant )
               n=gFace[n].Next;
   
            gFace[f].Next = gFace[n].Next;
   	      gFace[n].Next = f;
         }
		}
	}
}
/*
void BuildBackTree()
{
   POINT v[3];
	Current = -1;
	for( int f=0; f<FCount; f++ ) {
      int v1 = gFace[f].v1;
      int v2 = gFace[f].v2;
      int v3 = gFace[f].v3;        
		v[0] = scrc[v1];
      v[1] = scrc[v2];
      v[2] = scrc[v3];
      
      if( (gFace[f].Flags & 0x0008) == 0 )
         if( (gFace[f].Flags & 0x0001) ||   
            (v[1].x-v[0].x)*(v[2].y-v[1].y) - (v[1].y-v[0].y)*(v[2].x-v[1].x)>=0 ) {

			   gFace[f].Distant = -(int)(rVertex[v1].z + rVertex[v2].z + rVertex[v3].z);
			   gFace[f].Next=-1;
			   if( Current == -1 ) 
               Current=f;
            else if( gFace[Current].Distant>gFace[f].Distant ) { 
               gFace[f].Next=Current; Current=f;
            } else {
				   int n=Current;
			      while( gFace[n].Next!=-1 && gFace[gFace[n].Next].Distant<gFace[f].Distant )
			      	 n=gFace[n].Next;

               gFace[f].Next = gFace[n].Next;
			      gFace[n].Next = f;	
            }
		   }
	}
}

void BuildTransTree()
{
   POINT v[3];
	Current = -1;
	for( int f=0; f<FCount; f++ ) {
      int v1 = gFace[f].v1;
      int v2 = gFace[f].v2;
      int v3 = gFace[f].v3;        
		v[0] = scrc[v1];
      v[1] = scrc[v2];
      v[2] = scrc[v3];
      
      if( gFace[f].Flags & 0x0008 )
         if( (gFace[f].Flags & 0x0001) ||
             (v[1].x-v[0].x)*(v[2].y-v[1].y) - (v[1].y-v[0].y)*(v[2].x-v[1].x)>=0 ) {
			
            gFace[f].Distant = -(int)(rVertex[v1].z + rVertex[v2].z + rVertex[v3].z);
			   gFace[f].Next=-1;
			   if (Current==-1) 
               Current=f; 
            else if( gFace[Current].Distant<gFace[f].Distant ) {
               gFace[f].Next=Current; Current=f; 
            } else {
				   int n=Current;
			      while( gFace[n].Next!=-1 && gFace[gFace[n].Next].Distant>gFace[f].Distant )
			         n=gFace[n].Next;

               gFace[f].Next = gFace[n].Next;
			      gFace[n].Next = f;	
            }
		   }
	}
}
*/
void CenterWindow( HWND hwndChild, HWND hwndParent )
{
   RECT rcChild, rcParent;
   GetWindowRect( hwndChild, &rcChild );
   GetWindowRect( hwndParent, &rcParent );
   SetWindowPos( hwndChild, 0,
        (rcParent.right - rcParent.left - rcChild.right + rcChild.left ) / 2,
        (rcParent.bottom - rcParent.top - rcChild.bottom + rcChild.top ) / 2,
      0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW
   );
}

void ConvertDifference( int dsx, int dsy, float& dx, float& dy, float& dz )
{
   float scaler = g_3dwininfo[CurEditor].Scaler;

   switch( CurEditor ) {
      case 0 :
         dx = dsx/scaler;
         dy = dsy/scaler;
         dz = 0.0f;
         return;

      case 1 :
         dx = 0.0f;
         dy = dsy/scaler;
         dz = -dsx/scaler;
         return;

      case 2 :
         dx = dsx/scaler;
         dy = 0.0f;
         dz = dsy/scaler;
         return;
   }
}


#pragma warning( disable : 4244 )
int CalibrateImageForCamera( void )
{
	for( int v = 0; v < VCount; v++ ) {
      float x1 = gVertex[v].x * g_cosa - gVertex[v].z * g_sina;
      float z1 = gVertex[v].z * g_cosa + gVertex[v].x * g_sina;
		float y1 = gVertex[v].y;

      rVertex[v].x = x1;
      rVertex[v].z = z1 * g_cosb - y1 * g_sinb;
	  rVertex[v].y = y1 * g_cosb + z1 * g_sinb;
	}

   float xMax = rVertex[0].x, xMin = rVertex[0].x, yMax = rVertex[0].y, yMin = rVertex[0].y;
   int ixMax = -1, ixMin = -1, iyMax = -1, iyMin = -1;

   for( v = 1; v < VCount; v++ ) {
      if( rVertex[v].x > xMax ) {
         xMax = rVertex[v].x;
         ixMax = v;
      } else if( rVertex[v].x < xMin ) {
         xMin = rVertex[v].x;
         ixMin = v;
      }

      if( rVertex[v].y > yMax ) {
         yMax = rVertex[v].y;
         iyMax = v;
      } else if( rVertex[v].y < yMin ) {
         yMin = rVertex[v].y;
         iyMin = v;
      }
   }

   float xScale = (float)(g_3dwininfo[3].DefPos.right-g_3dwininfo[3].DefPos.left-10) / (float)( xMax-xMin ) / 1.2;
   float yScale = (float)(g_3dwininfo[3].DefPos.bottom-g_3dwininfo[3].DefPos.top-10) / (float)( yMax-yMin ) / 1.2;

   g_3dwininfo[3].Scaler = ( xScale<yScale ) ? xScale : yScale;

   g_3dwininfo[3]._xc0 = (xMax+xMin) / 2.0f;
   g_3dwininfo[3]._yc0 = (yMax+yMin) / 2.0f;

   return 1;
}

#pragma warning( disable : 4244 )
int CalibrateImage( int win )
{
   if( win == 3 )
      return CalibrateImageForCamera( );

   int tmpCurrent = CurEditor;
   CurEditor = win;

   float xMin, yMin, zMin;
   float xMax, yMax, zMax;
   int ixMax, iyMax, izMax;
   int ixMin, iyMin, izMin;

   switch( win ) {
      case 0 : {
         xMax = gVertex[0].x, xMin = gVertex[0].x;
         yMax = gVertex[0].y, yMin = gVertex[0].y;

         for( int i = 1; i < VCount; i++ ) {
            if( gVertex[i].x > xMax ) {
               xMax = gVertex[i].x;
               ixMax = i;
            } else if( gVertex[i].x < xMin ) {
               xMin = gVertex[i].x;
               ixMin = i;
            }

            if( gVertex[i].y > yMax ) {
               yMax = gVertex[i].y;
               iyMax = i;
            } else if( gVertex[i].y < yMin ) {
               yMin = gVertex[i].y;
               iyMin = i;
            }
         }
         float xScale = (float)(g_3dwininfo[0].DefPos.right-g_3dwininfo[0].DefPos.left-10) / (float)( xMax-xMin ) / 1.2;
         float yScale = (float)(g_3dwininfo[0].DefPos.bottom-g_3dwininfo[0].DefPos.top-10) / (float)( yMax-yMin ) / 1.2;

         g_3dwininfo[0].Scaler = ( xScale<yScale ) ? xScale : yScale;
         g_3dwininfo[0]._xc0 = (xMax+xMin) / 2.0f;
         g_3dwininfo[0]._yc0 = (yMax+yMin) / 2.0f;
         if (RevView) g_3dwininfo[0]._xc0*=-1;

         return 1;
               }
      case 1 :{
         zMax = gVertex[0].z, zMin = gVertex[0].z;
         yMax = gVertex[0].y, yMin = gVertex[0].y;

         for( int i = 1; i < VCount; i++ ) {
            if( gVertex[i].z > zMax ) {
               zMax = gVertex[i].z;
               izMax = i;
            } else if( gVertex[i].z < zMin ) {
               zMin = gVertex[i].z;
               izMin = i;
            }

            if( gVertex[i].y > yMax ) {
               yMax = gVertex[i].y;
               iyMax = i;
            } else if( gVertex[i].y < yMin ) {
               yMin = gVertex[i].y;
               iyMin = i;
            }
         }
         float zScale = (float)(g_3dwininfo[1].DefPos.right-g_3dwininfo[1].DefPos.left-10) / (float)( zMax-zMin ) / 1.2;
         float yScale = (float)(g_3dwininfo[1].DefPos.bottom-g_3dwininfo[1].DefPos.top-10) / (float)( yMax-yMin ) / 1.2;

         g_3dwininfo[1].Scaler = ( zScale<yScale ) ? zScale : yScale;
         g_3dwininfo[1]._xc0 = (zMax+zMin) / 2.0f;
         g_3dwininfo[1]._yc0 = (yMax+yMin) / 2.0f;
         if (RevView) g_3dwininfo[1]._xc0*=-1;

         return 1;
               }
      case 2 :{
         xMax = gVertex[0].x, xMin = gVertex[0].x;
         zMax = gVertex[0].z, yMin = gVertex[0].z;

         for( int i = 1; i < VCount; i++ ) {
            if( gVertex[i].x > xMax ) {
               xMax = gVertex[i].x;
               ixMax = i;
            } else if( gVertex[i].x < xMin ) {
               xMin = gVertex[i].x;
               ixMin = i;
            }

            if( gVertex[i].z > zMax ) {
               zMax = gVertex[i].z;
               izMax = i;
            } else if( gVertex[i].z < zMin ) {
               zMin = gVertex[i].z;
               izMin = i;
            }
         }
         float xScale = (float)(g_3dwininfo[2].DefPos.right-g_3dwininfo[2].DefPos.left-10) / (float)( xMax-xMin ) / 1.2;
         float zScale = (float)(g_3dwininfo[2].DefPos.bottom-g_3dwininfo[2].DefPos.top-10) / (float)( zMax-zMin ) / 1.2;

         g_3dwininfo[2].Scaler = ( xScale<zScale ) ? xScale : zScale;
         g_3dwininfo[2]._xc0 = (xMax+xMin) / 2.0f;
         g_3dwininfo[2]._yc0 = -(zMax+zMin) / 2.0f;
         if (RevView) g_3dwininfo[2]._xc0*=-1;
         if (RevView) g_3dwininfo[2]._yc0*=-1;
         return 1;
               }
   }

   CurEditor = tmpCurrent;
   return 1;
}





void ConvertToScreen( int& sx, int& sy )
{
   sx += g_3dwininfo[CurEditor].DefPos.left;
   sy += g_3dwininfo[CurEditor].DefPos.top;
}

void FromMainToEditorCoords( int& sx, int& sy )
{
   sy -= g_3dwininfo[CurEditor].DefPos.top;
   sx -= g_3dwininfo[CurEditor].DefPos.left;
}

void AdjustMouseCoords( POINT& p )
{
   POINT pm;
   GetCursorPos( &pm );

   int iPossibleCur = -1;

   if( pm.x > g_3dwininfo[ CurEditor ].DefPos.left &&
       pm.x < g_3dwininfo[ CurEditor ].DefPos.right &&
       pm.y > g_3dwininfo[ CurEditor ].DefPos.top &&
       pm.y < g_3dwininfo[ CurEditor ].DefPos.bottom )

       iPossibleCur = CurEditor;
   else {
      for( int i = 0; i < 4; i++ )
         if( pm.x > g_3dwininfo[i].DefPos.left &&
             pm.x < g_3dwininfo[i].DefPos.right &&
             pm.y > g_3dwininfo[i].DefPos.top &&
             pm.y < g_3dwininfo[i].DefPos.bottom ) {

            iPossibleCur = i;
            break;
         }
   }

   if (iPossibleCur!=-1) CurEditor = iPossibleCur;
   p.x = pm.x - g_3dwininfo[ CurEditor ].DefPos.left;
   p.y = pm.y - g_3dwininfo[ CurEditor ].DefPos.top;
}



void HideObject(int o)
{
  gObj[o].hide = 1;
  oObj[o].hide = 1;

  for (int v=0; v<VCount; v++) 
   if (gVertex[v].owner == o) {
     gVertex[v].hide = TRUE;
     oVertex[v].hide = TRUE;
   }
  RefreshFrame();
}

void ShowAll()
{
   for (int o=0; o<128; o++) {
     gObj[o].hide = 0;
     oObj[o].hide = 0;
   }

   for (int v=0; v<VCount; v++) {
     gVertex[v].hide = FALSE;
     oVertex[v].hide = FALSE;
   }
  RefreshFrame();
}
