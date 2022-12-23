#include <windows.h>    // includes basic windows functionality
#include <stdio.h>
#include <math.h>
#include "animator.h"   // includes application-specific information



void UpdateEditorWindow(int w)
{
  int static firsttime=1;
  if (firsttime) {InitVars();firsttime=0;}

  if( w == -1 )
	   for( int i = 0; i < 4; i++ ) 
         DrawEditorWindow( dc->hwnd[i], dc->hdc[i], dc->hdcComp[i] );
  else 
    DrawEditorWindow( dc->hwnd[w], dc->hdc[w], dc->hdcComp[w] );

  //RenderCircle(x,y, 3, color);
}



void ReLocateWindows()
{     
   int w = GetSystemMetrics(SM_CXMAXIMIZED)-8;
   int h = GetSystemMetrics(SM_CYMAXIMIZED) - GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYMENU)-6;
   //if (w>1024) w = 1024;
   //if (h>740) h=740;

   eww = (w - 110) / 2;
   ewh = (h - 27) / 2;

   ezw = w-110;
   ezh = h-27;

   DoZoom();
   DoZoom();

   MoveWindow(g_ToolWin, w-106, 23, 104, h-30, TRUE);

   trkww = ((w-210) / 10)*10;
   MoveWindow(hwndKeyBar, 4, 1, trkww+2, 18, TRUE);

   MoveWindow(hwndKeyScrl, w-202, 1, 92, 18, TRUE);
   SetScrollRange(hwndKeyScrl,SB_CTL,0,MAXKEY*10-trkww, TRUE);

   MoveWindow(hwndKeyText, w-106, 1, 104, 18, TRUE);
   
}


void CreateEditorWindows( void )
{
   int i, x, y;

   // w=800, h=565
   int w = GetSystemMetrics(SM_CXMAXIMIZED)-8;
   int h = GetSystemMetrics(SM_CYMAXIMIZED) - GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYMENU)-6;
   //if (w>1024) w = 1024;
   //if (h>740) h=740;

   eww = (w - 110) / 2;
   ewh = (h - 27) / 2;

   ezw = w-110;
   ezh = h-27;

   for( i = 0; i < 4; i++ ) {
      x = 3 + (i & 1) * eww; 
      y = 23 + (i/2) * ewh;
      g_3dwin[i] = CreateWindow("3DEditor", NULL,WS_CHILD,
         x, y, eww-3, ewh-3, g_MWin, NULL, g_hInst, NULL);			 
      ShowWindow(g_3dwin[i],SW_SHOW);
      UpdateWindow(g_3dwin[i]);

	   g_3dwininfo[i].DefPos.left=x;
	   g_3dwininfo[i].DefPos.top=y+iCaptionMenuOffset;
	   g_3dwininfo[i].DefPos.right=x+eww-3;
	   g_3dwininfo[i].DefPos.bottom=y+ewh-3+iCaptionMenuOffset;
   }

    //AddToolKey( 8,128, LoadIcon( g_hInst, "ICO_MATRIX" ),    idt_setmatrix,  1 );

    AddToolKey(  8,  8, LoadIcon( g_hInst, "ICO_MOV" ),    idt_move, 1 );
	AddToolKey(  50, 8, LoadIcon( g_hInst, "ICO_IKMOV" ),  idt_ikmove, 1 );

    AddToolKey(  8,48, LoadIcon( g_hInst, "ICO_ROT") ,    idt_rotate , 1 );
    AddToolKey( 50,48, LoadIcon( g_hInst, "ICO_ROL" ),    idt_rotatel, 1 );

    AddToolKey(  8,88, LoadIcon( g_hInst, "ICO_SC2" ),    idt_2dscale,  1 );
    AddToolKey( 50,88, LoadIcon( g_hInst, "ICO_SC3" ),    idt_3dscale,  1 );

	AddToolKey(  8, 128, LoadIcon( g_hInst, "ICO_IKROT" ),  idt_ikrot, 1 );
	AddToolKey(  50, 128, LoadIcon( g_hInst, "ICO_SETSPHERE" ),  idt_setsphere, 1 );

    AddToolKey( 0,  360, LoadIcon(g_hInst, "ICO_0X0"), IDM_ZERO, 0 );
	AddToolKey( 32, 360, LoadIcon(g_hInst, "ICO_ANI"), IDM_ANI,  0 );

    AddToolKey( 0,  392, LoadIcon( g_hInst, "ICO_TEXT" ), idt_textured,  0 );
    AddToolKey( 32, 392, LoadIcon( g_hInst, "ICO_FLAT" ), idt_flat,      0 );
    AddToolKey( 64, 392, LoadIcon( g_hInst, "ICO_WF" ),   idt_wireframe, 0 );

	AddToolKey( 0,  424, LoadIcon(g_hInst, "ICO_ZOOM"), idt_zoom,    0);
	AddToolKey( 32, 424, LoadIcon(g_hInst, "ICO_MAXO"), idt_max1,    0);
    AddToolKey( 64, 424, LoadIcon(g_hInst, "ICO_MAXA"), idt_maxa,    0);
	
    //AddToolKey( 0,  456, LoadIcon(g_hInst, "ICO_IN"),   IDM_ZOOMIN,  0);
	//AddToolKey( 32, 456, LoadIcon(g_hInst, "ICO_OUT"),  IDM_ZOOMOUT, 0);
	//AddToolKey( 64, 456, LoadIcon(g_hInst, "ICO_SCRL"), idt_mov,     1);

    AddToolKey( 0,  456, LoadIcon(g_hInst, "ICO_HOLD"),    idt_hold,    0 );
	AddToolKey( 32, 456, LoadIcon(g_hInst, "ICO_FET"),     idt_fetch,   0 );

	g_ToolWin = CreateWindow("3DTolls", NULL, WS_CHILD | WS_CLIPSIBLINGS,
      w-106, 23, 104, h-30, g_MWin, NULL, g_hInst, NULL);       

	hwnd3dMode = CreateWindow("BUTTON", "3D", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | BS_CENTER, 
      6, 168, 30, 17, g_ToolWin, NULL, g_hInst, NULL);  

	hwndM1 = CreateWindow("BUTTON", "M1", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | BS_CENTER, 
      6+30, 168, 30, 17, g_ToolWin, NULL, g_hInst, NULL);  

	hwndM2 = CreateWindow("BUTTON", "M2", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | BS_CENTER, 
      6+60, 168, 30, 17, g_ToolWin, NULL, g_hInst, NULL);  




	hwndLRSwap = CreateWindow("BUTTON", "l-r swap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, 
      6, 186, 92, 17, g_ToolWin, NULL, g_hInst, NULL);

    hwndCopyKey = CreateWindow("BUTTON", "Copy", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, 
      6, 204, 92, 17, g_ToolWin, NULL, g_hInst, NULL);

    hwndCopyL = CreateWindow("BUTTON", "L", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, 
     37, 222, 30, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndCopyO = CreateWindow("BUTTON", "O", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER, 
     68, 222, 30, 16, g_ToolWin, NULL, g_hInst, NULL);

    hwndPasteKey = CreateWindow("BUTTON", "Paste", WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON | BS_CENTER, 
      6, 244, 92, 17, g_ToolWin, NULL, g_hInst, NULL);
    
    hwndPasteU = CreateWindow("BUTTON", "U", WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON | BS_CENTER, 
      6, 262, 30, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndPasteL = CreateWindow("BUTTON", "L", WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON | BS_CENTER, 
     37, 262, 30, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndPasteO = CreateWindow("BUTTON", "O", WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON | BS_CENTER, 
     68, 262, 30, 16, g_ToolWin, NULL, g_hInst, NULL);

    hwndLod1 = CreateWindow("BUTTON", "1", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | WS_DISABLED, 
      4, 500, 20, 20, g_ToolWin, NULL, g_hInst, NULL);
    hwndLod2 = CreateWindow("BUTTON", "2", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | WS_DISABLED, 
     24, 500, 20, 20, g_ToolWin, NULL, g_hInst, NULL);
    hwndLod3 = CreateWindow("BUTTON", "3", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | WS_DISABLED, 
     44, 500, 20, 20, g_ToolWin, NULL, g_hInst, NULL);
    hwndLod4 = CreateWindow("BUTTON", "4", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | WS_DISABLED, 
     64, 500, 20, 20, g_ToolWin, NULL, g_hInst, NULL);




    hwndDamage1 = CreateWindow("BUTTON", "1", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
      4, 530, 22, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndDamage2 = CreateWindow("BUTTON", "2", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
     26, 530, 22, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndDamage3 = CreateWindow("BUTTON", "3", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
     48, 530, 22, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndDamage4 = CreateWindow("BUTTON", "4", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
     70, 530, 22, 16, g_ToolWin, NULL, g_hInst, NULL);
    
    hwndDamage5 = CreateWindow("BUTTON", "5", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
      4, 546, 22, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndDamage6 = CreateWindow("BUTTON", "6", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
     26, 546, 22, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndDamage7 = CreateWindow("BUTTON", "7", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
     48, 546, 22, 16, g_ToolWin, NULL, g_hInst, NULL);
    hwndDamage8 = CreateWindow("BUTTON", "8", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY | BS_PUSHLIKE,
     70, 546, 22, 16, g_ToolWin, NULL, g_hInst, NULL);

    
    hwndLoopPlay = CreateWindow("BUTTON", "Loop", 
     WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY,
      6, 294, 92, 14, g_ToolWin, NULL, g_hInst, NULL);
    SendMessage(hwndLoopPlay, BM_SETCHECK, 1, 0);

    hwndWorld    = CreateWindow("BUTTON", "All", 
     WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_NOTIFY,
      6, 314, 92, 14, g_ToolWin, NULL, g_hInst, NULL);
    SendMessage(hwndWorld, BM_SETCHECK, 1, 0);
    LoopPlay = TRUE;

    hwndAcceleration = CreateWindow("SCROLLBAR", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
      5, 333, 62, 18, g_ToolWin, NULL, g_hInst, NULL);
    SetScrollRange(hwndAcceleration, SB_CTL,-31,31,TRUE);
    hwndAText = CreateWindow("STATIC", " 0", WS_CHILD | WS_VISIBLE,
      70, 332, 32, 18, g_ToolWin, NULL, g_hInst, NULL);
    WorldMode = TRUE;

    
    ShowWindow(g_ToolWin,SW_SHOW);
	UpdateWindow(g_ToolWin);


    trkww = ((w-210) / 10)*10;
    hwndKeyBar = CreateWindow("KEYSBAR", NULL, WS_CHILD | WS_VISIBLE,
      4, 1, trkww+2, 18, g_MWin, NULL, g_hInst, NULL);

    hwndKeyScrl = CreateWindow("SCROLLBAR", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ,
      w-202, 1, 92, 18, g_MWin, NULL, g_hInst, NULL);
    SetScrollRange(hwndKeyScrl,SB_CTL,0,MAXKEY*10-trkww,FALSE);

    hwndKeyText = CreateWindow("STATIC", " K: 0   T: 00.0", WS_CHILD | WS_VISIBLE | WS_BORDER,
      w-106, 1, 104, 18, g_MWin, NULL, g_hInst, NULL);
}

#pragma warning( disable : 4245 )
LRESULT APIENTRY EditWndProc( HWND hWnd, UINT message, UINT wParam, LONG lParam)
{	
   PAINTSTRUCT ps;
   g_ew = -1;
   int tmp,o;    

   for (int i=0; i<4; i++)  
      if( hWnd==g_3dwin[i] ) 
         g_ew=i;


    switch( message ) {
      case WM_KEYDOWN: MessageBeep(-1); return 0;
	  case WM_MOUSEMOVE:
        if( !MouseCaptured ) {
            if( g_ew == CurEditor )
               SetCurPosInfo( LOWORD(lParam), HIWORD(lParam) );
		      else SetWindowTitle("");
        }
      
		 break;

      case WM_LBUTTONDOWN:		
         if (g_ew!=CurEditor) {
			   o = CurEditor; CurEditor=g_ew;
			   InvalidateRect(g_3dwin[o],NULL,FALSE);
			   InvalidateRect(g_3dwin[CurEditor],NULL,FALSE);			 			 
            g_scaler = g_3dwininfo[ CurEditor ].Scaler;
		   } else {
            if (GetKeyState(VK_MENU) & 0x8000) {
             if (CurEditor==3) CurCommand = IDM_ANI;
                          else CurCommand = idt_mov;
             StartCapture();			 
             break;
            }

            tmp = DefineObjectCenter(LOWORD(lParam), HIWORD(lParam) );

			if (CurTool == idt_setmatrix) 
				if (tmp!=-1) {
				CurObject=tmp;
				ReActivateIK = 1;
				DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_SETMATRIX), hWnd,  (DLGPROC)SetIKParams);
				RefreshFrame();
				break;
			}

            if (CurTool == idt_mov) {
             CurCommand = CurTool;
             StartCapture();
             break;
            }

            if (tmp != -1 ) {
             if (tmp != CurObject) {
               CurObject = tmp;               
               RefreshFrame(); }             
             CurCommand = CurTool;
             StartCapture();
             }
           }
         break;

      case WM_SETCURSOR :
         if (g_ew!=CurEditor) SetCursor( hcurArrow );
          else 
           if (CurTool == idt_move || CurTool == idt_2dscale || CurTool == idt_ikmove) {
             if (iMoveMode == 1) SetCursor( hcurMoveH );
             if (iMoveMode == 2) SetCursor( hcurMoveV );
             if (iMoveMode == 3) SetCursor( hcurMoveHV);
           } else if (CurTool==idt_mov) SetCursor( hcurHand );
                                   else SetCursor( hcurRect );
         return 0;      

	  case WM_RBUTTONDOWN :
		 tmp = DefineObjectCenter(LOWORD(lParam), HIWORD(lParam) );
		 if (tmp!=-1) {
				CurObject=tmp;
				ReActivateIK = 1;
				DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_SETMATRIX), hWnd,  (DLGPROC)SetIKParams);
				RefreshFrame();
				break;
			}

         if( (wParam & MK_LBUTTON)!=0) {
            PostMessage(g_MWin,WM_COMMAND,IDM_ZOOM,0);
         } else PostMessage(g_MWin,WM_COMMAND,IDM_WMODE,0);

         break;
            
        case WM_NCPAINT: break;           
        case WM_ERASEBKGND: break;
        case WM_PAINT : {
	    	BeginPaint(hWnd, &ps );
            for ( int i = 0; i < 4; i++ )
             if( dc->hwnd[i] == hWnd ) break;
            DrawEditorWindow( dc->hwnd[i], dc->hdc[i], dc->hdcComp[i] );
			EndPaint( hWnd, &ps );
            return 0;
         }
        case WM_CREATE :  return 0;
        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}

int IsMouseInWindow( int x, int y )
{
   RECT* rc = &g_3dwininfo[ CurEditor ].DefPos;

   return ( x>=rc->left && x<=rc->right && y<=rc->bottom && y>=rc->top );
}

