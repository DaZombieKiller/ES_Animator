#include <windows.h>    // includes basic windows functionality
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "animator.h"   // includes application-specific information


BOOL LoadComplete;

typedef struct _FNAME
{
	char szTest1[80];		// a test buffer containing the file selected
	char szTest2[80];       // a test buffer containing the file path
} FNAME, FAR * LPFNAME;

LPVOID arg1, arg2;
int type;
RECT rcObj[8];
int status[8];
int iCurrentRect;
int ptrCoord;


int ParseFileName( char* fname, char* path, char* name )
{
   if( fname == NULL ) return 0;

   int index = 0, i = 0;;
   char* tmp = fname;
   while( *tmp ) {
      if( *tmp++ == '\\' )
         index = i;
      i++;
   }

   i = 0;
   if( path != NULL ) {
      int index1 = ( index==2 ) ? 3 : index;
      CopyMemory( path, fname, index1 );
      path[ index1 ] = 0;
      i++;
   }

   if (name!=NULL) {
      int len = lstrlen(fname)-index-1;
      CopyMemory( name, fname+index+1, len );
      name[ len ] = 0;
      i++;
   }
   return i;
}




BOOL CALLBACK OpenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg) {
      case WM_INITDIALOG:
         CenterWindow(GetParent(hDlg));
         return TRUE;
      }

      return FALSE;
}

void SaveCursorPos()
{	
	GetCursorPos(&__cursor_pos);
}

void RestoreCursorPos()
{
	SetCursorPos(__cursor_pos.x,__cursor_pos.y);
}

void CenterWindow(HWND hwnd)
{
   RECT rc;
   GetWindowRect( hwnd, &rc );
    
   SetWindowPos( hwnd, NULL,
		GetSystemMetrics(SM_CXSCREEN)/2-(rc.right - rc.left)/2,
		GetSystemMetrics(SM_CYSCREEN)/2-(rc.bottom - rc.top)/2, 0, 0,
      SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER
   );
}

void CenterWindowToMouse(HWND hwnd)
{
        POINT ms;
		RECT rc;
        GetWindowRect(hwnd, &rc);   
		GetCursorPos(&ms);
		int w=rc.right - rc.left;
		int h=rc.bottom - rc.top;
		ms.x-=w>>2;
		ms.y-=(h-(h>>2));
		if (ms.x<4) ms.x=4;		if (ms.x+w>790) ms.x=790-w;
		if (ms.y<4) ms.y=4;		if (ms.y+h>590) ms.y=590-h;

        SetWindowPos(hwnd, NULL, 
			ms.x, ms.y, 0, 0,
            SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}


DWORD Color32_16(WORD c)
{
    int r = (c>>10) & 31;
    int g = (c>> 5) & 31;
    int b = (c>> 0) & 31;
    return ((r*8)<<16) + ((g*8)<<8) + (b*8);
}


void LoadTextureTGA(int t, char* tname)
{

   HANDLE hfile = CreateFile(tname, GENERIC_READ, FILE_SHARE_READ,
                             NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   if( hfile==INVALID_HANDLE_VALUE ) {
      char t[128];
      wsprintf(t, "Error opening file: %s", tname);
      MessageBox( NULL, t, "Error", MB_OK|MB_ICONSTOP );
      return;
   }

   SetFilePointer(hfile, 12, 0, FILE_BEGIN);

   DWORD l;
   WORD bpp,w,h;
   BOOL al;

   ReadFile( hfile, &w, 2,   &l, NULL ); 
   ReadFile( hfile, &h, 2,   &l, NULL ); 
   ReadFile( hfile, &bpp, 2, &l, NULL ); 
   Textures[t].W = w;
   Textures[t].H = h;

   al = (bpp>>8)==8;
   bpp &= 0xFF;

   if (!Textures[t].lpTexture)
       Textures[t].lpTexture = malloc(Textures[t].W * Textures[t].H * 4);

   SetFilePointer(hfile, 18, 0, FILE_BEGIN);

   if (bpp == 32)
       for (int y=0; y<h; y++)
           ReadFile(hfile, ((DWORD*)Textures[t].lpTexture+(h-1-y)*w), w*4, &l, NULL);
   
   if (bpp == 16) {
       WORD line[256];
       for (int y=0; y<h; y++) {
           ReadFile(hfile, line, w*2, &l, NULL);
           for (int x=0; x<w; x++) 
            *((DWORD*)Textures[t].lpTexture+(h-1-y)*w+x) = Color32_16(line[x])|0xFF000000;
       }
   }

   CloseHandle(hfile);
}


int LoadTextureBMP(int t, char* tname, int show_err)
{
   HANDLE hfile = CreateFile(tname, GENERIC_READ, FILE_SHARE_READ,
                             NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if( hfile==INVALID_HANDLE_VALUE ) {
      if (show_err) {
      char t[128];
      wsprintf(t, "Error opening file: %s", tname);
      MessageBox( NULL, t, "Error", MB_OK|MB_ICONSTOP );
      return 0;
	  }
	  return 0;
   }
   byte fRGB[2048][3];
   BITMAPFILEHEADER bmpFH;
   BITMAPINFOHEADER bmpIH;
   DWORD l;

   ReadFile( hfile, &bmpFH, sizeof( BITMAPFILEHEADER ), &l, NULL );
   ReadFile( hfile, &bmpIH, sizeof( BITMAPINFOHEADER ), &l, NULL );

   int w = Textures[t].W = bmpIH.biWidth;
   int h = Textures[t].H = bmpIH.biHeight;
/*
   if (Textures[t].lpTexture) {
	   free(Textures[t].lpTexture);
	   Textures[t].lpTexture=0;
   }
*/
   if (!Textures[t].lpTexture)
	   Textures[t].lpTexture = malloc(Textures[t].W*Textures[t].H*4);

   for (int y=0; y<h; y++) {      
      ReadFile( hfile, fRGB, 3*w, &l, NULL );
      for (int x=0; x<w; x++)
        *((DWORD*)Textures[t].lpTexture + (h-y-1)*w+x) = 
            ((int)fRGB[x][2]<<16) + ((int)fRGB[x][1]<< 8) + ((int)fRGB[x][0]) + 0xFF000000;
   }
   CloseHandle(hfile);
   return 1;
}


void SetLod(int l)
{
  if (l>=LodCount) l = 0;
  CurLod = l;

  SendMessage(hwndLod1,  BM_SETSTATE, (CurLod==0), 0);//(CurLod==0));
  SendMessage(hwndLod2,  BM_SETSTATE, (CurLod==1), 0);
  SendMessage(hwndLod3,  BM_SETSTATE, (CurLod==2), 0);
  SendMessage(hwndLod4,  BM_SETSTATE, (CurLod==3), 0);
  
  memcpy(gVertex, gVertexL[l], sizeof(gVertex) );
  memcpy(gFace,   gFaceL[l],   sizeof(gFace)   );
  memcpy(gObj,    gObjL[l],    sizeof(gObj)    );

  memcpy(oVertex , gVertex , sizeof(oVertex) );
  memcpy(oObj, gObj, sizeof(oObj) );

  VCount = VCountL[l];
  FCount = FCountL[l];
  OCount = OCountL[l];
  memcpy(FbyTCount, FbyTCountL[l], 8*4);  

  for (int o=0; o<OCount; o++) ovbegin[o] = 2048;
  for (o=0; o<OCount; o++) 
	  for (int v=0; v<VCount; v++) 
			if (gVertex[v].owner == o) 
			   if (ovbegin[o]==2048) ovbegin[o]=v;
  for (o=0; o<OCount; o++) if (ovbegin[o]==2048) ovbegin[o]=0;    
}
/*
void Load3dfNew(char* FName, HANDLE hfile)
{
   DWORD l;
   ReadFile( hfile, &TCount,      4,         &l, NULL );
   for (int t=0; t<TCount; t++) {
     ReadFile( hfile, &Textures[t].tname,      16,         &l, NULL );     
     char tname[128];
     if (strlen(ModelPath)>4)
       wsprintf(tname, "%s\\%s", ModelPath, Textures[t].tname);
     else
       wsprintf(tname, "%s%s", ModelPath, Textures[t].tname);
     if (strstr(Textures[t].tname, ".bmp"))  LoadTextureBMP(t, tname);
     if (strstr(Textures[t].tname, ".tga"))  LoadTextureTGA(t, tname);
     // load texture //
   }

   ReadFile( hfile, &LodCount,      4,         &l, NULL );
   //LodCount = 1;

   for (int lc=0; lc<LodCount; lc++) {
       ReadFile( hfile, &VCountL[lc],      4,         &l, NULL );
       ReadFile( hfile, &FCountL[lc],      4,         &l, NULL );
       ReadFile( hfile, &OCountL[lc],      4,         &l, NULL );

       ReadFile( hfile, gVertexL[lc],    VCountL[lc]*sizeof(TPoint3d), &l, NULL );
       ReadFile( hfile, gFace2,          FCountL[lc]*sizeof(TFace2), &l, NULL );
       ReadFile( hfile, gObjL[lc],       OCountL[lc]*sizeof(TObj), &l, NULL );

       SetFilePointer(hfile, FCountL[lc], NULL, FILE_CURRENT);
       ReadFile( hfile, &FbyTCountL[lc][0],   TCount*4, &l, NULL );

       for (int f=0; f<FCountL[lc]; f++) {
        gFaceL[lc][f].v1 = gFace2[f].v1;
        gFaceL[lc][f].v2 = gFace2[f].v2;
        gFaceL[lc][f].v3 = gFace2[f].v3;
        gFaceL[lc][f].Flags = gFace2[f].Flags;
        memcpy(&gFaceL[lc][f].tax, &gFace2[f].tax, 4*6);
       }

       if (!TCount)            
           FbyTCountL[lc][0]=FCountL[lc];       
   }

   if (!TCount) 
      TCount = 1;

   SetLod(0);   
}*/


void Load3df(char* FName)
{
    DWORD l;
	HANDLE hfile = CreateFile(FName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if( hfile==INVALID_HANDLE_VALUE ) {
      char t[128];
      wsprintf(t, "Error opening file: %s", FName);
      MessageBox( NULL, t, "Error", MB_OK|MB_ICONSTOP );
      return;
   }

    DWORD id;
    int v,o,vi=0;
    BOOL TRUECOLOR = FALSE;
    
    ReadFile( hfile, &id,		   4,         &l, NULL );    

	char logt[1024];
    if (id==0x7665694B) {          
        ReadFile( hfile, &id,      4,         &l, NULL );
        if (id!=NEW_3DF_VERSION) {
            CloseHandle(hfile);            
            wsprintf(logt, "Incorrect file version:\n%s", FName);
            MessageBox(g_MWin,logt,"Error",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
            return;
        }
        SetFilePointer(hfile, 128, NULL, FILE_BEGIN);
        Load3dfNew(FName, hfile);
        goto OK;
    }

OK:
   CloseHandle(hfile);
   LoadComplete = TRUE;

   for (o=0; o<OCount; o++) {
        ObjRLkoeff[o][0] = 50.f;
        ObjRLkoeff[o][1] = 50.f;
        ObjRLkoeff[o][2] = 0.f;
        ObjRLkoeff[o][3] = 0.f;
   }

   lstrcpy( SourceModelName, FName );   
   _splitpath( FName, NULL, NULL, ModelName, NULL );

   for (o=0; o<OCount; o++) 
	 strcpy(gObj[o].OName, strlwr(gObj[o].OName));

   BOOL BadNames = FALSE;
   for (o=0; o<OCount; o++) 
	for (int i=0; i<OCount; i++)
	 if (o!=i)
		 if (!strcmp(gObj[o].OName, gObj[i].OName))           
		   BadNames = TRUE;
		 
   ClearTrack();
   memcpy(oVertex , gVertex , sizeof(oVertex) );
   memcpy(oObj, gObj, sizeof(oObj) );
   
   if (BadNames)
	MessageBox(g_MWin,"Model contain duplicated object names.\nSome animation will be lost!","Error",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
   
   int j = CurEditor;
   for( int i = 0; i < 4; i++ )
        CalibrateImage( i );
   CurEditor = j;

   ResetTrackTextures();
   /*
   if (!OrgTextSize) 
	 MessageBox(g_MWin,"Model does not contain texture. Preview mode will be disabled.","Warning",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
   */
}


int GetObjectByName(LPSTR name)
{  
  strlwr(name);
  for (int k=0; k<OCount; k++) {
    if (!strcmp(name, strlwr(gObj[k].OName)) )
     return k;
  }

  return 127;
}


void LoadTrk(char* FName)
{
	HANDLE hfile = CreateFile(FName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 
    if( hfile==INVALID_HANDLE_VALUE ) {
      MessageBox( NULL, "Error opening file!", "Animator", MB_OK|MB_ICONSTOP );
      return;
    }

    int oc,kc;
    char on [32];
    char amissname[128][32];
    char askipname[128][32];
    BOOL afound[128];
    DWORD l;
    int amisscnt = 0;
    int askipcnt = 0;

    for (l=0; l<128; l++) afound[l]=FALSE;
	 
	ReadFile(hfile, &oc,  4,  &l,  NULL );
    ReadFile(hfile, &kc,  4,  &l,  NULL );
    ReadFile(hfile, &RotSequence,  4,  &l,  NULL );
    ReadFile(hfile, &aniKPS,  4,  &l,  NULL );
    if (RotSequence!=1) RotSequence = 0;
    CurFrame = 0;

    for (int o=0; o<oc; o++) {
      ReadFile(hfile, on, 32, &l, NULL);
      int i = GetObjectByName(on);
      if (i==127) {
        strcpy(amissname[amisscnt],on);
        amisscnt++;
      }
      afound[i] = TRUE;
      int ak,ck;
      ReadFile(hfile, &ak,  4,  &l,  NULL );
      for (int k=0; k<ak; k++) {
       ReadFile(hfile, &ck,  4,  &l,  NULL );
       ReadFile(hfile, &Track[ck][i], sizeof(TKey), &l, NULL);
	   
	   Track[ck][i].Yfi*=-1;
       /*
       Track[ck][i].Xfi*=-1;
       Track[ck][i].Zfi*=-1;       
       Track[ck][i].ddy*= 2;
       Track[ck][i].ddz*=-2;
       Track[ck][i].ddx*=-2;
       */
      }
    }

    CloseHandle(hfile);
	
    LoadComplete = TRUE;
    UpdateTrackInfo();
    for (o=0; o<OCount; o++) {
      Track[0][o].active = TRUE;
      Track[MaxFrame][o].active = TRUE;
    }

    
    for (o=0; o<OCount; o++) 
     if (!afound[o]) {
      strcpy(askipname[askipcnt],gObj[o].OName);
      askipcnt++;
     }

    if (askipcnt) {
      char mess[1024];
      mess[0] = 0;
      strcat(mess,"These objects have no animation:\n  ");
      for (o=0; o<askipcnt; o++) {
       strcat(mess,askipname[o]);
       strcat(mess,"\n  ");       
      }
      MessageBox( NULL, mess, "Warning", MB_OK|MB_ICONWARNING );
    }

    if (amisscnt) {
      char mess[1024];
      mess[0] = 0;
      strcat(mess,"These animations have no owners\n  ");
      for (o=0; o<amisscnt; o++) {
       strcat(mess,amissname[o]);
       strcat(mess,"\n  ");       
      }
      MessageBox( NULL, mess, "Warning", MB_OK|MB_ICONWARNING );
    }
	ResetTrackTextures();
}


BOOL OpenModel( HWND hWnd)
{
	OPENFILENAME OpenFileName;
	FNAME sFName; 	
	TCHAR szFile[MAX_PATH] = "\0";
    strcpy( szFile, "");

	// Fill in the OPENFILENAME structure to support a template and hook.
	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = hWnd;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "Original model files [*.3df]\0*.3df\0\0";
    OpenFileName.lpstrCustomFilter = 0;
	OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = LastFilterIndex;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (ModelPath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                      else OpenFileName.lpstrInitialDir = ModelPath;
    OpenFileName.lpstrTitle        = "Open a File";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = NULL;
    OpenFileName.lCustData         = (LPARAM)&sFName;
	OpenFileName.lpfnHook 		   = (LPOFNHOOKPROC)OpenDlgProc;
	OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING;
	

	// Call the common dialog function.
    if (GetOpenFileName(&OpenFileName)) {
		LastFilterIndex = OpenFileName.nFilterIndex;
		ClearModel();
        ClearTrack();

		LoadComplete = FALSE;
		
        ParseFileName(OpenFileName.lpstrFile, ModelPath, NULL);
        Load3df(OpenFileName.lpstrFile);       
              
        if (!LoadComplete) ClearModel();
        UpdateLodKeys();
	} 

   InitIK();
   RefreshFrame();
   SetWindowTitle("");

   return 0;
}

BOOL OpenTrack( HWND hWnd)
{
	OPENFILENAME OpenFileName;
	FNAME sFName; 	
	TCHAR szFile[MAX_PATH] = "\0";
    strcpy( szFile, "");

	// Fill in the OPENFILENAME structure to support a template and hook.
	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = hWnd;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "Animation Track [*.trk]\0*.trk\0\0";
    OpenFileName.lpstrCustomFilter = 0;
	OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (TrackPath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                      else OpenFileName.lpstrInitialDir = TrackPath;    
    OpenFileName.lpstrTitle        = "Open a Track";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = NULL;
    OpenFileName.lCustData         = (LPARAM)&sFName;
	OpenFileName.lpfnHook 		   = (LPOFNHOOKPROC)OpenDlgProc;
	OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_NOCHANGEDIR | OFN_EXPLORER| OFN_HIDEREADONLY | OFN_ENABLESIZING;
	

	// Call the common dialog function.
    if (GetOpenFileName(&OpenFileName)) {		
		ClearTrack();				
        lstrcpy(SourceTrackName, OpenFileName.lpstrFile );
        LoadTrk(OpenFileName.lpstrFile);
        lstrcpy( SourceTrackName, OpenFileName.lpstrFile );   
        _splitpath(OpenFileName.lpstrFile, NULL, NULL, TrackName, NULL );     
        ParseFileName(OpenFileName.lpstrFile, TrackPath, NULL);
	} 

   RefreshFrame();
   SetWindowTitle("");
   return 0;
}


void SaveModel(char* FName)
{
	HANDLE hfile = CreateFile(FName, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 
    if( hfile==INVALID_HANDLE_VALUE ) {
      MessageBox( NULL, "Error opening file!", "Animator", MB_OK|MB_ICONSTOP );
      return;
    }
	 
   DWORD l;
   WriteFile( hfile, &TCount,      4,         &l, NULL );

   for (int t=0; t<TCount; t++)
       WriteFile( hfile, &Textures[t].tname,      16,         &l, NULL );

   WriteFile( hfile, &LodCount,      4,         &l, NULL );

   //LodCount = 1;
   for (int lc=0; lc<LodCount; lc++) {

       for (int f=0; f<FCountL[lc]; f++) {
         gFace2[f].v1 = gFaceL[lc][f].v1;
         gFace2[f].v2 = gFaceL[lc][f].v2;
         gFace2[f].v3 = gFaceL[lc][f].v3;
         gFace2[f].Flags = gFaceL[lc][f].Flags;
         memcpy(&gFace2[f].tax, &gFaceL[lc][f].tax, 4*6);
       }

       WriteFile( hfile, &VCountL[lc],      4,         &l, NULL );
       WriteFile( hfile, &FCountL[lc],      4,         &l, NULL );
       WriteFile( hfile, &OCountL[lc],      4,         &l, NULL );

       WriteFile( hfile, gVertexL[lc],    VCountL[lc]*sizeof(TPoint3d), &l, NULL );
       WriteFile( hfile, gFace2,          FCountL[lc]*sizeof(TFace2), &l, NULL );
       WriteFile( hfile, gObjL[lc],       OCountL[lc]*sizeof(TObj), &l, NULL );

       SetFilePointer(hfile, FCountL[lc], NULL, FILE_CURRENT);
       WriteFile( hfile, &FbyTCountL[lc][0],   TCount*4, &l, NULL );
   }

     SetLod(0);
    return;
}

void SaveTrack(char* FName)
{
	
	HANDLE hfile = CreateFile(FName, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 
    if( hfile==INVALID_HANDLE_VALUE ) {
      MessageBox( NULL, "Error opening file!", "Animator", MB_OK|MB_ICONSTOP );
      return;
    }
    
    DWORD l;
	SetWindowTitle("Saving...");    
	WriteFile(hfile, &OCount,    4,  &l,  NULL );
    WriteFile(hfile, &MaxFrame,  4,  &l,  NULL );
    WriteFile(hfile, &RotSequence,  4,  &l,  NULL );
    WriteFile(hfile, &aniKPS,  4,  &l,  NULL );
    for (int o=0; o<OCount; o++) {
      WriteFile(hfile, gObj[o].OName, 32, &l, NULL); 
      int ak = 0;
      int k;
      for (k=0; k<=MaxFrame; k++)
       if (Track[k][o].active) ak++;
      WriteFile(hfile, &ak,  4,  &l,  NULL );

      for (k=0; k<=MaxFrame; k++)
       if (Track[k][o].active) {
        WriteFile(hfile, &k,  4,  &l,  NULL );

	    Track[k][o].Yfi*=-1;
     
        WriteFile(hfile, &Track[k][o], sizeof(TKey), &l, NULL);

		Track[k][o].Yfi*=-1;
       }
    }	    

    CloseHandle(hfile);
    iChanged = 0; 
    SetWindowTitle("Saved.");   
	SetWindowTitle("Save disabled.");   
}



void SaveAs()
{
    OPENFILENAME OpenFileName;
    char szFile[MAX_PATH] = "\0";
    strcpy( szFile, TrackName);
    strcat( szFile, ".trk");
    // Fill in the OPENFILENAME structure to support a template and hook.
    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = g_MWin;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "Animation Track [*.trk]\0*.trk\0\0";
    OpenFileName.lpstrCustomFilter = 0;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (TrackPath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                      else OpenFileName.lpstrInitialDir = TrackPath;
    OpenFileName.lpstrTitle        = "Save File As";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = "trk";
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook                  = (LPOFNHOOKPROC)OpenDlgProc;
    OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER| OFN_HIDEREADONLY | OFN_ENABLESIZING;

        // Call the common dialog function.
    if (!GetSaveFileName(&OpenFileName)) return;
    strcpy(SourceTrackName, szFile);
    _splitpath( szFile, NULL, NULL, TrackName, NULL );     
    ParseFileName(szFile, TrackPath, NULL);

    SaveTrack(SourceTrackName);
}

void SaveIK(char* FName)
{
	HANDLE hfile = CreateFile(FName, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 
    if( hfile==INVALID_HANDLE_VALUE ) {
      MessageBox( NULL, "Error opening file!", "Animator", MB_OK|MB_ICONSTOP );
      return;
    }

	int o;
	LinkResource2 Lres[256];
	for(o=0;o<OCount;o++) {
		for(int a=0;a<3;a++) {
			Lres[o].restrict[a] = LInfo[o].restrict[a];
			Lres[o].minFi[a] = LInfo[o].minFi[a];
			Lres[o].maxFi[a] = LInfo[o].maxFi[a];
		}
	   Lres[o].mass = LInfo[o].mass;
	}
	
    DWORD l, ID = 0x3DFE0914;
	int ver = 2;
    WriteFile( hfile, &ID,		4,         &l, NULL );
	WriteFile( hfile, &ver,		4,         &l, NULL );
	WriteFile( hfile, &OCount,	4,         &l, NULL );

	for(o=0;o<OCount;o++) {
		//int len = strlen(gObj[o].OName);
		WriteFile( hfile, gObj[o].OName, 32,  &l, NULL );
	}

	WriteFile( hfile, Lres,		OCount*sizeof(LinkResource2),  &l, NULL );
	WriteFile( hfile, SphereR,  OCount*4,					   &l, NULL );
	WriteFile( hfile, locBase,  OCount*4,					   &l, NULL );
	WriteFile( hfile, af,		OCount*4,					   &l, NULL );

	WriteFile( hfile, bmp_locked, 4*4, &l, NULL);
	WriteFile( hfile, bmp_Scale,  4*4, &l, NULL);
	WriteFile( hfile, bmp_c,      4*sizeof(POINT), &l, NULL);
	WriteFile( hfile, bmp_pos0,   4*sizeof(Vector3d), &l, NULL);
	WriteFile( hfile, bmp_fname,  32, &l, NULL);
	WriteFile( hfile, bmp_lname,  32, &l, NULL);
	WriteFile( hfile, bmp_Path,   1024, &l, NULL);
		
	WriteFile( hfile, &bmp_Begin,  4, &l, NULL);
	WriteFile( hfile, &bmp_Step,   4, &l, NULL);

	MessageBeep(-1);
	CloseHandle(hfile);
	SetWindowTitle("IK Saved."); 
    return;
}

void SaveIKAs()
{
    OPENFILENAME OpenFileName;
    char szFile[MAX_PATH] = "\0";
    strcpy( szFile, IKName);
    strcat( szFile, ".ikf");
    // Fill in the OPENFILENAME structure to support a template and hook.
    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = g_MWin;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "IK info file [*.ikf]\0*.ikf\0\0";
    OpenFileName.lpstrCustomFilter = 0;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (IKPath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                   else OpenFileName.lpstrInitialDir = IKPath;
    OpenFileName.lpstrTitle        = "Save File As";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = "ikf";
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook          = (LPOFNHOOKPROC)OpenDlgProc;
    OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER| OFN_HIDEREADONLY | OFN_ENABLESIZING;

    // Call the common dialog function.
    if (!GetSaveFileName(&OpenFileName)) return;
    strcpy(SourceIKName, szFile);
    _splitpath( szFile, NULL, NULL, IKName, NULL );
    ParseFileName(szFile, IKPath, NULL);

	SaveIK(SourceIKName);
}

void LoadIK(char* FName)
{
	HANDLE hfile = CreateFile(FName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if( hfile==INVALID_HANDLE_VALUE ) {
      MessageBox( NULL, "Error opening file!", "Animator", MB_OK|MB_ICONSTOP );
      return;
    }

	int OCount_;
    DWORD l, ID, ver;

    ReadFile( hfile, &ID,		4,         &l, NULL );
	ReadFile( hfile, &ver,		4,         &l, NULL );
	ReadFile( hfile, &OCount_,	4,         &l, NULL );

	OCount_;
	if (ID!=0x3DFE0914) {
      MessageBox( NULL, "Unknown file format", "IK Animator", MB_OK|MB_ICONSTOP);
      return;
    }
	if (ver!=2) {
		MessageBox( NULL, "Incorrect version", "IK Animator", MB_OK|MB_ICONSTOP);
		return;
	}

	int o;
	char OName[256][32];
	int locBase_[256];
	int af_[256];
	LinkResource2 Lres[256];

	for(o=0; o<OCount_; o++)
		ReadFile( hfile, OName[o], 32,  &l, NULL );

	ReadFile( hfile, Lres,	   OCount_*sizeof(LinkResource2), &l, NULL );
	ReadFile( hfile, SphereR,  OCount_*4,					  &l, NULL );
	ReadFile( hfile, locBase_, OCount_*4,					  &l, NULL );
	ReadFile( hfile, af_,	   OCount_*4,					  &l, NULL );

	ReadFile( hfile, bmp_locked, 4*4, &l, NULL);
	ReadFile( hfile, bmp_Scale,  4*4, &l, NULL);
	ReadFile( hfile, bmp_c,      4*sizeof(POINT), &l, NULL);
	ReadFile( hfile, bmp_pos0,   4*sizeof(Vector3d), &l, NULL);
	ReadFile( hfile, bmp_fname,  32, &l, NULL);
	ReadFile( hfile, bmp_lname,  32, &l, NULL);
	ReadFile( hfile, bmp_Path,   1024, &l, NULL);

	ReadFile( hfile, &bmp_Begin,  4, &l, NULL);
	ReadFile( hfile, &bmp_Step,   4, &l, NULL);

	CloseHandle(hfile);
	ResetTrackTextures();

	int OFound[256];
	int FoundCnt=0;
	for(o=0; o<OCount; o++) {	// for all model objects
		for(int o_=0; o_<OCount_; o_++) {

			if (strcmp(gObj[o].OName,OName[o_])!=0) continue;
			af[o] = af_[o_];
			locBase[o] = locBase_[o_];
			MessageBeep(-1);
			for(int a=0;a<3;a++) {
				LInfo[o].restrict[a] = Lres[o_].restrict[a];
				LInfo[o].minFi[a] = Lres[o_].minFi[a];
				LInfo[o].maxFi[a] = Lres[o_].maxFi[a];
			}
			LInfo[o].mass = Lres[o_].mass;
			OFound[FoundCnt] = o;
			FoundCnt++;
		}	
	}

	// restrict angels
	for(o=0; o<OCount; o++) 
		for(int a=0;a<3;a++) {
	 	 if (LInfo[o].minFi[a]*180.f/pi<-178) LInfo[o].minFi[a]=-178.f*pi/180.f;
		 if (LInfo[o].minFi[a]*180.f/pi>178) LInfo[o].minFi[a]=178.f*pi/180.f;

		 if (LInfo[o].maxFi[a]*180.f/pi>178) LInfo[o].maxFi[a]=178.f*pi/180.f;
		 if (LInfo[o].maxFi[a]*180.f/pi<-178)  LInfo[o].maxFi[a]=-178.f*pi/180.f;
		}
	UpdateEditorWindow(-1);
}


void OpenIK()
{
	OPENFILENAME OpenFileName;
	ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	FNAME sFName; 	
	TCHAR szFile[MAX_PATH] = "\0";
    strcpy( szFile, "");

	// Fill in the OPENFILENAME structure to support a template and hook.
	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "Original model files [*.ikf]\0*.ikf\0\0";
    OpenFileName.lpstrCustomFilter = 0;
	OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = LastFilterIndex;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (IKPath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                   else OpenFileName.lpstrInitialDir = IKPath;
    OpenFileName.lpstrTitle        = "Open a IK info";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = NULL;
    OpenFileName.lCustData         = (LPARAM)&sFName;
	OpenFileName.lpfnHook 		   = (LPOFNHOOKPROC)OpenDlgProc;
	OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_NOCHANGEDIR | OFN_EXPLORER| OFN_HIDEREADONLY | OFN_ENABLESIZING;
	

	// Call the common dialog function.
    if (GetOpenFileName(&OpenFileName)) {
        lstrcpy( SourceIKName, OpenFileName.lpstrFile );
        _splitpath(OpenFileName.lpstrFile, NULL, NULL, IKName, NULL );
        ParseFileName(OpenFileName.lpstrFile, IKPath, NULL);
		LoadIK(OpenFileName.lpstrFile);
	}
}


void SaveVTable()
{
    OPENFILENAME OpenFileName;
    char szFile[MAX_PATH] = "\0";
    strcpy( szFile, TrackName);
    strcat( szFile, ".vtl");
    TPoint3dsi IntModel[VALUE_X];

    // Fill in the OPENFILENAME structure to support a template and hook.
    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = g_MWin;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "Vertexes Table  [*.vtl]\0*.vtl\0\0";
    OpenFileName.lpstrCustomFilter = 0;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (VTablePath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                       else OpenFileName.lpstrInitialDir = VTablePath;
    OpenFileName.lpstrTitle        = "Save VTable To";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = "vtl";
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook                  = (LPOFNHOOKPROC)OpenDlgProc;
    OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_NOCHANGEDIR | OFN_EXPLORER| OFN_HIDEREADONLY | OFN_ENABLESIZING;

    // Call the common dialog function.
    if (!GetSaveFileName(&OpenFileName)) return;         

	HANDLE hfile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 
    if( hfile==INVALID_HANDLE_VALUE ) {
      MessageBox( NULL, "Error opening file!", "Animator", MB_OK|MB_ICONSTOP );
      return;
    }
    SetWindowTitle("Saving...");   
    ParseFileName(szFile, VTablePath, NULL);

    
    DWORD l;    

	WriteFile(hfile, &VCount, 4, &l, NULL);
    WriteFile(hfile, &aniKPS, 4, &l, NULL);
    WriteFile(hfile, &MaxFrame, 4, &l, NULL);
    int c = CurFrame;
    for (CurFrame=0; CurFrame<=MaxFrame; CurFrame++) {
     RefreshFrameAni();
     RefreshModel();     

     for (int v=0; v<VCount; v++) {
       IntModel[v].x = (int)(gVertex[v].x * 16.f);
       IntModel[v].y = (int)(gVertex[v].y * 16.f);
       IntModel[v].z = (int)(gVertex[v].z * 16.f);
     }
     WriteFile(hfile, IntModel, 6 * VCount, &l, NULL);     
    }
    CloseHandle(hfile);
    
    CurFrame = c;
    RefreshFrame();
    
    SetWindowTitle("Saved.");
}

void Load3dfNew(char* FName, HANDLE hfile)
{
   DWORD l;
   ReadFile( hfile, &TCount,      4,         &l, NULL );
   for (int t=0; t<TCount; t++) {
     ReadFile( hfile, &Textures[t].tname,      16,         &l, NULL );     
     char tname[128];
     if (strlen(ModelPath)>4)
       wsprintf(tname, "%s\\%s", ModelPath, Textures[t].tname);
     else
       wsprintf(tname, "%s%s", ModelPath, Textures[t].tname);
     if (strstr(Textures[t].tname, ".bmp"))  LoadTextureBMP(t, tname);
     if (strstr(Textures[t].tname, ".tga"))  LoadTextureTGA(t, tname);
     // load texture //
   }

   ReadFile( hfile, &LodCount,      4,         &l, NULL );
   //LodCount = 1;
   for (int lc=0; lc<LodCount; lc++) {
       ReadFile( hfile, &VCountL[lc],      4,         &l, NULL );
       ReadFile( hfile, &FCountL[lc],      4,         &l, NULL );
       ReadFile( hfile, &OCountL[lc],      4,         &l, NULL );

       ReadFile( hfile, gVertexL[lc],    VCountL[lc]*sizeof(TPoint3d), &l, NULL );       
       ReadFile( hfile, gFace2,          FCountL[lc]*sizeof(TFace2), &l, NULL );

       ReadFile( hfile, gObjL[lc],       OCountL[lc]*sizeof(TObj), &l, NULL );

       SetFilePointer(hfile, FCountL[lc], NULL, FILE_CURRENT);
       ReadFile( hfile, &FbyTCountL[lc][0],   TCount*4, &l, NULL );   

	   float x[3]; x[0]=x[1]=x[2]=0;
	   float y[3]; y[0]=y[1]=y[2]=0;

       for (int f=0; f<FCountL[lc]; f++) {
        gFaceL[lc][f].v1 = gFace2[f].v1;
        gFaceL[lc][f].v2 = gFace2[f].v2;
        gFaceL[lc][f].v3 = gFace2[f].v3;
        gFaceL[lc][f].Flags = gFace2[f].Flags;
        memcpy(&gFaceL[lc][f].tax, &gFace2[f].tax, 4*6);
		x[0] += gFace2[f].tax;
		y[0] += gFace2[f].tay;
		x[1] += gFace2[f].tbx;
		y[1] += gFace2[f].tby;
		x[2] += gFace2[f].tcx;
		y[2] += gFace2[f].tcy;

       }
       if (!TCount)            
           FbyTCountL[lc][0]=FCountL[lc];
   }

   if (!TCount) 
      TCount = 1;

   SetLod(0);   
}

void Save3DF()
{
    OPENFILENAME OpenFileName;
    char szFile[MAX_PATH] = "\0";
    strcpy( szFile, ModelName);
    strcat( szFile, "_ani.3df");
    TPoint3dsi IntModel[VALUE_X];

    // Fill in the OPENFILENAME structure to support a template and hook.
    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = g_MWin;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "Original model files [*.3df]\0*.3df\0\0";
    OpenFileName.lpstrCustomFilter = 0;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (ModelPath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                      else OpenFileName.lpstrInitialDir = ModelPath;
    OpenFileName.lpstrTitle        = "Save 3df";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = "3df";
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook                  = (LPOFNHOOKPROC)OpenDlgProc;
    OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_NOCHANGEDIR | OFN_EXPLORER| OFN_HIDEREADONLY | OFN_ENABLESIZING;

    // Call the common dialog function.
    if (!GetSaveFileName(&OpenFileName)) return;         

	HANDLE hfile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 
    if( hfile==INVALID_HANDLE_VALUE ) {
      MessageBox( NULL, "Error opening file!", "Animator", MB_OK|MB_ICONSTOP );
      return;
    }
    SetWindowTitle("Saving...");           

    DWORD id,l;
    int v,o,vi=0;    
    
    id=0x7665694B;
    WriteFile( hfile, &id,      4,         &l, NULL );    
    id=NEW_3DF_VERSION;
    WriteFile( hfile, &id,      4,         &l, NULL );

    SetFilePointer(hfile, 128, NULL, FILE_BEGIN);


    WriteFile( hfile, &TCount,      4,         &l, NULL );
    for (int t=0; t<TCount; t++) 
       WriteFile( hfile, &Textures[t].tname,      16,         &l, NULL );          
   

    id = 1;
    WriteFile( hfile, &id,      4,         &l, NULL );   
    
    WriteFile( hfile, &VCountL[CurLod],      4,         &l, NULL );
    WriteFile( hfile, &FCountL[CurLod],      4,         &l, NULL );
    WriteFile( hfile, &OCountL[CurLod],      4,         &l, NULL );

    for (int f=0; f<FCountL[CurLod]; f++) {
        gFace2[f].v1 = gFaceL[CurLod][f].v1;
        gFace2[f].v2 = gFaceL[CurLod][f].v2;
        gFace2[f].v3 = gFaceL[CurLod][f].v3;
        gFace2[f].Flags = gFaceL[CurLod][f].Flags;
        memcpy(&gFace2[f].tax, &gFaceL[CurLod][f].tax, 4*6);
    }

    WriteFile( hfile, gVertex,     VCountL[CurLod]*sizeof(TPoint3d), &l, NULL );       
    WriteFile( hfile, gFace2,      FCountL[CurLod]*sizeof(TFace2), &l, NULL );
    WriteFile( hfile, gObj,        OCountL[CurLod]*sizeof(TObj), &l, NULL );

    SetFilePointer(hfile, FCountL[CurLod], NULL, FILE_CURRENT);
    WriteFile( hfile, &FbyTCountL[CurLod][0],   TCount*4, &l, NULL );   

    CloseHandle(hfile);
    
    SetWindowTitle("Saved.");
}