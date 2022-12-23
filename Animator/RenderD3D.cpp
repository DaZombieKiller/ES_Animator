
#define INITGUID

#include <windows.h>    
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include "resource.h"


#include <ddraw.h>
#include <d3d.h>

#include "animator.h"  

typedef struct TagVector2d {
 float x,y;
} Vector2d;


BOOL DirectActive;
LPDIRECTDRAW7           lpDD;
LPDIRECTDRAWSURFACE7    lpddPrimary               = NULL;
LPDIRECTDRAWSURFACE7    lpddBack                  = NULL;
LPDIRECTDRAWSURFACE7    lpddZBuffer               = NULL;
LPDIRECTDRAWSURFACE7    lpddTexture               = NULL;
LPDIRECTDRAWSURFACE7    lpddCurTexture            = NULL;
DDSURFACEDESC2          ddsd;

LPDIRECT3D7             lpd3d                     = NULL;
LPDIRECT3DDEVICE7       lpd3dDevice               = NULL;
D3DTLVERTEX             ddvtx[3*4096];
WORD                    vxindex[3*4096];
D3DTLVERTEX             *lpVertex;
D3DLVERTEX              *lpVertex3;
Vector2d                gScrp[4096];

LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer;

HRESULT                 hRes;

BOOL                    fDeviceFound              = FALSE;
DWORD                   dwDeviceBitDepth          = 0UL;
GUID                    guidDevice;
char                    szDeviceName[256];
char                    szDeviceDesc[256];
D3DDEVICEDESC           d3dHWDeviceDesc;
HDC                     ddBackDC;
DWORD                   d3dbase_color;
BOOL                    D3DACTIVE, D3DBEGINS;

int d3dTexturesMem;
int d3dLastTexture;
int d3dMemUsageCount;
int d3dMemLoaded;
int vcount,icount;
int lasttexture;

void D3DERROR(char *mess, HRESULT hres)
{
	char t[128];
	wsprintf(t,"%s failed with error %X", mess, hres);
	MessageBox(g_MWin,t,"Error",IDOK);
}

RECT D3Drc;
int WinW, WinH;
float WinCX, WinCY;

typedef struct _d3dmemmap {
  int cpuaddr, size, lastused;  
  LPDIRECTDRAWSURFACE7    lpddTexture;  
} Td3dmemmap;    

#define d3dmemmapsize 256
Td3dmemmap d3dMemMap[d3dmemmapsize+2];

#define _ZSCALE  -16.f
#define _AZSCALE (1.f /  16.f);


void d3dClearBuffers()
{      
  DWORD color = 0x00808080;
  if (UserFillColor==0) color=0;
  if (UserFillColor==1) color=0x00404040;
  if (lpd3dDevice)
  lpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                     color, 1.0f, 0L );     
}


void ResetTextureMap()
{
  d3dMemUsageCount = 0;  
  d3dLastTexture = d3dmemmapsize+1;
  for (int m=0; m<d3dmemmapsize+2; m++) {
      d3dMemMap[m].lastused    = 0;
      d3dMemMap[m].cpuaddr     = 0;
	  if (d3dMemMap[m].lpddTexture) {
		  d3dMemMap[m].lpddTexture->Release();
		  d3dMemMap[m].lpddTexture = NULL;
	  }      
  }
}

void d3dStartRenderBuffer()
{
   vcount = 0;
   icount = 0;
   lpVertex = &ddvtx[0];
}

void d3dFlushRenderBuffer()
{
  lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, ddvtx, vcount, D3DDP_WAIT);
  vcount = 0;
}

void d3dFlushRenderBufferL()
{
  lpd3dDevice->DrawPrimitive(D3DPT_LINELIST, D3DFVF_TLVERTEX, ddvtx, vcount, D3DDP_WAIT);
  vcount = 0;
}

void InitDirect3DStates()
{

   lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
   
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS);
   
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_CLIPPING, TRUE);        
      
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);   

   lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
   
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);   
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
   lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

   lpd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
   lpd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
   lpd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
   //lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x00);
   //lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);      
      
   
   lpd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
   lpd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
   lpd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
   
}

HRESULT CreateDirect3D( HWND hwnd, DWORD dwWidth, DWORD dwHeight)
{ 
 lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );

 DDSURFACEDESC2 ddsd;
 ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
 ddsd.dwSize         = sizeof(DDSURFACEDESC2);

 //Create the primary surface.
 ddsd.dwFlags        = DDSD_CAPS;
 ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
 hRes = lpDD->CreateSurface( &ddsd, &lpddPrimary, NULL );
 if (FAILED(hRes)) {
	   //D3DERROR("lpDD->CreateSurface(Primary)", hRes);
	   return hRes;   
   }

//Create the backBuffer.
 ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
 ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
 ddsd.dwWidth  = dwWidth;
 ddsd.dwHeight = dwHeight;
 hRes = lpDD->CreateSurface( &ddsd, &lpddBack, NULL );
 if (FAILED(hRes)) {
	   //D3DERROR("lpDD->CreateSurface(BackBuffer)", hRes);
	   return hRes;   
 }
 
 hRes = lpDD->QueryInterface( IID_IDirect3D7, (VOID**)&lpd3d );
 if (FAILED(hRes)) {
	   //D3DERROR("lpDD->QueryInterface(IID_IDirect3D7)", hRes);
	   return hRes;   
 }


 //======== create Z-buffer ==========//
 DDPIXELFORMAT ddpfZBuffer;
  
 ZeroMemory(&ddpfZBuffer, sizeof(DDPIXELFORMAT));
 ddpfZBuffer.dwSize = sizeof(DDPIXELFORMAT); 
 ddpfZBuffer.dwFlags = DDPF_ZBUFFER;
 ddpfZBuffer.dwZBufferBitDepth = 24;
 ddpfZBuffer.dwZBitMask=0xFFFF;

 ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
 ddsd.dwSize         = sizeof(DDSURFACEDESC2);
 ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
 ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
 ddsd.dwWidth  = dwWidth;
 ddsd.dwHeight = dwHeight;
 memcpy( &ddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT) );

 hRes = lpDD->CreateSurface( &ddsd, &lpddZBuffer, NULL );
 if (FAILED(hRes)) {
    ddpfZBuffer.dwZBufferBitDepth = 32;
    ddpfZBuffer.dwZBitMask=0xFFFFFF;
    memcpy( &ddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT) );
    hRes = lpDD->CreateSurface( &ddsd, &lpddZBuffer, NULL );
    if (FAILED(hRes)) {
	  //D3DERROR("lpDD->CreateSurface(ZBuffer)", hRes);
      return hRes;        
    }
 }

 hRes = lpddBack->AddAttachedSurface( lpddZBuffer );
 if (FAILED(hRes)) {
	   //D3DERROR("lpddBack->AddAttachedSurface(lpddZBuffer)", hRes);
	   return hRes;
 }

//======== create clipper ========
 
 LPDIRECTDRAWCLIPPER lpddClipper;
 hRes = lpDD->CreateClipper( 0, &lpddClipper, NULL);   
 if (lpddClipper) {
   lpddClipper->SetHWnd( 0, hwnd );   
   lpddPrimary->SetClipper( lpddClipper );   
   lpddClipper->Release( );
 }

//===== create device =========//
   
 hRes = lpd3d->CreateDevice(IID_IDirect3DHALDevice,  lpddBack, &lpd3dDevice);

 if (FAILED(hRes)) {
	   D3DERROR("CreateDevice(IID_IDirect3DHALDevice)", hRes);
	   return hRes;   
 }

 D3DVIEWPORT7 vp = { 0, 0, dwWidth, dwHeight, 0.0f, 1.0f };
 hRes = lpd3dDevice->SetViewport( &vp );
 if (FAILED(hRes)) {
	   D3DERROR("lpd3dDevice->SetViewport(&vp)", hRes);
	   return hRes;   
 }

 



 return DD_OK;
}





void InitDirectDraw()
{      
   HRESULT hres;
   hres = DirectDrawCreateEx( NULL, (VOID**)&lpDD,  IID_IDirectDraw7, NULL );
   if( hres != DD_OK ) return;	  	     
   DirectActive = TRUE;
}


void ShutDownD3D()
{
  if (!D3DBEGINS) return;
  D3DACTIVE = FALSE;
  D3DBEGINS = FALSE;

  if (lpd3dDevice)
    hRes = lpd3dDevice->EndScene();
  
  ResetTextureMap();
  
  if (NULL != lpd3dVertexBuffer) {
      lpd3dVertexBuffer->Release();
      lpd3dVertexBuffer = NULL;
   }
  
  if (NULL != lpd3dDevice) {
      lpd3dDevice->Release( );
      lpd3dDevice = NULL;
   }

  if (NULL != lpddBack) {
      lpddBack->Release();
      lpddBack = NULL;
   }

  if (NULL != lpddZBuffer) {
      lpddZBuffer->Release( );
      lpddZBuffer = NULL;
   }

  if (NULL != lpddPrimary) {
        lpddPrimary->Release( );
        lpddPrimary = NULL;
   }
 
  if (NULL != lpd3d) {
      lpd3d->Release( );
      lpd3d = NULL;
   }


  
}


HWND lastWND;
void ActivateD3D(HWND hwndD3D)
{   	
    if (hwndD3D!=lastWND) ShutDownD3D();
    lastWND = hwndD3D;
	if (!DirectActive) return;
	if (D3DACTIVE) return;

	D3DBEGINS = TRUE;
		
    GetWindowRect(hwndD3D, &D3Drc);

	//if (CaptureMode == cpmAnimate) {
    
    D3Drc.left  +=1;
	D3Drc.right -=1;
	D3Drc.top   +=1;
	D3Drc.bottom-=1;	
	
	
	WinW = D3Drc.right  - D3Drc.left; 
	WinH = D3Drc.bottom - D3Drc.top;
	WinCX = (float)(WinW / 2);
	WinCY = (float)(WinH / 2);	

    HRESULT hRes = CreateDirect3D(hwndD3D, WinW, WinH);
	if (FAILED(hRes))  return; 

    InitDirect3DStates();

	
    d3dMemLoaded = 0;	                   
    D3DACTIVE = TRUE;    	
	
    d3dLastTexture = d3dmemmapsize+1;	
	
}














void InsertFxMM(int m)
{
   for (int mm=d3dmemmapsize-1; mm>m; mm--)
    d3dMemMap[m] = d3dMemMap[m-1];
}



BOOL d3dAllocTexture(int i, int w, int h, int bpp)
{
    
   DDSURFACEDESC2 ddsd;
   DDPIXELFORMAT  ddpf;
   ZeroMemory( &ddpf, sizeof(DDPIXELFORMAT) );
   ddpf.dwSize  = sizeof(DDPIXELFORMAT);
   ddpf.dwFlags = DDPF_RGB; 

   if (bpp==4) {
    ddpf.dwRGBBitCount = 32;   
    ddpf.dwFlags |= DDPF_ALPHAPIXELS;
    ddpf.dwRGBAlphaBitMask  = 0xFF000000;
    ddpf.dwRBitMask = 0xFF0000;
    ddpf.dwGBitMask = 0xFF00; 
    ddpf.dwBBitMask = 0xFF;  
   } else {
    ddpf.dwRGBBitCount = 16;   
    ddpf.dwFlags |= DDPF_ALPHAPIXELS;
    ddpf.dwRGBAlphaBitMask  = 0x8000;
    ddpf.dwRBitMask = 0x7c00;
    ddpf.dwGBitMask = 0x3e0; 
    ddpf.dwBBitMask = 0x1f;      
   }

   ZeroMemory(&ddsd, sizeof(ddsd));
   ddsd.dwSize          = sizeof(ddsd);
   ddsd.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
   ddsd.dwWidth         = w;
   ddsd.dwHeight        = h;
   ddsd.dwMipMapCount   = 4;
   ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
/*
   if (intViewFlags & vfmipmaps) {
       ddsd.dwFlags       |=DDSD_MIPMAPCOUNT;
       ddsd.ddsCaps.dwCaps|=DDSCAPS_MIPMAP|DDSCAPS_COMPLEX;
   }
*/   
   CopyMemory( &ddsd.ddpfPixelFormat, &ddpf, sizeof(DDPIXELFORMAT) );

   hRes = lpDD->CreateSurface( &ddsd, &d3dMemMap[i].lpddTexture, NULL);
   if (FAILED(hRes)) {
      //D3DERROR("lpDD->CreateSurface()", hRes);
	  d3dMemMap[i].lpddTexture = NULL;      
      return FALSE;   }   
     
   return TRUE;
}



void d3dDownLoadTexture(int i, int w, int h, int bpp, LPVOID tptr)   
{           
   
   LPDIRECTDRAWSURFACE7 lpddDest;

   lpddDest = d3dMemMap[i].lpddTexture;
   if (!lpddDest) return;
   d3dMemMap[i].cpuaddr = (int) tptr;
   d3dMemMap[i].size    = w*h*bpp;
   d3dMemLoaded+=w*h*bpp;

   DDSURFACEDESC2 ddsd;   
   ZeroMemory( &ddsd, sizeof(DDSURFACEDESC) );
   ddsd.dwSize = sizeof(DDSURFACEDESC);   
   
   if (lpddDest->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL ) != DD_OK ) return;
   if (!ddsd.lpSurface) return;
   CopyMemory( ddsd.lpSurface, tptr, w*h*bpp );
   lpddDest->Unlock( NULL );   

/*
   if (!(intViewFlags & vfmipmaps)) return;
     
   DDSCAPS2 ddsCaps;
   ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
   ddsCaps.dwCaps2 = 0;
   ddsCaps.dwCaps3 = 0;
   ddsCaps.dwCaps4 = 0;
   lpddDest->GetAttachedSurface( &ddsCaps, &lpddDest);
   if (!lpddDest) return;
   lpddDest->Release();

   tptr=(BYTE*)tptr+w*h*bpp; w/=2; h/=2;
   if( lpddDest->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL ) != DD_OK ) return;
   CopyMemory( ddsd.lpSurface, tptr, w*h*bpp );
   lpddDest->Unlock( NULL );   



   ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
   ddsCaps.dwCaps2 = 0;
   ddsCaps.dwCaps3 = 0;
   ddsCaps.dwCaps4 = 0;
   lpddDest->GetAttachedSurface( &ddsCaps, &lpddDest);
   if (!lpddDest) return;
   lpddDest->Release();

   tptr=(BYTE*)tptr+w*h*bpp; w/=2; h/=2;
   if( lpddDest->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL ) != DD_OK ) return; //DDLOCK_WAIT
   CopyMemory( ddsd.lpSurface, tptr, w*h*bpp );
   lpddDest->Unlock( NULL );   

   ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
   ddsCaps.dwCaps2 = 0;
   ddsCaps.dwCaps3 = 0;
   ddsCaps.dwCaps4 = 0;
   lpddDest->GetAttachedSurface( &ddsCaps, &lpddDest);
   if (!lpddDest) return;
   lpddDest->Release();

   tptr=(BYTE*)tptr+w*h*bpp; w/=2; h/=2;
   if( lpddDest->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) != DD_OK ) return;
   CopyMemory( ddsd.lpSurface, tptr, w*h*bpp );
   lpddDest->Unlock( NULL );   

   //---------------------------------------------------------------------------   
   */
}


int DownLoadTexture(LPVOID tptr, int w, int h, int bpp)
{   
    
   int textureSize = w*h*bpp;
   int fxm = 0;
   int m;

//========== if no memory used ==========//
   if (!d3dMemMap[0].cpuaddr) {
     d3dAllocTexture(0, w, h, bpp);
	 d3dDownLoadTexture(0, w, h, bpp, tptr);
     return 0;
   }
   //return 0;


//====== search for last used block and try to alloc next ============//
   for (m = 0; m < d3dmemmapsize; m++) 
	 if (!d3dMemMap[m].cpuaddr) 
		 if (d3dAllocTexture(m, w, h, bpp)) {
			 d3dDownLoadTexture(m, w, h, bpp, tptr);
             return m;
		 } else break;

   
		    
//====== search for unused texture and replace it with new ============//
   int unusedtime = 2;
   int rt = -1;
   for (m = 0; m < d3dmemmapsize; m++) {
	 if (!d3dMemMap[m].cpuaddr) break;
	 if (d3dMemMap[m].size != w*h*bpp) continue;

	 int ut = d3dMemUsageCount - d3dMemMap[m].lastused;
     
	 if (ut >= unusedtime) {
            unusedtime = ut;
            rt = m;   }
	}

   if (rt!=-1) {
	   d3dDownLoadTexture(rt, w, h, bpp, tptr);
	   return rt;
   }
   
   ResetTextureMap();

   d3dAllocTexture(0, w, h, bpp);
   d3dDownLoadTexture(0, w, h, bpp, tptr);
   
   return 0;

}




void d3dSetTexture(LPVOID tptr, int w, int h, int bpp)
{    
  if (!tptr) return;
	
  if (d3dMemMap[d3dLastTexture].cpuaddr == (int)tptr) return;

  int fxm = -1;
  for (int m=0; m<d3dmemmapsize; m++) {
     if (d3dMemMap[m].cpuaddr == (int)tptr) { fxm = m; break; }
     if (!d3dMemMap[m].cpuaddr) break;
  }

  if (fxm==-1) fxm = DownLoadTexture(tptr, w, h, bpp);

  d3dMemMap[fxm].lastused = d3dMemUsageCount;  
  lpddCurTexture = d3dMemMap[fxm].lpddTexture;  
  d3dLastTexture = fxm;
  
  lpd3dDevice->SetTexture(0, lpddCurTexture);  
}


void RenderQuad2d()
{
        lpVertex->sx = gScrp[0].x;
        lpVertex->sy = gScrp[0].y;
        lpVertex->sz = 64.f / (4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = 0;
        lpVertex->tv = 0;
        lpVertex->rhw = 1;
        lpVertex++;

        lpVertex->sx = gScrp[1].x;
        lpVertex->sy = gScrp[1].y;
        lpVertex->sz = 64.f / (4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = 1;
        lpVertex->tv = 0;
        lpVertex->rhw = 1;
        lpVertex++;

        lpVertex->sx = gScrp[2].x;
        lpVertex->sy = gScrp[2].y;
        lpVertex->sz = 64.f / (4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = 1;
        lpVertex->tv = 1;
        lpVertex->rhw = 1;
        lpVertex++;
////////////////////////////////////////
        lpVertex->sx = gScrp[2].x;
        lpVertex->sy = gScrp[2].y;
        lpVertex->sz = 64.f / (4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = 1;
        lpVertex->tv = 1;
        lpVertex->rhw = 1;
        lpVertex++;

        lpVertex->sx = gScrp[3].x;
        lpVertex->sy = gScrp[3].y;
        lpVertex->sz = 64.f / (4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = 0;
        lpVertex->tv = 1;
        lpVertex->rhw = 1;
        lpVertex++;

        lpVertex->sx = gScrp[0].x;
        lpVertex->sy = gScrp[0].y;
        lpVertex->sz = 64.f / (4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = 0;
        lpVertex->tv = 0;
        lpVertex->rhw = 1;
        lpVertex++;

        vcount+=3;
		vcount+=3;
}

void ResetTrackTextures()
{
 for(int e=0;e<2;e++)
	 for(int u=8+512*e;u<8+512*(e+1);u++) {
		 if (!Textures[u].lpTexture) continue;
		 free(Textures[u].lpTexture);
		 Textures[u].lpTexture=0;
	 }
}
/*
void DoHalt(char *str, CBlock &b)
{
	MessageBox(g_MWin, str, "Pharser syntax error", MB_OK);
}

int ReadVariable(CBlock &b, char *var_name, float &var)
{
	if(b.CmpSymbol( var_name ) ) {
		b.Skip(1);
		if (!b.GetFloat( &var )) DoHalt("Error: needs float value",b);
		return 1;
	}
	return 0;
}

int ReadVariable(CBlock &b, char *var_name, int &var)
{
	if(b.CmpSymbol( var_name ) ) {
		b.Skip(1);
		if (!b.GetInt( &var )) DoHalt("Error: needs int value",b);
		return 1;
	}
	return 0;
}


void ReadVectorInt(CBlock &b, Vector3d &pos)
{
	int x=0,y=0,z=0;
	char ch;
			   if (!b.GetInt( &x )) DoHalt("Error: needs value 1",b);
	b.Skip(1); if (!b.GetChar(&ch)) DoHalt("Error: needs <,>",b);
	b.Skip(1); if (!b.GetInt( &y )) DoHalt("Error: needs value 2",b);
	b.Skip(1); if (!b.GetChar(&ch)) DoHalt("Error: needs <,>",b);
	b.Skip(1); if (!b.GetInt( &z )) DoHalt("Error: needs value 3",b);

	pos.x = (float)x;
	pos.y = (float)y;
	pos.z = (float)z;
}

void LoadTrackCorrespond()
{
	char tpath[1024];
	wsprintf(tpath, "%s\\%s", ModelPath, "ys.txt");

}
*/
int LoadTrackTexture(int e, int t)
{
	int tn = 8+t+e*512;
	if (Textures[tn].lpTexture) return 1;
	char tpath[1024];
	char tname[128];
	if (e>=2) return 0;
	if (e==0) 
		sprintf(tname, "%s%02d.bmp",bmp_fname, bmp_Begin + t*bmp_Step);
	if (e==1) 
		sprintf(tname, "%s%02d.bmp",bmp_lname, bmp_Begin + t*bmp_Step);
	wsprintf(tpath, "%s\\%s", bmp_Path, tname);
	int i = LoadTextureBMP(tn, tpath, 0);
	return 1;
}

void RenderTrackTexture()
{
	if (bmp_disabled) return;
	int t_no = CurFrame;
	if (g_ew >= 2) return;
	if (!LoadTrackTexture(g_ew, t_no)) return;

	d3dStartRenderBuffer();

	int tn = t_no+8+512*g_ew;
	int tex_w= Textures[tn].W;
	int tex_h= Textures[tn].H;

	 if (!tex_w) return;
	 if (!tex_h) return;

	d3dSetTexture(Textures[tn].lpTexture, Textures[tn].W, Textures[tn].H, 4);


	POINT c, p0,p1;

	float sc;
	 
	sc = bmp_Scale[g_ew];

	c = bmp_c[g_ew];

	if (bmp_locked[g_ew]) {
		 sc *= g_3dwininfo[g_ew].Scaler;
		 ConvertPoint(bmp_pos0[g_ew].x,bmp_pos0[g_ew].y,bmp_pos0[g_ew].z, (int&)c.x, (int&)c.y);
		 bmp_c[g_ew] = c;
		 //c.x -= g_3dwininfo[g_ew]._xc0*g_3dwininfo[g_ew].Scaler;
		 //c.y += g_3dwininfo[g_ew]._yc0*g_3dwininfo[g_ew].Scaler;
	}
	

	 gScrp[0].x = c.x-sc*tex_w/2;
	 gScrp[0].y = c.y-sc*tex_h;

	 gScrp[1].x = c.x+sc*tex_w/2;
	 gScrp[1].y = c.y-sc*tex_h;

	 gScrp[2].x = c.x+sc*tex_w/2;
	 gScrp[2].y = c.y;

	 gScrp[3].x = c.x-sc*tex_w/2;
	 gScrp[3].y = c.y;

	 RenderQuad2d();


    d3dFlushRenderBuffer();
}

Vector3d CalcSurfaceNormal(int f)
{
   Vector3d nv = {0,0,0};
   Vector3d v[4];
   v[0] = gVertex[gFace[f].v1].pos;
   v[1] = gVertex[gFace[f].v2].pos;
   v[2] = gVertex[gFace[f].v3].pos;

   MulVectorsVect(SubVectors(v[2],v[0]),
                  SubVectors(v[1],v[0]), nv);

   if (IS_ZEROV(nv))
	   MulVectorsVect(SubVectors(v[3],v[0]),
					  SubVectors(v[2],v[0]), nv);

   if (IS_ZEROV(nv))
	   MulVectorsVect(SubVectors(v[3],v[0]),
					  SubVectors(v[1],v[0]), nv);

   if (IS_ZEROV(nv))
	   MulVectorsVect(SubVectors(v[3],v[0]),
					  SubVectors(v[2],v[0]), nv);

   if (IS_ZEROV(nv))
	   MulVectorsVect(SubVectors(v[3],v[1]),
					  SubVectors(v[2],v[1]), nv);


   NormVector(nv,1.0f);

   return nv;
}
void RenderTexturedView()
{
    int f = -1;
    for (int tx=0; tx<TCount; tx++) {
     d3dSetTexture(Textures[tx].lpTexture, Textures[tx].W, Textures[tx].H, 4);

     d3dStartRenderBuffer();
     for(int ff=0; ff<FbyTCount[tx]; ff++ ) {
        f++;
        Vector2d v[3];
	    int v1 = gFace[f].v1; 
        int v2 = gFace[f].v2;
        int v3 = gFace[f].v3;

        if (gVertex[ v1 ].hide || gVertex[ v2 ].hide || gVertex[ v3 ].hide) continue;

		if (gFace[f].Flags & DamageMask) continue;

        if (!(gFace[f].Flags & 1)) {
		 v[0] = gScrp[v1]; v[1] = gScrp[v2]; v[2] = gScrp[v3]; 
		 if (((v[1].x-v[0].x)*(v[2].y-v[1].y) - (v[1].y-v[0].y)*(v[2].x-v[1].x))<0) continue;
        }

        lpVertex->sx = gScrp[v1].x;
        lpVertex->sy = gScrp[v1].y;
        lpVertex->sz = 64.f / (rVertex[v1].z+4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = gFace[f].tax;
        lpVertex->tv = gFace[f].tay;
        lpVertex->rhw = 1;
        lpVertex++;

        lpVertex->sx = gScrp[v2].x;
        lpVertex->sy = gScrp[v2].y;
        lpVertex->sz = 64.f / (rVertex[v2].z+4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = gFace[f].tbx;
        lpVertex->tv = gFace[f].tby;
        lpVertex->rhw = 1;
        lpVertex++;

        lpVertex->sx = gScrp[v3].x;
        lpVertex->sy = gScrp[v3].y;
        lpVertex->sz = 64.f / (rVertex[v3].z+4048);
        lpVertex->color = 0xFFFFFFFF;
        lpVertex->tu = gFace[f].tcx;
        lpVertex->tv = gFace[f].tcy;
        lpVertex->rhw = 1;
        lpVertex++;
        vcount+=3;
    }

    d3dFlushRenderBuffer();
    }
}
    


void RenderFlatView()
{
   Vector2d v[3];
   lpd3dDevice->SetTexture(0, NULL);  
   d3dLastTexture = d3dmemmapsize+1;
   CalcRotatedModel();
   BuildTree( );

   d3dStartRenderBuffer();     

   int f=Current;
   while (f!=-1) {
       int v1 = gFace[f].v1; 
       int v2 = gFace[f].v2;     
       int v3 = gFace[f].v3;
       if( gVertex[ v1 ].hide || gVertex[ v2 ].hide || gVertex[ v3 ].hide ) 
        if (gFace[f].Flags & DamageMask) {
               f = gFace[f].Next;
               continue;
       }

       v[0] = gScrp[v1]; v[1] = gScrp[v2]; v[2] = gScrp[v3];  
       if  ((v[1].x-v[0].x)*(v[2].y-v[1].y) - (v[1].y-v[0].y)*(v[2].x-v[1].x)>=0) {                       
               float _x1 = rVertex[v2].x - rVertex[v1].x;
               float _y1 = rVertex[v2].y - rVertex[v1].y;
               float _z1 = rVertex[v2].z - rVertex[v1].z;
      
               float _x2 = rVertex[v3].x - rVertex[v2].x;
               float _y2 = rVertex[v3].y - rVertex[v2].y;
               float _z2 = rVertex[v3].z - rVertex[v2].z;
      
               float _nx = _y1*_z2  -  _y2*_z1;
               float _ny = _x2*_z1  -  _x1*_z2;
               float _nz = _x1*_y2  -  _x2*_y1;
      
               int alpha = 128;
               if (UserDrawMode==2) {
                 float scalar = (lx*_nx + ly*_ny + lz*_nz)*120.f;
                 alpha = 128 + scalar  / (float)(sqrt(_nx*_nx + _ny*_ny + _nz*_nz) * (sqrt(lx*lx + ly*ly + lz*lz)));
               }
      
               DWORD color = (alpha * 0x010101) + 0xFF000000;

               lpVertex->sx = gScrp[v1].x;
               lpVertex->sy = gScrp[v1].y;
               lpVertex->sz = 64.f / (rVertex[v1].z+4048);
               lpVertex->color = color;        
               lpVertex->rhw = 1;
               lpVertex++;

               lpVertex->sx = gScrp[v2].x;
               lpVertex->sy = gScrp[v2].y;
               lpVertex->sz = 64.f / (rVertex[v2].z+4048);
               lpVertex->color = color;        
               lpVertex->rhw = 1;
               lpVertex++;

               lpVertex->sx = gScrp[v3].x;
               lpVertex->sy = gScrp[v3].y;
               lpVertex->sz = 64.f / (rVertex[v3].z+4048);
               lpVertex->color = color;        
               lpVertex->rhw = 1;
               lpVertex++;

               vcount+=3;
       }
       f = gFace[ f ].Next;
   }
   d3dFlushRenderBuffer();
}


void FillVertex2d(float x, float y, DWORD color)
{
    lpVertex->sx = x;
    lpVertex->sy = y;
    lpVertex->sz = 0.5;
    lpVertex->color = color;        
    lpVertex->rhw = 1;
    lpVertex++;
}


void RenderLine(float x1, float y1, float x2, float y2, DWORD color)
{
               lpVertex->sx = x1;
               lpVertex->sy = y1;
               lpVertex->sz = 0.5;
               lpVertex->color = color;        
               lpVertex->rhw = 1;
               lpVertex++;

               lpVertex->sx = x2;
               lpVertex->sy = y2;
               lpVertex->sz = 0.5;
               lpVertex->color = color;        
               lpVertex->rhw = 1;
               lpVertex++;

               vcount+=2;
               if (vcount>1024) {
                   d3dFlushRenderBufferL();
                   d3dStartRenderBuffer(); 
               }
}


void RenderCircle(float cx, float cy, float R, DWORD color)
{
    int T = 8+R/10;
    if (T>128) T = 128;
    for (int t=0; t<=T; t++) {
        float a = 2*3.1415*t / T;
        FillVertex2d(cx + cos(a)*R, cy+sin(a)*R, color);
              a = 2*3.1415*(t+1) / T;
        FillVertex2d(cx + cos(a)*R, cy+sin(a)*R, color);
        vcount+=2;
    }        
}


void RenderObjectP(float x, float y, DWORD color)
{
    RenderLine(x-1, y-1, x+1, y-1, color);
    RenderLine(x+1, y-1, x+1, y+1, color);
    RenderLine(x+1, y+1, x-1, y+1, color);
    RenderLine(x-1, y+1, x-1, y-1, color);
    RenderLine(x-1, y-1, x+1, y+1, color);
}

void RenderObject(float x, float y, DWORD color)
{

   RenderCircle(x,y, 3, color);
   RenderObjectP(x,y, color);
}

void RenderWireView()
{
			
    Vector2d v[3];
    lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
    d3dStartRenderBuffer();     

    DWORD color;

    for( int o=0; o<OCount; o++ ) {
         if( gObj[ o ].hide ) continue;
         if (o == CurObject ) color = 0xFFC80000; else
            if (Track[CurFrame][o].active) color = 0xFFB4B400; 
                                     else  color = 0xFF00009B;
            ConvertPoint(gObj[o].ox, gObj[o].oy, gObj[o].oz, x, y);
            scro[o].x = x;
            scro[o].y = y;
            RenderObject(x,y, color);
         }

    d3dFlushRenderBufferL();

    d3dStartRenderBuffer();

    if (SHOWSCELETON)       
        if( g_3dwininfo[g_ew].ShowObj )
         for( int o=0; o<OCount; o++ ) {
          if( gObj[o].hide ) continue;
          if( gObj[o].owner==-1) continue;
          if( gObj[gObj[o].owner].hide ) continue;

          RenderLine(scro[o].x, scro[o].y, scro[gObj[o].owner].x, scro[gObj[o].owner].y, 0xFFB4B400);
         }

    
    int f;
    for( f=0; f<FCount; f++ ) {
		      int v1 = gFace[f].v1; int v2 = gFace[f].v2;	int v3 = gFace[f].v3;
              if( gVertex[ v1 ].hide || gVertex[ v2 ].hide || gVertex[ v3 ].hide ) 
                 continue;
			  if (gFace[f].Flags & DamageMask) continue;

		      v[0] = gScrp[v1]; v[1] = gScrp[v2]; v[2] = gScrp[v3];
              if (((v[1].x-v[0].x)*(v[2].y-v[1].y) - (v[1].y-v[0].y)*(v[2].x-v[1].x))<0) {
                  RenderLine(v[0].x, v[0].y, v[1].x, v[1].y, 0xFF404040);
                  RenderLine(v[1].x, v[1].y, v[2].x, v[2].y, 0xFF404040);
                  RenderLine(v[2].x, v[2].y, v[0].x, v[0].y, 0xFF404040);
              }
   			   
    }

          
	for (f=0; f<FCount; f++) {
		      int v1 = gFace[f].v1; int v2 = gFace[f].v2;	int v3 = gFace[f].v3;
              if( gVertex[ v1 ].hide || gVertex[ v2 ].hide || gVertex[ v3 ].hide ) 
                 continue;
			  if (gFace[f].Flags & DamageMask) continue;

		      v[0] = gScrp[v1]; v[1] = gScrp[v2]; v[2] = gScrp[v3];
		      if (((v[1].x-v[0].x)*(v[2].y-v[1].y) - (v[1].y-v[0].y)*(v[2].x-v[1].x))>=0) {  
                  RenderLine(v[0].x, v[0].y, v[1].x, v[1].y, 0xFFE0E0E0);
                  RenderLine(v[1].x, v[1].y, v[2].x, v[2].y, 0xFFE0E0E0);
                  RenderLine(v[2].x, v[2].y, v[0].x, v[0].y, 0xFFE0E0E0);
            }
    }

    d3dFlushRenderBufferL();
}


void Renderd3dView()
{
    if (!lpd3dDevice) 
        return;
	d3dClearBuffers();
    hRes = lpd3dDevice->BeginScene( );		
    if (FAILED(hRes)) return;
		       
    g_sina = (float)sin( g_alpha );
    g_cosa = (float)cos( g_alpha );
    g_sinb = (float)sin( g_beta );
    g_cosb = (float)cos( g_beta );
       
    for( int v=0; v<VCount; v++ ) {
      float z = gVertex[v].z;
      
      float x1 = gVertex[v].x * g_cosa - z * g_sina;
      float z1 = z * g_cosa + gVertex[v].x * g_sina;
      float y1 = gVertex[v].y;

      rVertex[v].x = x1;
      rVertex[v].z = z1 * g_cosb - y1 * g_sinb;
	  rVertex[v].y = y1 * g_cosb + z1 * g_sinb;
        
      gScrp[v].x = WinCX + (int) ( (rVertex[v].x - g_3dwininfo[3]._xc0) * g_scaler);
      gScrp[v].y = WinCY - (int) ( (rVertex[v].y - g_3dwininfo[3]._yc0) * g_scaler);
	}
	
	if (UserDrawMode==3) RenderTexturedView();
    if (UserDrawMode==2 ||
        UserDrawMode==1) RenderFlatView();
    if (UserDrawMode==0) RenderWireView();

   
    hRes = lpd3dDevice->EndScene(); 	
    Sleep(0);
    lpddBack->GetDC( &ddBackDC );
    lpddBack->ReleaseDC( ddBackDC );	 
	hRes = lpddPrimary->Blt(&D3Drc, lpddBack, NULL, DDBLT_WAIT, NULL );
    
}


void Renderd2dView()
{
    if (!lpd3dDevice) 
        return;
	d3dClearBuffers();
    hRes = lpd3dDevice->BeginScene( );		
    if (FAILED(hRes)) return;

    float xsspeed = 0;
    float ysspeed = 0;

    DWORD color;
    
    d3dStartRenderBuffer();

    lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);

    if( g_3dwininfo[g_ew].GridStep > 0 )  {
	  int gs = g_3dwininfo[g_ew].GridStep;
      int gcnt = (int) ((float)(w)/(float)0.5/(float)gs/g_scaler) +1 ;
	   int gx; int gy;

	   if (gcnt<(w>>1)) 
	      if (g_3dwininfo[g_ew].GridLines) {			  
	         for (gx=-gcnt; gx<=gcnt; gx++) { 
		         x = g_xc + (int) ( (gx * gs - g_3dwininfo[g_ew]._xc0 - xsspeed * CurFrame) * g_scaler);  
                 RenderLine(x,0, x,h, 0xFF707070); 
			 }
	         for (gy=-gcnt; gy<=gcnt; gy++) { 
                  y = g_yc - (int) ( (gy * gs - g_3dwininfo[g_ew]._yc0 - ysspeed * CurFrame) * g_scaler);                    
                  RenderLine(0,y, w,y, 0xFF707070); 
             }
	         } 
    }


    if (g_3dwininfo[g_ew].CenterLine) {
      x = g_xc - int( (g_3dwininfo[g_ew]._xc0) * g_scaler);
      y = g_yc + int( (g_3dwininfo[g_ew]._yc0) * g_scaler);
      RenderLine(x, 0, x, h, 0xFF505050);
	  RenderLine(0, y, w, y, 0xFF505050);
	}        

    d3dFlushRenderBufferL();

//=================================//
	  lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	RenderTrackTexture();
    lpd3dDevice->SetTexture(0, NULL);  
    d3dLastTexture = d3dmemmapsize+1;
	lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

    lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS);
    if (SHOWMODELT)
    {
    d3dStartRenderBuffer();
    for (int vv=0; vv<VCount; vv++) {
	   ConvertPoint(gVertex[vv].x,gVertex[vv].y,gVertex[vv].z, (int&)scrc[vv].x, (int&)scrc[vv].y);
       if (g_ew == 0) if (RevView) rVertex[vv].z = -gVertex[vv].z; else rVertex[vv].z = gVertex[vv].z;
       if (g_ew == 1) if (RevView) rVertex[vv].z = gVertex[vv].x; else rVertex[vv].z = -gVertex[vv].x;
       if (g_ew == 2) rVertex[vv].z = gVertex[vv].y;
       gScrp[vv].x = scrc[vv].x;
       gScrp[vv].y = scrc[vv].y;
    }
    RenderTexturedView();
    lpd3dDevice->SetTexture(0, NULL);  
    d3dLastTexture = d3dmemmapsize+1;
    }

//=====================================//
    
    lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
    d3dStartRenderBuffer();
    ConvertPoint( bp.x, bp.y, bp.z, x, y ); 	
	if (((int)(bp.x) != 0) || 
		((int)(bp.y) != 0) || 
		((int)(bp.z) != 0)  ) {
     RenderLine(x, 0, x, h, 0x80000080);
	 RenderLine(0, y, w, y, 0x80000080);
	}
    
    int xx,yy;
    if (PathObject!=-1) {
      ConvertPoint(ObjPath[PathOBegin].x,ObjPath[PathOBegin].y,ObjPath[PathOBegin].z, xx, yy);
      for (int k=PathOBegin; k<=PathOEnd; k++) {
        ConvertPoint(ObjPath[k].x,ObjPath[k].y,ObjPath[k].z, x, y);
        RenderLine(xx, yy, x, y, 0xFF004600);
        xx = x; yy =y;
        if (ObjPath[k].owner) {
          RenderObject(x,y, 0xFF004600);
        } else RenderObjectP(x,y, 0xFF004600);
      } 
    }

    for (int vv=0; vv<VCount; vv++)
	   ConvertPoint(gVertex[vv].x,gVertex[vv].y,gVertex[vv].z, (int&)scrc[vv].x, (int&)scrc[vv].y);


    if (SHOWMODEL)
	for (int d=!SHOWBACK; d<2; d++) {

        if (SHOWMODELT) {
         if( d==0 ) continue;
               else color = 0x50B0B0B0; 
        } else {
         if( d==0 ) color = 0xFF606060; 
               else color = 0xFFE0E0E0; 
        }
      

	  for (int f=0; f<FCount; f++) {
	     int v1 = gFace[f].v1;
	     int v2 = gFace[f].v2;
	     int v3 = gFace[f].v3;
         if( gVertex[ v1 ].hide || gVertex[ v2 ].hide || gVertex[ v3 ].hide )  continue; // one of the face points is hidden
		 if (gFace[f].Flags & DamageMask) continue;

	     POINT v[4];
	     v[0] = scrc[v1]; v[1] = scrc[v2]; v[2] = scrc[v3]; v[3]=v[0];							

	      int ux = v[1].x-v[0].x;
	      int uy = v[1].y-v[0].y;
	      int vx = v[2].x-v[1].x;
	      int vy = v[2].y-v[1].y;	
	
  	      if (d==0) {
            if( (ux*vy - uy*vx)>=0) continue;               
         } else {
            if( (ux*vy - uy*vx) <0)  continue;               
         }

         RenderLine(v[0].x, v[0].y, v[1].x, v[1].y, color);
         RenderLine(v[1].x, v[1].y, v[2].x, v[2].y, color);
         RenderLine(v[2].x, v[2].y, v[0].x, v[0].y, color);
	   }
	}

    if( g_3dwininfo[g_ew].ShowObj )
      for( int o=0; o<OCount; o++ ) {
         if( gObj[ o ].hide ) continue;         
         ConvertPoint(gObj[o].ox, gObj[o].oy, gObj[o].oz, x, y);         
         scro[o].x = x;
         scro[o].y = y;
	  }

    if (SHOWSCELETON)
    if( g_3dwininfo[g_ew].ShowObj )
      for( int o=0; o<OCount; o++ ) {
         if( gObj[o].hide ) continue;
         if(gObj[o].owner==-1) continue;
         if( gObj[gObj[o].owner].hide ) continue;

         RenderLine(scro[o].x, scro[o].y, scro[gObj[o].owner].x, scro[gObj[o].owner].y, 0xFFB4B400);
      }

    if( g_3dwininfo[g_ew].ShowObj )
      for( int o=0; o<OCount; o++ ) {
         if( gObj[ o ].hide ) continue;         
         ConvertPoint(gObj[o].ox, gObj[o].oy, gObj[o].oz, x, y);         
         scro[o].x = x;
         scro[o].y = y;

         if( o == CurObject ) continue;
         if (Track[CurFrame][o].active) color = 0xFFB4B400;
                                  else  color = 0xFF00009B; 
		 if (locBase[o]) color = 0xFF303000;

		 if (SHOWSPHERES) {
			//RenderCircle(scro[o].x,scro[o].y, 10*g_3dwininfo[g_ew].Scaler, 0xFFA00000);
			//RenderCircle(scro[o].x,scro[o].y, 20*g_3dwininfo[g_ew].Scaler, 0xFFB4B400);
			 RenderCircle(scro[o].x,scro[o].y, SphereR[o]*g_3dwininfo[g_ew].Scaler, 0xFFB4B400);
		 }
         RenderObject(scro[o].x,scro[o].y, color);         
      }


    if( OCount && CurObject != -1 ) {
        ConvertPoint( gObj[CurObject].ox, gObj[CurObject].oy, gObj[CurObject].oz, x, y );
        RenderObject(x,y, 0xFFC80000);
		if (SHOWSPHERES) {
			RenderCircle(x,y, SphereR[CurObject]*g_3dwininfo[g_ew].Scaler, 0xFFA00000);
		//RenderCircle(x,y, 10*g_3dwininfo[g_ew].Scaler, 0xFFA00000);
		//RenderCircle(x,y, 20*g_3dwininfo[g_ew].Scaler, 0xFFB4B400);
		}
    }

    if (SHOWMODEL)
	if( g_3dwininfo[g_ew].PointType )
      for( vv=0; vv<VCount; vv++ )
         if( !gVertex[ vv ].hide )
            RenderObjectP(scrc[vv].x,scrc[vv].y,0xFF505050);	



    d3dFlushRenderBufferL();
		
    hRes = lpd3dDevice->EndScene(); 	
    Sleep(0);
    lpddBack->GetDC( &ddBackDC );
    lpddBack->ReleaseDC( ddBackDC );	 
	hRes = lpddPrimary->Blt(&D3Drc, lpddBack, NULL, DDBLT_WAIT, NULL );
    
}
