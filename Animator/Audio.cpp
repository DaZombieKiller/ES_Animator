#include <windows.h>  
#include "animator.h"   
#include <dsound.h>
#include <stdio.h>

int iSoundActive;

#undef RELEASE
#ifdef __cplusplus
#define RELEASE(x) if (x != NULL) {x->Release(); x = NULL;}
#else
#define RELEASE(x) if (x != NULL) {x->lpVtbl->Release(x); x = NULL;}
#endif


void CloseAudio();

//////////////////////  //////////////////////  //////////////////////  //////////////////////  
//////////////////////  IO / Loading //////////////////////  //////////////////////  //////////////////////  
//////////////////////  //////////////////////  //////////////////////  //////////////////////  

typedef struct _FNAME
{
	char szTest1[80];		// a test buffer containing the file selected
	char szTest2[80];       // a test buffer containing the file path
} FNAME, FAR * LPFNAME;

typedef struct  
{
    BYTE    RIFF[4];           // "RIFF"
    DWORD   dwSize;          //  Size of data to follow
    BYTE    WAVE[4];         // "WAVE"
    BYTE    fmt_[4];         // "fmt "
    DWORD   dwFormatSize;    // Format size = 16
    WORD    FormatTag;       // Format type (PCM=1)
    WORD    dwCh;            // Number of Channels {1-Mono, 2-Stereo}
    DWORD   dwSRate;         // Sample Rate
    DWORD   dwAvgBPS;        // BytesPerSecond {Ch*BPS*Srate}
    WORD    dwBlkAlign;      // Block align {BPS*Ch}
    WORD    BitsPerSample;   // Sample size {8 or 16}
    BYTE    DATA[4];         // "DATA"
    DWORD   dwDSize;         // Number of Samples
} WaveHeader;

WAVEFORMATEX wf; //Primary wave format
LPDIRECTSOUND lpDS;
LPDIRECTSOUNDBUFFER lpDSBPrimary, lpDSBSecondary;
DSBUFFERDESC dsbd, dsbd2;


int  sfxLength;
short int* sfxData;


void LoadWav(char* FName, bool update)
{
  DWORD l;

  char t[128];
  wsprintf(t,"Loading: %s", FName);
  SetWindowTitle(t);

  if (lpDSBSecondary) { lpDSBSecondary->Stop(); RELEASE(lpDSBSecondary)}

  if (update)
    HeapFree(GHeap, 0, (void*)sfxData);

  HANDLE hfile = CreateFile(FName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   if( hfile==INVALID_HANDLE_VALUE ) {
      char m[128];
	  wsprintf(m,"Error opening file: %s", FName);
      MessageBox( NULL, m, "Character Studio", MB_OK|MB_ICONEXCLAMATION);
      return; }  

   WaveHeader wave;
   
   ReadFile(hfile, &wave, sizeof(wave), &l, NULL);
   
   sfxLength =  wave.dwDSize;

   sfxData = (short int*) HeapAlloc( GHeap, 0, sfxLength );
   if (!sfxData) return;
   
   ReadFile( hfile, sfxData, sfxLength, &l, NULL );

 
   CloseHandle(hfile);
 
  wf.wFormatTag      = wave.FormatTag;
  wf.nChannels       = wave.dwCh;
  wf.nSamplesPerSec  = wave.dwSRate;
  wf.nAvgBytesPerSec = wave.dwAvgBPS;
  wf.nBlockAlign     = wave.dwBlkAlign;
  wf.wBitsPerSample  = wave.BitsPerSample;
  wf.cbSize          = 0;

 // ========= creating secondary ================//
   dsbd2.dwSize = sizeof( DSBUFFERDESC );
   dsbd2.dwFlags = DSBCAPS_GLOBALFOCUS;
   dsbd2.dwBufferBytes = sfxLength;
   dsbd2.lpwfxFormat = &wf; 

  
   HRESULT err = lpDS->CreateSoundBuffer(&dsbd2, &lpDSBSecondary, NULL );
   if( err != DS_OK ) MessageBox(NULL,"Error","CreateSecondary",MB_OK);
   //if( err != DS_OK ) { ParseError("CreateSoundBuffer (secondary)",err); return false;}   
   //printf(" CreateSoundBuffer (secondary)\n");

  LPVOID Buffer1 = NULL, Buffer2 = NULL;
  DWORD BufSize1 = 0, BufSize2 = 0;
  
  lpDSBSecondary->Lock(0,sfxLength,&Buffer1,&BufSize1,&Buffer2,&BufSize2,0);

  CopyMemory(Buffer1,sfxData, sfxLength);
  if ( BufSize2 > 0) CopyMemory(Buffer2,sfxData+BufSize1,BufSize2);

  lpDSBSecondary->Unlock(Buffer1,BufSize1,Buffer2,BufSize2);


   _splitpath(FName, NULL, NULL, sfxName, NULL );

   if (wave.dwSRate != 22050 )
   {
    char rate[127];
    wsprintf(rate,"__%i__", wave.dwSRate);
    strncat(sfxName, rate, 9);
   }
   SetWindowTitle("");
 
  //wsprintf(t,"Loaded %s : %s", FName, sfxName);
  //MessageBox( NULL, t, "Character Studio", MB_OK);

}


void CloseWav()
{
 if (sfxData) HeapFree(GHeap, 0, (void*)sfxData);
 CloseAudio();
}


void LoadSoundFX(bool update)
{

	OPENFILENAME OpenFileName;
	FNAME sFName; 	
	TCHAR szFile[MAX_PATH] = "\0";
    strcpy( szFile, "");
	
	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = g_MWin;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = "Windows PCM WAV [*.wav]\0*.wav\0\0";
    OpenFileName.lpstrCustomFilter = 0;
	OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (SoundPath[0] == 0) OpenFileName.lpstrInitialDir = NULL;
                      else OpenFileName.lpstrInitialDir = SoundPath;
    OpenFileName.lpstrTitle        = "Load Sound FX";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = NULL;
    OpenFileName.lCustData         = (LPARAM)&sFName;
	OpenFileName.lpfnHook 		   = (LPOFNHOOKPROC)OpenDlgProc;
	OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING;
	

if (update)
{
      wsprintf(szFile,"%s\\%s.wav", SoundPath, sfxName);
      LoadWav(szFile, update);
	  return;
}

	// Call the common dialog function.
    if (GetOpenFileName(&OpenFileName)) 
	{				        
        LoadWav(OpenFileName.lpstrFile, false);
        ParseFileName(OpenFileName.lpstrFile, SoundPath, NULL);
	}

   if (g_3dwininfo[3].Zoomed==0) DoZoom();
}


//////////////////////  //////////////////////  //////////////////////  //////////////////////  
//////////////////////  Audio //////////////////////  //////////////////////  //////////////////////  
//////////////////////  //////////////////////  //////////////////////  //////////////////////  
void ParseError(LPSTR em,HRESULT err);



int InitAudio()
{  
   HRESULT err;
   printf("InitAudio\n");

   strcpy(sfxName, "nowave");

   WAVEFORMATEX pwf; //Primary wave format
   
   #define Ch 2
   pwf.wFormatTag      = WAVE_FORMAT_PCM;
   pwf.nChannels       = Ch;
   pwf.nSamplesPerSec  = 44100;
   pwf.nAvgBytesPerSec = 44100*2*Ch;
   pwf.nBlockAlign     = 2*Ch;
   pwf.wBitsPerSample  = 16;
   pwf.cbSize          = 0;

   err = DirectSoundCreate(NULL, &lpDS, NULL);
   if( err != DS_OK ) { ParseError("DirectSoundCreate",err); return false;}
   
   HWND hwnd = GetForegroundWindow();
   err = lpDS->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
   if( err != DS_OK ) { ParseError("SetCooperativeLevel",err); return false;}

// ========= creating primary ================//
   dsbd.dwSize = sizeof( DSBUFFERDESC );
   dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
   dsbd.dwBufferBytes = 0; //must be zero for primary buffer
   dsbd.lpwfxFormat = NULL;
   dsbd.dwReserved = 0;
   

   err = lpDS->CreateSoundBuffer(&dsbd,&lpDSBPrimary,NULL);
   if( err != DS_OK ) { ParseError("Create Primary buffer ",err); return false;}
   printf(" CreateSoundBuffer (primary)\n");
   
   err = lpDSBPrimary->SetFormat(&pwf);
   if( err != DS_OK ) { ParseError("Primary SetFormat",err); return false;}   
   printf(" SetFormat (primary)\n");
    
   return true;
}


void CloseAudio()
{
 printf("CloseAudio\n");
 lpDSBSecondary->Stop();
 RELEASE(lpDSBSecondary);
 RELEASE(lpDSBPrimary);
 RELEASE(lpDS);
}



void PlaySound()
{
  if (!lpDSBSecondary) return;
  lpDSBSecondary->Play(0, 0, DSBPLAY_LOOPING );
}

void StopSound()
{
  if (!lpDSBSecondary) return;
  lpDSBSecondary->Stop();
  lpDSBSecondary->SetCurrentPosition(0);
}


LPSTR TranslateDSError( HRESULT hr ) 
{
    switch (hr) {
	 case DSERR_ALLOCATED:	       return "DSERR_ALLOCATED";
	 case DSERR_CONTROLUNAVAIL:	   return "DSERR_CONTROLUNAVAIL";
	 case DSERR_INVALIDPARAM:	   return "DSERR_INVALIDPARAM";
	 case DSERR_INVALIDCALL:	   return "DSERR_INVALIDCALL";
	 case DSERR_GENERIC:		   return "DSERR_GENERIC";
	 case DSERR_PRIOLEVELNEEDED:   return "DSERR_PRIOLEVELNEEDED";
	 case DSERR_OUTOFMEMORY:	   return "DSERR_OUTOFMEMORY";
	 case DSERR_BADFORMAT:		   return "DSERR_BADFORMAT";
	 case DSERR_UNSUPPORTED:	   return "DSERR_UNSUPPORTED";
	 case DSERR_NODRIVER:		   return "DSERR_NODRIVER";
	 case DSERR_ALREADYINITIALIZED:return "DSERR_ALREADYINITIALIZED";
	 case DSERR_NOAGGREGATION:	   return "DSERR_NOAGGREGATION";
	 case DSERR_BUFFERLOST:		   return "DSERR_BUFFERLOST";
	 case DSERR_OTHERAPPHASPRIO:   return "DSERR_OTHERAPPHASPRIO";
	 case DSERR_UNINITIALIZED:	   return "DSERR_UNINITIALIZED";
     case DSERR_NOINTERFACE:       return "DSERR_NOINTERFACE";
	 default:			           return "Unknown error";}
}
void ParseError(LPSTR em, HRESULT err) { printf( "DirectSound error: %s %s: %Xh\n", em, TranslateDSError(err), err); }






