#include <windows.h>    // includes basic windows functionality

#include "animator.h"   // includes application-specific information
#include <commctrl.h>
#include <stdio.h>
#include <math.h>


  

BOOL ObjInWork[128];

void ClearModel()
{
   OrgTextSize = 0;
   OrgTextHeight = 0;
   TCount = 0;
   FbyTCount[0] = 0;
   LodCount = 0;
   CurLod = 0;
   SetLod(0);

	CurCommand = -1;	    
	CurObject = 0;    
    strcpy(gObj[0].OName, "noname");
	iCurFace = -1;
	CurFlags = 0;
    if( g_3dwininfo[CurEditor].Zoomed )
     DoZoom( );

    CurEditor = 0;

    PathObject = -1;
	OCount = 0;
	FCount = 0;
	VCount = 0;

    g_alpha = 0.0f; g_beta = 0.0f;

	for( int j=0; j<4; j++ ) {
		g_3dwininfo[j].Scaler=(float)0.75;
        g_3dwininfo[j]._xc0=(float)0.0;
		g_3dwininfo[j]._yc0=(float)0.0; 
    }
    g_3dwininfo[0]._yc0 = 100.f;
	g_3dwininfo[1]._yc0 = 100.f;
	g_3dwininfo[3]._yc0 = 100.f;


    g_alpha = pi;
	for( j = 0; j < 4; j++ ) {		
      g_3dwininfo[j].PointType  = 0;
      g_3dwininfo[j].CenterLine = 1;
      g_3dwininfo[j].GridStep   =16;
	  g_3dwininfo[j].ShowObj    = 1;
      g_3dwininfo[j].GridLines  = 1;
      g_3dwininfo[j].MouseAlign = 0;   
      g_3dwininfo[j].Zoomed     = 0;
   }
	g_3dwininfo[3].CenterLine = 0; 

   bp.x = (float)0.0; bp.y = (float)0.0; bp.z = (float)0.0;
   strcpy(ModelName, "noname");

   SendMessage( g_ToolWin, WM_LBUTTONDOWN, 0x1, 0x009f0015 );
   
}




void ClearTrack()
{
  aniKPS = 20;
  RotSequence = 0;
  PathObject = -1;
  PathBegin=-1; 
  PathEnd=-1;
  for (int k=0; k<MAXKEY; k++)
   for (int o=0; o<128; o++) {
    Track[k][o].active = (k==0);
    Track[k][o].acceleration = 0;;

    Track[k][o].Xfi = 0.f;
    Track[k][o].Yfi = 0.f;
    Track[k][o].Zfi = 0.f;

    Track[k][o].ddx = 0.f;
    Track[k][o].ddy = 0.f;
    Track[k][o].ddz = 0.f;

    Track[k][o].scx = 1.f;
    Track[k][o].scy = 1.f;
    Track[k][o].scz = 1.f;
   }   

   KeyDIBOfs = 0;
   CurFrame  = 0;
   strcpy(TrackName, "noname");
   strcpy(SourceTrackName, "noname.trk");
}



void UpdateTrackInfo()
{
  FillMemory(WorldKeyA, sizeof(WorldKeyA), 1);
  FillMemory(UsedKeyA, sizeof(UsedKeyA), 0);
  for (int k=0; k<MAXKEY; k++)
   for (int o=0; o<OCount; o++) 
    if (Track[k][o].active) {
       UsedKeyA[k] = TRUE;
       MaxFrame = k; } else WorldKeyA[k] = FALSE; 
}


void MirrorChild(int owner)
{
  for (int o=0; o<OCount; o++)
   if (Track[CurFrame][o].active)
    if (gObj[o].owner == owner) {
	  Track[CurFrame][o].Yfi*=-1;
	  Track[CurFrame][o].Zfi*=-1;
	  MirrorChild(o);
   }
}



void DrawKeyDIB()
{
  RECT rc;
  int x2, y2;
	   
  GetClientRect    (hwndKeyBar, &rc);
  x2=rc.right-rc.left;
  y2=rc.bottom-rc.top;

  HDC _hdc = GetDC(hwndKeyBar);  
  HDC hdc = CreateCompatibleDC(_hdc);    
  
  HBITMAP hbitmapOld = (HBITMAP)SelectObject(hdc,g_DIBKEY);
  
  BitBlt(_hdc,1,1,x2-2,y2-2, hdc, KeyDIBOfs,0, SRCCOPY);
  SelectObject(hdc,hbitmapOld);

  DeleteDC(hdc);
  ReleaseDC(hwndKeyBar, _hdc);
}

void DrawKeyBar(HWND hWnd, HDC _hdc)
{
     RECT rc;
     int x2, y2, k;

     UpdateTrackInfo();
     
     ProcessAcceleration();
     SetScrollPos(hwndKeyScrl,SB_CTL,KeyDIBOfs,TRUE);

     char buf[32];
     sprintf(buf," K:%3d  T: %2.2f",CurFrame, (float)CurFrame / aniKPS);
     SetWindowText(hwndKeyText, buf);
    	       
     GetClientRect    (hWnd, &rc);
	 x2=rc.right-rc.left;
	 y2=rc.bottom-rc.top;

     HPEN _hpenOld = (HPEN)SelectObject(_hdc,g_hpen2);
     Draw3dKey(_hdc,0,0,x2-1,y2-1,1,g_hpen3dDn,0,g_hpen3dUp);
     Draw3dKey(_hdc,0,0,x2-1,y2-1,1,g_hpen5,0,g_hpenB);
     
     HDC hdc = CreateCompatibleDC(_hdc);
     
     HPEN    hpenOld = (HPEN)SelectObject(hdc,g_hpen2);		
     HBRUSH  hbrushOld = (HBRUSH)SelectObject(hdc,hbr3DFace);
     HBITMAP hbitmapOld = (HBITMAP)SelectObject(hdc,g_DIBKEY);

     //PatBlt(hdc, (MaxFrame+1)*10,0, MAXKEY*10, 16, PATCOPY );   
     PatBlt(hdc, 0,0, MAXKEY*10, 16, PATCOPY );   
     SelectObject(hdc,hbrCyan);
     PatBlt(hdc, 0, 2, (MaxFrame+1)*10, 14, PATCOPY );   

	 if (PathObject!=-1) {
      if (PathBegin==-1) PathOBegin=0; else PathOBegin=PathBegin;
      if (PathEnd  ==-1) PathOEnd  =MaxFrame; else PathOEnd=PathEnd;	 
	  SelectObject(hdc,hbrLGreen);     
      PatBlt(hdc, 1+PathOBegin*10, 2, 9+(PathOEnd-PathOBegin)*10, 14, PATCOPY );   
	 }


     SelectObject(hdc,hbrBlue);     
     PatBlt(hdc, 1+CurFrame*10, 2, 9, 14, PATCOPY );   
     
     //SelectObject(hdc,hbrBlue);     
     SelectObject(hdc,g_mybrush[12]);
     DrawRect(hdc, 0,2,MAXKEY*10-1, y2-3);
     for (k=0; k<MAXKEY; k++) {
       if (k%aniKPS == 0) { PatBlt(hdc, k*10, 1, 11, 1, PATCOPY); } //PatBlt(hdc, k*10, 14, 11, 1, PATCOPY); }
       Line(hdc, k*10, 2, k*10, 16);     
     }

     SelectObject(hdc,g_mybrush[44]);
     for (k=0; k<=MaxFrame; k++) {       
       if (WorldKeyA[k]) {                
        DrawKey(hdc, 2+k*10, 5, 8+k*10, 13, g_hpenD, g_hpen1);        
        PatBlt(hdc, 3+k*10, 6, 5, 7, PATCOPY );                    
        } else 
       if (Track[k][CurObject].active) {                    
          PatBlt(hdc, 3+k*10, 6, 5, 6, PATCOPY );                    
          DrawKey(hdc, 2+k*10, 5, 8+k*10, 12, g_hpen4, g_hpen4);
        } else if (UsedKeyA[k]) 
         DrawKey(hdc, 2+k*10, 5, 8+k*10, 12, g_hpen4, g_hpen4);

     }
     
     BitBlt(_hdc,1,1,x2-2,y2-2, hdc, KeyDIBOfs,0, SRCCOPY);

     SelectObject(hdc,hpenOld);
     SelectObject(hdc,hbrushOld);
     SelectObject(hdc,hbitmapOld);
     DeleteDC(hdc);
     
     SelectObject(_hdc,_hpenOld);
}




void ScrollKeyBar(int Delta, int spd) 
{
 int DK = 1;
 if (Delta<0) DK = -1;
 DK*=spd;
 unsigned int NextTime = timeGetTime();
 SetScrollPos(hwndKeyScrl,SB_CTL,KeyDIBOfs+Delta,TRUE);
 for ( ; ; ) {
  if (Delta==0) return;
  if (abs(DK)>abs(Delta)) DK = Delta;
  Delta-=DK;
  KeyDIBOfs+=DK;

  if (KeyDIBOfs<0) { KeyDIBOfs=0; DrawKeyDIB(); return; }
  if (KeyDIBOfs>MAXKEY*10-trkww) { KeyDIBOfs=MAXKEY*10-trkww; DrawKeyDIB(); return; }

  DrawKeyDIB(); 
  NextTime+=2;
  while (timeGetTime() < NextTime) ;
 } 
}


void UpdateKeyBar()
{
  HDC hdc = GetDC(hwndKeyBar);
  DrawKeyBar(hwndKeyBar, hdc);
  ReleaseDC(hwndKeyBar, hdc);
}


void UpdateLodKeys()
{
  EnableWindow(hwndLod1, (LodCount>0));
  EnableWindow(hwndLod2, (LodCount>1));
  EnableWindow(hwndLod3, (LodCount>2));
  EnableWindow(hwndLod4, (LodCount>3));
}

void CheckCurFrame()
{
  if (CurFrame*10<KeyDIBOfs) { KeyDIBOfs = CurFrame*10; DrawKeyDIB(); }
  if (CurFrame*10>KeyDIBOfs+(trkww-10)) { KeyDIBOfs = CurFrame*10-(trkww-10); DrawKeyDIB(); }
}



int kLastxPos = -1;
int kLastKey  = -1;

void ProcessKeyMove(int& c)
{
  int k,o;

  while (c>kLastKey)  {
   for (k=MaxFrame; k>=kLastKey; k--)
    for (o=0; o<OCount; o++) Track[k+1][o] = Track[k][o];
   for (o=0; o<OCount; o++) Track[kLastKey][o].active = FALSE;
   kLastKey++;
   UpdateTrackInfo();
  } 

  while (c<kLastKey) {
   
   for (o=0; o<OCount; o++) 
    if (Track[kLastKey-1][o].active) { c = kLastKey;  return; }

   for (k=kLastKey; k<=MaxFrame; k++)
    for (o=0; o<OCount; o++) Track[k-1][o] = Track[k][o];
   for (o=0; o<OCount; o++) Track[MaxFrame][o].active = FALSE;
   kLastKey--;
   UpdateTrackInfo();
  }

  
}




LRESULT APIENTRY KeyWndProc( HWND hWnd, UINT message, UINT wParam, LONG lParam)
{	
	HDC hdc;
	PAINTSTRUCT ps;
    int c,xPos;
 

    switch (message) {      
        case WM_RBUTTONUP  :   kLastxPos = -1;   break;
        case WM_LBUTTONUP  :   kLastKey  = -1;   break;
        
        case WM_RBUTTONDOWN:
			if (GetKeyState(VK_SHIFT) & 0x8000) {
		      xPos = LOWORD(lParam)-2; if (xPos<0) xPos = 0;                              
			  PathEnd = (xPos + KeyDIBOfs)  / 10;
			  if (PathEnd>MaxFrame) { PathBegin=-1; PathEnd=-1; }
			  if (PathEnd<PathBegin) PathBegin=PathEnd;
			  RefreshFrame();
			  return 0;
			}

            kLastxPos = LOWORD(lParam);
            break;

        case WM_MOUSEMOVE   : 
            if (wParam & MK_RBUTTON && kLastxPos!=-1) {
              ScrollKeyBar(kLastxPos - LOWORD(lParam), 4);
              kLastxPos = LOWORD(lParam); 
              return 0;
            }
            if (!(wParam & MK_LBUTTON)) return 0;            

        case WM_LBUTTONDOWN:
            xPos = LOWORD(lParam)-2;
            if (xPos<0) xPos = 0;
            xPos+=KeyDIBOfs;            
            c = xPos / 10;            

			if (GetKeyState(VK_SHIFT) & 0x8000) {
			  PathBegin = c;
			  if (PathBegin>MaxFrame) { PathBegin=-1; PathEnd=-1; }
			  if (PathEnd!=-1)
			   if (PathEnd<PathBegin) PathEnd=PathBegin;
			  RefreshFrame();
			  return 0;
			}


            if (GetKeyState(VK_MENU) & 0x8000) 
              if (kLastKey==-1) { 
               if (c<=MaxFrame) kLastKey = c; 
              } else ProcessKeyMove(c);

            if (c == CurFrame) return 0;
            CurFrame = c;
            if (message == WM_MOUSEMOVE && CurFrame>MaxFrame) CurFrame = MaxFrame;
                      
            if (kLastKey==-1)
             if (CurFrame>MaxFrame) 
              if (MessageBox(g_MWin, 
               "Do you want to extend track with new world key?",
               "Verify", MB_YESNO) == IDYES) {
             for (int o=0; o<OCount; o++) Track[CurFrame][o] = Track[MaxFrame][o];
             UpdateKeyBar();             
            }

            if (CurFrame>MaxFrame) CurFrame = MaxFrame;
            RefreshFrame();
            return 0;

        case WM_PAINT:             // message: command from application menu 
	    	hdc=BeginPaint(hWnd, &ps );
            DrawKeyBar(hWnd, hdc);
			EndPaint( hWnd, &ps );
			return 0;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0); 
}


void CreatePath()
{
  int cf = CurFrame;
  int cs,k;   
  cs = 0; 

  for (k=0; k<=MaxFrame; k++) 
   for (int o=0; o<OCount; o++) {
     int* lpK = (int*) &Track[k][o];
     for (int m=0; m<(sizeof(TKey)<<2); m++) 
      cs+=*(lpK + m);
   }
  cs += PathObject;
  cs += MaxFrame;
  cs += PathBegin;
  cs += PathEnd;

  if (cs == PathCSum) return;

  FillMemory(ObjInWork, sizeof(ObjInWork), 0);
  int o = PathObject;
  for (;;) {
    ObjInWork[o] = TRUE;
    o = gObj[o].owner;
    if (o == gObj[o].owner) break;
    if (o==-1) break;
  }

  //MessageBeep(0xFFFFFFFF);
  PathCSum = cs;

  if (PathBegin==-1) PathOBegin=0; else PathOBegin=PathBegin;
  if (PathEnd  ==-1) PathOEnd  =MaxFrame; else PathOEnd=PathEnd;

  for (k=PathOBegin; k<=PathOEnd; k++) {    
    CurFrame = k;    
    RefreshFrameAni();
    DisableVT = TRUE;
    RefreshModel();
    ObjPath[k].x = gObj[PathObject].ox;
    ObjPath[k].y = gObj[PathObject].oy;
    ObjPath[k].z = gObj[PathObject].oz;
    ObjPath[k].owner = Track[k][CurObject].active;
  }
  CurFrame = cf;  
}


void SmoothObject(int f, int o)
{
	if (f-1<0 || f+1>MaxFrame)  return;
	
	//Track[f][o].active = 1;
/*
	float Xfi1 = Track[f-1][o].Xfi*pi/180.f;
	float Yfi1 = Track[f-1][o].Yfi*pi/180.f;
	float Zfi1 = Track[f-1][o].Zfi*pi/180.f;

	float Xfi2 = Track[f+1][o].Xfi*pi/180.f;
	float Yfi2 = Track[f+1][o].Yfi*pi/180.f;
	float Zfi2 = Track[f+1][o].Zfi*pi/180.f;

	float Xfi = Xfi1 + SubAngles(Xfi2, Xfi1)/2;
	float Yfi = Xfi1 + SubAngles(Yfi2, Yfi1)/2;
	float Zfi = Xfi1 + SubAngles(Zfi2, Zfi1)/2;
/*
	float Xfi_ = Xfi + SubAngles(Track[f][o].Xfi*pi/180, Xfi)/2;
	float Yfi_ = Yfi + SubAngles(Track[f][o].Yfi*pi/180, Yfi)/2;
	float Zfi_ = Zfi + SubAngles(Track[f][o].Zfi*pi/180, Zfi)/2;

	Xfi *= 180.f/pi;
	Yfi *= 180.f/pi;
	Zfi *= 180.f/pi;

	Track[f][o].Xfi = Xfi;
	Track[f][o].Yfi = Yfi;
	Track[f][o].Zfi = Zfi;
*/

	Track[f][o].Xfi += (Track[f-1][o].Xfi + Track[f+1][o].Xfi)/2;
	Track[f][o].Yfi += (Track[f-1][o].Yfi + Track[f+1][o].Yfi)/2;
	Track[f][o].Zfi += (Track[f-1][o].Zfi + Track[f+1][o].Zfi)/2;

	Track[f][o].ddx += (Track[f-1][o].ddx + Track[f+1][o].ddx)/2;
	Track[f][o].ddy += (Track[f-1][o].ddy + Track[f+1][o].ddy)/2;
	Track[f][o].ddz += (Track[f-1][o].ddz + Track[f+1][o].ddz)/2;

	Track[f][o].scx += (Track[f-1][o].scx + Track[f+1][o].scx)/2;
	Track[f][o].scy += (Track[f-1][o].scy + Track[f+1][o].scy)/2;
	Track[f][o].scz += (Track[f-1][o].scz + Track[f+1][o].scz)/2;

	Track[f][o].Xfi *= 0.5;
	Track[f][o].Yfi *= 0.5;
	Track[f][o].Zfi *= 0.5;

	Track[f][o].ddx *= 0.5;
	Track[f][o].ddy *= 0.5;
	Track[f][o].ddz *= 0.5;

	Track[f][o].scx *= 0.5;
	Track[f][o].scy *= 0.5;
	Track[f][o].scz *= 0.5;
}

void SmoothObjectAllFrames(int o)
{
	MessageBeep(-1);
	for (int k=0; k<=MaxFrame; k++) {
	  //Track[CurFrame][o].active = 1;
	  if (Track[k][o].active)
	  SmoothObject(k, o);
	}
}

void CalcAllAnglesAllFrames()
{
  Vector3d  ObjData[MAXKEY][128];
  Vector3d  ObjVec[MAXKEY][128];

  MessageBeep(-1);
  IKLink   Links2[256];
  int CurFrame_ = CurFrame;
  for (CurFrame=0; CurFrame<=MaxFrame; CurFrame++) {

     //CurFrame=0;
	 RefreshFrameAni();
	 RefreshModel();
	 BuildLinks3D(Links, gObj);
	 for (int o=1; o<OCount; o++) {
		  	Track[CurFrame][o].ddx = 0;
			Track[CurFrame][o].ddy = 0;
			Track[CurFrame][o].ddz = 0;
	 }
	 for (o=1; o<OCount; o++) {
		  	Track[CurFrame][o].Xfi = 0;
			Track[CurFrame][o].Yfi = 0;
			Track[CurFrame][o].Zfi = 0;
			Track[CurFrame][o].scx = 1;
			Track[CurFrame][o].scy = 1;
			Track[CurFrame][o].scz = 1;
	 }

 	 RefreshFrameAni();
	 RefreshModel();

	 BuildLinks3D(Links2, gObj);
     RecalcGlobalBasisesFromAngles(Basis);

	 CalcIKBasisesBy2Links(Links, Links2);

	 StartJoint = 0;
	 StartIKJoint = 0;
	 int n2 = Links[StartJoint].owner;
	 if (n2==-1) CalcObjectAnglesByBasis(StartJoint, IdentityBasis, 0);
 
	 UpdateObjPosFromBasis(gObj, StartJoint);
  }
  CurFrame = CurFrame_;
}

/*
void CalcAllAnglesAllFrames()
{
  MessageBeep(-1);
  for (int k=0; k<=MaxFrame; k++)  {
	  
	  RefreshFrameAni();
	  RefreshModel();
	  BuildLinks3D(Links, gObj);

	  //Calc
	  RecalcGlobalBasisesFromAngles(Basis);		// this function can be used only at start transforming
	  
	  //MoveLinks3DR(Links[n].owner, n, v);
	  SetChildBasis(n,gObj,opos0);
	    
	  StartJoint=0;
	  int n2 = Links[StartJoint].owner;
	  if (n2==-1) CalcObjectAnglesByBasis(StartJoint, IdentityBasis, 0);
	  
	  UpdateObjPosFromBasis(gObj, StartJoint);
	  RefreshFrameAni();
	  RefreshModel();
  }
}
*/
void SmoothAllObjectsAllFrames()
{
  MessageBeep(-1);
  for (int k=0; k<=MaxFrame; k++)
   for (int o=0; o<OCount; o++)
	   if (Track[k][o].active)
	   SmoothObject(k, o);
}

void RefreshFrame()
{
   UpdateKeyBar();
   CheckCurFrame();
   if (PathObject!=-1) CreatePath();
   RefreshFrameAni();
   RefreshModel();
   UpdateEditorWindow(-1);
}


void ActivateKey(int CurFrame, int o)
{
    if (Track[CurFrame][o].active) return;
    int k1,k2;
    for (k1=CurFrame-1; (k1>=0 && !Track[k1][o].active); k1--);
    for (k2=CurFrame+1; (k2<=MaxFrame && !Track[k2][o].active); k2++);      

    float t = (float)(CurFrame - k1);
    float T = (float)(k2 - k1);
    T = t/T;
    float a0 = (float)Track[k1][o].acceleration/32.f;
    float a1 = a0*(1-T)/(1+a0*T);
    float a2 = a0*(1-T)/(1-a0*T);
    Track[CurFrame][o] = FrameAni[o];
    Track[CurFrame][o].active = TRUE;
    Track[CurFrame][o].acceleration = (int)(a1*32.f);
    Track[k1      ][o].acceleration = (int)(a2*32.f);
}


void DoInsertKey()
{
   int code;
   if (WorldKeyA[CurFrame]) {MessageBeep(0xFFFFFFFF); return; }
   
   if (Track[CurFrame][CurObject].active) 
      code = (MessageBox(g_MWin,
              "1 Extend Local to World?",
              "Verify", MB_YESNO)); else
      code = (MessageBox(g_MWin,
              "2 Insert new World key?\n[Yes] - World\n[No] - Local",
              "Verify", MB_YESNOCANCEL));
   
   if (code == IDCANCEL) return;

   if (Track[CurFrame][CurObject].active) {
        if (code==IDNO) return;
        for (int o=0; o<OCount; o++) {
          ActivateKey(CurFrame, o);          
        }
    } else {
        if (code==IDYES) 
         for (int o=0; o<OCount; o++) {          
          ActivateKey(CurFrame, o);    
         } else {
           ActivateKey(CurFrame, CurObject);
          }
    }
   
   RefreshFrame();   
}


void DoDeleteKey()
{
   int code,o;
     
   if ((!WorldKeyA[CurFrame] && !Track[CurFrame][CurObject].active) || CurFrame == 0) {MessageBeep(0xFFFFFFFF); return; }
   
   if (WorldKeyA[CurFrame]) 
      code = (MessageBox(g_MWin,
              "Delete World key?\n[Yes] - World\n[No] - Local","Verify", MB_YESNOCANCEL)); else
      code = (MessageBox(g_MWin,
              "Delete Local key","Verify", MB_YESNO));
   if (code == IDCANCEL) return;

   if (WorldKeyA[CurFrame]) {
    if (code == IDNO) Track[CurFrame][CurObject].active = FALSE;
      else 
     for (o=0; o<64; o++) 
      Track[CurFrame][o].active = FALSE; 
    } else {
    if (code == IDNO) return;
    Track[CurFrame][CurObject].active = FALSE;
    }
   
   UpdateTrackInfo();
   for (o=0; o<OCount; o++) Track[MaxFrame][o].active = TRUE;
   if (CurFrame>MaxFrame) CurFrame = MaxFrame;   
   RefreshFrame();
}
















//=========================== animation mathematics ==================================//


void RefreshObjectAni(int o, int CurFrame)
{
     int k1,k2;
     float k;

     for (k1=CurFrame-1; (k1>=0 && !Track[k1][o].active); k1--);
     for (k2=CurFrame+1; (k2<=MaxFrame && !Track[k2][o].active); k2++);      
     float t = (float)(CurFrame - k1);
     float T = (float)(k2 - k1);

     //======= acceleration =========================================//

     t /= T;

     float a = (float)Track[k1][o].acceleration / 32.f;     
     float v = 1 - a;     

     k =  a*t*t + v*t;     

     //======= create current key ===================================//
   
     FrameAni[o].Xfi = Track[k1][o].Xfi + (Track[k2][o].Xfi - Track[k1][o].Xfi) * k;
     FrameAni[o].Yfi = Track[k1][o].Yfi + (Track[k2][o].Yfi - Track[k1][o].Yfi) * k;
     FrameAni[o].Zfi = Track[k1][o].Zfi + (Track[k2][o].Zfi - Track[k1][o].Zfi) * k;

     FrameAni[o].ddx = Track[k1][o].ddx + (Track[k2][o].ddx - Track[k1][o].ddx) * k;
     FrameAni[o].ddy = Track[k1][o].ddy + (Track[k2][o].ddy - Track[k1][o].ddy) * k;
     FrameAni[o].ddz = Track[k1][o].ddz + (Track[k2][o].ddz - Track[k1][o].ddz) * k;

     FrameAni[o].scx = Track[k1][o].scx + (Track[k2][o].scx - Track[k1][o].scx) * k;
     FrameAni[o].scy = Track[k1][o].scy + (Track[k2][o].scy - Track[k1][o].scy) * k;
     FrameAni[o].scz = Track[k1][o].scz + (Track[k2][o].scz - Track[k1][o].scz) * k;

	 FrameAni[o].active = Track[CurFrame][o].active;
	 FrameAni[o].acceleration = Track[CurFrame][o].acceleration;
	 Track[CurFrame][o] = FrameAni[o];
}


void RefreshFrameAni()
{
   int o;
      
   for (o=0; o<OCount; o++) {
     if (DisableVT & (!ObjInWork[o])) continue;
     if (Track[CurFrame][o].active) {
       FrameAni[o] = Track[CurFrame][o]; 
       continue;
     }

     RefreshObjectAni(o, CurFrame);     
   }
}



void MulMatrix(TMatrix M1, TMatrix M2, TMatrix& Mr) 
{
  
  Mr[0][0] = M1[0][0] * M2[0][0] + M1[0][1] * M2[1][0] + M1[0][2] * M2[2][0];
  Mr[0][1] = M1[0][0] * M2[0][1] + M1[0][1] * M2[1][1] + M1[0][2] * M2[2][1];
  Mr[0][2] = M1[0][0] * M2[0][2] + M1[0][1] * M2[1][2] + M1[0][2] * M2[2][2];

  Mr[1][0] = M1[1][0] * M2[0][0] + M1[1][1] * M2[1][0] + M1[1][2] * M2[2][0];
  Mr[1][1] = M1[1][0] * M2[0][1] + M1[1][1] * M2[1][1] + M1[1][2] * M2[2][1];
  Mr[1][2] = M1[1][0] * M2[0][2] + M1[1][1] * M2[1][2] + M1[1][2] * M2[2][2];

  Mr[2][0] = M1[2][0] * M2[0][0] + M1[2][1] * M2[1][0] + M1[2][2] * M2[2][0];
  Mr[2][1] = M1[2][0] * M2[0][1] + M1[2][1] * M2[1][1] + M1[2][2] * M2[2][1];
  Mr[2][2] = M1[2][0] * M2[0][2] + M1[2][1] * M2[1][2] + M1[2][2] * M2[2][2];

}

void CalcMatrix(float Fx, float Fy, float Fz)
{
  //Matrix Mx, My, Mz, Mt;
  float cx,sx,cy,sy,cz,sz;

  cx=(float)cos(Fx);  sx=(float)sin(Fx);
  cy=(float)cos(Fy);  sy=(float)sin(Fy);
  cz=(float)cos(Fz);  sz=(float)sin(Fz);

/*
  Mx[0][0] = 1; Mx[0][1] = 0; Mx[0][2] =  0;
  Mx[1][0] = 0; Mx[1][1] =cx; Mx[1][2] =-sx;
  Mx[2][0] = 0; Mx[2][1] =sx; Mx[2][2] = cx;  

  My[0][0] = cy; My[0][1] = 0; My[0][2] =-sy;
  My[1][0] =  0; My[1][1] = 1; My[1][2] = 0;
  My[2][0] = sy; My[2][1] = 0; My[2][2] = cy; 

  Mz[0][0] = cz; Mz[0][1] =-sz; Mz[0][2] = 0;
  Mz[1][0] = sz; Mz[1][1] = cz; Mz[1][2] = 0;
  Mz[2][0] =  0; Mz[2][1] =  0; Mz[2][2] = 1; 
  
  MulMatrix(Mx,My, Mt);
  MulMatrix(Mz, Mt, M); */
  

  //if (RotSequence==0) {
  sy *=-1;
  //                                          ZXY
  M[0][0] = cz*cy+sz*sx*sy;  M[0][1] = -sz*cx;           M[0][2]=-cz*sy+sz*sx*cy;
  M[1][0] = sz*cy-cz*sx*sy;  M[1][1] = cz*cx;            M[1][2]=-sz*sy-cz*sx*cy;
  M[2][0] = cx*sy;           M[2][1] = sx;               M[2][2]= cx*cy; 
  /*} else {
  sy *=-1;
  //                                          XZY
  M[0][0] = cz*cy;            M[0][1] = -sz;          M[0][2] = -cz*sy;
  M[1][0] = cx*sz*cy-sx*sy;   M[1][1] = cx*cz;        M[1][2] = -sy*sz*cx-sx*cy;
  M[2][0] = sx*sz*cy+cx*sy;   M[2][1] = sx*cz;        M[2][2] = -sy*sz*sx+cx*cy;  
  }
  */
/*
//										   XYZ
   M[0][0] = cy*cz;			   M[0][1] = -cy*sz;		   M[0][2] = sy;
   M[1][0] = sx*sy*cz+cx*sz;   M[1][1] = -sx*sy*sz+cx*cz;  M[1][2] = -sx*cy;
   M[2][0] = -cx*sy*cz+sx*sz;  M[2][1] = cx*sy*sz+sx*cz;   M[2][2] = cx*cy;
*/
}






void RotatePoint(float &xx,float &yy,float &zz)
{
  float x=xx-acx; 
  float y=yy-acy; 
  float z=zz-acz;

  xx = x*M[0][0] + y*M[0][1] + z*M[0][2] + acx;
  yy = x*M[1][0] + y*M[1][1] + z*M[1][2] + acy;
  zz = x*M[2][0] + y*M[2][1] + z*M[2][2] + acz;
}



/*
void _RotateChild(int papa)
{  
  for (int n=0; n<OCount; n++) 
   if (gObj[n].owner == papa) {    
	 MulMatrix(M, gMatrix[n], Mt);
	 memcpy(&gMatrix[n], Mt, sizeof(Mt));
     RotatePoint(gObj[n].ox, gObj[n].oy, gObj[n].oz);    
    _RotateChild(n);
   } 
}*/

void RotateChild(int papa)
{
  int n,i;
  for (n=0; n<OCount; n++) 
   if (gObj[n].owner==papa) {    
     RotatePoint(gObj[n].ox, gObj[n].oy, gObj[n].oz);
    if (!DisableVT)
     for (i=0; i<VCount; i++)
       if (gVertex[i].owner==n) 
        RotatePoint(gVertex[i].x, gVertex[i].y, gVertex[i].z);
      RotateChild(n);
   } 
}


void RotateObject(int no)
{
 int s;

 if (fabs(FrameAni[no].Xfi) < 0.001f && 
     fabs(FrameAni[no].Yfi) < 0.001f && 
     fabs(FrameAni[no].Zfi) < 0.001f) return;

 CalcMatrix(FrameAni[no].Xfi*pi/180.f,
            FrameAni[no].Yfi*pi/180.f,
            FrameAni[no].Zfi*pi/180.f);             
 
 acx = gObj[no].ox;
 acy = gObj[no].oy;
 acz = gObj[no].oz;
 
 s=gObj[no].owner; 
 gObj[no].owner = 255;
 RotateChild(255);
 gObj[no].owner = s;
}




//================================ moving ==========================//
void MoveChild(int papa)
{
  int n,i;
  for (n=0; n<OCount; n++) 
   if (gObj[n].owner==papa) {
    gObj[n].ox += acx;
    gObj[n].oy += acy;
    gObj[n].oz += acz;       

   if (!DisableVT)
    for (i=0; i<VCount; i++) 
      if (gVertex[i].owner==n) {
        gVertex[i].x+=acx;
        gVertex[i].y+=acy;
        gVertex[i].z+=acz;
      }
    MoveChild(n);
  }
}

void MoveObject(int o)
{
  int s;
  acx = FrameAni[o].ddx;
  acy = FrameAni[o].ddy;
  acz = FrameAni[o].ddz;
  
  if (fabs(acx)<0.01f &&
	  fabs(acy)<0.01f &&
	  fabs(acz)<0.01f	  ) return;

  s = gObj[o].owner;
  gObj[o].owner = 255;
  MoveChild(255);
  gObj[o].owner = s;
}
//======================================================================//


void _MoveObjectV(int o) 
{	
  if (fabs(ODelta[o].x)<0.01f &&
	  fabs(ODelta[o].y)<0.01f &&
	  fabs(ODelta[o].z)<0.01f	  ) return;

   gObj[o].ox+=ODelta[o].x;
   gObj[o].oy+=ODelta[o].y;
   gObj[o].oz+=ODelta[o].z;

   if (!DisableVT)
    for (int i=ovbegin[o]; i<VCount; i++) 
      if (gVertex[i].owner==o) {
        gVertex[i].x+=ODelta[o].x;
        gVertex[i].y+=ODelta[o].y;
        gVertex[i].z+=ODelta[o].z;
      } else break;
}

void _MoveChild(int papa)
{ 
  int n,i;
  for (n=0; n<OCount; n++) 
   if (gObj[n].owner==papa) {
    ODelta[n].x += acx;
    ODelta[n].y += acy;
    ODelta[n].z += acz;
    _MoveChild(n);

  }
}

void _MoveObject(int o)
{
  int s;

  acx = FrameAni[o].ddx;
  acy = FrameAni[o].ddy;
  acz = FrameAni[o].ddz;

  if (fabs(acx)<0.01f &&
	  fabs(acy)<0.01f &&
	  fabs(acz)<0.01f	  ) return;

  s = gObj[o].owner;
  gObj[o].owner = 255;
  _MoveChild(255);
  gObj[o].owner = s;
}







//============================ scaling =================================//


void ScaleObject(int o)
{
  int s;
  acx = gObj[o].ox;
  acy = gObj[o].oy;
  acz = gObj[o].oz;
    
  asx = FrameAni[o].scx;
  asy = FrameAni[o].scy;
  asz = FrameAni[o].scz;

  if ( fabs(asx-1.0f)<0.001 && 
	   fabs(asy-1.0f)<0.001 && 
	   fabs(asz-1.0f)<0.001) return;

  if (!DisableVT)
    for (int i=ovbegin[o]; i<VCount; i++) 
      if (gVertex[i].owner==o) {
        gVertex[i].x = (gVertex[i].x - acx) * asx + acx;
        gVertex[i].y = (gVertex[i].y - acy) * asy + acy;
        gVertex[i].z = (gVertex[i].z - acz) * asz + acz;
      } else break;

}
//======================================================================//





void _RotateObjectV(int o)
{
  int i;  
  if (DisableVT) return;
  
  acx = gObj[o].ox;
  acy = gObj[o].oy;
  acz = gObj[o].oz;
  
  memcpy(&M, &gMatrix[o], sizeof(M));

  for (i=ovbegin[o]; i<VCount; i++) 
	  if (gVertex[i].owner==o) {
		gVertex[i].x+=gObj[o].ox-oObj[o].ox;
		gVertex[i].y+=gObj[o].oy-oObj[o].oy;
		gVertex[i].z+=gObj[o].oz-oObj[o].oz;

        RotatePoint(gVertex[i].x, gVertex[i].y, gVertex[i].z);				
	  } else break;
  
}


void _RotateChild(int papa)
{  
  for (int n=0; n<OCount; n++)
   if (gObj[n].owner == papa) {
	 MulMatrix(M, gMatrix[n], Mt);
	 memcpy(&gMatrix[n], Mt, sizeof(Mt));
     RotatePoint(gObj[n].ox, gObj[n].oy, gObj[n].oz);
    _RotateChild(n);
   }
}


void _RotateObject(int no)
{
 int s;

 if (fabs(FrameAni[no].Xfi) < 0.001f && 
     fabs(FrameAni[no].Yfi) < 0.001f && 
     fabs(FrameAni[no].Zfi) < 0.001f) return;

 CalcMatrix(FrameAni[no].Xfi*pi/180.f,
            FrameAni[no].Yfi*pi/180.f,
            FrameAni[no].Zfi*pi/180.f);
 
 acx = gObj[no].ox;
 acy = gObj[no].oy;
 acz = gObj[no].oz;
 
 s=gObj[no].owner; 
 gObj[no].owner = 255;
 _RotateChild(255);
 gObj[no].owner = s;
}


void RefreshModel()
{
   int o;
	 
   //return;

   FillMemory(ODelta,  OCount*16, 0);
   FillMemory(ORDelta, OCount*16, 0);
   FillMemory(gMatrix, OCount*sizeof(TMatrix), 0);

   memcpy(gVertex, oVertex, VCount * sizeof(TPoint3d) );
   memcpy(gObj,    oObj,    OCount * sizeof(TObj)     );
   
   for (int m=0; m<OCount; m++) {
	   gMatrix[m] [0][0] = 1.0f;
	   gMatrix[m] [1][1] = 1.0f;
	   gMatrix[m] [2][2] = 1.0f;	   
   }
/*
   for (o=OCount-1; o>=0; o--) {
     if (DisableVT & (!ObjInWork[o])) continue;
       _RotateObject(o); }

   for (o=OCount-1; o>=0; o--)
      _RotateObjectV(o);
*/
/*
   for (o=0; o<OCount-1; o++) {
     if (DisableVT & (!ObjInWork[o])) continue;
       _RotateObject(o); }

   for (o=0; o<OCount-1; o++)
      _RotateObjectV(o);

   return;*/

   for (o=OCount-1; o>=0; o--) {
     if (DisableVT & (!ObjInWork[o])) continue;
       ScaleObject(o); }


if (GetAsyncKeyState(VK_SHIFT) & 0x8000)  {

   for (o=OCount-1; o>=0; o--) {
     if (DisableVT & (!ObjInWork[o])) continue;
       RotateObject(o); 
   }

} else {
   for (o=OCount-1; o>=0; o--) {
     if (DisableVT & (!ObjInWork[o])) continue;
       _RotateObject(o); }
   for (o=OCount-1; o>=0; o--)
       _RotateObjectV(o);
}

if (GetAsyncKeyState(VK_SHIFT) & 0x8000)  {

   for (o=OCount-1; o>=0; o--) {
    if (DisableVT & (!ObjInWork[o])) continue;
    MoveObject(o); }

} else {
	
   for (o=OCount-1; o>=0; o--) {
    if (DisableVT & (!ObjInWork[o])) continue;
    _MoveObject(o); }
   for (o=OCount-1; o>=0; o--)     
    _MoveObjectV(o); 

}

   DisableVT = FALSE;
}



BOOL ProcessMouseK()
{
	MSG msg;
    PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE );
    if (msg.message == WM_LBUTTONDOWN) return TRUE;
    if (msg.message == WM_RBUTTONDOWN) return TRUE;
    return FALSE;
}


unsigned int LastATime;
void PlayAni()
{
  if (MaxFrame==0) return;
  if (timeGetTime()<LastATime) return;  
  BOOL b = TRUE;
  int c = CurFrame;
  unsigned int FTime;
  int MK;
  if (LoopPlay) MK = MaxFrame-1; else MK = MaxFrame;
  for ( ; ; ) {
    FTime = timeGetTime() + 1000 / aniKPS;
    //UpdateKeyBar();
    //CheckCurFrame();
    RefreshFrameAni();
    RefreshModel();
    UpdateEditorWindow(CurEditor);
    if (ProcessMouseK()) break;
    
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 ||
        GetAsyncKeyState(VK_SPACE ) & 0x8000  ) 
        { if (!b) break; } else b = FALSE;

    CurFrame++;
    if (CurFrame > MK) {
     CurFrame = 0;
     if (!LoopPlay) break;
    }
    while (timeGetTime() < FTime) ;
  }
  if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 || !LoopPlay) CurFrame = c;
  RefreshFrame();
  LastATime = timeGetTime() + 100;
}




BOOL ProcessMouse()
{	
	MSG msg;
    PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE );

    if (msg.message == WM_LBUTTONDOWN) return TRUE;

        

    POINT p;    
	GetCursorPos(&p);

    int dx = p.x - 400;
    int dy = p.y - 300;

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { 
	    g_3dwininfo[CurEditor].Scaler*=(float)(1.0+float(dy)/100);
		if (g_3dwininfo[CurEditor].Scaler>100) g_3dwininfo[CurEditor].Scaler=(float)100;
		if (g_3dwininfo[CurEditor].Scaler<0.1) g_3dwininfo[CurEditor].Scaler=(float)0.1; 
     } else  
	if ( (GetAsyncKeyState(VK_MENU) & 0x8000) || (CurEditor!=3)) {
		g_3dwininfo[CurEditor]._xc0 -= (dx) / g_3dwininfo[CurEditor].Scaler;
		g_3dwininfo[CurEditor]._yc0 += (dy) / g_3dwininfo[CurEditor].Scaler; 		 
	} else {         
         if (CurEditor==3) {
	      g_alpha-=(float)(dx)/100;
	      g_beta-=(float)(dy)/100;	
         }
     }

	SetCursorPos(400,300);
    return FALSE;
}

void CalcIKBasisesBy2Links(IKLink *Links, IKLink *Links2)
{
	 int iobj[256];
	 int o;
	 ZeroMemory(iobj, sizeof(iobj));
	 for (o=OCount-1; o>0; o--) {  // SetNew Vectors
		 int n1 = o;
		 int n2 = Links[n1].owner;
		 int n3 = Links[n2].owner;
		 if (n1==-1) continue;
		 if (n2==-1) continue;
		 if (n3==-1) continue;

		 if (iobj[n2]) continue;
		 iobj[n2] = 1;

		 Vector3d oldV = Links2[n1].v;
		 Vector3d newV = Links[n1].v;
		 
		 float OldVLen = VectAbs(oldV);	if (OldVLen==0.f) OldVLen = 0.00000000001;
		 float NewVLen = VectAbs(newV);	if (NewVLen==0.f) NewVLen = 0.00000000001;
		 
		 ScaleVector(oldV, 1.f/OldVLen);
		 ScaleVector(newV, 1.f/NewVLen);
 
		 int  BasisChanged = CalcNewObjBasis3DR(n1,n2,n3, oldV, newV);
	 }
}

void CheckIKLinksDist(int n, TObj *Obj, IKLink *Links)
{
	 for(int ch = 0; ch<Links[n].chCnt; ch++) {
		 int child = Links[n].children[ch];
		 Vector3d real_v = SubVectors(Obj[child].pos, Obj[n].pos);
		 float real_d = VectAbs(real_v);
		 Vector3d move_v = ScaledVector(Links[child].v, (real_d-Links[child].d));
		 Obj[child].pos = AddVectors(Obj[child].pos, move_v);
		 CheckIKLinksDist(child, Obj, Links);
	 }
}

void SetIKBasisAngels(int n, float Xfi, float Yfi, float Zfi)
{
	 TObj gObj2[256];
	 IKLink   Links2[256];

	 int shift_state = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

	 RefreshFrameAni();
	 RefreshModel();
	 BuildLinks3D(Links, gObj);

	 int o;
	 for (o=OCount-1; o>=0; o--) {
		 gObj2[o] = gObj[o];
	 }

     RecalcGlobalBasisesFromAngles(Basis);

	 int own = Links[n].owner;

	 if (own==-1) return;

	 // set basis
	 Vector3d oX,oY,oZ;
	 CalcMatrix(Xfi,Yfi,Zfi);
	 TransposeMatrix();		// get new restricted basis
	 SET_VECTOR(oX, M[0][0], M[0][1], M[0][2]);
	 SET_VECTOR(oY, M[1][0], M[1][1], M[1][2]);
	 SET_VECTOR(oZ, M[2][0], M[2][1], M[2][2]);

	 TBasis NewBas;
	 NewBas.X = GetVectorInGlobal(Basis[own], oX);  // new Basis
	 NewBas.Y = GetVectorInGlobal(Basis[own], oY);
 	 NewBas.Z = GetVectorInGlobal(Basis[own], oZ);

	 // rotate child object positions
	 for(int ch = 0; ch<Links[n].chCnt; ch++) {
		 int child = Links[n].children[ch];
		 Vector3d v = ScaledVector(Links[child].v, Links[child].d);
		 Vector3d loc_v = GetVectorInLocal(Basis[n], v);
		 Vector3d new_v = GetVectorInGlobal(NewBas, loc_v);
		 gObj2[child].pos = SubVectors(gObj[n].pos, new_v);
	 }

	 if (!shift_state) {
		 
		 CheckIKLinksDist(CurObject, gObj2, Links);
		 
		 BuildLinks3D(Links2, gObj2);  // buildNew Links
		 
		 CalcIKBasisesBy2Links(Links2, Links);
	 }

	 Basis[n] = NewBas;

	 StartJoint = n;
	 StartIKJoint = 0;
	 int n2 = Links[StartJoint].owner;
	 //if (n2==-1) CalcObjectAnglesByBasis(StartJoint, IdentityBasis, 0);
	 if (n2==-1) CalcObjectAnglesByBasis(StartJoint, IdentityBasis, 0);
	 else CalcObjectAnglesByBasis(StartJoint, Basis[n2],0);
	 
	 UpdateObjPosFromBasis(gObj, StartJoint);

	 RefreshFrameAni();
	 RefreshModel();
}

void CalcAutoTrackAngles()
{
  Vector3d  ObjData[MAXKEY][128];
  Vector3d  ObjVec[MAXKEY][128];

  IKLink   Links2[256];
  int CurFrame_ = CurFrame;
  for (CurFrame=0; CurFrame<=MaxFrame; CurFrame++) {

     //CurFrame=0;
	 RefreshFrameAni();
	 RefreshModel();
	 RecalcGlobalBasisesFromAngles(Basis);
	 BuildLinks3D(Links, gObj);
	 FillMemory(NeedTrackCure, sizeof(NeedTrackCure), 0);
	 /*for (int o=1; o<OCount-1; o++) {
		 if (Links[o].chCnt<=0) continue;
		  	Track[CurFrame][o].ddx = 0;
			Track[CurFrame][o].ddy = 0;
			Track[CurFrame][o].ddz = 0;
			NeedTrackCure[o]=1;
	 }*/
	 for (int o=1; o<OCount-1; o++) {
		 if (Links[o].chCnt<=0) continue;

		 if (Track[CurFrame][o].active==0) continue;
		 int fr = CurFrame-1;
		 while (fr>0 && Track[fr][o].active==0) fr--;
		 if (fr<0) fr=0;
		 float Xfi1 = Track[fr][o].Xfi*pi/180;
		 float Yfi1 = Track[fr][o].Yfi*pi/180;
		 float Zfi1 = Track[fr][o].Zfi*pi/180;

		 float Xfi2 = Track[CurFrame][o].Xfi*pi/180;
		 float Yfi2 = Track[CurFrame][o].Yfi*pi/180;
		 float Zfi2 = Track[CurFrame][o].Zfi*pi/180;
		 
		 Vector3d dist;
		 /*dist.x = SubAngles(Xfi1, Xfi2);
		 dist.y = SubAngles(Yfi1, Yfi2);
		 dist.z = SubAngles(Zfi1, Zfi2);*/

		 dist.x = fabs(Xfi1-Xfi2)*180/pi;
		 dist.y = fabs(Yfi1-Yfi2)*180/pi;
		 dist.z = fabs(Zfi1-Zfi2)*180/pi;

		 if (dist.x<DefAngleX && dist.y<DefAngleY && dist.z<DefAngleZ) continue;
		 //float angle = VectAbs(dist)*180/pi;
		 //if (angle<180) continue;

		  	Track[CurFrame][o].ddx = 0;
			Track[CurFrame][o].ddy = 0;
			Track[CurFrame][o].ddz = 0;
		  	Track[CurFrame][o].Xfi = 0;
			Track[CurFrame][o].Yfi = 0;
			Track[CurFrame][o].Zfi = 0;
		  	Track[CurFrame][o].scx = 1;
			Track[CurFrame][o].scy = 1;
			Track[CurFrame][o].scz = 1;
			NeedTrackCure[o]=1;
			MessageBeep(-1);
	 }

 	 RefreshFrameAni();
	 RefreshModel();
/*
	 BuildLinks3D(Links2, gObj);
     RecalcGlobalBasisesFromAngles(Basis);

	 CalcIKBasisesBy2Links(Links, Links2);*/

	 StartJoint = 0;
	 StartIKJoint = 0;
	 int n2 = Links[StartJoint].owner;
	 if (n2==-1) CalcObjectAnglesByBasis(StartJoint, IdentityBasis, 0, 1);
	 //else CalcObjectAnglesByBasis(StartJoint, Basis[n2]);
 
	 UpdateObjPosFromBasis(gObj, StartJoint);
  }
  CurFrame = CurFrame_;
}

void PlayAniRealTime2()
{
  if (timeGetTime()<LastATime) return;  
  BOOL b = TRUE;
  int c = CurFrame;
  TPoint3d* AniData[MAXKEY];
  Vector3d  ObjData[MAXKEY][128];

  if (MaxFrame==0) return;

  ShowCursor(FALSE);
  SetCursorPos(400,300);  

  for (CurFrame=0; CurFrame<=MaxFrame; CurFrame++) {
    RefreshFrameAni();
    RefreshModel();
    AniData[CurFrame] = (TPoint3d*) HeapAlloc(GHeap, HEAP_ZERO_MEMORY, 16*VCount);
    memcpy(AniData[CurFrame], gVertex, 16*VCount);
    for (int o=0; o<OCount; o++)
        memcpy(&ObjData[CurFrame][o].x, &gObj[o].ox, 3*4);
  } 
  int FTime;
  int AniTime = MaxFrame * 1000 / aniKPS;
  int T0;
  int SplineD;
  
  FTime = 0;
  MouseCaptured = 1;
  for ( ; ; ) {
    T0 = timeGetTime();           
    
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 ||
        GetAsyncKeyState(VK_SPACE ) & 0x8000  ) 
        { if (!b) break; } else b = FALSE;

	if (ProcessMouse()) break;

    CurFrame = (MaxFrame * FTime * 256) / AniTime;
    SplineD = CurFrame & 0xFF;
    CurFrame = (CurFrame>>8);
    
    float k = (float)(SplineD) / 256.f;
    memcpy(gVertex, AniData[CurFrame], 16*VCount);
    for (int v=0; v<VCount; v++) {
      gVertex[v].x+= (AniData[CurFrame+1][v].x - AniData[CurFrame][v].x) * k;
      gVertex[v].y+= (AniData[CurFrame+1][v].y - AniData[CurFrame][v].y) * k;
      gVertex[v].z+= (AniData[CurFrame+1][v].z - AniData[CurFrame][v].z) * k;
    }

    for (int o=0; o<OCount; o++) {
      gObj[o].ox= ObjData[CurFrame][o].x*(1-k) + ObjData[CurFrame+1][o].x * k;
      gObj[o].oy= ObjData[CurFrame][o].y*(1-k) + ObjData[CurFrame+1][o].y * k;      
      gObj[o].oz= ObjData[CurFrame][o].z*(1-k) + ObjData[CurFrame+1][o].z * k;      
    }

    UpdateEditorWindow(CurEditor);

    FTime+=timeGetTime() - T0;
    if (FTime >= AniTime) {
      FTime=0;
      if (!LoopPlay) break;
    }
  }

  for (CurFrame=0; CurFrame<=MaxFrame; CurFrame++) 
    HeapFree(GHeap, 0, AniData[CurFrame]);

  MouseCaptured = 0;
  CurFrame = c;

  RefreshFrame();
  LastATime = timeGetTime() + 100;
  ShowCursor(TRUE);
}

void PlayAniRealTime()
{
  if (timeGetTime()<LastATime) return;  
  BOOL b = TRUE;
  int c = CurFrame;
  TPoint3d* AniData[MAXKEY];
  Vector3d  ObjData[MAXKEY][128];

  if (MaxFrame==0) return;

  ShowCursor(FALSE);
  SetCursorPos(400,300);  

  for (CurFrame=0; CurFrame<=MaxFrame; CurFrame++) {
    RefreshFrameAni();
    RefreshModel();
    AniData[CurFrame] = (TPoint3d*) HeapAlloc(GHeap, HEAP_ZERO_MEMORY, 16*VCount);
    memcpy(AniData[CurFrame], gVertex, 16*VCount);
    for (int o=0; o<OCount; o++)
        memcpy(&ObjData[CurFrame][o].x, &gObj[o].ox, 3*4);
  }
  
 
  int FTime;
  int AniTime = MaxFrame * 1000 / aniKPS;
  int T0;
  int SplineD;
  
  FTime = 0;
  MouseCaptured = 1;
  for ( ; ; ) {
    T0 = timeGetTime();           
    
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 ||
        GetAsyncKeyState(VK_SPACE ) & 0x8000  ) 
        { if (!b) break; } else b = FALSE;

	if (ProcessMouse()) break;

    CurFrame = (MaxFrame * FTime * 256) / AniTime;
    SplineD = CurFrame & 0xFF;
    CurFrame = (CurFrame>>8);
    
    float k = (float)(SplineD) / 256.f;
    memcpy(gVertex, AniData[CurFrame], 16*VCount);
    for (int v=0; v<VCount; v++) {
      gVertex[v].x+= (AniData[CurFrame+1][v].x - AniData[CurFrame][v].x) * k;
      gVertex[v].y+= (AniData[CurFrame+1][v].y - AniData[CurFrame][v].y) * k;
      gVertex[v].z+= (AniData[CurFrame+1][v].z - AniData[CurFrame][v].z) * k;
    }

    for (int o=0; o<OCount; o++) {
      gObj[o].ox= ObjData[CurFrame][o].x*(1-k) + ObjData[CurFrame+1][o].x * k;
      gObj[o].oy= ObjData[CurFrame][o].y*(1-k) + ObjData[CurFrame+1][o].y * k;      
      gObj[o].oz= ObjData[CurFrame][o].z*(1-k) + ObjData[CurFrame+1][o].z * k;      
    }

    UpdateEditorWindow(CurEditor);

    FTime+=timeGetTime() - T0;
    if (FTime >= AniTime) {
      FTime=0;
      if (!LoopPlay) break;
    }
  }

  for (CurFrame=0; CurFrame<=MaxFrame; CurFrame++) 
    HeapFree(GHeap, 0, AniData[CurFrame]);

  MouseCaptured = 0;
  CurFrame = c;

  RefreshFrame();
  LastATime = timeGetTime() + 100;
  ShowCursor(TRUE);
}