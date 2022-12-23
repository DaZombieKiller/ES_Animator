#include <windows.h>    // includes basic windows functionality

#include "animator.h"   // includes application-specific information
#include <commctrl.h>
#include <stdio.h>
#include <math.h>

void DoZoom()
{
	if (g_3dwininfo[CurEditor].Zoomed==0) {
      for (int w=0; w<4; w++) {
         MoveWindow(g_3dwin[w],2000,2000,2,2,FALSE);
         g_3dwininfo[w].wflags|=0x0001; 
      }

      g_3dwininfo[CurEditor].Scaler *= 2.0f; 
      g_3dwininfo[CurEditor].wflags&=0xFFFE;
	  MoveWindow(g_3dwin[CurEditor],3,23,ezw-3,ezh-3,TRUE);
      g_3dwininfo[CurEditor].Zoomed=1;

      g_3dwininfo[CurEditor].DefPos.left   = 3;
      g_3dwininfo[CurEditor].DefPos.top    = 23+iCaptionMenuOffset;
      g_3dwininfo[CurEditor].DefPos.right  = ezw;
      g_3dwininfo[CurEditor].DefPos.bottom = ezh+20+iCaptionMenuOffset;

      UpdateEditorWindow( CurEditor );
	} else { 
      g_3dwininfo[CurEditor].Scaler /= 2.0f; 	   
      g_3dwininfo[CurEditor].Zoomed=0;

      for( int i = 0; i < 4; i++ ) {
         
         int x = 3 + (i & 1) * eww; 
         int y = 23 + (i/2) * ewh;

	      g_3dwininfo[i].DefPos.left   = x;
	      g_3dwininfo[i].DefPos.top    = y+iCaptionMenuOffset;
	      g_3dwininfo[i].DefPos.right  = x+eww-3;
	      g_3dwininfo[i].DefPos.bottom = y+ewh-3+iCaptionMenuOffset;

         MoveWindow( g_3dwin[i], x, y, eww-3, ewh-3, TRUE );
         g_3dwininfo[i].wflags &= 0xfffe; 
      }

	  InvalidateRect( g_MWin, NULL, TRUE );
      UpdateEditorWindow( -1 );
   }
}

void ExecuteCommandKey(HWND hWnd)
{
	switch (CurCommand)
	{
     case idt_in: 
	     g_3dwininfo[CurEditor].Scaler*=(float)1.25;
		 UpdateEditorWindow(CurEditor);
		 break;

      case idt_out: 
	     g_3dwininfo[CurEditor].Scaler/=(float)1.25;
		 UpdateEditorWindow(CurEditor);	 
		 break;
	 
      case idt_zero:
		  g_alpha = (float)0.0;
		  g_beta = (float)0.0;
          UpdateEditorWindow(3);	  
		 break;

      case idt_zoom :          
         DoZoom();
			 break;

	  case idt_ani:
         CurCommand=IDM_ANI;
         StartCapture();
		 break;
	  default: MessageBox( hWnd, "Unimplemented command key pressed!", "Message", MB_OK );
	}
}

BOOL APIENTRY About( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);
            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {        
                EndDialog(hDlg, TRUE);        
                return TRUE;
            }
            break;
    }
    return FALSE;                           
}



BOOL APIENTRY Info( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char buf[80];	
	MEMORYSTATUS ms;
    switch (message)
    {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);
			ms.dwLength = sizeof(MEMORYSTATUS);
			GlobalMemoryStatus(&ms);

			wsprintf(buf,"bla bla bla...",ScreenColorDepth);			
            SetWindowText(GetDlgItem(hDlg,IDC_VINFO),buf);			
            SetWindowText(GetDlgItem(hDlg,IDC_MINFO),"bla bla bla...");

            wsprintf(buf,"%d",OCount); SetWindowText(GetDlgItem(hDlg,IDC_OCNT),buf);
			wsprintf(buf,"%d",FCount); SetWindowText(GetDlgItem(hDlg,IDC_FCNT),buf);
			wsprintf(buf,"%d",VCount); SetWindowText(GetDlgItem(hDlg,IDC_VCNT),buf);

			return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)   {        
                EndDialog(hDlg, TRUE);        
                return TRUE;   }
            break;
    }
    return FALSE;                           
}

void SetDlgItemFloat(HWND hDlg, UINT item , float f)
{
	char buf[128];
	sprintf(buf,"%1.5f",f);
	SetWindowText(	GetDlgItem(hDlg, item), buf);
}
float GetDlgItemFloat(HWND hDlg, UINT item)
{
	char buf[128];
	GetWindowText(	GetDlgItem(hDlg, item), buf, 128);
	return atof(buf);
}


void EnableEditsByState(HWND hDlg, int TmpSt[3])
{
  for(int i=0;i<3;i++) { 
	  if (TmpSt[0]==BST_CHECKED) {EnableWindow(GetDlgItem(hDlg, IDC_MIN1), TRUE);
								  EnableWindow(GetDlgItem(hDlg, IDC_MAX1), TRUE);}
							else {EnableWindow(GetDlgItem(hDlg, IDC_MIN1), FALSE);
								  EnableWindow(GetDlgItem(hDlg, IDC_MAX1), FALSE);}
	  if (TmpSt[1]==BST_CHECKED) {EnableWindow(GetDlgItem(hDlg, IDC_MIN2), TRUE);
								  EnableWindow(GetDlgItem(hDlg, IDC_MAX2), TRUE);}
							else {EnableWindow(GetDlgItem(hDlg, IDC_MIN2), FALSE);
								  EnableWindow(GetDlgItem(hDlg, IDC_MAX2), FALSE);}
	  if (TmpSt[2]==BST_CHECKED) {EnableWindow(GetDlgItem(hDlg, IDC_MIN3), TRUE);
								  EnableWindow(GetDlgItem(hDlg, IDC_MAX3), TRUE);}
							else {EnableWindow(GetDlgItem(hDlg, IDC_MIN3), FALSE);
								  EnableWindow(GetDlgItem(hDlg, IDC_MAX3), FALSE);}
	}
}

BOOL APIENTRY SetIKParams( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char buf[80];	
	MEMORYSTATUS ms;
	int CheckVal,i, CheckFixPos[2] = {BST_UNCHECKED, BST_UNCHECKED};
	static int ActiveState[3] = {BST_CHECKED, BST_CHECKED, BST_CHECKED};
	int TmpSt[32];

    switch (message)
    {
        case WM_INITDIALOG:                
			{
			CenterWindow(hDlg);
			sprintf(buf, "Set IK Info for %s (%d)",gObj[CurObject].OName, CurObject);
			SetWindowText(hDlg, buf);

			Vector3d nF = AppForce[CurObject];
			NormVector(nF, 1.f);
			SetDlgItemFloat(hDlg,IDC_FX, nF.x);
			SetDlgItemFloat(hDlg,IDC_FY, nF.y);
			SetDlgItemFloat(hDlg,IDC_FZ, nF.z);
			SetDlgItemFloat(hDlg,IDC_FABS, AbsAppForce[CurObject]);
			SetDlgItemFloat(hDlg,IDC_WABS, VectAbs(LInfo[CurObject].W));
			
			SetDlgItemInt(hDlg,IDC_CURX, (int)(Track[CurFrame][CurObject].Xfi), true);
			SetDlgItemInt(hDlg,IDC_CURY, (int)(Track[CurFrame][CurObject].Yfi), true);
			SetDlgItemInt(hDlg,IDC_CURZ, (int)(Track[CurFrame][CurObject].Zfi), true);

			SetDlgItemInt(hDlg,IDC_MIN1, (int)(LInfo[CurObject].minFi[0]*180/pi), true);
			SetDlgItemInt(hDlg,IDC_MIN2, (int)(LInfo[CurObject].minFi[1]*180/pi), true);
			SetDlgItemInt(hDlg,IDC_MIN3, (int)(LInfo[CurObject].minFi[2]*180/pi), true);
			SetDlgItemInt(hDlg,IDC_MAX1, (int)(LInfo[CurObject].maxFi[0]*180/pi), true);
			SetDlgItemInt(hDlg,IDC_MAX2, (int)(LInfo[CurObject].maxFi[1]*180/pi), true);
			SetDlgItemInt(hDlg,IDC_MAX3, (int)(LInfo[CurObject].maxFi[2]*180/pi), true);

			SetDlgItemFloat(hDlg,IDC_EDITMASS, LInfo[CurObject].mass);
				
			CheckVal = locBase[CurObject] ? BST_CHECKED : BST_UNCHECKED;
			SendDlgItemMessage(hDlg,IDC_CHECKBASE,BM_SETCHECK, CheckVal,0);

			CheckVal = CheckFixPos[0] = flagFixPos[CurObject] ? BST_CHECKED : BST_UNCHECKED;
			SendDlgItemMessage(hDlg,IDC_CHECKFIX,BM_SETCHECK, CheckVal,0);

			CheckVal = af[CurObject] ? BST_CHECKED : BST_UNCHECKED;
			SendDlgItemMessage(hDlg,IDC_CHECKFORCE,BM_SETCHECK, CheckVal,0);

			SendDlgItemMessage(hDlg,IDC_ACTIVEX,BM_SETCHECK, ActiveState[0], 0);
			SendDlgItemMessage(hDlg,IDC_ACTIVEY,BM_SETCHECK, ActiveState[1], 0);
			SendDlgItemMessage(hDlg,IDC_ACTIVEZ,BM_SETCHECK, ActiveState[2], 0);

			for(i=0;i<3;i++) TmpSt[i] = LInfo[CurObject].restrict[i] ? BST_CHECKED : BST_UNCHECKED;
			
			EnableEditsByState(hDlg, TmpSt);

			SendDlgItemMessage(hDlg,IDC_RESTRICTX,BM_SETCHECK, TmpSt[0], 0);
			SendDlgItemMessage(hDlg,IDC_RESTRICTY,BM_SETCHECK, TmpSt[1], 0);
			SendDlgItemMessage(hDlg,IDC_RESTRICTZ,BM_SETCHECK, TmpSt[2], 0);

			}
			return (TRUE);

        case WM_COMMAND:
			if (LOWORD(wParam)==IDC_RESTRICTX || LOWORD(wParam)==IDC_RESTRICTY || LOWORD(wParam)==IDC_RESTRICTZ) {
				  TmpSt[0] = SendDlgItemMessage(hDlg,IDC_RESTRICTX,BM_GETCHECK, 0, 0);
				  TmpSt[1] = SendDlgItemMessage(hDlg,IDC_RESTRICTY,BM_GETCHECK, 0, 0);
				  TmpSt[2] = SendDlgItemMessage(hDlg,IDC_RESTRICTZ,BM_GETCHECK, 0, 0);
				  EnableEditsByState(hDlg, TmpSt);
				return TRUE;
			}

			if ((LOWORD(wParam) == IDSETMIN) || (LOWORD(wParam) == IDSETMAX) ||
				(LOWORD(wParam) == IDUPDATEFIX))   {
				TmpSt[0] = SendDlgItemMessage(hDlg,IDC_ACTIVEX, BM_GETCHECK, 0,0);
				TmpSt[1] = SendDlgItemMessage(hDlg,IDC_ACTIVEY, BM_GETCHECK, 0,0);
				TmpSt[2] = SendDlgItemMessage(hDlg,IDC_ACTIVEZ, BM_GETCHECK, 0,0);
				TmpSt[3] = SendDlgItemMessage(hDlg,IDC_CHECKFIX,BM_GETCHECK, 0,0);

				if (LOWORD(wParam) == IDSETMIN) {
				  if(TmpSt[0]==BST_CHECKED) SetDlgItemInt(hDlg,IDC_MIN1, (int)(Track[CurFrame][CurObject].Xfi), true);
				  if(TmpSt[1]==BST_CHECKED) SetDlgItemInt(hDlg,IDC_MIN2, (int)(Track[CurFrame][CurObject].Yfi), true);
		 		  if(TmpSt[2]==BST_CHECKED) SetDlgItemInt(hDlg,IDC_MIN3, (int)(Track[CurFrame][CurObject].Zfi), true);
				}
				if (LOWORD(wParam) == IDSETMAX) {
				  if(TmpSt[0]==BST_CHECKED) SetDlgItemInt(hDlg,IDC_MAX1, (int)(Track[CurFrame][CurObject].Xfi), true);
				  if(TmpSt[1]==BST_CHECKED) SetDlgItemInt(hDlg,IDC_MAX2, (int)(Track[CurFrame][CurObject].Yfi), true);
		 		  if(TmpSt[2]==BST_CHECKED) SetDlgItemInt(hDlg,IDC_MAX3, (int)(Track[CurFrame][CurObject].Zfi), true);
				}
				if (LOWORD(wParam) == IDUPDATEFIX) {
					MessageBeep(-1);
				    
				}

				return TRUE;
			}

            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)   { //ok
				if (LOWORD(wParam) == IDOK) {
					CheckVal = SendDlgItemMessage(hDlg,IDC_CHECKBASE,BM_GETCHECK, 0,0);
					locBase[CurObject] = (CheckVal==BST_CHECKED) ? 1 : 0;

					CheckVal = CheckFixPos[1] = SendDlgItemMessage(hDlg,IDC_CHECKFIX,BM_GETCHECK, 0,0);
					flagFixPos[CurObject] = (CheckVal==BST_CHECKED) ? 1 : 0;

					CheckVal = SendDlgItemMessage(hDlg,IDC_CHECKFORCE,BM_GETCHECK, 0,0);
					af[CurObject] = (CheckVal==BST_CHECKED) ? 1 : 0;
				
				int restrict[6],a;
				restrict[0] = GetDlgItemInt(hDlg,IDC_MIN1, NULL, true);
				restrict[1] = GetDlgItemInt(hDlg,IDC_MIN2, NULL, true);
				restrict[2] = GetDlgItemInt(hDlg,IDC_MIN3, NULL, true);
				restrict[3] = GetDlgItemInt(hDlg,IDC_MAX1, NULL, true);
				restrict[4] = GetDlgItemInt(hDlg,IDC_MAX2, NULL, true);
				restrict[5] = GetDlgItemInt(hDlg,IDC_MAX3, NULL, true);

				// ANGELS & MASS

				  for(a=0;a<3;a++) {
					if (restrict[a]<-179) restrict[a]=-179;
					if (restrict[a+3]>179) restrict[a+3]=179;
				 	LInfo[CurObject].minFi[a] = (float)restrict[a]*pi/180.f;
			 	 	LInfo[CurObject].maxFi[a] = (float)restrict[a+3]*pi/180.f;
				  }

				  LInfo[CurObject].mass = GetDlgItemFloat(hDlg,IDC_EDITMASS);

				// RESTRICT FLAG
				  TmpSt[0] = SendDlgItemMessage(hDlg,IDC_RESTRICTX,BM_GETCHECK, 0, 0);
				  TmpSt[1] = SendDlgItemMessage(hDlg,IDC_RESTRICTY,BM_GETCHECK, 0, 0);
				  TmpSt[2] = SendDlgItemMessage(hDlg,IDC_RESTRICTZ,BM_GETCHECK, 0, 0);
		 		  
				  for(i=0;i<3;i++) {
					  if (TmpSt[0]==BST_CHECKED) LInfo[CurObject].restrict[0] = 1; 
											else LInfo[CurObject].restrict[0] = 0; 
					  if (TmpSt[1]==BST_CHECKED) LInfo[CurObject].restrict[1] = 1; 
											else LInfo[CurObject].restrict[1] = 0; 
					  if (TmpSt[2]==BST_CHECKED) LInfo[CurObject].restrict[2] = 1; 
											else LInfo[CurObject].restrict[2] = 0;	}

				}	// if ok

				if (CheckFixPos[1]==BST_CHECKED)
					if (CheckFixPos[0]==BST_UNCHECKED)
						vecFixPos[CurObject] = gObj[CurObject].pos;
				// OK or cancel: Save Active state
				ActiveState[0] = SendDlgItemMessage(hDlg,IDC_ACTIVEX,BM_GETCHECK, 0,0);
				ActiveState[1] = SendDlgItemMessage(hDlg,IDC_ACTIVEY,BM_GETCHECK, 0,0);
				ActiveState[2] = SendDlgItemMessage(hDlg,IDC_ACTIVEZ,BM_GETCHECK, 0,0);
				
				CheckAllRestrictions();
                EndDialog(hDlg, TRUE);        
                return TRUE;   }
            break;
    }
    return FALSE;                           
}

BOOL APIENTRY IKProp( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char buf[80];	
    switch (message)
    {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);
			CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1+IKCtrlMode);
			SetDlgItemInt( hDlg, IDC_EDIT1, IKCtrlJoints, 0);

			return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)   {
				if (LOWORD(wParam) == IDOK) {
					IKCtrlMode = !IsDlgButtonChecked(hDlg,IDC_RADIO1);
					IKCtrlJoints = GetDlgItemInt( hDlg, IDC_EDIT1, NULL, 0);
				}

                EndDialog(hDlg, TRUE);
                return TRUE;   }
            break;
    }
    return FALSE;                           
}

BOOL APIENTRY SetPhysic( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char buf[80];	
	MEMORYSTATUS ms;
	int CheckVal;
    switch (message)
    {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);
			sprintf(buf,"%1.5f",TimeScale);
			SetWindowText(	GetDlgItem(hDlg, IDC_EDITTIME), buf);
			SetDlgItemInt(hDlg,IDC_EDITG, (int)(Gravity), true);
			SetDlgItemInt(hDlg,IDC_EDITWINDPOWER, 0, true);
			SetDlgItemInt(hDlg,IDC_EDITWINDX, 0, true);
			SetDlgItemInt(hDlg,IDC_EDITWINDY, 0, true);
			SetDlgItemInt(hDlg,IDC_EDITWINDZ, 0, true);
			SetDlgItemInt(hDlg,IDC_EDITDUMP,  0, true);
		//SendDlgItemMessage(hDlg,IDC_CHECKBASE,BM_SETCHECK, CheckVal,0);
			return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)   {
				if (LOWORD(wParam) == IDOK) {
					Gravity = GetDlgItemInt(hDlg,IDC_EDITG, NULL, true);
					GetWindowText( GetDlgItem(hDlg, IDC_EDITTIME), buf, 64 );
					TimeScale = atof(buf);
				}

                EndDialog(hDlg, TRUE);        
                return TRUE;   }
            break;
    }
    return FALSE;                           
}



//============= roll track ========
BOOL APIENTRY RollTrk( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);															
			//SetDlgItemInt(hDlg, IDC_EDIT, (UINT)1, FALSE);            
            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {   
				if (LOWORD(wParam) == IDOK) {
			      BOOL b;                  
				  int roll = GetDlgItemInt(hDlg, IDC_EDIT, &b, FALSE);				  				  				  
                  for (int r=0; r<roll; r++) {
                    memcpy(FrameTmp, Track[0], sizeof(FrameTmp));
                    for (int k=0; k<MaxFrame; k++) memcpy(Track[k], Track[k+1], sizeof(FrameTmp));
                    memcpy(Track[MaxFrame], FrameTmp, sizeof(FrameTmp));                    
                  }
                }
                EndDialog(hDlg, TRUE);        
                return TRUE;
            }
            break;
    }
    return FALSE;                           
}

int GetFilePath(char *Path)
{
   char szFile[MAX_PATH] = "\0";

	// Fill in the OPENFILENAME structure to support a template and hook.
   OPENFILENAME OpenFileName;

	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = g_MWin;
    OpenFileName.hInstance         = g_hInst;
    OpenFileName.lpstrFilter       = 0;  //"uber map [*.ubr]\0*.ubr\0\0"
    OpenFileName.lpstrCustomFilter = 0;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    if (Path[0])
    OpenFileName.lpstrInitialDir   = Path; else
    OpenFileName.lpstrInitialDir   = NULL;
    OpenFileName.lpstrTitle        = "Select Path";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = 0;
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook 		   = (LPOFNHOOKPROC)OpenDlgProc;
    OpenFileName.lpTemplateName    = 0;
    OpenFileName.Flags             = OFN_ENABLEHOOK | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER;
	
	// Call the common dialog function.
   if (!GetOpenFileName(&OpenFileName)) return 0;
   //strcpy(MapFileName, szFile);
   char drv[256], Path_[1024];
   _splitpath( szFile, drv, Path_, 0, 0);
   sprintf(Path, "%s%s", drv, Path_);
   //strcpy(name, szFile);
   return 1;
}


BOOL APIENTRY TrackBmpDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);														         
            SetDlgItemText(hDlg, IDC_EDIT1, bmp_fname);
            SetDlgItemText(hDlg, IDC_EDIT2, bmp_lname);
			SetDlgItemText(hDlg, IDC_EDIT3, bmp_Path);
			CheckDlgButton(hDlg, IDC_CHECK1, bmp_disabled);

			SetDlgItemInt(hDlg, IDC_EDIT4, bmp_Begin,0);
			SetDlgItemInt(hDlg, IDC_EDIT5, bmp_Step,0);
            return (TRUE);

        case WM_COMMAND:
			if (LOWORD(wParam) == IDC_BUTTON1) {
				if (!GetFilePath(bmp_Path)) break;
				SetDlgItemText(hDlg, IDC_EDIT3, bmp_Path);
				break;
			}
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {   
				if (LOWORD(wParam) == IDOK) {
					GetDlgItemText(hDlg, IDC_EDIT1, bmp_fname, 256);
					GetDlgItemText(hDlg, IDC_EDIT2, bmp_lname, 256);
					GetDlgItemText(hDlg, IDC_EDIT3, bmp_Path,  1024);
					bmp_Begin = GetDlgItemInt(hDlg, IDC_EDIT4,0,0);
					bmp_Step = GetDlgItemInt(hDlg, IDC_EDIT5,0,0);

					bmp_disabled = IsDlgButtonChecked(hDlg,IDC_CHECK1);
					ResetTrackTextures();
                }
                EndDialog(hDlg, TRUE);
                return TRUE;
            }
            break;
    }
    return FALSE;                           
}


BOOL APIENTRY RLParams( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);															
			//SetDlgItemInt(hDlg, IDC_EDIT, (UINT)1, FALSE);            
            SetDlgItemInt(hDlg, IDC_EDIT1, (UINT)ObjRLkoeff[CurObject][0], FALSE);
            SetDlgItemInt(hDlg, IDC_EDIT2, (UINT)ObjRLkoeff[CurObject][1], FALSE);
            SetDlgItemInt(hDlg, IDC_EDIT3, (UINT)ObjRLkoeff[CurObject][2], FALSE);
            SetDlgItemInt(hDlg, IDC_EDIT4, (UINT)ObjRLkoeff[CurObject][3], FALSE);
            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {   
				if (LOWORD(wParam) == IDOK) {
			      BOOL b;
				  ObjRLkoeff[CurObject][0] = (float)GetDlgItemInt(hDlg, IDC_EDIT1, &b, FALSE);
                  ObjRLkoeff[CurObject][1] = (float)GetDlgItemInt(hDlg, IDC_EDIT2, &b, FALSE);
                  ObjRLkoeff[CurObject][2] = (float)GetDlgItemInt(hDlg, IDC_EDIT3, &b, FALSE);
                  ObjRLkoeff[CurObject][3] = (float)GetDlgItemInt(hDlg, IDC_EDIT4, &b, FALSE);                  
                }
                EndDialog(hDlg, TRUE);        
                return TRUE;
            }
            break;
    }
    return FALSE;                           
}


//============= edit mode =========
BOOL APIENTRY EditMode( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:                
			CenterWindowToMouse(hDlg);			
			CheckDlgButton(hDlg,IDC_ALLWIN,1);
			CheckDlgButton(hDlg,IDC_CENTER,g_3dwininfo[CurEditor].CenterLine);
			CheckDlgButton(hDlg,IDC_POINT,g_3dwininfo[CurEditor].PointType);			
			CheckDlgButton(hDlg,IDC_SHOWOBJ,g_3dwininfo[CurEditor].ShowObj);
			CheckDlgButton(hDlg,IDC_GRIDLINES,g_3dwininfo[CurEditor].GridLines);
			CheckDlgButton(hDlg,IDC_MOUSE,g_3dwininfo[CurEditor].MouseAlign);
			CheckDlgButton(hDlg,IDC_ALLWIN,AssignToAll);			


			SendDlgItemMessage(hDlg, IDC_COMBO, CB_ADDSTRING,  0, (LPARAM) (LPCTSTR) "None");			
			SendDlgItemMessage(hDlg, IDC_COMBO, CB_ADDSTRING,  0, (LPARAM) (LPCTSTR) "1");			
			SendDlgItemMessage(hDlg, IDC_COMBO, CB_ADDSTRING,  0, (LPARAM) (LPCTSTR) "4");			
			SendDlgItemMessage(hDlg, IDC_COMBO, CB_ADDSTRING,  0, (LPARAM) (LPCTSTR) "8");			
			SendDlgItemMessage(hDlg, IDC_COMBO, CB_ADDSTRING,  0, (LPARAM) (LPCTSTR) "16");
			SendDlgItemMessage(hDlg, IDC_COMBO, CB_ADDSTRING,  0, (LPARAM) (LPCTSTR) "32");
			SendDlgItemMessage(hDlg, IDC_COMBO, CB_ADDSTRING,  0, (LPARAM) (LPCTSTR) "64");
						
			if (g_3dwininfo[CurEditor].GridStep>0)
				SetDlgItemInt(hDlg, IDC_COMBO, (UINT)g_3dwininfo[CurEditor].GridStep, FALSE); else
				SetDlgItemText(hDlg, IDC_COMBO, "None");
						
            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {   
				if (LOWORD(wParam) == IDOK) {				  
			      BOOL b;
                  int pt = (int) IsDlgButtonChecked(hDlg,IDC_POINT);
                  int cl = (int) IsDlgButtonChecked(hDlg,IDC_CENTER); 
				  int gl = (int) IsDlgButtonChecked(hDlg,IDC_GRIDLINES); 
				  int so = (int) IsDlgButtonChecked(hDlg,IDC_SHOWOBJ); 
				  int ma = (int) IsDlgButtonChecked(hDlg,IDC_MOUSE); 				  
				  int gs = GetDlgItemInt(hDlg, IDC_COMBO, &b, FALSE);				  				  
				  AssignToAll=(int) IsDlgButtonChecked(hDlg,IDC_ALLWIN);
                  for (int i=0; i<3; i++) 
				   if (i==CurEditor || AssignToAll)  {    
					  g_3dwininfo[i].PointType = pt;
                      g_3dwininfo[i].CenterLine = cl;
                      g_3dwininfo[i].GridLines = gl;
					  g_3dwininfo[i].ShowObj = so;
                      g_3dwininfo[i].MouseAlign = ma;
					  g_3dwininfo[i].GridStep = gs;
				      UpdateEditorWindow(i);   }
				}
                EndDialog(hDlg, TRUE);        
                return TRUE;
            }

            break;
    }
    return FALSE;                           
}



//============= edit mode =========
BOOL APIENTRY AProp( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);															
			SetDlgItemInt(hDlg, IDC_EDIT, (UINT)aniKPS, FALSE);				            
			SetDlgItemInt(hDlg, IDC_EDIT2, (UINT)DefAngleX, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT3, (UINT)DefAngleY, FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT4, (UINT)DefAngleZ, FALSE);

            SendDlgItemMessage(hDlg,IDC_COMBO, CB_ADDSTRING, 0, (LPARAM) "Z-X-Y");
            SendDlgItemMessage(hDlg,IDC_COMBO, CB_ADDSTRING, 0, (LPARAM) "X-Z-Y");     
            
            SendDlgItemMessage(hDlg,IDC_COMBO, CB_SETCURSEL, RotSequence, 0);
            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {   
				if (LOWORD(wParam) == IDOK) {				  
			      BOOL b;                  
				  aniKPS = GetDlgItemInt(hDlg, IDC_EDIT, &b, FALSE);				  				  				  
				  DefAngleX = GetDlgItemInt(hDlg, IDC_EDIT2, 0, FALSE);
				  DefAngleY = GetDlgItemInt(hDlg, IDC_EDIT3, 0, FALSE);
				  DefAngleZ = GetDlgItemInt(hDlg, IDC_EDIT4, 0, FALSE);

                  RotSequence = SendDlgItemMessage(hDlg,IDC_COMBO, CB_GETCURSEL, 0, 0);
                }
                EndDialog(hDlg, TRUE);        
                return TRUE;
            }

            break;
    }
    return FALSE;                           
}

//========== user mode ================
BOOL APIENTRY CamMode( HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:                
			CenterWindow(hDlg);
			CheckRadioButton(hDlg, IDC_MODE1, IDC_MODE4, IDC_MODE1 + UserDrawMode);
			CheckRadioButton(hDlg, IDC_COLOR1, IDC_COLOR4, IDC_COLOR1 + UserFillColor);
			CheckDlgButton(hDlg,IDC_POINT,g_3dwininfo[3].PointType);
			CheckDlgButton(hDlg,IDC_REVVIEW, RevView);
			CheckDlgButton(hDlg,IDC_ZBUF,ZBuffer);
			SendDlgItemMessage(hDlg,IDC_SLIDER,TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 64) );
            SendDlgItemMessage(hDlg,IDC_SLIDER,TBM_SETPOS, 1, GammaCorection);

            return (TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {   
   				if (LOWORD(wParam) == IDOK) {
				      for (int j=0; j<3; j++)
			    	      if( IsDlgButtonChecked(hDlg,IDC_COLOR1+j) ) {
                        UserFillColor = j;
                        break;
                     }
				      for( j=0; j<4; j++ )
			    	      if( IsDlgButtonChecked( hDlg, IDC_MODE1+j ) ) {
                        UserDrawMode = j;
                        break;
                     }
                  g_3dwininfo[3].PointType = (int) IsDlgButtonChecked(hDlg,IDC_POINT);
                  if (RevView!=IsDlgButtonChecked(hDlg,IDC_REVVIEW)) g_alpha+=pi;
                  RevView = IsDlgButtonChecked(hDlg,IDC_REVVIEW);
                  ZBuffer = (BOOL) IsDlgButtonChecked(hDlg,IDC_ZBUF);
				      GammaCorection = SendDlgItemMessage(hDlg,IDC_SLIDER,TBM_GETPOS, 0, 0 );
				   }
               EndDialog(hDlg, TRUE);                       
               return TRUE;
            }

            break;
    }
    return FALSE;                           
}


//========= TREE ================//
HTREEITEM oitems[128];
TV_INSERTSTRUCT is;
TV_ITEM it;
int _CurObject;


void AddChildFor(HWND hDlg, int father)
{
	for( int i=0; i<OCount; i++ )
	if( gObj[i].owner == father ) {		
		if( father == -1 ) 
         is.hParent = NULL; 
      else 
         is.hParent = oitems[father];
		
		is.hInsertAfter=TVI_LAST;
		if (father==-1) 
         is.item.state = TVIS_BOLD; 
      else 
        is.item.state = 0;
		is.item.stateMask = TVIS_BOLD;
		is.item.pszText = gObj[i].OName;
		is.item.cchTextMax=32;
		is.item.cChildren=0;

		is.item.mask=TVIF_TEXT | TVIF_STATE;
		
	    oitems[i] = (HTREEITEM)
          SendDlgItemMessage(hDlg, IDC_TREEO, TVM_INSERTITEM,
            (WPARAM) 0, (LONG) (LPTV_INSERTSTRUCT) &is);        		
		AddChildFor(hDlg,i);
		SendDlgItemMessage(hDlg, IDC_TREEO, TVM_EXPAND,
            (WPARAM) (UINT) TVE_EXPAND, (LPARAM) (HTREEITEM) oitems[i]);
   }
}


void UpDateTree(HWND hDlg)
{
   TreeView_DeleteAllItems(GetDlgItem(hDlg,IDC_TREEO));
   AddChildFor(hDlg, -1);
   SendMessage(GetDlgItem(hDlg, IDC_TREEO), TVM_SELECTITEM,
                                  TVGN_CARET, (LPARAM) (HTREEITEM) oitems[CurObject]);
}

BOOL APIENTRY OList( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message ) {
        case WM_INITDIALOG:                			
   		       CenterWindow(hDlg);
			   UpDateTree(hDlg);			   
			   return TRUE;

         case WM_NOTIFY:
			   if( (int) wParam == IDC_TREEO) {
			      if ( (((LPNMHDR) lParam)->code) == TVN_SELCHANGING) {				
				      NM_TREEVIEW * pnmtv;
                      pnmtv = (NM_TREEVIEW FAR *) lParam;
				      HTREEITEM newit = pnmtv->itemNew.hItem;
				      for (int i=0; i<OCount; i++)
					   if (newit == oitems[i])
                        _CurObject = i;                     
			      } 
            } 
            break;

         case WM_COMMAND:
			   switch  LOWORD(wParam) {
			    case IDOK:    
                     CurObject = _CurObject;
			    case IDCANCEL:
                     EndDialog(hDlg, TRUE);        
                     return TRUE;
            }
            break;    
   }
   return FALSE;                           
}










int LoadBitmapFile( char* fname )
{
   hbmp = LoadBitmap( g_hInst, MAKEINTRESOURCE( IDB_POINT ) );
   if( hbmp == NULL )
      return 0;

   return 1;
}




void PutPointInWindow( POINT& p )
{
   TEDITWININFO* ptr = &g_3dwininfo[ CurEditor ];

   p.x -= 3;
   p.y -= iCaptionMenuOffset;

   p.y = p.y % (ptr->DefPos.bottom - ptr->DefPos.top);
   p.x = p.x % (ptr->DefPos.right - ptr->DefPos.left);
}



void SetBuildPos( void )
{
   POINT p;
   GetCursorPos( &p );

   int i = CurEditor;
   RECT rc;
   GetWindowRect( g_3dwin[ CurEditor ], &rc );

   if( ( p.x > rc.left && p.x < rc.right ) &&
       ( p.y > rc.top  && p.y < rc.bottom ) ) {

      float x, y, z;

      ConvertPointBack( p.x-rc.left, p.y-rc.top,    x, y, z );
      switch( CurEditor ) {
         case 0:
            bp.x = x;
            bp.y = y;
            break;    
         case 1:
            bp.z = z;
            bp.y = y;
            break;    
         case 2:
            bp.x = x;
            bp.z = z;
            break;    
           }

      
      UpdateEditorWindow( -1 );
   }
}



int DefineObjectCenter( int sx, int sy )
{
   int x, y;
   
   ConvertPointForCurrent( gObj[CurObject].ox, gObj[CurObject].oy, gObj[CurObject].oz, x, y );
   if( abs( x-sx ) <= 3 && abs( y-sy ) <= 3 && !gObj[CurObject].hide )
      return CurObject;   
   
   for( int i = 0; i < OCount; i++ ) {      
      ConvertPointForCurrent( gObj[i].ox, gObj[i].oy, gObj[i].oz, x, y );
      if( abs( x-sx ) <= 3 && abs( y-sy ) <= 3 && !gObj[i].hide )
         return i;
   }

   return -1;
}


