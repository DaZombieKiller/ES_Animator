//#define STRICT

#ifdef	RC_INVOKED
#include <winuser.h>
#else
#include <windows.h>
#endif
#include "resource.h"

#define NEW_3DF_VERSION 4

// CMF
#define CHUNK_MARKER 1
#define CHUNK_TEXTURE_COUNT 2
#define CHUNK_TEXTURE_NAMES 3
#define CHUNK_FACE_COUNT 8209
#define CHUNK_VERTEX_COUNT 8210
#define CHUNK_FACE_INDICES 8211
#define CHUNK_TEXTURE_INDICES 8220
#define CHUNK_TEXTURE_COORDS1 8224
#define CHUNK_TEXTURE_COORDS2 8225
#define CHUNK_VERTEX_POSITIONS 8227
#define CHUNK_OBJECT_COUNT 61456
#define CHUNK_OBJECT_NAMES 61457
#define CHUNK_OBJECT_POSITIONS 61458
#define CHUNK_OBJECT_INDICES 61460
#define CHUNK_FACE_FLAGS 61475
#define CHUNK_VERTEX_BONE_IDS 61488
#define CHUNK_UNKNOWN 12288

#define macro_vector3d(X,Y,Z,NAME)\
	union {\
		struct {\
		  float X,Y,Z;\
		};\
		Vector3d NAME;\
	};
typedef struct TagVector3d {
 float x,y,z;
} Vector3d;

typedef struct _EditWinInfo {
	char* Title;
	float Scaler, _xc0, _yc0;
	int _xc,_yc;	
	int Zoomed;
    int wflags;
	int CenterLine;
	int PointType;

    int GridStep;
	int GridLines;
	int ShowObj;
	int MouseAlign;
	RECT DefPos;
} TEDITWININFO;

typedef struct _Key {
  int active,acceleration;
  float Xfi, Yfi,Zfi;
  macro_vector3d(ddx,ddy,ddz,ddv);
  float scx, scy, scz;    
} TKey;

typedef struct _ToolKey {
	 int x; int y; int cmd; int IsTool;
     HICON  bmp;
} TTOOLKEY;

typedef struct _PalItem {
	 BYTE R;
	 BYTE G;
	 BYTE B;
} TPalItem;

typedef struct _Point3d {
   union {
	struct {
	float x,y,z; 
	};
	Vector3d pos;
   };
	short owner; 
	short hide;
} TPoint3d;

typedef struct _TTexture {
  LPVOID lpTexture;
  int W,H;
  char tname[16];
} TTexture;

typedef struct _Point3dsi {
	short int x,y,z; 	
} TPoint3dsi;

typedef struct _FacePrev {
   int v1, v2, v3;
   int tax; int tbx; int tcx;
   int tay; int tby; int tcy;
   WORD Flags,DMask;
   int Distant, Next, group;
   char reserv[12];
} TFacePrev;

typedef struct _Face {
   int v1, v2, v3;   
   float tax, tbx, tcx,
         tay, tby, tcy;
   DWORD Flags;
   int Next,Distant;
} TFace;

typedef struct _Face2 {
   WORD   v1, v2, v3;  
#if NEW_3DF_VERSION >= 6
   DWORD  Flags;
#else
   WORD  Flags;
#endif
   float  tax, tbx, tcx,
          tay, tby, tcy;   
} TFace2;

typedef struct _Obj {
   char OName [32];
   macro_vector3d(ox,oy,oz,pos);
   short owner; 
   short hide;
} TObj;

typedef struct _Point4d {
	float x; 
	float y; 
	float z;
} TPoint4d;

typedef struct tagRECORD {
   int size;
   LPVOID data;
} RECORD;

typedef struct tagLINKEDOBJECTS {
   int iTotalObj;
   int obj[ 128 ];

   int iTotalVertex;
   int v[ 1024 ];
} LINKEDOBJECTS;
	
class DEVICECONTEXT {
   public :
      HWND hwnd[4];
      HDC hdc[4], hdcComp[4];
      int count;

      DEVICECONTEXT( HWND* win, int c ) {
         if( c > 4 )
            return;

         count = c;
         for( int i = 0; i < count; i++ ) {
            hwnd[i]    = win[i];
            hdc[i]     = GetDC( hwnd[i] );
            hdcComp[i] = CreateCompatibleDC( hdc[i] );
         }
      }
      ~DEVICECONTEXT( ) {
         for( int i = 0; i < count; i++ ) {
            DeleteDC( hdcComp[ i ] );
            ReleaseDC( hwnd[i], hdc[i] );
         }
      }
};

#define idt_zero           22
#define idt_ani            23

#define idt_zoom           25
#define idt_max1           26
#define idt_maxa           27
#define idt_in             28
#define idt_out            29                   
#define idt_mov            30
#define idt_hold           31
#define idt_fetch          32

#define idt_textured       34
#define idt_flat           35
#define idt_wireframe      36

#define idt_move           40
#define idt_rotate         41
#define idt_rotatel        42
#define idt_3dscale        44
#define idt_2dscale        45

#define idt_ikmove         50
#define idt_setmatrix	   51
#define idt_setsphere	   52

#define idt_m3d        60
#define idt_m2d        61

#define idt_ikrot	   63


#define cbmKey             1
#define cbmLinks           2
#define cbmObject          3

#define MOVE_EVERYTHERE    1
#define MOVE_VERTICAL      2
#define MOVE_HORISONTAL    3

#define VALUE_X            4024
#define MAXKEY             256

#define FLAG_DOUBLESIDE          1
#define FLAG_DARKBACK            2
#define FLAG_OPACITY             4
#define FLAG_TRANSPARENT         8

#define pi 3.1415926535f


#ifdef _MAIN_
 #define _EXTORNOT 
 WORD vbFILL[3] = {0x0000, 7 + 7*32 + 7*32*32, 15 + 15*32 + 15*32*32 };
 int lx = 40;
 int ly =  -100;
 int lz = 0;
#else
 #define _EXTORNOT extern
 extern WORD vbFILL[3];
 extern int lx, ly, lz;
#endif

typedef float TMatrix[3][3];

typedef struct tagBUILDPOS {
   float x, y, z;
} BUILDPOS;

_EXTORNOT HINSTANCE g_hInst;		// the current instance		
_EXTORNOT HWND g_MWin;
_EXTORNOT HWND g_ToolWin;
_EXTORNOT HWND g_MapWin;
_EXTORNOT HANDLE haccelTbl;
_EXTORNOT LINKEDOBJECTS lobj;

_EXTORNOT LPVOID lpVideoBuf;
_EXTORNOT LPVOID lpKeyBuf;
_EXTORNOT TTexture Textures[8+8912];


_EXTORNOT HANDLE GHeap;
_EXTORNOT int ScreenColorDepth;

_EXTORNOT TPalItem pal256[256];
_EXTORNOT void (*HLineT) (void);
typedef void (*HLT) (void);
_EXTORNOT HLT HLineProc[4];
_EXTORNOT HLT HZLineProc[4];

_EXTORNOT HWND g_3dwin [4];
_EXTORNOT TEDITWININFO g_3dwininfo  [4];

_EXTORNOT int CurObject;
_EXTORNOT int iCurFace;
_EXTORNOT WORD CurFlags;
_EXTORNOT BOOL ZBuffer, SHOWSCELETON, SHOWBACK, SHOWMODEL, SHOWMODELT, SHOWSPHERES, DisableVT;
_EXTORNOT int LastFilterIndex;
_EXTORNOT int iCaptionMenuOffset;

_EXTORNOT int CurTool;
_EXTORNOT int CurEditor;
_EXTORNOT int CurCommand;
_EXTORNOT int ToolKeyCnt;
_EXTORNOT TTOOLKEY ToolKey[64];
_EXTORNOT int MouseCaptured;

_EXTORNOT int UserDrawMode;
_EXTORNOT int UserFillColor;




_EXTORNOT HPEN g_hpen3d;
_EXTORNOT HPEN g_hpen3dUp;
_EXTORNOT HPEN g_hpen3dDn;

_EXTORNOT HPEN g_hpenF;
_EXTORNOT HPEN g_hpenE;
_EXTORNOT HPEN g_hpenD;
_EXTORNOT HPEN g_hpenC;
_EXTORNOT HPEN g_hpenB;
_EXTORNOT HPEN g_hpenA;
_EXTORNOT HPEN g_hpen9;
_EXTORNOT HPEN g_hpen8;
_EXTORNOT HPEN g_hpen7;
_EXTORNOT HPEN g_hpen6;
_EXTORNOT HPEN g_hpen5;
_EXTORNOT HPEN g_hpen4;
_EXTORNOT HPEN g_hpen3;
_EXTORNOT HPEN g_hpen2;
_EXTORNOT HPEN g_hpen1;
_EXTORNOT HPEN g_hpen0;
_EXTORNOT HPEN g_hpen_R;
_EXTORNOT HPEN g_hpen_B;
_EXTORNOT HPEN g_hpen_B2;
_EXTORNOT HPEN hpenBP;
_EXTORNOT HPEN hpenCross;
_EXTORNOT HPEN hpenCurrentFaceV;
_EXTORNOT HPEN hpenCurrentFaceH;
_EXTORNOT HPEN hpenRed, hpenBlue, hpenMagenta, hpenGreen;

_EXTORNOT HBRUSH g_mybrush[64];
_EXTORNOT HBRUSH g_mypen[64];
_EXTORNOT HBRUSH hbrNewPoint, hbrRed, hbrBlue, hbrCyan, hbrGreen, hbrLGreen,
                 hbrMagenta, hbr3DFace, hbrSelPoint;
_EXTORNOT HCURSOR hcurArrow,hcurRect,hcurMoveHV,hcurMoveH,hcurMoveV,hcurHand;

// Model Definition
_EXTORNOT char ModelName[20], TrackName[20], IKName[20];
_EXTORNOT char SourceTrackName[ 1024 ], SourceModelName[ 1024 ], SourceIKName[ 1024 ];
_EXTORNOT char ModelPath[255], TrackPath[255], VTablePath[255], IKPath[255];

_EXTORNOT int VCount, FCount, OCount;
_EXTORNOT int tmpVCount, tmpFCount, tmpOCount;
_EXTORNOT int OrgTextSize;
_EXTORNOT int OrgTextHeight;
_EXTORNOT int GammaCorection;

_EXTORNOT TMatrix   gMatrix[256];
_EXTORNOT TPoint3d  ORDelta [256], ODelta[256];

//======= lods ===========//
_EXTORNOT int       VCountL[4], FCountL[4], OCountL[4];
_EXTORNOT TPoint3d  gVertexL[4][VALUE_X];
_EXTORNOT TFace     gFaceL  [4][VALUE_X];
_EXTORNOT TObj      gObjL   [4][256];
_EXTORNOT int       FbyTCountL[4][8];

_EXTORNOT TPoint3d  gVertex[VALUE_X];
_EXTORNOT TPoint3d  oVertex[VALUE_X];
_EXTORNOT int       hidden[VALUE_X];
_EXTORNOT TPoint4d  rVertex[VALUE_X];
_EXTORNOT TFace     gFace [VALUE_X];
_EXTORNOT TFace2    gFace2[VALUE_X];
_EXTORNOT TFacePrev gFaceP[VALUE_X];
_EXTORNOT int       ovbegin[256];
_EXTORNOT TObj      gObj[256];
_EXTORNOT TObj      oObj[256];
_EXTORNOT POINT     scrc[VALUE_X];
_EXTORNOT POINT     scro[256];
_EXTORNOT TPoint3d  ObjPath[MAXKEY];
_EXTORNOT unsigned int ZSize[VALUE_X];
_EXTORNOT BUILDPOS bp;
_EXTORNOT float     ObjRLkoeff[256][4];
_EXTORNOT int       TCount, LodCount, CurLod;
_EXTORNOT int       FbyTCount[8];


#define IS_ZEROV(V) ((fabs(V.x)<=0.01f) && (fabs(V.y)<=0.01f) && (fabs(V.z)<=0.01f))

//============ animation ===================//
_EXTORNOT int  NeedTrackCure[128];

_EXTORNOT TKey Track[MAXKEY][128];
_EXTORNOT TKey FrameAni[128];
_EXTORNOT TKey FrameTmp[128];
_EXTORNOT TKey ClipboardFrame[128];
_EXTORNOT BOOL WorldKeyA[MAXKEY];
_EXTORNOT BOOL UsedKeyA[MAXKEY];
_EXTORNOT int  KeyDIBOfs,CurFrame,MaxFrame,aniKPS,WorldMode,PathObject,LoopPlay;
_EXTORNOT int  PathCSum,ClipboardMode,CopyLinksCount,
               PathBegin,PathEnd,PathOBegin,PathOEnd,RotSequence;
_EXTORNOT HWND hwndKeyBar,hwndKeyScrl,hwndKeyText,
               hwndAcceleration,hwndAText, hwndLoopPlay, hwndWorld, hwndLRSwap,
               hwndCopyKey,  hwndCopyU,  hwndCopyL,  hwndCopyO,
			   hwndPasteKey, hwndPasteU, hwndPasteL, hwndPasteO,
               hwndLod1, hwndLod2, hwndLod3, hwndLod4,
			   hwndDamage1,hwndDamage2,hwndDamage3,hwndDamage4,
			   hwndDamage5,hwndDamage6,hwndDamage7,hwndDamage8,
			   
			   hwnd3dMode, hwndM1, hwndM2;

_EXTORNOT int DamageMode, DamageMask;

_EXTORNOT int ShiftMode, CtrlMode;


//==========================================//
_EXTORNOT int VideoOffset[1000];
_EXTORNOT RECT rcEditor[ 4 ];
_EXTORNOT int g_ew;
_EXTORNOT int g_xc;
_EXTORNOT int g_yc;
_EXTORNOT int ewh, eww, ezw, ezh, trkww;
_EXTORNOT int iLinkedObject;
_EXTORNOT float g_scaler;
_EXTORNOT float g_sina;
_EXTORNOT float g_cosa;
_EXTORNOT float g_sinb;
_EXTORNOT float g_cosb;
_EXTORNOT float g_alpha;
_EXTORNOT float g_beta;
_EXTORNOT float DX, DY, DZ, alpha, betha, scale; // global deltas
_EXTORNOT int DivTbl[10240];
_EXTORNOT LPVOID lpHoldBuffer;
_EXTORNOT char szWinTitle[ 255 ];

_EXTORNOT POINT __cursor_pos;
_EXTORNOT POINT lastmpos;
_EXTORNOT HBITMAP g_MemBMP;
_EXTORNOT HBITMAP g_DIBBMP, g_DIBKEY;
_EXTORNOT int AssignToAll;

_EXTORNOT HBITMAP hbmp;
_EXTORNOT DEVICECONTEXT* dc;


_EXTORNOT POINT pInsPoint[ 3 ];
_EXTORNOT int ylo, yhi, w, h, x, y, Current;
_EXTORNOT int iFacePoint, iNextObject;
_EXTORNOT int NewFace[3];
_EXTORNOT int iChanged;
_EXTORNOT BOOL RevView;

_EXTORNOT int iMoveMode,iRotMode;
_EXTORNOT int yMaxTex;

_EXTORNOT POINT MousePos;
_EXTORNOT int tmpCurEditor;
_EXTORNOT float tmpXC0, tmpYC0;

_EXTORNOT char szFPS[ 16 ];

_EXTORNOT float gDX, gDY, gDZ; // by me

_EXTORNOT float acx,acy,acz,asx,asy,asz;
_EXTORNOT TMatrix M,Mt;
void CalcMatrix(float Fx, float Fy, float Fz);
void RotatePoint(float &xx,float &yy,float &zz);


long APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM );
BOOL APIENTRY About(HWND, UINT, WPARAM, LPARAM );
BOOL APIENTRY Info(HWND, UINT, WPARAM, LPARAM );
// IK
BOOL APIENTRY SetIKParams( HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY SetPhysic( HWND hDlg, UINT message, UINT wParam, LONG lParam);

BOOL APIENTRY IKProp( HWND hDlg, UINT message, UINT wParam, LONG lParam);

_EXTORNOT int IKCtrlMode;
_EXTORNOT int IKCtrlJoints;

// bitmaps
BOOL APIENTRY TrackBmpDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam);

BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
BOOL OpenModel( HWND );
BOOL OpenTrack( HWND );

void ProcessCapturedMouse();
void ExecuteCommand(int);
void ExecuteCommandKey(HWND);
void DoZoom(void);

void CreateEditorWindows( void );
void CenterWindow(HWND hwnd);
void CenterWindowToMouse(HWND hwnd);
void ClearModel(void);

void DrawKeyDIB();
void DrawKeyBar(HWND, HDC);
void ScrollKeyBar(int, int);
void CheckCurFrame();
void UpdateKeyBar();
void UpdateLodKeys();
void UpdateTrackInfo();
void ClearTrack(void);
void MirrorChild(int);

void ProcessAcceleration();
void CreatePath();
void CalcAllAnglesAllFrames();
void SmoothObject(int f, int o);
void SmoothObjectAllFrames(int o);
void SmoothAllObjectsAllFrames();
void RefreshFrame();
void RefreshModel();
void RefreshObjectAni(int, int);
void RefreshFrameAni();
void DoDeleteKey();
void DoInsertKey();
void PlayAni();
void PlayAniRealTime();
void CalcAutoTrackAngles();
void SetIKBasisAngels(int CurO, float X, float Y, float Z);


void ResetTrackTextures();

void SetLod(int);
void SaveCursorPos(void);
void RestoreCursorPos(void);
void SetMainWindowText(LPSTR);
LRESULT CALLBACK EditWndProc(HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK ToolWndProc(HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK MapperProc(HWND, UINT, WPARAM, LPARAM );
LRESULT APIENTRY KeyWndProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY EditMode( HWND, UINT, WPARAM, LPARAM );
BOOL APIENTRY AProp( HWND, UINT, WPARAM, LPARAM );
BOOL APIENTRY RollTrk( HWND, UINT, WPARAM, LPARAM );
BOOL APIENTRY RLParams( HWND, UINT, WPARAM, LPARAM );
BOOL APIENTRY CamMode( HWND, UINT, WPARAM, LPARAM );

int GetKeyOnCommand(int);
void UpdateEditorWindow(int);

void Delay(unsigned);
void Line(HDC, int, int, int, int);
void DrawRect(HDC, int, int, int, int);
void DrawKey(HDC, int, int, int, int, HPEN, HPEN);
void Draw3dKey(HDC, int, int, int, int, int, HPEN, HPEN,HPEN);
void DrawBar(HDC, int, int, int, int);

void DrawCameraView( HDC );
void DrawNormalView( HDC );
void DrawEditorWindow( HWND, HDC, HDC );

void StopCapture( int OK );
void StartCapture( void );

void SaveTrack(LPSTR);
void SaveAs();
void SaveVTable();

void SaveIKAs();
void OpenIK();
void Load3dfNew(char* FName, HANDLE hfile);
void Save3DF();
void SaveCMF();

void Load3df( char* );
void LoadTrk( char* );
void ConvertPointBack( int, int, float&, float&, float& );

void FinishApplication( void );

void ConvertPoint(float, float, float, int&, int& );
void AddToolKey( int, int, HICON, int, int );
void SetCurPosInfo( int, int );
void FPUstosq( LPVOID, int, WORD );
void CalcRotatedModel( void );
void BuildTree( void );
void BuildBackTree( void );
void Put3Point( HDC, int, int, HBRUSH = 0 );
void BuildTransTree( void );
void CenterWindow( HWND, HWND );




int IsMouseInWindow( int, int );
void WrapCoords( DWORD&, int&, int& );
void ConvertDifference( int, int, float&, float&, float& );
void ConvertDifferencePan( float&, float&, float& );
int ConvertDifferenceToAngle( int, float& );
void PutMouseInWindow( POINT& );
int ConvertDifferenceToScale( int );

void ConvertPointForCurrent( float, float, float, int&, int& );
void PutPointInWindow( POINT& );
int DefineObjectCenter( int sx, int sy );
void SetBuildPos( void );

BOOL CALLBACK OpenDlgProc( HWND, UINT, WPARAM, LPARAM );
BOOL APIENTRY OList( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int CalibrateImage( int );
void HideObject(int o);
void ShowAll();
void ReleaseMouseCapture( int, int );
void BuildModel( void );
void DrawModel( void );
int SaveBmp( WORD*, int, int );

void BeepOnEvent( void );
void DrawToolKey( HDC, int, BOOL );
void ConvertToScreen( int&, int& );
void FromMainToEditorCoords( int&, int& );
void AdjustMouseCoords( POINT& );
void SetWindowTitle( LPSTR );
void ReLocateWindows();
void Renderd3dView();
void Renderd2dView();
void ActivateD3D(HWND hwndD3D);
void ShutDownD3D();
void InitDirectDraw();

int LoadTextureBMP(int t, char* tname, int show_err=1);
void InitVars();

_EXTORNOT float bmp_Scale[4];
//_EXTORNOT POINT bmp_pos[4];
_EXTORNOT POINT bmp_c[4];
_EXTORNOT Vector3d bmp_pos0[4];
_EXTORNOT int bmp_locked[4];
_EXTORNOT char bmp_fname[256];
_EXTORNOT char bmp_lname[256];
_EXTORNOT char bmp_Path[1024];

_EXTORNOT int bmp_Begin;
_EXTORNOT int bmp_Step;

_EXTORNOT int bmp_disabled;

_EXTORNOT Vector3d OVec[256];
_EXTORNOT int      OVecFound[256];

//// Audio
_EXTORNOT char SoundPath[255];
_EXTORNOT char sfxName[32];
int ParseFileName( char* fname, char* path, char* name );
void LoadSoundFX(bool);
void CloseWav();
void PlaySound();
int InitAudio();
void StopSound();

// IK

#include "ik.h"
#include "mathem.h"
#include <crtdbg.h>

_EXTORNOT DWORD l;
#include "FileSys\\templ.h"
#include "FileSys\\files_io.h"
//#include "FileSys\\files_io.cpp"

