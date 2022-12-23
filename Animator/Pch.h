// file: pch.h
// precompiled header

//#ifndef __PCH_H__INCLUDED__
//#define __PCH_H__INCLUDED__
#define _d3d

#ifdef _MAIN_
 #define _EXTORNOT 
#else
 #define _EXTORNOT extern
#endif

#define _SCREEN_SAVER
#define DATA_DIR "data"

// win32
#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>

// std C-libries
#include <crtdbg.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <malloc.h>

// OpenGL headers
#ifdef _opengl
   #include <gl\gl.h>
   #include <gl\glu.h>
#endif

#ifdef _d3d
  #include <d3d8.h>
  #include <d3d8types.h>
  #include <d3dtypes.h>
  #include <d3dx8core.h>
#endif

// DirectX
#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>
#include <dsound.h>

#include "..\game\resource.h"

#include "..\macros\templ.h"
#include "..\filesys\files_io.h"

#include "..\mathem\3dmath.h"
#include "..\mesh\mesh.h"
#include "..\game\render.h"
#include "..\camera\camera.h"
#include "..\picture\picture.h"
#include "..\log\log.h"
#include "..\effects\effects.h"
#include "..\input\input.h"

#include "..\game\res.h"
#include "..\game\game.h"

#ifdef _SCREEN_SAVER
#include "scrnsave.h"
extern HINSTANCE hMainInstance;
#endif

//#endif /*__PCH_H__INCLUDED__*/

