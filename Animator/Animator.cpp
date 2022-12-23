#include <windows.h>
#include <stdio.h>

#define _MAIN_
#include "animator.h"
#include "resource.h"
#include <commctrl.h>

extern HANDLE hEvent;

void FinishApplication( void )
{
   char inipath[1024];
   DWORD l;
   GetWindowsDirectory(inipath, 512);
   strcat(inipath,"\\anitrk.ini");
   HANDLE hfile =
   CreateFile(inipath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if( hfile==INVALID_HANDLE_VALUE ) return;
   
   WriteFile(hfile, ModelPath, 255, &l ,NULL);
   WriteFile(hfile, TrackPath, 255, &l ,NULL);
   WriteFile(hfile, VTablePath, 255, &l ,NULL);
   
   WriteFile(hfile,  IKPath, 255, &l ,NULL);
   WriteFile(hfile, SoundPath, 255, &l ,NULL);
   WriteFile(hfile, &DefAngleX,    4, &l ,NULL);
   WriteFile(hfile, &DefAngleY,    4, &l ,NULL);
   WriteFile(hfile, &DefAngleZ,    4, &l ,NULL);
   CloseHandle(hfile);
}



void LoadIni( void )
{
   char inipath[1024];
   DWORD l;
   GetWindowsDirectory(inipath, 512);
   strcat(inipath,"\\anitrk.ini");
   HANDLE hfile =
   CreateFile(inipath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if( hfile==INVALID_HANDLE_VALUE ) return;
   
   ReadFile(hfile, ModelPath, 255, &l ,NULL);
   ReadFile(hfile, TrackPath, 255, &l ,NULL);
   ReadFile(hfile, VTablePath, 255, &l ,NULL);

   ReadFile(hfile, IKPath, 255, &l ,NULL);
   ReadFile(hfile, SoundPath, 255, &l ,NULL);

   ReadFile(hfile, &DefAngleX,    4, &l ,NULL);
   ReadFile(hfile, &DefAngleY,    4, &l ,NULL);
   ReadFile(hfile, &DefAngleZ,    4, &l ,NULL);


   CloseHandle(hfile);
}

#include <math.h>

void InitVars()
{
	bmp_Scale[0] = bmp_Scale[1] = bmp_Scale[2] = bmp_Scale[3] = 1.f;

	POINT c;
	//ConvertPoint(0,0,0, (int&)c.x, (int&)c.y);
	for(int u=0;u<3;u++)
	ConvertPoint(0,0,0, (int&)bmp_c[u].x,(int&)bmp_c[u].y);
	
	strcpy(bmp_fname, "rf_");
	strcpy(bmp_lname, "rs_");

	bmp_Begin = 1;
	bmp_Step = 1;

	strcpy(bmp_Path, ModelPath);
	//bmp_pos[1]=bmp_pos[2]=bmp_pos[3]=bmp_pos[0]=c;
}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow )
{

   g_hInst = hInstance;

   InitDirectDraw();
   InitCommonControls( );

   if( !InitApplication(hInstance) )
      return (FALSE);

   if( !InitInstance(hInstance, nCmdShow) )
       return (FALSE);

   if (!InitAudio()) MessageBox(NULL,"Error","Audio",MB_OK);          //Audio
 
   UpdateEditorWindow(-1);

   MSG msg;

   //for(;;)  Debug();
   //return 0;

  for(;;) {
   //GetMessage( &msg, NULL, 0, 0 ) 
   Vector3d v;

   if (IKStarted) {
	   ProcessSyncro(); 
	   if (!MouseCaptured) {
	     if (ReActivateIK) {ReActivateIK=0; StartIKProcess(); ProcessSyncro(); ProcessSyncro();}
		 //ProcessRealPhysic();
		 RefreshFrame();
		 UpdateEditorWindow(-1);}
   }

   while( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) ) {
		if( !TranslateAccelerator( g_MWin, (HACCEL)haccelTbl, &msg ) ) {
          TranslateMessage( &msg );
          DispatchMessage( &msg );
		  if (msg.message == WM_QUIT) goto END;
		}
   }
   if (CurCommand==idt_ikmove) if (IK_RushObjects()!=0) UpdateEditorWindow(-1);
   
  }
  END:
   FinishApplication( );

   return msg.wParam;

}

BOOL InitApplication(HANDLE hInstance)       
{
   WNDCLASS  wc;

    wc.style         = CS_DBLCLKS | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC)MainWndProc; 
    wc.cbClsExtra    = 0;                   
    wc.cbWndExtra    = 0;                   
    wc.hInstance     = (HINSTANCE)hInstance;           
    wc.hIcon         = (HICON)LoadIcon((HINSTANCE)hInstance, "APPICON");
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
 	wc.hbrBackground = CreateSolidBrush(0x242424);
	wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    wc.lpszMenuName  = "ANIMATORMenu";
    wc.lpszClassName = "AnimatorWClass";
    if( !RegisterClass(&wc) )
        return FALSE;

 	wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC)ToolWndProc;
    wc.cbClsExtra    = 0;                  
    wc.cbWndExtra    = 0;        
    wc.hInstance     = (HINSTANCE)hInstance; 
	wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );    
	wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	wc.lpszMenuName  = NULL;
    wc.lpszClassName = "3DTolls";
    if( !RegisterClass(&wc) )
      return FALSE;


    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC)KeyWndProc;
    wc.cbClsExtra    = 0;                  
    wc.cbWndExtra    = 0;        
    wc.hInstance     = (HINSTANCE)hInstance; 
	wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );;
    wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_3DFACE));    
    //wc.hbrBackground = CreateSolidBrush(0x00808020);
	wc.lpszMenuName  = NULL;
    wc.lpszClassName = "KEYSBAR";
    if( !RegisterClass(&wc) )
      return FALSE;
	
	wc.style         = CS_DBLCLKS | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC)EditWndProc; 
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = (HINSTANCE)hInstance;
	wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = CreateSolidBrush(0x006f6f6f);
 	wc.lpszMenuName  = NULL;
    wc.lpszClassName = "3DEditor";

	return RegisterClass(&wc);
}

BOOL InitInstance( HANDLE hInstance, int nCmdShow)           
{
    g_hpenF = CreatePen( PS_SOLID, 1, 0x00FFFFFF );
    g_hpenE = CreatePen( PS_SOLID, 1, 0x00E7E7E7 );
	g_hpenD = CreatePen( PS_SOLID, 1, 0x00D0D0D0 );
    g_hpenC = CreatePen( PS_SOLID, 1, 0x00C0C0C0 );
	g_hpenB = CreatePen( PS_SOLID, 1, 0x00B0B0B0 );
	g_hpenA = CreatePen( PS_SOLID, 1, 0x00A0A0A0 );
	g_hpen9 = CreatePen( PS_SOLID, 1, 0x00909090 );
	g_hpen8 = CreatePen( PS_SOLID, 1, 0x00808080 );
	g_hpen7 = CreatePen( PS_SOLID, 1, 0x00707070 );
	g_hpen6 = CreatePen( PS_SOLID, 1, 0x00606060 );
	g_hpen5 = CreatePen( PS_SOLID, 1, 0x00505050 );
	g_hpen4 = CreatePen( PS_SOLID, 1, 0x00404040 );
	g_hpen3 = CreatePen( PS_SOLID, 1, 0x00303030 );
    g_hpen2 = CreatePen( PS_SOLID, 1, 0x00202020 );
	g_hpen1 = CreatePen( PS_SOLID, 1, 0x00101010 );
	g_hpen0 = CreatePen( PS_SOLID, 1, 0x00000000 );
	g_hpen_R = CreatePen( PS_SOLID, 1, 0x00000070 );
	g_hpen_B = CreatePen( PS_SOLID, 1, 0x003030C0 );
	g_hpen_B2= CreatePen( PS_SOLID, 1, 0x00A0A070 );
	
	g_hpen3d   = CreatePen( PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
	g_hpen3dUp = CreatePen( PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT)-0x00080808);
	g_hpen3dDn = CreatePen( PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));

   hpenBP = CreatePen( PS_DOT, 0, RGB( 0, 0, 180 ) );
   hpenCross = CreatePen( PS_SOLID, 0, RGB( 0, 0, 125 ) );
   hbrNewPoint = CreateSolidBrush( RGB( 255, 0, 0 ) );
   hpenCurrentFaceV = CreatePen( PS_SOLID, 0, RGB( 0, 255, 255 ) );
   hpenCurrentFaceH = CreatePen( PS_SOLID, 0, RGB( 0, 155, 155 ) );
   hpenRed       = CreatePen( PS_SOLID, 0, RGB( 200, 0, 0 ) );
   hpenBlue      = CreatePen( PS_SOLID, 0, RGB( 0, 0, 155 ) );
   hpenMagenta   = CreatePen( PS_SOLID, 0, RGB(180, 180, 0 ) );
   hpenGreen     = CreatePen( PS_SOLID, 0, RGB(  0,  70, 0 ) );

   hbrRed        = CreateSolidBrush( RGB( 200, 0, 0 ) );
   hbrCyan       = CreateSolidBrush( RGB( 0, 140, 140 ) );   
   hbrGreen      = CreateSolidBrush( RGB( 0,  70,   0 ) );   
   hbrLGreen     = CreateSolidBrush( RGB( 0, 140,   0 ) );   
   hbrMagenta    = CreateSolidBrush( RGB(220, 220, 0 ) );
   
   hbr3DFace     = CreateSolidBrush(GetSysColor(COLOR_3DFACE));   
   hbrBlue       = CreateSolidBrush( RGB( 0, 0, 155 ) );
   hbrSelPoint   = CreateSolidBrush( RGB( 255, 255, 0 ) );

   iCaptionMenuOffset = GetSystemMetrics( SM_CYCAPTION ) + GetSystemMetrics( SM_CYMENU );

   for( int b=0; b<64; b++ )
		g_mybrush[b] = CreateSolidBrush(b * 0x00040404);
	for( b=0; b<64; b++ )
		g_mypen[b] = (HBRUSH)CreatePen(PS_SOLID,0,b * 0x00040404);

    LoadIni();
  /*g_3dwininfo[0].Title = "X/Y  Front";
	g_3dwininfo[1].Title = "Z/Y  Left";
	g_3dwininfo[2].Title = "X/Z  Top";
	g_3dwininfo[3].Title = "User";*/

	UserDrawMode   = 3;

    UserFillColor  = 2;
	AssignToAll    = 1;
	GammaCorection = 24;
    ZBuffer        = FALSE; 
    CurTool        = idt_rotate;
    RevView = TRUE;

	//SHOWSCELETON = 1;

	DivTbl[0] = 0x7fffffff;
    DivTbl[1] = 0x7fffffff;
    DivTbl[2] = 0x7fffffff;

   for( int i = 3; i < 10240; i++ )
      DivTbl[ i ] = (int) ((float)0x100000000 / i);
   

	if( !(haccelTbl = LoadAccelerators( g_hInst, "aa" )) ) {
      MessageBox( NULL, "Can't load accelerators!", "Animator", MB_OK|MB_ICONSTOP );
      return FALSE;
	}

   g_MWin = CreateWindow( "AnimatorWClass", "Animator",
  		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | 
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MAXIMIZEBOX | WS_MAXIMIZE,
		-0, -0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, g_hInst, NULL
   );

   if( !g_MWin )
      return FALSE;

  	HDC hdc = GetDC(g_MWin);
  	g_MemBMP = CreateCompatibleBitmap(hdc, 1024, 800);
	BITMAP bmp;
	GetObject( g_MemBMP, sizeof(BITMAP),&bmp );
	
	if( bmp.bmBitsPixel<16) {
	   MessageBox( NULL, "This program requires 16, 24 or 32 bit Color Mode", "Error", 
		   MB_ICONERROR || MB_OK);
	   return(FALSE); 
   }
		
	ScreenColorDepth = bmp.bmBitsPixel;
		
	GHeap = HeapCreate( 0, 16000000, 64000000 );
    

	
   BITMAPINFOHEADER bmih;
   bmih.biSize          = sizeof( BITMAPINFOHEADER ); 
   bmih.biWidth         =  GetSystemMetrics(SM_CXSCREEN);
   bmih.biHeight        = -GetSystemMetrics(SM_CYSCREEN);
   bmih.biPlanes        = 1;
   bmih.biBitCount      = 16;
   bmih.biCompression   = BI_RGB; 
   bmih.biSizeImage     = 0;
   bmih.biXPelsPerMeter = 400; 
   bmih.biYPelsPerMeter = 400; 
   bmih.biClrUsed       = 0; 
   bmih.biClrImportant  = 0;

   BITMAPINFO binfo;
   binfo.bmiHeader = bmih;
   g_DIBBMP = CreateDIBSection(hdc, &binfo, DIB_RGB_COLORS, &lpVideoBuf, NULL, 0);

   
   bmih.biWidth         = MAXKEY*10; 
   bmih.biHeight        = 16; 
   binfo.bmiHeader = bmih;
   g_DIBKEY = CreateDIBSection(hdc, &binfo, DIB_RGB_COLORS, &lpKeyBuf, NULL, 0);

   
   hcurArrow  = LoadCursor( NULL, IDC_ARROW );
   hcurRect   = LoadCursor( g_hInst, "IDC_RECT");
   hcurMoveHV = LoadCursor( g_hInst, "IDC_MOVEHV");
   hcurMoveH  = LoadCursor( g_hInst, "IDC_MOVEH");
   hcurMoveV  = LoadCursor( g_hInst, "IDC_MOVEV");
   hcurHand   = LoadCursor( g_hInst, "IDC_HAND2");
		
   CreateEditorWindows( );

   ClearModel();
   ClearTrack();

   dc = new DEVICECONTEXT( g_3dwin, 4 );

   DamageMode = 0;
   DamageMask = 0xAAAA0000;
   
   iMoveMode = 3;
   iRotMode = 0;
   iNextObject = 1;
   //SHOWMODEL = TRUE;
   SHOWMODEL = FALSE;
   SHOWMODELT = TRUE;
   SHOWBACK = 0;
   pInsPoint[0].x = -1; pInsPoint[0].y = -1;
   pInsPoint[1].x = -1; pInsPoint[1].y = -1;
   pInsPoint[2].x = -1; pInsPoint[2].y = -1;

   ShowWindow( g_MWin, SW_SHOWMAXIMIZED);
   UpdateWindow( g_MWin );

   InitVars();

   iChanged = 0;
   VCount = FCount = OCount = 0;

   for (int t=0; t<8; t++)
     Textures[t].lpTexture = HeapAlloc( GHeap, HEAP_ZERO_MEMORY, 1024*1024*4 );   

   ReleaseDC( g_MWin, hdc );

   return TRUE;               
}





void CheckSpeed()
{ 
  unsigned int t = timeGetTime();
  for (CurFrame = 0; CurFrame<=MaxFrame; CurFrame++) {
    //UpdateKeyBar();
    //CheckCurFrame();
	//DisableVT = TRUE;
    RefreshFrameAni();    
	RefreshModel();
    //UpdateEditorWindow(3);
  }  
  t=timeGetTime() - t;
  char buf[200];
  wsprintf(buf,"Time: %d msc.",t);
  MessageBox(NULL, buf, "Animator", MB_OK);
  CurFrame = 0;
  RefreshFrame();
}


TPoint3d map3pos[1024];
int mappingx[1024], mappingy[1024];
int mapcnt;

void SaveMapping()
{
/* mapcnt = VCount;

   for (int v=0; v<mapcnt; v++)
       map3pos[v] = gVertex[v];

   for (int f=0; f<FCount; f++) {
       mappingx[gFace[f].v1] = gFace[f].tax; mappingy[gFace[f].v1] = gFace[f].tay;
       mappingx[gFace[f].v2] = gFace[f].tbx; mappingy[gFace[f].v2] = gFace[f].tby;
       mappingx[gFace[f].v3] = gFace[f].tcx; mappingy[gFace[f].v3] = gFace[f].tcy;
   }
   */
}


void FindMapping(TPoint3d p, int &tx, int &ty)
{
    /*
    float d = 1024;
    for (int v=0; v<mapcnt; v++) {
        float dd =       (p.x-map3pos[v].x)*(p.x-map3pos[v].x)+
                         (p.y-map3pos[v].y)*(p.y-map3pos[v].y)+
                         (p.z-map3pos[v].z)*(p.z-map3pos[v].z);
        if (dd>d) continue;
        d=dd;
        tx = mappingx[v];
        ty = mappingy[v];
    }
    */
}

void RestoreMapping()
{
    /*
    for (int f=0; f<FCount; f++) {
        FindMapping(gVertex[gFace[f].v1], gFace[f].tax, gFace[f].tay);
        FindMapping(gVertex[gFace[f].v2], gFace[f].tbx, gFace[f].tby);
        FindMapping(gVertex[gFace[f].v3], gFace[f].tcx, gFace[f].tcy);
    }*/
}



#pragma warning( disable : 4245 )
LONG APIENTRY MainWndProc( HWND hWnd, UINT message, UINT wParam, LONG lParam )
{
   RECT rc;  
   int tmp;
   POINT p;

   switch (message) {
      case WM_CREATE:			return 0;
      case WM_SIZE:         return 0;
	  case WM_ACTIVATE :
			if( MouseCaptured ) 
              StopCapture( FALSE );

			return DefWindowProc(hWnd, message, wParam, lParam);

      case WM_SYSCOMMAND :
			if( g_MWin==GetFocus() && wParam == SC_RESTORE) 
            return 0; 
         else
				return DefWindowProc( hWnd, message, wParam, lParam );

      case WM_DISPLAYCHANGE:
          ReLocateWindows();
          return 0;
//=============== Keydown ===================//
      case WM_KEYDOWN :
         switch( (int)wParam ) {
			case 'A' :
				if (CurEditor>=2) return 0;
				MessageBeep(-1);
				if (!bmp_locked[CurEditor]) {
					bmp_locked[CurEditor]= 1;
					bmp_Scale[CurEditor] /= g_3dwininfo[CurEditor].Scaler;
					
					ConvertPointBack(bmp_c[CurEditor].x, bmp_c[CurEditor].y,
						bmp_pos0[CurEditor].x,bmp_pos0[CurEditor].y,bmp_pos0[CurEditor].z);
					//bmp_pos[CurEditor].x += g_3dwininfo[CurEditor]._xc0*g_3dwininfo[CurEditor].Scaler;
					//bmp_pos[CurEditor].y -= g_3dwininfo[CurEditor]._yc0*g_3dwininfo[CurEditor].Scaler;
				}
				else {
					bmp_Scale[CurEditor] *= g_3dwininfo[CurEditor].Scaler;
					//bmp_pos[CurEditor].x -= g_3dwininfo[CurEditor]._xc0*g_3dwininfo[CurEditor].Scaler;
					//bmp_pos[CurEditor].y += g_3dwininfo[CurEditor]._yc0*g_3dwininfo[CurEditor].Scaler;

					bmp_locked[CurEditor] = 0;
				}

				UpdateEditorWindow(-1);
				return 0;

            case 'O' :
               AdjustMouseCoords( p );
               tmp = DefineObjectCenter( p.x, p.y );
               if( tmp != -1 ) { CurObject = tmp; RefreshFrame(); }
               SetWindowTitle("");
               return 0;

            case 'P' :
               AdjustMouseCoords( p );
               tmp = DefineObjectCenter( p.x, p.y );
			   if (tmp==-1) { PathBegin=-1; PathEnd=-1; }
               PathObject = tmp; RefreshFrame();
               return 0;
            case 'T' :         
               SetBuildPos( );
               return 0;
            case 'H' :
               AdjustMouseCoords( p );
               if (GetKeyState(VK_CONTROL) & 0x8000) 
                ShowAll(); else {
                tmp = DefineObjectCenter( p.x, p.y );
                if (tmp!=-1) HideObject(tmp); }
               return 0;

            case VK_INSERT: DoInsertKey(); return 0;
            case VK_DELETE: DoDeleteKey(); return 0;

            case VK_TAB :
               if (CurTool == idt_ikmove ||
				   CurTool == idt_move   ||
                   CurTool == idt_2dscale) {
                    iMoveMode+=1;
                    if (iMoveMode>3) iMoveMode = 1;

                    GetCursorPos(&p);
                    if (MouseCaptured) {
                      SetCursorPos(380,310);
                      Delay(10);

                      if (iMoveMode == 1) SetCursor( hcurMoveH );
                      if (iMoveMode == 2) SetCursor( hcurMoveV );
                      if (iMoveMode == 3) SetCursor( hcurMoveHV);

                      ShowCursor(TRUE);
                      Delay(180);
                      ShowCursor(FALSE);
                      Delay(10);
                    }
                    SetCursorPos(p.x, p.y);
               }

               if (CurTool == idt_rotate ||
                   CurTool == idt_rotatel || 
				   CurTool == idt_ikrot) {
                    iRotMode++;
                    if (iRotMode>2) iRotMode=0;
                    if (MouseCaptured) {
                      StopCapture(FALSE);
                      CurCommand = CurTool;
                      StartCapture();
                    }
               }
               return 0;

            case VK_RETURN:
               CheckSpeed();
               return 0;

            case VK_UP : ScrollKeyBar(200,1); return 0;              
            case VK_DOWN : ScrollKeyBar(-200,1); return 0;              
            case VK_HOME : CurFrame = 0; RefreshFrame(); return 0;
            case VK_END  : CurFrame = MaxFrame; RefreshFrame(); return 0;


            case VK_LEFT :
            case VK_SUBTRACT:
              CurFrame--;
              if (CurFrame<0) CurFrame=MaxFrame; 
              RefreshFrame();
              return 0;

            case VK_RIGHT :
            case VK_ADD   :
              CurFrame++;
              if (CurFrame>MaxFrame) CurFrame=0;
              RefreshFrame();
              return 0;

         }
         return 0;

//=============== End Keydown ===================//
 
      


//=============== Mouse =========================//
      case WM_LBUTTONDOWN :
			if( MouseCaptured )
            StopCapture( TRUE );
			return 0;

      case WM_RBUTTONDOWN :
         if( !MouseCaptured )  return 0;                              
         StopCapture( FALSE );         
         return 0;

      case WM_LBUTTONUP :
			if( MouseCaptured && CurCommand == -1 )
            StopCapture( FALSE );
			return 0;

      case WM_NCMOUSEMOVE :
      case WM_MOUSEMOVE   :
			if( MouseCaptured )            		   
               ProcessCapturedMouse();
			return 0;
//============ End mouse ===========================//


      
//============ Tool key ============================//
	  case WM_TIMER:
			//ProcessRealPhysic();
			//RefreshFrame();
		  return 0;

      case WM_USER: {	
			int i=GetKeyOnCommand(CurCommand);
		    rc.left=ToolKey[i].x;  rc.top=ToolKey[i].y;
			rc.right=rc.left+31;   rc.bottom=rc.top+31;

			InvalidateRect(g_ToolWin,&rc,FALSE);
			SendMessage(g_ToolWin,WM_PAINT,0,0);

			SendMessage(hWnd,WM_COMMAND,CurCommand,0);

			if( !MouseCaptured ) {
				CurCommand=-1;
				Delay(80);
           	    InvalidateRect(g_ToolWin,&rc,FALSE);
				SendMessage( g_ToolWin, WM_PAINT, 0, 0 );
          }
			return 0;
		} 
//=========== End Tool key =========================//

//=========== Keys scroller =========================//
      case WM_HSCROLL:
         if (lParam == (int)hwndKeyScrl) 
            switch ( (int) LOWORD(wParam) ) {        
              case SB_PAGERIGHT: ScrollKeyBar(20*10,2); break;
              case SB_LINERIGHT: ScrollKeyBar(80,1); break;
         
              case SB_PAGELEFT : ScrollKeyBar(-20*10,2); break;
              case SB_LINELEFT : ScrollKeyBar(-80,1); break;
         
              case SB_THUMBPOSITION:
              case SB_THUMBTRACK   :               
                /*KeyDIBOfs = (short int) HIWORD(wParam);
                DrawKeyDIB();*/
                int d = (short int) HIWORD(wParam) - KeyDIBOfs;
                ScrollKeyBar(d, 1+abs(d)/20);
               break;  
             }             
        return 0;
        


//=========== Menu =================================//
      case WM_COMMAND:	
		  ReActivateIK = 1;
         switch( LOWORD( wParam ) ) {                        

            case IDM_NEW :
               ClearTrack();
               RefreshFrame();     
               return 0;  

            case IDM_OPENNEXT:
              if (OCount==0) OpenModel( hWnd );
                   else OpenTrack(hWnd);
              return 0;
                              
            case IDM_FILEOPEN:
               OpenModel( hWnd );               
               return 0;

            case IDM_SETAUTOSPHERER:
               CalcLinkedBodySpheresR();
			   UpdateEditorWindow(-1);
               return 0;


            case IDM_OPENTRACK:
               OpenTrack(hWnd);               
               return 0;

	   /////Audio

            case IDM_WAV:
               LoadSoundFX(false);               
               return 0;

            case IDM_WAV_UPDATE:
               LoadSoundFX(true);               
               return 0;

            case IDM_PLAYSOUND : 

                if (!MouseCaptured) 
				{
 				  PlaySound();
                  PlayAniRealTime();
				}
				StopSound();
               return 0;

            case IDM_SAVE :                             
               SaveTrack(SourceTrackName);
               UpdateEditorWindow( CurEditor );
               return 0;

            case IDM_SAVEAS :               
               SaveAs();
               UpdateEditorWindow( CurEditor );
               return 0;

			case IDM_SAVE3DF :
				Save3DF();
			return 0;
			
			case IDM_SAVEIKAS :               
				SaveIKAs();
			   UpdateEditorWindow( CurEditor );
            return 0;

			case IDM_OpenIK : 
				OpenIK();
			   UpdateEditorWindow( CurEditor );
            return 0;


            case IDM_SAVEANI:
               SaveVTable();
               return 0;

            case IDM_OLIST:
              DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_OTREE), hWnd,   (DLGPROC)OList);
              UpdateEditorWindow(-1);
              return 0;

            case IDM_SHOWSCEL:
              SHOWSCELETON=!SHOWSCELETON;
              UpdateEditorWindow(-1);
              return 0;

            case IDM_SHOWMODEL:
              SHOWMODEL=!SHOWMODEL;
              UpdateEditorWindow(-1);
              return 0;
            case IDM_SHOWSPHERES:
              SHOWSPHERES=!SHOWSPHERES;
              UpdateEditorWindow(-1);
              return 0;

            case IDM_SHOWMODELT:
              SHOWMODELT=!SHOWMODELT;
              UpdateEditorWindow(-1);
              return 0;

			case IDM_SHOWBACK:
              SHOWBACK=!SHOWBACK;
              UpdateEditorWindow(-1);
              return 0;

			case ID_IKPROP :
			 DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_IKPROP), hWnd,  (DLGPROC)IKProp);
			return 0;
			
			case IDM_SetPhysic :
			 DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_SETPHYS), hWnd,  (DLGPROC)SetPhysic);
			return 0;

			case IDM_APPLYF :
			 ProcessRealPhysic();
			 RefreshFrame();
			 UpdateEditorWindow(-1);
			return 0;  
			case IDM_StartPhysic :
				//SetTimer(hWnd,1,0,NULL);
				StartIKProcess();
			return 0;  
			case IDM_EndPhysic :
				//KillTimer(hWnd,1);
				EndIKProcess();
			return 0;  


			case IDM_ZOOM :
            case idt_zoom :
               DoZoom();    
               return 0;

            case IDM_ZOOMIN: 
	             g_3dwininfo[CurEditor].Scaler *= 1.25f;
		         UpdateEditorWindow(CurEditor);
		         return 0;

            case IDM_ZOOMOUT: 
	             g_3dwininfo[CurEditor].Scaler /= 1.25f;
		         UpdateEditorWindow(CurEditor);
		         return 0;

            case IDM_ZERO:                    
					g_alpha = 0.0f;
                    if (RevView) g_alpha = pi;
	                g_beta = 0.0f;
                    UpdateEditorWindow(3);	  
					return 0;

            case IDM_ANI: {
                    if (CurEditor!=3) {
                     CurEditor = 3;
                     UpdateEditorWindow(-1);
                    }
					CurCommand=IDM_ANI;
                    StartCapture();							                                
		         return 0;
                          }

            case IDM_PLAYANI : 
                if (!MouseCaptured) PlayAni();
                return 0;

            case IDM_PLAYREAL : 
                if (!MouseCaptured) 
                 //if (CurEditor==3 && UserDrawMode>1) 
                     PlayAniRealTime();
                       //else PlayAni();
                return 0;

			case IDM_MIRRORKEY: {
                int s = gObj[CurObject].owner;
                gObj[CurObject].owner = 255;
                MirrorChild(255);
                gObj[CurObject].owner = s;
                RefreshFrame();
				return 0;
			}

            case IDM_INVERTTRACK: {
                TKey tmpk[128];
                for (int k=0; k<MaxFrame/2; k++) {
                  memcpy(tmpk, Track[k], sizeof(tmpk));
                  memcpy(Track[k], Track[MaxFrame-k], sizeof(tmpk));
                  memcpy(Track[MaxFrame-k], tmpk, sizeof(tmpk));
                }
                RefreshFrame();
                return 0;
            }

            case IDM_REVERSETRACK: {                
                for (int k=0; k<=MaxFrame; k++) 
                for (int o=0; o<OCount; o++)
                  if (Track[k][o].active) {
                      Track[k][o].Xfi*=-1;
                      Track[k][o].Zfi*=-1;
                      Track[k][o].ddz*=-1;
                }
                RefreshFrame();
                return 0;
            }

            case ID_SAVEMAP :
                SaveMapping();
                return 0;
				
				
            case ID_RESTOREMAP :
                RestoreMapping();
                return 0;

            case ID_TRACKBMP: {
				DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_TRACKBMP), hWnd,  (DLGPROC)TrackBmpDlgProc);
				RefreshFrame();
             return 0;}

            case ID_OBJSMOOTH: {
				SmoothObjectAllFrames(CurObject);
				RefreshFrame();
				return 0;
			}
			case ID_OBJSMOOTHALL: {
				SmoothAllObjectsAllFrames();
				return 0;
			 }
			case ID_RECALCALLANGLES: {
				CalcAllAnglesAllFrames();
				return 0;
			}
			case ID_CALCANGELS: {
				CalcAutoTrackAngles();
				RefreshFrame();
			return 0;
			}


			case ID_SYNCHRO: {
			if (CurEditor>=2) return 0;
			if (g_3dwininfo[CurEditor].Zoomed) return 0;
			int e = CurEditor;
			int from = !CurEditor;
			if (e==0) from = 1;
			if (e==1) from = 0;
			bmp_Scale[e] = bmp_Scale[from];
			bmp_c[e] = bmp_c[from];
			bmp_pos0[e] = bmp_pos0[from];
			bmp_locked[e] = bmp_locked[from];
			g_3dwininfo[e]._xc = g_3dwininfo[from]._xc;
			g_3dwininfo[e]._yc = g_3dwininfo[from]._yc;
			if (from!=0)
			g_3dwininfo[e]._xc0 = g_3dwininfo[from]._xc0;
			g_3dwininfo[e]._yc0 = g_3dwininfo[from]._yc0;
			g_3dwininfo[e].Scaler = g_3dwininfo[from].Scaler;
			RefreshFrame();
			MessageBeep(-1);
			return 0; }


            case ID_EDIT_RLPROP: {
             DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_RLPARAMS), hWnd,  (DLGPROC)RLParams);
             RefreshFrame();
             return 0;
            }

            case IDM_ROLLTRACK: {
             DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_ROLL), hWnd,  (DLGPROC)RollTrk);
             RefreshFrame();  
             return 0;
            }

            case IDM_EXIT:
               if( MessageBox( NULL, "  Quit Animator?",
                  "Animator", MB_YESNO | MB_ICONQUESTION ) == IDNO )
                  return 0;

               CloseWav();    /// Audio
               PostMessage( g_MWin, WM_CLOSE, 0, 0 );
               return 0;

			case IDM_ABOUT:
               DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_ABOUT), hWnd,  (DLGPROC)About);
               return 0;

            case IDM_INFO:
               DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_INFO), hWnd,  (DLGPROC)Info);
               return 0;

            case ID_EDIT_PROPERTIES:
               DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_APROP), hWnd,  (DLGPROC)AProp);
               RefreshFrame();
               return 0;

            case IDM_WMODE:
               if( CurEditor==3 ) {
                  DialogBox(g_hInst,  MAKEINTRESOURCE(IDD_CAMERA), g_MWin,  (DLGPROC)CamMode );
                  UpdateEditorWindow(-1);
               } else     
                  DialogBox(g_hInst, MAKEINTRESOURCE(IDD_EMODE), g_MWin,  (DLGPROC)EditMode);
               return 0;

            case idt_max1 : {
               if( !VCount )
                  return 0;

               CalibrateImage( CurEditor );
               UpdateEditorWindow( CurEditor );
               return 0; }

            case idt_maxa : {
               if( !VCount )
                  return 0;

               int j = CurEditor;
               for( int i = 0; i < 4; i++ )
                  CalibrateImage( i );
               CurEditor = j;

               UpdateEditorWindow( -1 );

               return 0;
                            }

            case idt_wireframe :
               UserDrawMode = 0;
               UpdateEditorWindow( 3 );               
               return 0;

            case idt_flat :
               UserDrawMode = 2;
               UpdateEditorWindow( 3 );
               return 0;

            case idt_textured :
               if (TCount) UserDrawMode = 3;
               UpdateEditorWindow( 3 );
               return 0;

            case idt_hold :
               MessageBeep(0xFFFFFFFF);
               SaveTrack("hold.trk");
               return 0;

            case idt_fetch :
               if( MessageBox( g_MWin, "Are you sure you want to do fetch?", "Animator",
                  MB_YESNO | MB_ICONQUESTION ) == IDNO )
                  return 0;

               MessageBeep(0xFFFFFFFF);
               LoadTrk("hold.trk");
               RefreshFrame();
               return 0;

            default:
               return (DefWindowProc(hWnd, message, wParam, lParam));
         }
         break;
//=================== end menu ===================================================//

      case WM_CLOSE :
         if( iChanged )
            if( MessageBox( NULL, "Would uou like to keep changes?", "Animator",
               MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL ) == IDYES );
         DestroyWindow( g_MWin );
         return 0;

      case WM_DESTROY:
         PostQuitMessage(0);
         return 0;

      default:
         return (DefWindowProc(hWnd, message, wParam, lParam));
   }
}


