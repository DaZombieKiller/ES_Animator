#include <windows.h>    // includes basic windows functionality
#include <stdio.h>
#include <math.h>
#include "animator.h"   // includes application-specific information



void Delay(unsigned dt)
{
   unsigned t = GetTickCount() + dt;

   while( GetTickCount() < t ) ;
}

void Line( HDC hdc, int x1, int y1, int x2, int y2 )
{
	MoveToEx( hdc, x1,y1, NULL );
	LineTo( hdc, x2, y2 );
}

void DrawRect( HDC hdc, int x1, int y1, int x2, int y2 )
{
   Line( hdc, x1,y1, x2, y1 );
   Line( hdc, x2,y1, x2, y2+1 );
   Line( hdc, x2,y2, x1, y2 );
   Line( hdc, x1,y2, x1, y1 );
}

void DrawKey( HDC hdc, int x1, int y1, int x2, int y2, HPEN penA, HPEN penB )
{
	HPEN hpenOld = (HPEN)SelectObject( hdc,penA );
	Line( hdc, x1,y2, x1, y1 );
   Line( hdc, x1,y1, x2, y1 );

   SelectObject( hdc,penB );
   Line( hdc, x2,y1, x2, y2+1 );
	Line( hdc, x2,y2, x1, y2 );

   SelectObject( hdc, hpenOld );
}

void Draw3dKey( HDC hdc, int x1, int y1, int x2, int y2, int h, HPEN penA, HPEN pen, HPEN penB )
{
   HPEN hpenOld = 0;

   if( pen!=0 ) {
	   hpenOld = (HPEN)SelectObject( hdc,pen );
      for( int y=y1; y<=y2; y++ )
		   Line( hdc, x1, y, x2+1, y );
   }

   for( int i=0; i<h; i++ )
	   DrawKey( hdc, x1+i, y1+i, x2-i, y2-i, penA, penB );

   if( hpenOld )
      SelectObject( hdc, hpenOld );
}

void DrawBar( HDC hdc, int x1, int y1, int x2, int y2 )
{
	for( int y=y1; y<=y2; y++ )
		Line( hdc, x1, y, x2+1, y );
}





void DrawNormalView( HDC hdc )
{
	
    HBRUSH hbrOld = (HBRUSH)SelectObject( hdc, g_mybrush[33] );
	PatBlt( hdc, 1, 1, w-1, h-1, PATCOPY );
	
	HPEN hpenOld = (HPEN)SelectObject( hdc, g_hpen7 );

	int xsspeed, ysspeed;
	switch (g_ew) {
	case 0: xsspeed = 0;  ysspeed = 0; break;
	case 1: xsspeed = 26; ysspeed = 0; break;
	case 2: xsspeed = 0;  ysspeed =-26; break;
	}

	xsspeed = 0;
	ysspeed = 0;

	//====== grid ==========//
	if( g_3dwininfo[g_ew].GridStep > 0 )  {
	  int gs = g_3dwininfo[g_ew].GridStep;
      int gcnt = (int) ((float)(w)/(float)0.5/(float)gs/g_scaler) +1 ;
	   int gx; int gy;

	   if (gcnt<(w>>1)) 
	      if (g_3dwininfo[g_ew].GridLines) {			  
	         for (gx=-gcnt; gx<=gcnt; gx++) { 
		         x = g_xc + (int) ( (gx * gs - g_3dwininfo[g_ew]._xc0 - xsspeed * CurFrame) * g_scaler);  
                 Line(hdc,x,0,x,h); 
			 }
	         for (gy=-gcnt; gy<=gcnt; gy++) { 
                  y = g_yc - (int) ( (gy * gs - g_3dwininfo[g_ew]._yc0 - ysspeed * CurFrame) * g_scaler);  
                  Line(hdc,0,y,w,y); 
             }
	         } 
    }
 
 //============== BP =======================//
	int bpspeed = 26;
	bpspeed = 0;

    ConvertPoint( bp.x, bp.y, bp.z + bpspeed * CurFrame, x, y );
 	SelectObject( hdc, hpenBP );
    SetBkMode( hdc, TRANSPARENT );
	if (((int)(bp.x) != 0) || 
		((int)(bp.y) != 0) || 
		((int)(bp.z) != 0)  ) {
     Line( hdc, x, 0, x, h );
	 Line( hdc, 0, y, w, y );
	}

 //============== Center Lines =======================//

	SelectObject(hdc,g_hpen5);
	if (g_3dwininfo[g_ew].CenterLine) {
      int xc = g_xc - int( (g_3dwininfo[g_ew]._xc0 + xsspeed * CurFrame) * g_scaler);
      int yc = g_yc + int( (g_3dwininfo[g_ew]._yc0 + ysspeed * CurFrame) * g_scaler);
      Line(hdc,xc,0,xc,h);
      Line(hdc,0,yc,w,yc); 
	}

 //============== path ===============================//
    SelectObject( hdc, hpenGreen);
    SelectObject( hdc, hbrGreen);
    int xx,yy;
   if (PathObject!=-1) {
    ConvertPoint(ObjPath[PathOBegin].x,ObjPath[PathOBegin].y,ObjPath[PathOBegin].z, xx, yy);
    for (int k=PathOBegin; k<=PathOEnd; k++) {
      ConvertPoint(ObjPath[k].x,ObjPath[k].y,ObjPath[k].z, x, y);
      Line(hdc, xx, yy, x, y);
      xx = x; yy =y;
       if (ObjPath[k].owner) {
        Arc( hdc, x-4, y-4, x+5, y+5, x-4, y, x-4, y );        
        PatBlt(hdc, x-1, y-1, 3, 3, PATCOPY); 
       } else PatBlt(hdc, x-1, y-1, 3, 3, PATCOPY); 
    } 
   }

 //=======================================================

    for (int vv=0; vv<VCount; vv++)
	   ConvertPoint(gVertex[vv].x,gVertex[vv].y,gVertex[vv].z, 
         (int&)scrc[vv].x, (int&)scrc[vv].y
      );
   
   if (SHOWMODEL)
	for (int d=0; d<2; d++) {
      if( d==0 ) SelectObject( hdc,g_hpen6 ); 
      else       SelectObject( hdc,g_hpenE );

	   for (int f=0; f<FCount; f++) {
	      int v1 = gFace[f].v1;
	      int v2 = gFace[f].v2;
	      int v3 = gFace[f].v3;
         if( gVertex[ v1 ].hide || gVertex[ v2 ].hide || gVertex[ v3 ].hide )
            continue; // one of the face points is hidden

		  if (gFace[f].Flags & DamageMask) continue;

	      POINT v[4];
	      v[0] = scrc[v1]; v[1] = scrc[v2]; v[2] = scrc[v3]; v[3]=v[0];							

	      int ux = v[1].x-v[0].x;
	      int uy = v[1].y-v[0].y;
	      int vx = v[2].x-v[1].x;
	      int vy = v[2].y-v[1].y;	
	
  	      if (d==0) {
            if( (ux*vy - uy*vx)<0)
               Polyline(hdc,v,4);
         } else {
            if( (ux*vy - uy*vx) >=0 )
               Polyline(hdc,v,4);
         }
	   }
	}
   
   SelectObject( hdc, hpenBlue );
   if( g_3dwininfo[g_ew].ShowObj )
      for( int o=0; o<OCount; o++ ) {
         if( gObj[ o ].hide ) continue;         

         ConvertPoint(gObj[o].ox, gObj[o].oy, gObj[o].oz, x, y);         
         scro[o].x = x;
         scro[o].y = y;

         if( o == CurObject ) continue;
         if (Track[CurFrame][o].active) {          
          SelectObject( hdc, hpenMagenta);         
          SelectObject( hdc, hbrMagenta);
         } else {
          SelectObject( hdc, hpenBlue );
          SelectObject( hdc, hbrBlue );         
         }
		 //RenderCircle(x,y, 30, color);
         Arc( hdc, x-4, y-4, x+5, y+5, x-4, y, x-4, y );         
         PatBlt(hdc, x-1, y-1, 3, 3, PATCOPY);
      }

//============== draw sceleton ==================//
   SelectObject( hdc, hpenMagenta );
  if (SHOWSCELETON)
   if( g_3dwininfo[g_ew].ShowObj )
      for( int o=0; o<OCount; o++ ) {
         if( gObj[o].hide ) continue;
         if(gObj[o].owner==-1) continue;
         if( gObj[gObj[o].owner].hide ) continue;

         Line(hdc, scro[o].x, scro[o].y, scro[gObj[o].owner].x, scro[gObj[o].owner].y);
      }

   if( OCount && CurObject != -1 ) {
      SelectObject( hdc, hpenRed );
      SelectObject( hdc, hbrRed );
      ConvertPoint( gObj[CurObject].ox, gObj[CurObject].oy, gObj[CurObject].oz, x, y );
      Arc( hdc, x-4, y-4, x+5, y+5, x-4, y, x-4, y );

  
      Rectangle( hdc, x-1, y-1, x+2, y+2 );
   }

	SelectObject( hdc, g_mybrush[5*4] );
   if (SHOWMODEL)
	if( g_3dwininfo[g_ew].PointType )
      for( vv=0; vv<VCount; vv++ )
         if( !gVertex[ vv ].hide )	 
			 Put3Point(hdc,scrc[vv].x,scrc[vv].y);



   SelectObject( hdc, hbrOld );
   SelectObject( hdc, hpenOld );
}




void DrawEditorWindow( HWND hWnd, HDC _hdc, HDC hdc )
{
   RECT rc;

	g_ew = -1;
	for (int i=0; i<4; i++) {
      if (hWnd==g_3dwin[i]) {
         g_ew=i;
         break;
      }
   }

   if (g_3dwininfo[g_ew].wflags & 0x0001) {
      return;
   }

    GetClientRect( hWnd, &rc );
	w = rc.right-rc.left-1;
	h = rc.bottom-rc.top-1;
	g_xc=w>>1; 
    g_yc=h>>1;
	g_scaler=g_3dwininfo[g_ew].Scaler;
    g_3dwininfo[g_ew]._xc=g_xc;
    g_3dwininfo[g_ew]._yc=g_yc;

    if (g_ew==3) {
        ActivateD3D(hWnd);
        Renderd3dView();
        if (!MouseCaptured)
           ShutDownD3D();
        HPEN hpenOld = (HPEN)SelectObject(_hdc,g_hpen7);
        if( g_ew!=CurEditor ) DrawKey( _hdc, 0,0,w,h,g_hpen3dDn,g_hpen3dUp );	
	                     else DrawKey( _hdc, 0,0,w,h,g_hpen3dUp,g_hpen3dDn );	
        SelectObject( _hdc, hpenOld );
        return;
    }


    {
        ActivateD3D(hWnd);
        Renderd2dView();
        if (!MouseCaptured)
           ShutDownD3D();
        HPEN hpenOld = (HPEN)SelectObject(_hdc,g_hpen7);
        if( g_ew!=CurEditor ) DrawKey( _hdc, 0,0,w,h,g_hpen3dDn,g_hpen3dUp );	
	                     else DrawKey( _hdc, 0,0,w,h,g_hpen3dUp,g_hpen3dDn );	
        SelectObject( _hdc, hpenOld );
        return;
    }



  	HPEN hpenOld = (HPEN)SelectObject(hdc,g_hpen7);
	HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, g_mybrush[9*4]);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hdc, g_MemBMP);

	if ( g_ew!=3 ) DrawNormalView( hdc );

	
   SetBkMode(hdc,TRANSPARENT);
   if( g_ew!=CurEditor ) 
      SetTextColor(hdc, 0x00101010);
	else 
      SetTextColor(hdc, 0x00E0E0E0);
   TextOut(hdc,6,2,g_3dwininfo[g_ew].Title,lstrlen(g_3dwininfo[g_ew].Title));

   if( g_ew!=CurEditor ) DrawKey( hdc, 0,0,w,h,g_hpen3dDn,g_hpen3dUp );	
	                else DrawKey( hdc, 0,0,w,h,g_hpen3dUp,g_hpen3dDn );	

   for( i = 0; i < iFacePoint; i++ ) {
      ConvertPoint( gVertex[ NewFace[i] ].x, gVertex[ NewFace[i] ].y, gVertex[ NewFace[i] ].z, x, y );
      Put3Point( hdc, x, y, hbrNewPoint );
   }

   if( iCurFace != -1 /*&& !( g_3dwin[3] == hWnd && UserDrawMode == 3 )*/ ) {
      int x1, x2, x3, y1, y2, y3;
      ConvertPoint( gVertex[ gFace[iCurFace].v1 ].x, gVertex[ gFace[iCurFace].v1 ].y, gVertex[ gFace[iCurFace].v1 ].z, x1, y1 );
      ConvertPoint( gVertex[ gFace[iCurFace].v2 ].x, gVertex[ gFace[iCurFace].v2 ].y, gVertex[ gFace[iCurFace].v2 ].z, x2, y2 );
      ConvertPoint( gVertex[ gFace[iCurFace].v3 ].x, gVertex[ gFace[iCurFace].v3 ].y, gVertex[ gFace[iCurFace].v3 ].z, x3, y3 );
   
      if( (x2-x1)*(y3-y2) - (y2-y1)*(x3-x2) <= 0 )
         SelectObject( hdc, hpenCurrentFaceH );
      else
         SelectObject( hdc, hpenCurrentFaceV );
      Line( hdc, x1, y1, x2, y2 );
      Line( hdc, x2, y2, x3, y3 );
      Line( hdc, x3, y3, x1, y1 );
   
      Put3Point( hdc, x1, y1, hbrNewPoint );
   }
      
	  
	BitBlt(_hdc,0,0,w+1,h+1, hdc, 0,0, SRCCOPY);

	SelectObject( hdc, hpenOld );
	SelectObject( hdc, oldbmp );
	SelectObject( hdc, oldbrush );
}

void Put3Point(HDC hdc, int x, int y, HBRUSH hbr )
{
   if( hbr == 0 )
	   PatBlt(hdc,x-1,y-1,3,3, PATCOPY );
   else {
      HBRUSH hbrOld = (HBRUSH)SelectObject( hdc, hbr );
      PatBlt(hdc,x-1,y-1,3,3,PATCOPY);
      SelectObject( hdc, hbrOld );
   }
}
