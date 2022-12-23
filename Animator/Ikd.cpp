#include <windows.h>    // includes basic windows functionality
#include "animator.h"
#include <math.h>
#include <stdio.h>

float DeltaT,_RealTime,PrevTime;

void CalcLinkMoment(int n, Vector3d R, Vector3d F)
{
   if (n==-1) return;
   Vector3d moment,Waccel;
   MulVectorsVect(F, R, moment);
   ScaleVector(moment, LinksI);
   Waccel = moment;

   ScaleVector(Waccel, DeltaT0);
   LInfo[n].W = AddVectors(LInfo[n].W, Waccel);
}

float SIGN(float f)
{
  if (f<0) return -1.f;
  return 1.f;
}

void ApplyLinkForce0(int n, Vector3d F)
{
	float sc;
	if (n==-1 || locBase[n]) return;			// is the base point
	float absF = VectAbs(F);

	AbsAppForce[n] += absF;
    AppForce[n] = AddVectors(AppForce[n], F);

	int owner = Links[n].owner;

	Vector3d R = Links[n].v;
	ScaleVector(R, -Links[n].d/128.f);
	CalcLinkMoment(n, R, F);
}

void ApplyLinkForce(int n, Vector3d F)
{
	float sc;
	if (n==-1) return;			// is the base point
	float absF = VectAbs(F);
		
	AbsAppForce[n] += absF;
    AppForce[n] = AddVectors(AppForce[n], F);

	int owner = Links[n].owner;

	Vector3d R = Links[n].v;
	ScaleVector(R, -Links[n].d/128.f);
	CalcLinkMoment(owner, R, F);

	Vector3d F1 = Links[n].v;
	//ScaleVector(F1, 1.f/Links[n].d);
	MulVectorsScal(F1, F, sc);
	//NormVector(F, 1.f);
	//ScaleVector(F, fabs(sc));
	//F1 = F;
	//ScaleVector(F1, fabs(sc)*0.5/absF);
	ScaleVector(F1, sc*0.9);
    //ApplyLinkForce(owner, F1);
}

void ApplyGravity(int n, Vector3d Gforce)
{
	float sc;
	if (n==-1 || locBase[n]) return;			// is the base point
	float absF = VectAbs(Gforce);
	//if (absF<0.0001) return;
	Vector3d R = Links[n].v;
	ScaleVector(R, -Links[n].d/128.0);
	CalcLinkMoment(Links[n].owner, R, Gforce);
}

/*
void ApplyForceToLink(int n, Vector3d F)
{
  int owner = Links[n].owner;
  Vector3d R = Links[owner].v;
  ScaleVector(R,-1.f);
  CalcLinkMoment(n, R, F); 
}*/


void MoveLinksF(int n1)
{
	//if (fabs(movev.x)<0.001 && fabs(movev.y)<0.001 && fabs(movev.z)<0.001) 
	//	return;
	if (locBase[n1]) return;
	if (n1==-1) return;		// is the base point

	int n2 = Links[n1].owner;
	int n3 = Links[n2].owner;

	float sina,cosa , sinb,cosb;
	Vector3d W = LInfo[n1].W;
	ScaleVector(W, DeltaT0);
	float absW = VectAbs(W);
	//LInfo[n1].Wt = absW;
	//Vector3d V = Links[n1].v;
	//Vector3d NewV = V;
	
	if (absW<pi/18000000.0) return;

	Vector3d NewV;

	sina = sin(absW);
	cosa = cos(absW);

	Vector3d Ax = W;
	ScaleVector(Ax, 1.f/absW);
	//NewV = AxisRotateVector(Ax, -sina, cosa, V);
	Vector3d oldv = Links[n1].v;

	for (int ch=0; ch<Links[n1].chCnt; ch++) {
	  int child = Links[n1].children[ch];
	  NewV = AxisRotateVector(Ax, -sina, cosa, Links[child].v);
	  //CheckAnglesRestriction(n1,n2,n3, NewV);
	  Links[child].v = NewV;
	}
/*
	sinb = sin(absW/4);
	cosb = cos(absW/4);

	for (ch=0; ch<Links[n1].chCnt; ch++)  {
	   int child = Links[n1].children[ch];
	   for (int ch0=0; ch0<Links[child].chCnt; ch0++)  {		// inertion
		   int child_= Links[child].children[ch0];
		   NewV = AxisRotateVector(Ax, sinb, cosb, Links[child_].v);
		   Links[child_].v = NewV;
	  }
	 }
*/
/*
	for (ch=0; ch<Links[n1].chCnt; ch++)  {
	  int child = Links[n1].children[ch];
	   for (int ch0=0; ch0<Links[child].chCnt; ch0++)  {
		  int child_ = Links[child].children[ch0];
		  Vector3d P1 = AddVectors(oldv, Links[child_].v);
		  //ScaleVector(P1, -1.f);
		  //ScaleVector(NewV, -1.f);
		  Vector3d childV_ = SubVectors(P1, NewV);
		  NormVector(childV_, 1.f);
		  Links[child_].v = childV_;
	  }
	}
*/
  
/*
	for (int ch=0; ch<Links[n1].chCnt; ch++)  {		// update child with inertion
	  int child = Links[n1].children[ch];	  
	  Vector3d P1 = AddVectors(oldv, Links[child].v);
	  Vector3d P2 = AddVectors(NewV, Links[child].v);
	  Vector3d F = SubVectors(P1, P2);

	  ScaleVector(F, 10);
	  ApplyLinkForce0(child, F);
	 }*/

	//float restricted = CheckAnglesRestriction(n1,n2,n3, NewV);
	//CheckNodechildren(n1,n2, NewV);
}

void MoveLinks_Phys(int n1)
{
	if (n1==-1) return;		// is the base point
	if (locBase[n1]) return;
	int n2 = Links[n1].owner;
	int n3 = Links[n2].owner;

	float sina,cosa , sinb,cosb;
	Vector3d W = LInfo[n1].W;
	ScaleVector(W, DeltaT0);
	float absW = VectAbs(W);
	//Vector3d V = Links[n1].v;
	//Vector3d NewV = V;
	Vector3d NewV;

	sina = sin(absW);
	cosa = cos(absW);

	Vector3d Ax = W;
	ScaleVector(Ax, 1.f/absW);

	for (int ch=0; ch<Links[n1].chCnt; ch++) {
	  int child = Links[n1].children[ch];
	  Basis[child].X = AxisRotateVector(Ax, -sina, cosa, Basis[child].X);
	  Basis[child].Y = AxisRotateVector(Ax, -sina, cosa, Basis[child].Y);
	  Basis[child].Z = AxisRotateVector(Ax, -sina, cosa, Basis[child].Z);
	}

	sinb = sin(absW/2);
	cosb = cos(absW/2);

	for (ch=0; ch<Links[n1].chCnt; ch++)  {
	   int child = Links[n1].children[ch];
	   for (int ch0=0; ch0<Links[child].chCnt; ch0++)  {		// inertion
		   int child_= Links[child].children[ch0];

		   Basis[child_].X = AxisRotateVector(Ax, sinb, cosb, Basis[child_].X);
		   Basis[child_].Y = AxisRotateVector(Ax, sinb, cosb, Basis[child_].Y);
		   Basis[child_].Z = AxisRotateVector(Ax, sinb, cosb, Basis[child_].Z);
	  }
	 }
}

void MoveAllLinks(int n)
{
 for(int i=OCount-1;i>=0;i--) MoveLinksF(i);
 
/*  if (locBase[0]) return;
  Vector3d vmove = AppForce[0];
  ScaleVector( vmove, 1.f/LInfo[0].mass*DeltaT0);
  Track[CurFrame][0].ddx+=vmove.x;
  Track[CurFrame][0].ddy+=vmove.y;
  Track[CurFrame][0].ddz+=vmove.z; */
}


void ProcessSyncro()
{  
   float TimeDt	;
   if (!PrevTime) _RealTime = PrevTime = timeGetTime();

   TimeDt = _RealTime - PrevTime;
   PrevTime = _RealTime;
   _RealTime = timeGetTime();

   DeltaT = (float)TimeDt / 1000.f;

   DeltaT *= TimeScale;
 
 if (DeltaT<0.001) DeltaT = 0.001;
   //if (DeltaT<0.00001f) DeltaT = 0.00001f;
}

void ApplyLinksInertionForce(int n, Vector3d Fin)
{
	int chCnt = Links[n].chCnt;

	Vector3d F = AddVectors(Fin, AppForce[n]);
	for(int ch=0;ch<chCnt;ch++)
	 	  ApplyLinksInertionForce(Links[n].children[ch], F);

	if (!af[n]) return;
	ScaleVector(Fin,-1.f);
//	ApplyLinkForce(n, Fin);
}

void ApplyLinksWorldForces()
{
	ZeroMemory(AppForce, OCount*sizeof(Vector3d));
	ZeroMemory(AbsAppForce, OCount*sizeof(int));

	for(int o=0;o<OCount;o++) {
		if (!af[CurObject]) continue;

		Vector3d W = LInfo[o].W;
		ScaleVector(W, 0.1);
		LInfo[o].W = SubVectors(LInfo[o].W, W);

		Vector3d G = {0,-Gravity,0};
		ScaleVector(G, LInfo[o].mass);

		//ApplyGravity(o, G);
		//if (ObjRes[0].oy>10) 
			ApplyLinkForce(o, G);
	}
	//Vector3d Fin = {0,0,0};
	//ApplyLinksInertionForce(0, Fin);

}


void CheckAllRestrictionsF()
{
	int Restrict;
	for(int i=0;i<OCount;i++) {
		Restrict = 0;
		if (LInfo[i].restrict[0]) if(Track[CurFrame][i].Xfi > LInfo[i].maxFi[0]*180.f/pi) {Track[CurFrame][i].Xfi = LInfo[i].maxFi[0]*180.f/pi; Restrict = 1;}
		if (LInfo[i].restrict[1]) if(Track[CurFrame][i].Yfi > LInfo[i].maxFi[1]*180.f/pi) {Track[CurFrame][i].Yfi = LInfo[i].maxFi[1]*180.f/pi; Restrict = 1;}
		if (LInfo[i].restrict[2]) if(Track[CurFrame][i].Zfi > LInfo[i].maxFi[2]*180.f/pi) {Track[CurFrame][i].Zfi = LInfo[i].maxFi[2]*180.f/pi; Restrict = 1;}

		if (LInfo[i].restrict[0]) if(Track[CurFrame][i].Xfi < LInfo[i].minFi[0]*180.f/pi) {Track[CurFrame][i].Xfi = LInfo[i].minFi[0]*180.f/pi; Restrict = 1;}
		if (LInfo[i].restrict[1]) if(Track[CurFrame][i].Yfi < LInfo[i].minFi[1]*180.f/pi) {Track[CurFrame][i].Yfi = LInfo[i].minFi[1]*180.f/pi; Restrict = 1;}
		if (LInfo[i].restrict[2]) if(Track[CurFrame][i].Zfi < LInfo[i].minFi[2]*180.f/pi) {Track[CurFrame][i].Zfi = LInfo[i].minFi[2]*180.f/pi; Restrict = 1;}
		
		if (Restrict) {
			SET_VECTOR(LInfo[i].W, 0,0,0);
			SET_VECTOR(AppForce[i], 0,0,0);
			AbsAppForce[i] = 0;
		}
	}
}

void CheckLinksCollisions()
{
/*
  // need apply force witf MoveLinks3D();
  Vector3d movev = {0,0,0};
  Vector3d pos0 = {ObjRes[0].ox,ObjRes[0].oy,ObjRes[0].oz};
  UpdateObjPos(ObjRes, 0, pos0);
  for(int o=1;o<OCount;o++) {
	  if (ObjRes[o].oy < 0) {
		  movev.y = -ObjRes[o].oy;
		  MoveLinks3D(Links[o].owner, o, movev);
		  UpdateObjPos(ObjRes, 0, pos0);
	  }	
  }
  if (ObjRes[0].oy<0) {
	  movev.y = -ObjRes[0].oy;
	  MoveLinks3D(Links[0].owner, 0, movev);  }
*/
}

void ProcessLinks()
{
	CopyMemory(ObjRes, gObj, OCount*sizeof(TObj));

    BuildLinks3D(Links, ObjRes);
	ApplyLinksWorldForces();
/*
    SET_VECTOR(LInfo[20].W, 0,1,0);
    NormVector(LInfo[20].W, 10*pi/2*1.f);
*/
    MoveAllLinks(0);
	//Vector3d v = Links[1].v;
	CheckLinksCollisions();

	RecalcGlobalBasisesFromAngles(prevBasis);
    BasisesLoadIdentity(Basis);

	//ChangeObjectAngles(0, Basis[0], 1);
	ChangeObjectAnglesE(0, Basis[0]);
    CheckAllRestrictionsF();
}

float LastUpdateTime;
float dTime = 1.0/40.0;

void ProcessRealPhysic()
{
 LastUpdateTime += DeltaT;
 while (LastUpdateTime>dTime) {
	 //ProcessLinks_Phys();
	 ProcessLinks();
	 LastUpdateTime-=dTime;
	 DeltaT0 = dTime;
}
 //DeltaT0 = LastUpdateTime;
 //ProcessLinks();
 //LastUpdateTime = 0;
 SetWindowTitle("");
}

void StartIKProcess()
{
   IKStarted = 1;
   DeltaT0 = dTime;

	for(int o=0;o<OCount;o++)	 
	  SET_VECTOR(LInfo[o].W, 0,0,0);
}

void EndIKProcess()
{
 IKStarted=0;
 ReActivateIK = 1;
}
