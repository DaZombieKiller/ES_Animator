// Inverse Kinematcs
#include <windows.h>    // includes basic windows functionality
#include "animator.h"
#include <math.h>
#include <stdio.h>

#define BASIS_ZXY 1
Vector3d AxisRotateVector(Vector3d Axis, float sinAng, float cosAng, Vector3d V)
{
 Vector3d Vres;

 Vector3d Zaxis,Xaxis,Yaxis;							// move points using current W velocity
 Zaxis = Axis;

 Vector3d tv; 
 tv.x=tv.y=tv.z=1.0;

 MulVectorsVect(tv,   Zaxis, Xaxis); NormVector(Xaxis,1.f);// X = |txZ|
 MulVectorsVect(Zaxis,Xaxis, Yaxis); // Y = |ZxX|

 //NormVector(Yaxis,1.f);
 //NormVector(Zaxis,1.f);

 Vector3d X;

 X.x = V.x*Xaxis.x + V.y*Xaxis.y + V.z*Xaxis.z;	
 X.y = V.x*Yaxis.x + V.y*Yaxis.y + V.z*Yaxis.z;	
 X.z = V.x*Zaxis.x + V.y*Zaxis.y + V.z*Zaxis.z;

 Vector3d Xrot;				
 Xrot.x = X.x*cosAng - X.y*sinAng;	
 Xrot.y = X.x*sinAng + X.y*cosAng;
 Xrot.z = X.z;

 Vres.x = Xrot.x*Xaxis.x + Xrot.y*Yaxis.x + Xrot.z*Zaxis.x;
 Vres.y = Xrot.x*Xaxis.y + Xrot.y*Yaxis.y + Xrot.z*Zaxis.y;
 Vres.z = Xrot.x*Xaxis.z + Xrot.y*Yaxis.z + Xrot.z*Zaxis.z;

 return Vres;
}

// n1 - child
// n2 - father    (node for checking restriction)
// n3 - papa of father

void CalcLinkedBodySpheresR()
{
	//Vector3d *Vert   = model->oVertex;
	//TObj     *Obj    = model->oObj;
	float s;
	for(int o=1;o<OCount;o++) {
	  float maxd=0;
	  int own = oObj[o].owner;
		for(int v=0;v<VCount;v++)
				if (gVertex[v].owner==o) 
				{
				  Vector3d bone = Links[o].v;
				  Vector3d vertex = *(Vector3d*)&oVertex[v].x;
				  Vector3d vec = SubVectors(oObj[o].pos,vertex);
				  MulVectorsScal(bone, vec, s);
				  ScaleVector(bone, s);	// projected vertex vector on the bone vector
				  if (s<0) continue;
				  float d = (vec.x*vec.x   + vec.y*vec.y   + vec.z*vec.z) - 
							(bone.x*bone.x + bone.y*bone.y + bone.z*bone.z);
				  //float d = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
				  if (d>maxd)  maxd=d;
				}

	SphereR[o] = sqrt(maxd);
	}	// all objects
}

void UpdateObjPos(TObj *gObj, IKLink *Links, int n, Vector3d pos)	// for updateAll use : UpdateObjPos(0, obj[0].pos)
{
	Vector3d pos1,v;
	gObj[n].pos = pos;

	for(int cnt=0;cnt<Links[n].chCnt;cnt++) {		// update each child
	 int child = Links[n].children[cnt];
	 v = Links[child].v;
	 ScaleVector(v, -Links[child].d);
	 pos1 = AddVectors(pos, v);
	 UpdateObjPos(gObj, Links, child, pos1);
	}
}

int CalcNewObjBasis2DR(int n1,int n2,int n3,Vector3d &oldV,Vector3d &newV)
{
  Vector3d vdif,vv, gvo, vo_,v;
  TBasis NewB;

  Vector3d RotAx = {0,0,0};

  gvo = oldV;  
  vo_ = GetVectorInLocal(Basis[n2],  oldV);	// get restricted vector
  //NormVector(vo_,1.f);

  vdif = SubVectors(v,gvo);
  float dif = VectAbs(vdif);
  
  float ang,sina,cosa;

  MulVectorsScal(oldV, newV, cosa);
  if (cosa>1) cosa=1;
  if (cosa<-1) cosa=-1;
  sina = sqrt(1-cosa*cosa);

  //if (fabs(cosa)>0.9999) return;			// do not calc new XYZ Angles
  ang = acos(cosa);

  if (fabs(ang)<0.001) return 0;

  ScaleVector(oldV, 100.f);
  ScaleVector(newV, 100.f);
  MulVectorsVect(oldV, newV, vv);
  NormVector(vv, 1.f);

  if (CurEditor==0) if (vv.z<0.0) RotAx.z = -1.f; else RotAx.z = 1.f;
  if (CurEditor==1) if (vv.x<0.0) RotAx.x = -1.f; else RotAx.x = 1.f;
  if (CurEditor==2) if (vv.y<0.0) RotAx.y = -1.f; else RotAx.y = 1.f; 

  vv = RotAx;

  TBasis *PapaBasis;
  if (n3==-1) {
    PapaBasis = &IdentityBasis;
  } else 
	PapaBasis = &Basis[n3];

  // keep n1 gBasis without changing orientation
  NewB = AxisRotateBasis(vv, sina,cosa, Basis[n2]);
  NewB = GetBasisInLocal(*PapaBasis,	NewB); // n2 -> n3 bases

  float rFi[3];
  int restrict;

  restrict = CalcBasisAngles_R3D(n2, NewB, rFi[0],rFi[1],rFi[2]); 
  CalcMatrix(rFi[0], rFi[1], rFi[2]);
  TransposeMatrix();					// get new restricted basis

  NewB = *(TBasis*)&M;
  
  NewB = GetBasisInGlobal(*PapaBasis, NewB);
  newV = GetVectorInGlobal(NewB,  vo_);				// get restricted vector

  Basis[n2] = NewB;									// save new basis

  return 0xFF00|(restrict<<1);		// basisChanged | Restrict1 | Restrict2
  
}


// n1 - dragging object
// n2 - changing basis
// n3 - owner basis
int CalcNewObjBasis3DR(int n1,int n2,int n3,Vector3d &oldV,Vector3d &newV)
{
  Vector3d vv, gvo, vo_,v;
  TBasis NewB;

  gvo = oldV;  
  vo_ = GetVectorInLocal(Basis[n2],  oldV);	// get restricted vector
  //NormVector(vo_,1.f);

  float ang,sina,cosa;

  MulVectorsScal(oldV, newV, cosa);
  if (cosa>1) cosa=1;
  if (cosa<-1) cosa=-1;
  sina = sqrt(1-cosa*cosa);

  //if (fabs(cosa)>0.9999) return;			// do not calc new XYZ Angles
  ang = acos(cosa);

  if (fabs(ang)<0.001) return 0;

  ScaleVector(oldV, 100.f);
  ScaleVector(newV, 100.f);
  MulVectorsVect(oldV, newV, vv);
  NormVector(vv, 1.f);

  TBasis *PapaBasis;
  if (n3==-1) {
    PapaBasis = &IdentityBasis;
  } else 
	PapaBasis = &Basis[n3];

  // keep n1 gBasis without changing orientation
  NewB = AxisRotateBasis(vv, sina,cosa, Basis[n2]);
  NewB = GetBasisInLocal(*PapaBasis,	NewB); // n2 -> n3 bases

  float rFi[3];
  int restrict;

  restrict = CalcBasisAngles_R3D(n2, NewB, rFi[0],rFi[1],rFi[2]); 
  CalcMatrix(rFi[0], rFi[1], rFi[2]);
  TransposeMatrix();					// get new restricted basis

  NewB = *(TBasis*)&M;
  
  NewB = GetBasisInGlobal(*PapaBasis, NewB);
  newV = GetVectorInGlobal(NewB,  vo_);				// get restricted vector

  Basis[n2] = NewB;									// save new basis

  return 0xFF00|(restrict<<1);		// basisChanged | Restrict1 | Restrict2
  
}



void GetchildrenFor(IKLink *Links, TObj *ObjRes, int father)
{
	Links[father].chCnt = 0;
	for(int c=0;c<OCount;c++) {				// check for child
		if (c==father) continue;
		if (ObjRes[c].owner == father) 
				Links[father].children[ Links[father].chCnt++ ] = c;
	}
}

void BuildLinks3D(IKLink *Links,TObj *ObjRes)
{
	if (OCount==0) return;
	int n;
	for(n=0;n<OCount;n++) {
		GetchildrenFor(Links,ObjRes, n);
		Links[n].owner = ObjRes[n].owner;
	}
	for(n=0;n<OCount;n++) {
		if (Links[n].owner==-1) continue;
		int own = Links[n].owner;
		Links[n].v = SubVectors(ObjRes[own].pos,ObjRes[n].pos);
		Links[n].d = VectAbs(Links[n].v);
		Links[n].dd =  Links[n].d*Links[n].d;
		ScaleVector(Links[n].v, 1.f/Links[n].d);
	}
}

void BasisesLoadIdentity(TBasis Basis[])
{
	for(int o=0;o<OCount;o++) {
		SET_VECTOR(Basis[o].X, 1,0,0);
		SET_VECTOR(Basis[o].Y, 0,1,0);
		SET_VECTOR(Basis[o].Z, 0,0,1);
	}
}

TBasis PapaBasis;

void RotateChild3(int papa, TBasis Basis_[])
{
  int n,i;
  for (int ch=0; ch<Links[papa].chCnt; ch++) {		// rotate all children
	 int n = Links[papa].children[ch];

	 Vector3d ObjPos;
//	 SET_VECTOR(ObjPos, gObj[n].ox, gObj[n].oy, gObj[n].oz);

  //   GetVectorInLocal(PapaBasis,  ObjPos);
	 Basis_[n].X = GetVectorInLocal(PapaBasis,  Basis_[n].X);
	 Basis_[n].Y = GetVectorInLocal(PapaBasis,  Basis_[n].Y);
	 Basis_[n].Z = GetVectorInLocal(PapaBasis,  Basis_[n].Z);

  //   RotatePoint(ObjPos);
	 Basis_[n].X = RotateVector(Basis_[n].X);
	 Basis_[n].Y = RotateVector(Basis_[n].Y);
	 Basis_[n].Z = RotateVector(Basis_[n].Z);

//	 GetVectorInGlobal(PapaBasis,  ObjPos);
	 Basis_[n].X = GetVectorInGlobal(PapaBasis,  Basis_[n].X);
	 Basis_[n].Y = GetVectorInGlobal(PapaBasis,  Basis_[n].Y);
	 Basis_[n].Z = GetVectorInGlobal(PapaBasis,  Basis_[n].Z);
/*
     gObj[n].ox = ObjPos.x;
	 gObj[n].oy = ObjPos.y;
	 gObj[n].oz = ObjPos.z;
*/
     RotateChild3(n,Basis_);
   } 
}

void RotateObjectchildren(int no, TBasis MainBasis, TBasis Basis_[], float Xfi, float Yfi, float Zfi)
{
 int s;

  if (fabs(Xfi) < 0.001f && 
      fabs(Yfi) < 0.001f && 
      fabs(Zfi) < 0.001f) return;

     CalcMatrix(Xfi,Yfi,Zfi);

     PapaBasis = MainBasis;

     RotateChild3(no, Basis_);

	 Basis_[no].X = GetVectorInLocal(PapaBasis,  Basis_[no].X);
	 Basis_[no].Y = GetVectorInLocal(PapaBasis,  Basis_[no].Y);
	 Basis_[no].Z = GetVectorInLocal(PapaBasis,  Basis_[no].Z);

  //   RotatePoint(ObjPos);
	 Basis_[no].X = RotateVector(Basis_[no].X);
	 Basis_[no].Y = RotateVector(Basis_[no].Y);
	 Basis_[no].Z = RotateVector(Basis_[no].Z);

//	 GetVectorInGlobal(PapaBasis,  ObjPos);
	 Basis_[no].X = GetVectorInGlobal(PapaBasis,  Basis_[no].X);
	 Basis_[no].Y = GetVectorInGlobal(PapaBasis,  Basis_[no].Y);
	 Basis_[no].Z = GetVectorInGlobal(PapaBasis,  Basis_[no].Z);
}

void axRotateChild(int papa, TBasis Basis_[], Vector3d vv, float sina,float cosa)
{
  int n,i;
  for (n=0; n<OCount; n++) 
	 if (gObj[n].owner==papa) {
	 Vector3d ObjPos;

	 Basis_[n].X = GetVectorInLocal(PapaBasis,  Basis_[n].X);
	 Basis_[n].Y = GetVectorInLocal(PapaBasis,  Basis_[n].Y);
	 Basis_[n].Z = GetVectorInLocal(PapaBasis,  Basis_[n].Z);

  //   RotatePoint(ObjPos);
	 Basis_[n].X = AxisRotateVector(vv, sina,cosa, Basis_[n].X);
	 Basis_[n].Y = AxisRotateVector(vv, sina,cosa, Basis_[n].Y);
	 Basis_[n].Z = AxisRotateVector(vv, sina,cosa, Basis_[n].Z);

//	 GetVectorInGlobal(PapaBasis,  ObjPos);
	 Basis_[n].X = GetVectorInGlobal(PapaBasis,  Basis_[n].X);
	 Basis_[n].Y = GetVectorInGlobal(PapaBasis,  Basis_[n].Y);
	 Basis_[n].Z = GetVectorInGlobal(PapaBasis,  Basis_[n].Z);
/*
     gObj[n].ox = ObjPos.x;
	 gObj[n].oy = ObjPos.y;
	 gObj[n].oz = ObjPos.z;
*/
     axRotateChild(n,Basis_,vv,sina,cosa);
   } 
}


void axRotateObjectchildren(int no, TBasis MainBasis, TBasis Basis_[], Vector3d vv, float sina,float cosa)
{
 int s;

// if(fabs(sina) < 0.01f) return;

 PapaBasis = MainBasis;

   axRotateChild(no, Basis_,vv, sina,cosa);

	Basis_[no].X = GetVectorInLocal(PapaBasis,  Basis_[no].X);
	Basis_[no].Y = GetVectorInLocal(PapaBasis,  Basis_[no].Y);
	Basis_[no].Z = GetVectorInLocal(PapaBasis,  Basis_[no].Z);

  //   RotatePoint(ObjPos);
	Basis_[no].X = AxisRotateVector(vv, sina,cosa, Basis_[no].X);
	Basis_[no].Y = AxisRotateVector(vv, sina,cosa, Basis_[no].Y);
	Basis_[no].Z = AxisRotateVector(vv, sina,cosa, Basis_[no].Z);

//	 GetVectorInGlobal(PapaBasis,  ObjPos);
	Basis_[no].X = GetVectorInGlobal(PapaBasis,  Basis_[no].X);
	Basis_[no].Y = GetVectorInGlobal(PapaBasis,  Basis_[no].Y);
	Basis_[no].Z = GetVectorInGlobal(PapaBasis,  Basis_[no].Z);
}

void MoveLinks3D(int n2, int n1, Vector3d movev)
{
	Moved[n1] = 1;
	if (fabs(movev.x)<0.001 && fabs(movev.y)<0.001 && fabs(movev.z)<0.001) return;

	 //Track[CurFrame][n1].active = TRUE;

	if (n2==StartIKJoint) return;
	if (locBase[n1]) return;		// is the base point
	if (n2==-1) { Track[CurFrame][n1].ddx+=movev.x;
				  Track[CurFrame][n1].ddy+=movev.y;
				  Track[CurFrame][n1].ddz+=movev.z; 
				  return; }

	int n3 = Links[n2].owner;
	if (n3==-1) { Track[CurFrame][n2].ddx+=movev.x;		// 
				  Track[CurFrame][n2].ddy+=movev.y;
				  Track[CurFrame][n2].ddz+=movev.z;  return;}

/*
	if (n3==-1 || locBase[n2]) {			// is the last link
		//ScaleVector(movev, 2.f);
		Vector3d NewV = Links[n1].v;
		ScaleVector(NewV, -Links[n1].d);
		//GetVectorN(NewV, movev);
		NewV = AddVectors(NewV, movev);
		NormVector(NewV, -1.f);
		//CheckAnglesRestriction2D(n1,n2,-1, NewV);
		CheckNodechildren3D(n1,n2, NewV);
		//ScaleVector(NewV, -1.f);
		Links[n1].v = NewV;
		//if (n1==1) MessageBeep(-1);
		return;
	}*/

	float AbsMove;		// abs movev
	Vector3d normMove;	// move vector
	Vector3d v1;		// -(n2 - n1)

	v1 = Links[n1].v;
	ScaleVector(v1, -1.f);
	
	normMove=movev;
	AbsMove = VectAbs(movev);
	ScaleVector(normMove, 1.f/AbsMove);

	Vector3d X0,Y0,Z0;
	float Ang, cosA, sinA;
	MulVectorsScal(v1,normMove, cosA);

	Ang = acos(cosA);
	if (fabs(Ang)<0.001) {
		//Links[n1].v = movev;
		//ScaleVector(Links[n1].v, -1.f);
		MoveLinks3D(n3, n2, movev);
		return;
	}

	//sinA = sqrt(1-cosA*cosA);
	Vector3d NewVec;		// not restricted


	Vector3d NewV = Links[n1].v;
	Vector3d oldV = Links[n1].v;
	//ScaleVector(NewV, -Links[n1].d*0.9f);
	ScaleVector(NewV, -Links[n1].d);
	//GetVectorN(NewV, movev);
	NewVec = AddVectors(NewV, movev);
	NormVector(NewVec, 1.f);
	NewV = NewVec;
	ScaleVector(NewV, -1.f);

	//float restricted = CheckAnglesRestriction(n1,n2,n3, NewV);
	CheckNodechildren3D(n1,n2, NewV);

	Links[n1].v = NewV;
	
	Vector3d transf_v;	// transfer vector
	transf_v = v1;

    //ScaleVector(transf_v, 0);			// transfer move
	ScaleVector(transf_v, AbsMove*cosA);			// transfer move
/*
	if (fabs(restricted)>0.0) {
		X0 = v1;
		MulVectorsVect(X0,normMove, Z0); NormVector(Z0,1.f);
		MulVectorsVect(Z0,X0,Y0);
		normMove = GetVectorInLocal(X0,Y0,Z0, normMove);
		normMove.x = 0;
		normMove = GetVectorInGlobal(X0,Y0,Z0, normMove);
		NormVector(normMove, restricted);
		
		transf_v = normMove;
		ScaleVector(transf_v, AbsMove);
		transf_v = movev;
		//ScaleVector(transf_v, 0);
		//ScaleVector(transf_v, restricted);
		//transf_v = AddVectors(transf_v, normMove);
	} else {
	//ScaleVector(transf_v, AbsMove*cosA);			// transfer move
	}*/
	MoveLinks3D(n3, n2, transf_v);
}

void DragLinks3D(int n, Vector3d v)
{ 
  ScaleVector(v, -1.0);
  
  ZeroMemory(DontCalcObj, OCount*4);
  ZeroMemory(Moved,		  OCount*4);

  CopyMemory(ObjRes, gObj, OCount*sizeof(TObj));

  BuildLinks3D(Links, ObjRes);

  MoveLinks3D( Links[n].owner, n, v);

  RecalcGlobalBasisesFromAngles(Basis);		// this function can be used only at start transforming

  CopyMemory(prevBasis, Basis, OCount*sizeof(TBasis));
  
  BasisesLoadIdentity(Basis);

  ChangeObjectAngles(0,Basis[0],2);

  CheckAllRestrictions();
}

///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// 2D Editors work/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void BuildLinks2D()
{
	if (OCount==0) return;

	int n;
	for(n=0;n<OCount;n++) {
		GetchildrenFor(Links, ObjRes, n);
		Links[n].owner = ObjRes[n].owner;
	}

	for(n=0;n<OCount;n++) {
		if (Links[n].owner==-1) continue;
		int own = Links[n].owner;
		if (CurEditor==0) {
			Links[n].v.x = ObjRes[own].ox - ObjRes[n].ox;
			Links[n].v.y = ObjRes[own].oy - ObjRes[n].oy;
			Links[n].v.z = 0;
			ObjRes[n].oz = 0;
		}
		if (CurEditor==1) {
			Links[n].v.x = 0;
			Links[n].v.y = ObjRes[own].oy - ObjRes[n].oy;
			Links[n].v.z = ObjRes[own].oz - ObjRes[n].oz;
			ObjRes[n].ox = 0;
		}
		if (CurEditor==2) {
			Links[n].v.x = ObjRes[own].ox - ObjRes[n].ox;
			Links[n].v.y = 0;
			Links[n].v.z = ObjRes[own].oz - ObjRes[n].oz;
			ObjRes[n].oy = 0;
		}
		Links[n].d = sqrt( float(
				Links[n].v.x*Links[n].v.x +
				Links[n].v.y*Links[n].v.y +
				Links[n].v.z*Links[n].v.z )) ;			
		Links[n].dd =  Links[n].d*Links[n].d;
		ScaleVector(Links[n].v, 1.f/Links[n].d);
	}
}

void CheckObjectchildren(int n, Vector3d X, Vector3d Y, Vector3d Z)
{
		//return;
	for (int ch=0;ch<Links[n].chCnt; ch++) {
		int child = Links[n].children[ch];
		Links[child].v = GetVectorInGlobal(X,Y,Z, Links[child].v);
		CheckObjectchildren(child, X, Y, Z);
	}
}

void CheckNodechildren2D(int exluden, int n, Vector3d newv)
{
	//return;
	float sina,cosa;
	
	if (Links[n].chCnt<=1) return;

	//2D
	Vector3d RotAx = {0,0,0};
	if (CurEditor==0) {RotAx.z = 1.f;}
	if (CurEditor==1) {RotAx.x = 1.f;}
	if (CurEditor==2) {RotAx.y = 1.f;}
/*	Vector3d vo_,vo;
	vo_.x = oObj[n2].ox - oObj[n1].ox;
	vo_.y = oObj[n2].oy - oObj[n1].oy;
	vo_.z = oObj[n2].oz - oObj[n1].oz;

	NormVector(vo_,1.f);
	Vector3d RotAx = {0,0,0};

	vo = GetVectorInGlobal(prevBasis[n2], vo_);
	if (fabs(v.x)<0.01 && fabs(v.y)<0.01 && fabs(v.z)<0.01) return;
*/

// 2D
	Vector3d X,Y,Z;
	SET_VECTOR(X, 1, 0, 0);
	SET_VECTOR(Y, 0, 1, 0);
	SET_VECTOR(Z, 0, 0, 1);

	Vector3d oldv,vv;
	
	oldv = Links[exluden].v;

	//if (fabs(newv.x)<0.01 && fabs(newv.y)<0.01 && fabs(newv.z)<0.01) {newv=oldv; return;}
	//NormVector(oldv, 1.f);
	//NormVector(newv, 1.f);

	MulVectorsScal(oldv, newv, cosa);
	
	if (cosa>1) cosa=1;
	if (cosa<-1) cosa=-1;
	
//	if (fabs(cosa)>0.99) return;			// do not calc new XYZ Angles
	float ang = acos(cosa);
	if (fabs(ang)<0.001) return;			// do not calc new XYZ Angles

	sina = sqrt(1 - cosa*cosa);

	MulVectorsVect(oldv, newv, vv);
	NormVector(vv, 1.f);

    if (CurEditor==0) if (vv.z<0.0) RotAx.z = -1.f;
    if (CurEditor==1) if (vv.x<0.0) RotAx.x = -1.f;
    if (CurEditor==2) if (vv.y<0.0) RotAx.y = -1.f; 
    vv = RotAx;

	//if (CurEditor==0) SET_VECTOR(vv, 0,0,-1);

	X = AxisRotateVector(vv, sina,cosa, X);
	Y = AxisRotateVector(vv, sina,cosa, Y);
	Z = AxisRotateVector(vv, sina,cosa, Z);

	for (int ch=0;ch<Links[n].chCnt; ch++) {
		int child = Links[n].children[ch];
		
		if (exluden==child) continue;
		
		DontCalcObj[child] = 1;
		//CheckObjectchildren(child,X,Y,Z);
		Links[child].v = GetVectorInGlobal(X,Y,Z, Links[child].v);
	}
}



void CheckNodechildren3D(int exluden, int n, Vector3d newv)
{
	//return;
	float sina,cosa;
	
	if (Links[n].chCnt<=1) return;
/*
	//2D
	Vector3d RotAx = {0,0,0};
// 2D
	Vector3d X,Y,Z;
	SET_VECTOR(X, 1, 0, 0);
	SET_VECTOR(Y, 0, 1, 0);
	SET_VECTOR(Z, 0, 0, 1);

	Vector3d oldv,vv;
	
	oldv = Links[exluden].v;

	MulVectorsScal(oldv, newv, cosa);
	
	if (cosa>1) cosa=1;
	if (cosa<-1) cosa=-1;
	
//	if (fabs(cosa)>0.99) return;			// do not calc new XYZ Angles
	float ang = acos(cosa);
	if (fabs(ang)<0.005) return;			// do not calc new XYZ Angles

	sina = sqrt(1 - cosa*cosa);

	MulVectorsVect(oldv, newv, vv);
	NormVector(vv, 1.f);
    vv = RotAx;

	//if (CurEditor==0) SET_VECTOR(vv, 0,0,-1);

	X = AxisRotateVector(vv, sina,cosa, X);
	Y = AxisRotateVector(vv, sina,cosa, Y);
	Z = AxisRotateVector(vv, sina,cosa, Z);
*/
	for (int ch=0;ch<Links[n].chCnt; ch++) {
		int child = Links[n].children[ch];
		
		if (exluden==child) continue;
		
		DontCalcObj[child] = 1;
		//CheckObjectchildren(child,X,Y,Z);
		//Links[child].v = GetVectorInGlobal(X,Y,Z, Links[child].v);
	}
}


void GetVectorN(Vector3d v, Vector3d &vec0)
{
 Vector3d mov = vec0;
 float s;
 NormVector(v,	  1.f);
 NormVector(vec0, 1.f);
 MulVectorsScal(v, vec0, s);
 float ang = acos(s);
 if (ang < 0.003) return;
 Vector3d vv;
 MulVectorsVect( v,vec0,  vv);    NormVector(vv, 1.f);
 MulVectorsVect( vv,v,    vec0);  NormVector(vec0, 1.f);
 
 MulVectorsScal(vec0, mov, s);
 ScaleVector(vec0, s);
 //MessageBeep(-1);
}

// n2 - owner, n1 - child

void RotateLinks2D(int n2,int n1, Vector3d oldv, Vector3d newv)
{
  Vector3d vv, RotAx = {0,0,0};
  if (CurEditor==0) {oldv.z = 0; newv.z = 0; RotAx.z = 1.f;}
  if (CurEditor==1) {oldv.x = 0; newv.x = 0; RotAx.x = 1.f;}
  if (CurEditor==2) {oldv.y = 0; newv.y = 0; RotAx.y = 1.f;}

  if (fabs(newv.x)<0.01 && fabs(newv.y)<0.01 && fabs(newv.z)<0.01) return;

  NormVector(oldv, 1.f);
  NormVector(newv, 1.f);

  float cosa, sina, ang;
  MulVectorsScal(oldv, newv, cosa);
  if (cosa>1) cosa=1;
  if (cosa<-1) cosa=-1;
  sina = sqrt(1-cosa*cosa);

  ang = acos(cosa);
  if (fabs(ang)<0.0005) return;			// do not calc new XYZ Angles
  MulVectorsVect(oldv, newv, vv);
  NormVector(vv, 1.f);

  if (CurEditor==0) if (vv.z<0.0) RotAx.z = -1.f;
  if (CurEditor==1) if (vv.x<0.0) RotAx.x = -1.f;
  if (CurEditor==2) if (vv.y<0.0) RotAx.y = -1.f;
  
  vv = RotAx;
  int n = n1;
  if (n==-1) return;
  Vector3d base, v;
  base.x = gObj[n1].ox;
  base.y = gObj[n1].oy;
  base.z = gObj[n1].oz;

  //return;
  for(;;) {
	v = Links[n].v;
    v.x += gObj[n].ox - base.x;
	v.y += gObj[n].oy - base.y;
	v.z += gObj[n].oz - base.z; 
	v = AxisRotateVector(vv, sina,cosa, v);
    v.x -= gObj[n].ox - base.x;
	v.y -= gObj[n].oy - base.y;
	v.z -= gObj[n].oz - base.z;
	Links[n].v = v;
	n = Links[n].owner;
	if (n==n2) return;
  }
}

// n2 - owner, n1 - child

void MoveLinks2DR(int n2, int n1, Vector3d movev)
{
  int o;
  if (fabs(movev.x)<0.0001 && fabs(movev.y)<0.0001 && fabs(movev.z)<0.0001) return;
  if (n1==StartIKJoint) return;
  if (n1==-1) return;	// no object
  if (locBase[n1]) return;		// is the base point
  if (n2==-1) return; 
  int n3 = Links[n2].owner;
  if (n3==-1)  return; 

  Moved[n2] = 1;
  int own_n1 = Links[n1].owner;
  //if (own_n1!=-1)
  //if (Links[own_n1].chCnt>1)
	//   { MoveLinks2DR(n3, n2, movev); return; } // if (n1 has brother)

  StartJoint = n2;
  Vector3d oldV, newV;
  Vector3d transf_v;					  // transfer vector
 
  Vector3d OldObjPos = gObj[n1].pos;
  Vector3d NewObjPos = AddVectors(gObj[n1].pos, movev);
			    oldV = SubVectors(gObj[n2].pos, OldObjPos);
			    newV = SubVectors(gObj[n2].pos, NewObjPos);

  if (CurEditor==0) {newV.z = 0; oldV.z = 0;}
  if (CurEditor==1) {newV.x = 0; oldV.x = 0;}
  if (CurEditor==2) {newV.y = 0; oldV.y = 0;}

  float OldVLen = VectAbs(oldV);	if (OldVLen==0.f) OldVLen = 0.00000000001;
  float NewVLen = VectAbs(newV);	if (NewVLen==0.f) NewVLen = 0.00000000001;

  ScaleVector(oldV, 1.f/OldVLen);
  ScaleVector(newV, 1.f/NewVLen);
  
  int  BasisChanged = CalcNewObjBasis2DR(n1,n2,n3, oldV, newV);

  if (!BasisChanged) { MoveLinks2DR(n3, n2, movev); return; }

  if (BasisChanged&2) {	// change only angular component for obj pos
	  gObj[n1].pos = SubVectors(gObj[n2].pos, ScaledVector(newV, OldVLen)); 
	  float traversed_path  = VectAbs(SubVectors(gObj[n1].pos,OldObjPos));
	  float total_path = VectAbs(movev);
	  float rest_path  = total_path - traversed_path;
	  if (rest_path<=0) return;
	  MoveLinks2DR(n3, n1, ScaledVector(movev, rest_path/total_path)); return; }

						// change both components for obj pos linear and angular
  gObj[n1].pos = SubVectors(gObj[n2].pos, ScaledVector(newV, NewVLen)); 
  transf_v = SubVectors(gObj[n1].pos, gObj[n2].pos);					// MOV: owner->n
  float transf_len = VectAbs(transf_v);
  ScaleVector(transf_v, (transf_len-OldVLen)/transf_len);

  MoveLinks2DR(n3, n2, transf_v);				// transfer inmfluence to next joint
}


// n2 - owner, n1 - child
// Calculates ONLY basises of Moved objects, and do not calculate Vectors and obj pos

void MoveLinks3DR(int n2, int n1, Vector3d movev)
{
  int o;
  if (fabs(movev.x)<0.01 && fabs(movev.y)<0.01 && fabs(movev.z)<0.001) return;

  if (n2==StartIKJoint) return;
  //if (n1==StartIKJoint) return;
  if (n1==-1) return;	// no object
  if (locBase[n1]) return;		// is the base point
  if (n2==-1) return; 
  int n3 = Links[n2].owner;
  if (n3==-1)  return;

  Moved[n2] = 1;
/*
  int own_n1 = Links[n1].owner;
  if (own_n1!=-1)
  if (Links[own_n1].chCnt>1)
	   { MoveLinks3DR(n3, n2, movev); return; } // if (n1 has brother)
*/
  StartJoint = n2;
  Vector3d oldV, newV;
  Vector3d transf_v;					  // transfer vector
 
  Vector3d OldObjPos = gObj[n1].pos;
  Vector3d NewObjPos = AddVectors(gObj[n1].pos, movev);
			    oldV = SubVectors(gObj[n2].pos, OldObjPos);
			    newV = SubVectors(gObj[n2].pos, NewObjPos);

  float OldVLen = VectAbs(oldV);	if (OldVLen==0.f) OldVLen = 0.00000000001;
  float NewVLen = VectAbs(newV);	if (NewVLen==0.f) NewVLen = 0.00000000001;

  ScaleVector(oldV, 1.f/OldVLen);
  ScaleVector(newV, 1.f/NewVLen);
  
  int  BasisChanged = CalcNewObjBasis3DR(n1,n2,n3, oldV, newV);

  if (!BasisChanged) { MoveLinks3DR(n3, n2, movev); return; }

  if (BasisChanged&2) {	// change only angular component for obj pos
	  gObj[n1].pos = SubVectors(gObj[n2].pos, ScaledVector(newV, OldVLen)); 
	  float traversed_path  = VectAbs(SubVectors(gObj[n1].pos,OldObjPos));
	  float total_path = VectAbs(movev);
	  float rest_path  = total_path - traversed_path;
	  if (rest_path<=0) return;
	  MoveLinks3DR(n3, n1, ScaledVector(movev, rest_path/total_path)); return; }

						// change both components for obj pos linear and angular
  gObj[n1].pos = SubVectors(gObj[n2].pos, ScaledVector(newV, NewVLen)); 
  transf_v = SubVectors(gObj[n1].pos, gObj[n2].pos);					// MOV: owner->n
  float transf_len = VectAbs(transf_v);
  ScaleVector(transf_v, (transf_len-OldVLen)/transf_len);

  MoveLinks3DR(n3, n2, transf_v);				// transfer inmfluence to next joint
}

void UpdateObjPos2D(int n, Vector3d pos)	// for updateAll use : UpdateObjPos(0, obj[0].pos)
{
	Vector3d pos1,v;

	 if (CurEditor==0) {
		ObjRes[n].ox = pos.x;
		ObjRes[n].oy = pos.y;
		ObjRes[n].oz = gObj[n].oz;
	 }
	 if (CurEditor==1) {
		ObjRes[n].oy = pos.y;
		ObjRes[n].oz = pos.z;
		ObjRes[n].ox = gObj[n].ox;
	 }
	 if (CurEditor==2) {
		ObjRes[n].ox = pos.x;
		ObjRes[n].oz = pos.z;
		ObjRes[n].oy = gObj[n].oy;
	 }

	for(int cnt=0;cnt<Links[n].chCnt;cnt++) {		// update each child
	 int child = Links[n].children[cnt];
	 v = Links[child].v;
	 //ScaleVector(v, -1.f);
	 ScaleVector(v, -Links[child].d);
	 pos1 = AddVectors(pos, v);
	 UpdateObjPos2D(child, pos1);
	}
}

/*
void CalcChildGlobalBasis(int n, TBasis Basis_[])
{
	int owner = gObj[n].owner;
	if (owner!=-1) {
	  Basis_[n].X = GetVectorInGlobal(Basis_[owner],  Basis_[n].X);
	  Basis_[n].Y = GetVectorInGlobal(Basis_[owner],  Basis_[n].Y);
	  Basis_[n].Z = GetVectorInGlobal(Basis_[owner],  Basis_[n].Z);
	}
	for (int ch=0;ch<Links[n].chCnt;ch++) 
	   CalcChildGlobalBasis(Links[n].children[ch], Basis_);
}

void RecalcGlobalBasisesFromAngles(TBasis Basis_[])		// Calculate basises from Angles - in global
{
	for (int o=0;o<OCount;o++) {
		CalcMatrix(Track[CurFrame][o].Xfi*pi/180.f,
				   Track[CurFrame][o].Yfi*pi/180.f,
				   Track[CurFrame][o].Zfi*pi/180.f);
		TransposeMatrix();
		SET_VECTOR(Basis_[o].X, M[0][0], M[0][1], M[0][2]);
	    SET_VECTOR(Basis_[o].Y, M[1][0], M[1][1], M[1][2]);
	    SET_VECTOR(Basis_[o].Z, M[2][0], M[2][1], M[2][2]);
	}
    CalcChildGlobalBasis(0, Basis_);
}
*/

void CalcChildGlobalBasis( int n, TBasis Basis_[])
{
	int owner = Links[n].owner;
	if (owner!=-1) {
	  Basis_[n].X = GetVectorInGlobal(Basis_[owner],  Basis_[n].X);
	  Basis_[n].Y = GetVectorInGlobal(Basis_[owner],  Basis_[n].Y);
	  Basis_[n].Z = GetVectorInGlobal(Basis_[owner],  Basis_[n].Z);
	}
	for (int ch=0;ch<Links[n].chCnt;ch++) 
	   CalcChildGlobalBasis( Links[n].children[ch], Basis_);
}

void RecalcGlobalBasisesFromAngles(TBasis *Basis_)// Calculate basises from Angles - in global
{
	for (int o=0;o<OCount;o++) {		//Track[CurFrame][o]
		CalcMatrix(Track[CurFrame][o].Xfi*pi/180.f,
				   Track[CurFrame][o].Yfi*pi/180.f,
				   Track[CurFrame][o].Zfi*pi/180.f);
		TransposeMatrix();
		SET_VECTOR(Basis_[o].X, M[0][0], M[0][1], M[0][2]);
	    SET_VECTOR(Basis_[o].Y, M[1][0], M[1][1], M[1][2]);
	    SET_VECTOR(Basis_[o].Z, M[2][0], M[2][1], M[2][2]);
	}
    CalcChildGlobalBasis(0, Basis_);
}


void RecalcLocalBasisesFromAngles(TBasis Basis_[])		// Calculate basises from Angles - in global
{
	for (int o=0;o<OCount;o++) {
		CalcMatrix(Track[CurFrame][o].Xfi*pi/180.f,
				   Track[CurFrame][o].Yfi*pi/180.f,
				   Track[CurFrame][o].Zfi*pi/180.f);
		TransposeMatrix();
		SET_VECTOR(Basis_[o].X, M[0][0], M[0][1], M[0][2]);
	    SET_VECTOR(Basis_[o].Y, M[1][0], M[1][1], M[1][2]);
	    SET_VECTOR(Basis_[o].Z, M[2][0], M[2][1], M[2][2]);
	}
}

void CheckAllRestrictions()
{
	for(int i=0;i<OCount;i++) {
		if (LInfo[i].restrict[0]) if(Track[CurFrame][i].Xfi > LInfo[i].maxFi[0]*180.0/pi) Track[CurFrame][i].Xfi = LInfo[i].maxFi[0]*180.0/pi;
		if (LInfo[i].restrict[1]) if(Track[CurFrame][i].Yfi > LInfo[i].maxFi[1]*180.0/pi) Track[CurFrame][i].Yfi = LInfo[i].maxFi[1]*180.0/pi;
		if (LInfo[i].restrict[2]) if(Track[CurFrame][i].Zfi > LInfo[i].maxFi[2]*180.0/pi) Track[CurFrame][i].Zfi = LInfo[i].maxFi[2]*180.0/pi;

		if (LInfo[i].restrict[0]) if(Track[CurFrame][i].Xfi < LInfo[i].minFi[0]*180.0/pi) Track[CurFrame][i].Xfi = LInfo[i].minFi[0]*180.0/pi;
		if (LInfo[i].restrict[1]) if(Track[CurFrame][i].Yfi < LInfo[i].minFi[1]*180.0/pi) Track[CurFrame][i].Yfi = LInfo[i].minFi[1]*180.0/pi;
		if (LInfo[i].restrict[2]) if(Track[CurFrame][i].Zfi < LInfo[i].minFi[2]*180.0/pi) Track[CurFrame][i].Zfi = LInfo[i].minFi[2]*180.0/pi;
	}
}

#define OPOS(obj) (*(Vector3d*)&(obj))

void MoveLinks2DTarget(int n, Vector3d Target)
{
	Vector3d F = OPOS(gObj[n].ox);
	Vector3d R = ScaledVector(Links[n].v, -Links[n].d);
	Vector3d torque;
	
	MulVectorsVect(R, F, torque);
}

void DragLinks2DRTarget(int n, Vector3d v)
{
  if (Links[n].owner==-1) return;

  ScaleVector(v, -1.0);

  StartJoint = -1;

  BuildLinks2D();

  MoveLinks2DTarget(n ,v);

  CheckAllRestrictions();

  //RecalcGlobalBasisesFromAngles(Basis);		// this function can be used only at start transforming

  //CopyMemory(prevBasis, Basis, OCount*sizeof(TBasis));
  
  //BasisesLoadIdentity(Basis);

  //ChangeObjectAngles(0,Basis[0],0);

}

bool IK_RushObjects()
{
	Vector3d edit_mask={1,1,1};
	switch(CurEditor) {
	  case 0: edit_mask.z = 0; break;
	  case 1: edit_mask.x = 0; break;
	  case 2: edit_mask.y = 0; break;
	}

  int changed=0;
  StartIKJoint = CurObject;
  if (OCount>1) {
		for(int o=OCount-1;o>0;o--) {
			if (flagFixPos[o]==0) continue;
			//for(int i=0; i<10; i++) {
			  Vector3d Movev = SubVectors(vecFixPos[o],gObj[o].pos);
			  Movev.x*=edit_mask.x;
			  Movev.y*=edit_mask.y;
			  Movev.z*=edit_mask.z;
			  if (fabs(Movev.x)<1.f && fabs(Movev.y)<1.f && fabs(Movev.z)<1.f) continue;
			  ScaleVector(Movev, -1.f);
			  DragLinks2DR(o, Movev);
	          RefreshFrameAni();
	          RefreshModel();
			  changed = 1;
			//}
		}
		//UpdateEditorWindow(-1);
	}
  StartIKJoint = 0;
  return changed;
}


void DragLinks2DR(int n, Vector3d v)
{
  ScaleVector(v, -1.0);
  if (n==0 || Links[n].owner==0) { Track[CurFrame][0].ddx+=v.x;  // base
								   Track[CurFrame][0].ddy+=v.y;
								   Track[CurFrame][0].ddz+=v.z; return;}

  StartJoint = -1;
  ZeroMemory(Moved, 4*OCount);

  StartIKJoint = 0;
  int Control = (GetKeyState(VK_CONTROL) & 0x8000) || CtrlMode;
  int shift_state = (GetAsyncKeyState(VK_SHIFT) & 0x8000) || ShiftMode;
  if (Control)
  if (Links[n].owner) 
	  StartIKJoint = Links[n].owner;

  RecalcGlobalBasisesFromAngles(Basis);		// this function can be used only at start transforming

  MoveLinks2DR(Links[n].owner, n, v);

  if (StartJoint==-1) return;
  int n2 = Links[StartJoint].owner;
  
  if (n2==-1) CalcObjectAnglesByBasis(StartJoint, IdentityBasis, shift_state);
		 else CalcObjectAnglesByBasis(StartJoint, Basis[n2], shift_state);

 // CalcObjectAnglesByBasis(0,IdentityBasis);
}

void SetChildBasis(int n, TObj *Obj, Vector3d oldpos)
{
  if (Links[n].chCnt!=1) return;
  int child = Links[n].children[0];
  Vector3d moved = SubVectors( Obj[child].pos, oldpos );
  Vector3d NewV  = AddVectors( Links[child].v, moved  );
  NormVector(NewV, 1.f);
  
}


void DragLinks3DR(int n, Vector3d v)
{
  ScaleVector(v, -1.0);

  if (n==0 || Links[n].owner==0) { Track[CurFrame][0].ddx+=v.x;
								   Track[CurFrame][0].ddy+=v.y;
								   Track[CurFrame][0].ddz+=v.z; return;}

  ZeroMemory(Moved, 4*OCount);

  DWORD t1=timeGetTime();

  Vector3d opos0 = gObj[n].pos;

  StartIKJoint = 0;

  int Control = (GetKeyState(VK_CONTROL) & 0x8000) || CtrlMode;
  int shift_state = (GetAsyncKeyState(VK_SHIFT) & 0x8000) || ShiftMode;
  
  if (Control)
  if (Links[n].owner && Links[Links[n].owner].owner) 
	  StartIKJoint = Links[Links[n].owner].owner;

  StartJoint = -1;
  //CopyMemory(ObjRes, gObj, sizeof(TObj)*OCount);	//r
  RecalcGlobalBasisesFromAngles(Basis);		// this function can be used only at start transforming

  MoveLinks3DR(Links[n].owner, n, v);
  SetChildBasis(n,gObj,opos0);

  //MoveBaseJoint(ObjRes, 0, v);	//r
  //UpdateLinks(Links, ObjRes);		//r
  
  if (StartJoint==-1) return;
  int n2 = Links[StartJoint].owner;
  if (n2==-1) CalcObjectAnglesByBasis(StartJoint, IdentityBasis, shift_state);
		 else CalcObjectAnglesByBasis(StartJoint, Basis[n2], shift_state);

  UpdateObjPosFromBasis(gObj, StartJoint);

 /*
 }
  DWORD t2=timeGetTime();
  float t=t2-t1;
  MessageBeep(-1);
*/
  //CalcObjectAnglesByBasis(0,IdentityBasis);
}



void RotateBasisLocal(int Naxis, float fi, Vector3d &X,Vector3d &Y,Vector3d &Z)
{
	Vector3d X0, Y0, Z0;
	SET_VECTOR(X0, 1,0,0);
	SET_VECTOR(Y0, 0,1,0);
	SET_VECTOR(Z0, 0,0,1);

	float sa = sin(fi);
	float ca = cos(fi);				

	if (Naxis==0) {
	 X0 = XAroundRotate(X0, sa, ca);
	 Y0 = XAroundRotate(Y0, sa, ca);
	 Z0 = XAroundRotate(Z0, sa, ca);
	}
	if (Naxis==1) {
	 X0 = YAroundRotate(X0, sa, ca);
	 Y0 = YAroundRotate(Y0, sa, ca);
	 Z0 = YAroundRotate(Z0, sa, ca);
	}
	if (Naxis==2) {
	 X0 = ZAroundRotate(X0, sa, ca);
	 Y0 = ZAroundRotate(Y0, sa, ca);
	 Z0 = ZAroundRotate(Z0, sa, ca);
	}

	X0 = GetVectorInGlobal(X,Y,Z, X0);
	Y0 = GetVectorInGlobal(X,Y,Z, Y0);
	Z0 = GetVectorInGlobal(X,Y,Z, Z0);

	X = X0;
	Y = Y0;
	Z = Z0;
}

void RotateBasisGlobal(float sa, float ca, Vector3d &oX,Vector3d &oY,Vector3d &oZ)
{
	Vector3d X,Y,Z;

	if (CurEditor==0) {
	   X = ZAroundRotate(oX, sa, ca);
	   Y = ZAroundRotate(oY, sa, ca);
	   Z = ZAroundRotate(oZ, sa, ca); }

	if (CurEditor==1) {
	   X = XAroundRotate(oX, sa, ca);
	   Y = XAroundRotate(oY, sa, ca);
	   Z = XAroundRotate(oZ, sa, ca); }

	if (CurEditor==2) {
	   X = YAroundRotate(oX, sa, ca);
	   Y = YAroundRotate(oY, sa, ca);
	   Z = YAroundRotate(oZ, sa, ca); }

	oX = X;
	oY = Y;
	oZ = Z;
}

void CalcBasisAnglesZXY(Vector3d X,Vector3d Y,Vector3d Z, float &Xfi, float &Yfi, float &Zfi, int alg);

 void CalcBasisAngles(Vector3d X,Vector3d Y,Vector3d Z, float &Xfi, float &Yfi, float &Zfi, int alg)
{
  if (1) { CalcBasisAnglesZXY(X,Y,Z, Xfi,Yfi,Zfi,alg); return; }	// ZXY

	Vector3d oX,oY,oZ;

	SET_VECTOR(oX, 1,0,0);
	SET_VECTOR(oY, 0,1,0);
	SET_VECTOR(oZ, 0,0,1);

	float s;
	Vector3d v;
	// Calc X angle
/*
	MulVectorsScal(oX, Z, s);
	if (fabs(s)>0.99) {  	// if we cannot calc Zproj -> move OY axis to Y
		MulVectorsScal(oY, Y, Xfi); Xfi = acos(Xfi);
	    if (fabs(Xfi)>0.001f) {
	  	  MulVectorsVect(oY, Y, v);
		  MulVectorsScal(v, oX, s);  if (s<0) Xfi *= -1;
		  RotateBasisLocal(0, Xfi, oX,oY,oZ);
		} else Xfi = 0;
	} else */ {

/*
	  Vector3d Zproj;
	  Zproj = GetVectorInLocal(oX,oY,oZ, Z);
	  Zproj.x = 0;
	  Zproj = GetVectorInGlobal(oX,oY,oZ, Zproj);
*/

	  Vector3d Zproj, N;		// project Zproj to oYoZ  (with Normal = oX)   YX ( Normal = oX; )
	  MulVectorsScal(oX, Z, s);
	  N = oX;
	  ScaleVector(N, -s);
	  Zproj = AddVectors(N, Z);
	  
	  NormVector(Zproj, 1.f);
	  MulVectorsScal(oZ, Zproj, Xfi);

       //superposition in surface oX_oZ_Z must be by short way

	   if (alg==0) if (Xfi<0) 	// only positive side for alg=0
			{ Xfi=-Xfi; ScaleVector(Zproj,-1.f); }  // (pi-Xfi), other side rot
      
	   if (alg==1) if (Xfi>0) 	// only negative side for alg=1
			{ Xfi=-Xfi; ScaleVector(Zproj,-1.f); }  // (pi-Xfi), other side rot

	  Xfi = acos(Xfi);
	  
	  //if (alg==1) Xfi=pi-Xfi;
	  //if (alg==2) Xfi=pi+Xfi;

	  if (fabs(Xfi)>0.001f) {
	  	 MulVectorsVect(oZ,Zproj, v);
		 MulVectorsScal(v, oX, s);  if (s<0) Xfi *= -1;
		 RotateBasisLocal(0, Xfi, oX,oY,oZ);
	  } else Xfi = 0;
	}  // Zproj->zero

	// Calc Y angle
	MulVectorsScal(oZ, Z, Yfi);	 Yfi = acos(Yfi);

	if (fabs(Yfi)>0.001f) {
		MulVectorsVect(oZ, Z, v);
		MulVectorsScal(v, oY, s);  if (s<0) Yfi *= -1;
		RotateBasisLocal(1, Yfi, oX,oY,oZ);
	} else Yfi = 0;

	// Calc Z angle
	MulVectorsScal(oX, X, Zfi);	 Zfi = acos(Zfi);

	if (fabs(Zfi)>0.001f) {
		MulVectorsVect(oX, X, v);
		MulVectorsScal(v, oZ, s);  if (s<0) Zfi *= -1;
		RotateBasisLocal(2, Zfi, oX,oY,oZ);
	} else Zfi = 0;
}

void CalcBasisAnglesZXY(Vector3d X,Vector3d Y,Vector3d Z, float &Xfi, float &Yfi, float &Zfi, int alg)
{

	Vector3d oX,oY,oZ;

	SET_VECTOR(oX, 1,0,0);
	SET_VECTOR(oY, 0,1,0);
	SET_VECTOR(oZ, 0,0,1);

	float s;
	Vector3d v;

	// Calc X angle
/*	MulVectorsScal(oZ, Y, s);
	if (fabs(s)>0.99) {  	// if we cannot calc Zproj -> move OY axis to Y
		MulVectorsScal(oX, X, Zfi); Zfi = acos(Zfi);
	    if (fabs(Zfi)>0.01f) {
	  	  MulVectorsVect(oX, X, v);
		  MulVectorsScal(v, oZ, s);  if (s<0) Zfi *= -1;
		  RotateBasisLocal(2, Zfi, oX,oY,oZ);
		} else Zfi = 0;

	} else*/ {
	  Vector3d Yproj, N;		// project Yproj to oXoY  (with Normal = oZ)
	  MulVectorsScal(oZ, Y, s);
	  N = oZ; 
	  ScaleVector(N, -s);
	  Yproj = AddVectors(N, Y);

	  NormVector(Yproj, 1.f);

	  MulVectorsScal(oY, Yproj, Zfi);

//	  if (alg==1) { Zfi=-Zfi; ScaleVector(Yproj,-1.f);}  // pi-alpha, other side rot  

	   if (alg==0) if (Zfi<0) 	// only positive side for alg=0
			{ Zfi=-Zfi; ScaleVector(Yproj,-1.f); }  // (pi-Zfi), other side rot
      
	   if (alg==1) if (Zfi>0) 	// only negative side for alg=1
			{ Zfi=-Zfi; ScaleVector(Yproj,-1.f); }  // (pi-Zfi), other side rot

	  Zfi = acos(Zfi);

	  if (fabs(Zfi)>0.001f) {
	  	 MulVectorsVect(ScaledVector(oY,100.f),ScaledVector(Yproj,100.f), v);
		 //MulVectorsVect(oY, Yproj, v);
		 MulVectorsScal(v, oZ, s);  if (s<0) Zfi *= -1;
		 RotateBasisLocal(2, Zfi, oX,oY,oZ);
	  } else Zfi = 0;
	}  // Zproj->zero

	// Calc Xfi
	MulVectorsScal(oY, Y, Xfi);	 Xfi = acos(Xfi);

	if (fabs(Xfi)>0.001f) {
		MulVectorsVect(ScaledVector(oY,100.f),ScaledVector(Y,100.f), v);
		//MulVectorsVect(oY, Y, v);
		MulVectorsScal(v, oX, s);  if (s<0) Xfi *= -1;
		RotateBasisLocal(0, Xfi, oX,oY,oZ);
	} else Xfi = 0;

	// Calc Yfi
	MulVectorsScal(oZ, Z, Yfi);	 Yfi = acos(Yfi);

	if (fabs(Yfi)>0.001f) {
		MulVectorsVect(ScaledVector(oZ,100.f),ScaledVector(Z,100.f), v);
		//MulVectorsVect(oZ, Z, v);
		MulVectorsScal(v, oY, s);  if (s<0) Yfi *= -1;
		RotateBasisLocal(1, Yfi, oX,oY,oZ);
	} else Yfi = 0;
}


int MAX_ALG=2;

int CalcBasisAngles_R2D(int n, Vector3d &X,Vector3d &Y,Vector3d &Z, float &newXfi, float &newYfi, float &newZfi)
{
 float Xfi[4], dX[4];
 float Yfi[4], dY[4];
 float Zfi[4], dZ[4];
 float absW[4];
 int i;

 //dX = SubAngles(-358*pi/180.f, 2*pi/180.f);
 float oXfi = Track[CurFrame][n].Xfi*pi/180.f;
 float oYfi = Track[CurFrame][n].Yfi*pi/180.f;
 float oZfi = Track[CurFrame][n].Zfi*pi/180.f;

 for (i=0; i<MAX_ALG; i++) {
  CalcBasisAngles(X,Y,Z, Xfi[i], Yfi[i], Zfi[i], i);

  dX[i] = SubAngles(Xfi[i], oXfi);
  dY[i] = SubAngles(Yfi[i], oYfi);
  dZ[i] = SubAngles(Zfi[i], oZfi);
  
  absW[i] = dX[i]*dX[i] + dY[i]*dY[i] + dZ[i]*dZ[i];
 }

 if (n==1) {
   Fi0[0] = oXfi;	// debug
   Fi0[1] = oYfi; 
   Fi0[2] = oZfi;
   Fi1[0] = Xfi[0];	// debug
   Fi1[1] = Yfi[0]; 
   Fi1[2] = Zfi[0];
   Fi2[0] = Xfi[1]; 
   Fi2[1] = Yfi[1]; 
   Fi2[2] = Zfi[1];
   dXfi[0] = dX[0];
 }

  int angn = 0;
  for (i=0;i<MAX_ALG;i++) if (absW[i]<absW[angn]) angn=i;
 
  if (n==1) algoritm = angn;

  newXfi = oXfi + dX[angn];
  newYfi = oYfi + dY[angn];
  newZfi = oZfi + dZ[angn];

  int restrict = RestrictAng(n, newXfi, newYfi, newZfi);
  /*ClipAnglePI(newXfi);
  ClipAnglePI(newYfi);
  ClipAnglePI(newZfi);*/

  if (!restrict) return 0;

  //if (n==1)
  //	MessageBeep(-1);

  float kX,kY,kZ;

  if (fabs(dX[angn])<0.000001) kX=1.f; else {kX = fabs(SubAngles(newXfi, oXfi) / dX[angn]);}
  if (fabs(dY[angn])<0.000001) kY=1.f; else {kY = fabs(SubAngles(newYfi, oYfi) / dY[angn]);}
  if (fabs(dZ[angn])<0.000001) kZ=1.f; else {kZ = fabs(SubAngles(newZfi, oZfi) / dZ[angn]);}

  float k;
  if (kX<=kY) k=kX;
  if (kZ<=k)  k=kZ;

  newXfi = oXfi + k*dX[angn];
  newYfi = oYfi + k*dY[angn];
  newZfi = oZfi + k*dZ[angn];

 return restrict;
}

int CalcBasisAngles_R3D(int n, Vector3d &X,Vector3d &Y,Vector3d &Z, float &newXfi, float &newYfi, float &newZfi, int m)
{
 float Xfi[6], dX[6];
 float Yfi[6], dY[6];
 float Zfi[6], dZ[6];
 float absW[6];
 int i;
 float oXfi = FrameAni[n].Xfi*pi/180.f;
 float oYfi = FrameAni[n].Yfi*pi/180.f;
 float oZfi = FrameAni[n].Zfi*pi/180.f;

 if (m==1) {
	 if (!NeedTrackCure[n]) {
		 newXfi = oXfi;
		 newYfi = oYfi;
		 newZfi = oZfi;
		 return 0;
	 }
	 int fr = CurFrame-1;
	 while (fr>0 && Track[fr][n].active==0) fr--;
	 if (fr<0) fr=0;
	 oXfi = Track[fr][n].Xfi*pi/180.f;
	 oYfi = Track[fr][n].Yfi*pi/180.f;
	 oZfi = Track[fr][n].Zfi*pi/180.f;
 }

 // here max_alg = 2 , but we need to consider more two variants 
 // for each algoritm: ( Xfi+pi,-(Xfi+pi) )

 for (i=0; i<2; i++) {
   CalcBasisAngles(X,Y,Z, Xfi[i], Yfi[i], Zfi[i], i);
 }

 for (i=0; i<2; i++) {
	   dX[i] = SubAngles(Xfi[i], oXfi);
	   dY[i] = SubAngles(Yfi[i], oYfi);
	   dZ[i] = SubAngles(Zfi[i], oZfi);

	   absW[i] = dX[i]*dX[i] + dY[i]*dY[i] + dZ[i]*dZ[i];
 }

/*
 if (n==20) {
   Fi0[0] = oXfi;	// debug
   Fi0[1] = oYfi; 
   Fi0[2] = oZfi;
   Fi1[0] = Xfi[0];	// debug
   Fi1[1] = Yfi[0]; 
   Fi1[2] = Zfi[0];
   Fi2[0] = Xfi[1]; 
   Fi2[1] = Yfi[1]; 
   Fi2[2] = Zfi[1];
   dXfi[0] = dX[0];
   a1= absW[0];
   a2= absW[1];
 }
*/

  int angn = 0;
  for (i=0;i<2;i++) if (absW[i]<=absW[angn]) angn=i;
 
  algoritm = angn;
/*
  ClipAnglePI(oXfi);
  ClipAnglePI(oYfi);
  ClipAnglePI(oZfi);
*/
  newXfi = oXfi + dX[angn];
  newYfi = oYfi + dY[angn];
  newZfi = oZfi + dZ[angn];

  int restrict = RestrictAng(n, newXfi, newYfi, newZfi);
/*
  ClipAnglePI(newXfi);
  ClipAnglePI(newYfi);
  ClipAnglePI(newZfi);
*/
  return restrict;
}

int RestrictAng( int n2, float &Xfi, float &Yfi, float &Zfi)
{
  int restricted = 0;
  if (LInfo[n2].restrict[0]) if (Xfi<LInfo[n2].minFi[0]) {Xfi = LInfo[n2].minFi[0]; restricted=1;}
  if (LInfo[n2].restrict[1]) if (Yfi<LInfo[n2].minFi[1]) {Yfi = LInfo[n2].minFi[1]; restricted=1;}
  if (LInfo[n2].restrict[2]) if (Zfi<LInfo[n2].minFi[2]) {Zfi = LInfo[n2].minFi[2]; restricted=1;}

  if (LInfo[n2].restrict[0]) if (Xfi>LInfo[n2].maxFi[0]) {Xfi = LInfo[n2].maxFi[0]; restricted=1;}
  if (LInfo[n2].restrict[1]) if (Yfi>LInfo[n2].maxFi[1]) {Yfi = LInfo[n2].maxFi[1]; restricted=1;}
  if (LInfo[n2].restrict[2]) if (Zfi>LInfo[n2].maxFi[2]) {Zfi = LInfo[n2].maxFi[2]; restricted=1;}

  return restricted; 
}
int CalcBasisAngles_R3D(int n, TBasis &b, float &newXfi, float &newYfi, float &newZfi, int m)
{
	return CalcBasisAngles_R3D(n, b.X,b.Y,b.Z, newXfi, newYfi, newZfi, m);
}
// start local vector, prev vector, current vector

void CalcObjectAngles2D(int n, TBasis PapaBasis, Vector3d vo3d, Vector3d v3d)
{
	float Xfi,Yfi,Zfi;
	int o;
	//if (n==17) 
	//	MessageBeep(-1);
	Vector3d RotAx = {0,0,0};
	if (CurEditor==0) {v3d.z = 0; vo3d.z = 0; RotAx.z = 1.f;}
	if (CurEditor==1) {v3d.x = 0; vo3d.x = 0; RotAx.x = 1.f;}
	if (CurEditor==2) {v3d.y = 0; vo3d.y = 0; RotAx.y = 1.f;}

	if (fabs(v3d.x)<0.01 && fabs(v3d.y)<0.01 && fabs(v3d.z)<0.01) {
     RotateObjectchildren(n, PapaBasis, Basis, Track[CurFrame][n].Xfi*pi/180.f,
											 Track[CurFrame][n].Yfi*pi/180.f,
											 Track[CurFrame][n].Zfi*pi/180.f);
	 return;
	}

	float cosa,sina;
	NormVector(vo3d, 1.f);
	NormVector(v3d, 1.f);
	MulVectorsScal(vo3d, v3d, cosa);
	if (cosa>1) cosa=1;
	if (cosa<-1) cosa=-1;
	sina = sqrt(1-cosa*cosa);

	//if (fabs(cosa)>0.9999) return;		// do not calc new XYZ Angles

	Vector3d vv;
	float ang = acos(cosa);
	float ang2 = ang*180/pi;

	if ((fabs(ang)<0.0005)) {
		RotateObjectchildren(n, PapaBasis, Basis, Track[CurFrame][n].Xfi*pi/180.f,
												Track[CurFrame][n].Yfi*pi/180.f,
												Track[CurFrame][n].Zfi*pi/180.f);
		return;			    // do not calc new XYZ Angles
	}
	 MulVectorsVect(vo3d, v3d, vv);
	 NormVector(vv, 1.f);

 	if (CurEditor==0) if (vv.z<0.0) RotAx.z = -1.f;
	if (CurEditor==1) if (vv.x<0.0) RotAx.x = -1.f;
	if (CurEditor==2) if (vv.y<0.0) RotAx.y = -1.f;

	vv = RotAx;

	Vector3d oX,oY,oZ;			

	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInLocal(Links[n].children[o], 
														 prevBasis[n], prevBasis);

	prevBasis[n].X = AxisRotateVector(vv, sina,cosa, prevBasis[n].X);
	prevBasis[n].Y = AxisRotateVector(vv, sina,cosa, prevBasis[n].Y);
	prevBasis[n].Z = AxisRotateVector(vv, sina,cosa, prevBasis[n].Z);

	oX = GetVectorInLocal(PapaBasis, prevBasis[n].X);
	oY = GetVectorInLocal(PapaBasis, prevBasis[n].Y);
	oZ = GetVectorInLocal(PapaBasis, prevBasis[n].Z);

	// RESTRICTION !!!

	int restricted = CalcBasisAngles_R2D(n,oX,oY,oZ, Xfi,Yfi,Zfi);
	if (restricted) {
		  CalcMatrix(Xfi,Yfi,Zfi);
		  TransposeMatrix();							     // get new restricted basis
		  SET_VECTOR(oX, M[0][0], M[0][1], M[0][2]);
		  SET_VECTOR(oY, M[1][0], M[1][1], M[1][2]);
		  SET_VECTOR(oZ, M[2][0], M[2][1], M[2][2]);
		  prevBasis[n].X = GetVectorInGlobal(PapaBasis, oX); // new Basis
		  prevBasis[n].Y = GetVectorInGlobal(PapaBasis, oY);
	 	  prevBasis[n].Z = GetVectorInGlobal(PapaBasis, oZ);
	}

	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInGlobal(Links[n].children[o], 
														  prevBasis[n], prevBasis);
    //axRotateObjectchildren(n, PapaBasis, Basis, vv, sina, cosa);
	RotateObjectchildren(n, PapaBasis, Basis, Xfi,Yfi,Zfi);

	Track[CurFrame][n].Xfi = (float)(Xfi*180.0/pi);
	Track[CurFrame][n].Yfi = (float)(Yfi*180.0/pi);
	Track[CurFrame][n].Zfi = (float)(Zfi*180.0/pi);
}

void CalcObjectAngles3D_(int n, TBasis PapaBasis, Vector3d vo3d, Vector3d v3d)
{
	float Xfi,Yfi,Zfi;
	int o;

	float cosa,sina;
	
	MulVectorsScal(vo3d, v3d, cosa);
	if (cosa>1) cosa=1;
	if (cosa<-1) cosa=-1;								  
	sina = sqrt(1-cosa*cosa);

	Vector3d  vv;
	float ang = acos(cosa);
	//float ang2 = ang*180/pi;

	if (fabs(ang)<0.005) {
		RotateObjectchildren(n, PapaBasis, Basis, Track[CurFrame][n].Xfi*pi/180.f,
												Track[CurFrame][n].Yfi*pi/180.f,
												Track[CurFrame][n].Zfi*pi/180.f);
		return;
	}										// do not calc new XYZ Angles

	MulVectorsVect(vo3d, v3d, vv);
	NormVector(vv, 1.f);

	Vector3d oX,oY,oZ;			

	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInLocal(Links[n].children[o], 
														 prevBasis[n], prevBasis);

	prevBasis[n].X = AxisRotateVector(vv, sina,cosa, prevBasis[n].X);
	prevBasis[n].Y = AxisRotateVector(vv, sina,cosa, prevBasis[n].Y);
	prevBasis[n].Z = AxisRotateVector(vv, sina,cosa, prevBasis[n].Z);

	oX = GetVectorInLocal(PapaBasis, prevBasis[n].X);
	oY = GetVectorInLocal(PapaBasis, prevBasis[n].Y);
	oZ = GetVectorInLocal(PapaBasis, prevBasis[n].Z);

	// RESTRICTION !!!

	int restricted = CalcBasisAngles_R3D(n,oX,oY,oZ, Xfi,Yfi,Zfi);
	if (RestrictAng(n,Xfi,Yfi,Zfi)) {
		 CalcMatrix(Xfi,Yfi,Zfi);
		 TransposeMatrix();		// get new restricted basis
		 SET_VECTOR(oX, M[0][0], M[0][1], M[0][2]);
		 SET_VECTOR(oY, M[1][0], M[1][1], M[1][2]);
		 SET_VECTOR(oZ, M[2][0], M[2][1], M[2][2]);

		 prevBasis[n].X = GetVectorInGlobal(PapaBasis, oX);		// new Basis
		 prevBasis[n].Y = GetVectorInGlobal(PapaBasis, oY);
	 	 prevBasis[n].Z = GetVectorInGlobal(PapaBasis, oZ);

		 SET_VECTOR(LInfo[n].W, 0,0,0);
		 SET_VECTOR(AppForce[n], 0,0,0);
		 AbsAppForce[n] = 0;
	}

	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInGlobal(Links[n].children[o], 
														  prevBasis[n], prevBasis);

    //axRotateObjectchildren(n, PapaBasis, Basis, vv, sina, cosa);
	RotateObjectchildren(n, PapaBasis, Basis, Xfi,Yfi,Zfi);

	Track[CurFrame][n].Xfi = (float)(Xfi*180.0/pi);
	Track[CurFrame][n].Yfi = (float)(Yfi*180.0/pi);
	Track[CurFrame][n].Zfi = (float)(Zfi*180.0/pi);
}

/*
void CalcObjectAngles3D_Phys(int n, TBasis PapaBasis)
{
	float Xfi,Yfi,Zfi;
	int o;

	Vector3d oX,oY,oZ;

	// RESTRICTION !!!
	CalcBasisAngles(oX,oY,oZ, Xfi,Yfi,Zfi);
	if (Restricted(n,Xfi,Yfi,Zfi)) {
		 CalcMatrix(Xfi,Yfi,Zfi);
		 TransposeMatrix();		// get new restricted basis
		 SET_VECTOR(Basis[n].X, M[0][0], M[0][1], M[0][2]);
		 SET_VECTOR(Basis[n].Y, M[1][0], M[1][1], M[1][2]);
		 SET_VECTOR(Basis[n].Z, M[2][0], M[2][1], M[2][2]);

		 SET_VECTOR(LInfo[n].W, 0,0,0);
		 SET_VECTOR(AppForce[n], 0,0,0);
		 AbsAppForce[n] = 0;
	}

	Track[CurFrame][n].Xfi = (float)(Xfi*180.0/pi);
	Track[CurFrame][n].Yfi = (float)(Yfi*180.0/pi);
	Track[CurFrame][n].Zfi = (float)(Zfi*180.0/pi);
}
*/

void CalcObjectAngles3D_Phys(int n, TBasis PapaBasis)
{
	float Xfi,Yfi,Zfi;
	int o;

	float cosa,sina;
/*	
	MulVectorsScal(vo3d, v3d, cosa);
	if (cosa>1) cosa=1;
	if (cosa<-1) cosa=-1;								  
	sina = sqrt(1-cosa*cosa);*/

	//sina = LInfo[n].SinWt;
	//cosa = LInfo[n].CosWt;

	//if (fabs(cosa)>0.9999) return;		// do not calc new XYZ Angles
	Vector3d  vv;
	float ang = acos(cosa);
	//float ang2 = ang*180/pi;

	if (fabs(ang)<0.003) {
		RotateObjectchildren(n, PapaBasis, Basis, Track[CurFrame][n].Xfi*pi/180.f,
												Track[CurFrame][n].Yfi*pi/180.f,
												Track[CurFrame][n].Zfi*pi/180.f);
		return;								
	}										// do not calc new XYZ Angles
	//MulVectorsVect(vo3d, v3d, vv);
	//NormVector(vv, 1.f);

	Vector3d oX,oY,oZ;			

	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInLocal(Links[n].children[o], 
														 prevBasis[n], prevBasis);

	prevBasis[n].X = AxisRotateVector(vv, sina,cosa, prevBasis[n].X);
	prevBasis[n].Y = AxisRotateVector(vv, sina,cosa, prevBasis[n].Y);
	prevBasis[n].Z = AxisRotateVector(vv, sina,cosa, prevBasis[n].Z);

	oX = GetVectorInLocal(PapaBasis, prevBasis[n].X);
	oY = GetVectorInLocal(PapaBasis, prevBasis[n].Y);
	oZ = GetVectorInLocal(PapaBasis, prevBasis[n].Z);

	// RESTRICTION !!!

	//CalcBasisAngles_(n, oX,oY,oZ, Xfi,Yfi,Zfi);
	if (RestrictAng(n,Xfi,Yfi,Zfi)) {
		 CalcMatrix(Xfi,Yfi,Zfi);
		 TransposeMatrix();		// get new restricted basis
		 SET_VECTOR(oX, M[0][0], M[0][1], M[0][2]);
		 SET_VECTOR(oY, M[1][0], M[1][1], M[1][2]);
		 SET_VECTOR(oZ, M[2][0], M[2][1], M[2][2]);

		 prevBasis[n].X = GetVectorInGlobal(PapaBasis, oX);		// new Basis
		 prevBasis[n].Y = GetVectorInGlobal(PapaBasis, oY);
	 	 prevBasis[n].Z = GetVectorInGlobal(PapaBasis, oZ);

		 SET_VECTOR(LInfo[n].W, 0,0,0);
		 SET_VECTOR(AppForce[n], 0,0,0);
		 AbsAppForce[n] = 0;
	}

	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInGlobal(Links[n].children[o], 
														  prevBasis[n], prevBasis);

    //axRotateObjectchildren(n, PapaBasis, Basis, vv, sina, cosa);
	RotateObjectchildren(n, PapaBasis, Basis, Xfi,Yfi,Zfi);

	Track[CurFrame][n].Xfi = (float)(Xfi*180.0/pi);
	Track[CurFrame][n].Yfi = (float)(Yfi*180.0/pi);
	Track[CurFrame][n].Zfi = (float)(Zfi*180.0/pi);
}



#include <stdio.h>

void ChangeObjectAngles_Phys(int n,TBasis PapaBasis)
{
  CalcObjectAngles3D_Phys(n, PapaBasis);
  for(int i=0;i<Links[n].chCnt;i++) 
		  ChangeObjectAngles_Phys(Links[n].children[i], Basis[n]);
}

void ChangeObjectAnglesE(int n, TBasis PapaBasis)		// GET ALL OBJECT ANGLE
{ 
      Vector3d v,vo, vs;		// current , old , start
	  float Xfi,Yfi,Zfi;

      if (Links[n].chCnt==0) return;

	  int child = Links[n].children[0];
/*
	  for(int ch=0;ch<Links[n].chCnt;ch++) { 
		  int Ochild = Links[n].children[ch];
		  if (Moved[Ochild]) child = Ochild;
	  }*/

	   v = Links[child].v;

	   vs.x = oObj[child].ox - oObj[n].ox;	// start vector
	   vs.y = oObj[child].oy - oObj[n].oy;
	   vs.z = oObj[child].oz - oObj[n].oz;
	   //vs = prevLinks[child].v;
	   //ScaleVector(v, 1.f/Links[child].d);
	   ScaleVector(v, -1.f);
	   NormVector(vs,  1.f);
	   //ScaleVector(vs, -1.f);
	   //ScaleVector(vs, 1.f/prevLinks[child].d);	// normalize vectors

	   vo = GetVectorInGlobal(prevBasis[n],  vs);

	   CalcObjectAngles3D_(n, PapaBasis, vo, v);	// 3d studio

	  for(int i=0;i<Links[n].chCnt;i++) 
		  ChangeObjectAnglesE(Links[n].children[i], Basis[n]);
}

void UpdateObjPosFromBasis(TObj *Obj, int n)
{
  int owner = Links[n].owner; 
  if (owner==-1) return;

  Vector3d oVec = SubVectors(OPOS(oObj[n].ox), OPOS(oObj[owner].ox));

  Vector3d V = GetVectorInGlobal(Basis[owner], oVec);
  OPOS(Obj[n].ox) = AddVectors(OPOS(Obj[owner].ox), V);

  for(int ch=0; ch<Links[n].chCnt; ch++)
	  UpdateObjPosFromBasis(Obj, Links[n].children[ch]);
}

void CalcObjectAnglesByBasis(int n, TBasis PapaBasis, int shift_state, int recalc_mind)
{
	float Xfi,Yfi,Zfi;
	Vector3d oX, oY, oZ;

	oX = GetVectorInLocal(PapaBasis, Basis[n].X);
	oY = GetVectorInLocal(PapaBasis, Basis[n].Y);
	oZ = GetVectorInLocal(PapaBasis, Basis[n].Z);

	if (Links[n].chCnt==0) return;	// do not calculate last object angels
	int r = CalcBasisAngles_R3D(n,oX,oY,oZ, Xfi,Yfi,Zfi, recalc_mind);

	if (r) {
		 CalcMatrix(Xfi,Yfi,Zfi);
		 TransposeMatrix();		// get new restricted basis
		 SET_VECTOR(oX, M[0][0], M[0][1], M[0][2]);
		 SET_VECTOR(oY, M[1][0], M[1][1], M[1][2]);
		 SET_VECTOR(oZ, M[2][0], M[2][1], M[2][2]);
		 Basis[n].X = GetVectorInGlobal(PapaBasis, oX);		// new Basis
		 Basis[n].Y = GetVectorInGlobal(PapaBasis, oY);
	 	 Basis[n].Z = GetVectorInGlobal(PapaBasis, oZ);
	}
	
	Track[CurFrame][n].Xfi = (float)(Xfi*180.f/pi);
	Track[CurFrame][n].Yfi = (float)(Yfi*180.f/pi);
	Track[CurFrame][n].Zfi = (float)(Zfi*180.f/pi);
	if (recalc_mind==0)	Track[CurFrame][n].active = 1;

	for(int ch=0;ch<Links[n].chCnt;ch++) {
		int child = Links[n].children[ch];
		if (shift_state) {
			if  (Moved[child]) // if shift -> use Moved flag
			CalcObjectAnglesByBasis(child, Basis[n], shift_state, recalc_mind);
		}
		else
			CalcObjectAnglesByBasis(child, Basis[n], shift_state, recalc_mind);
	}

}


void ChangeObjectAngles(int n, TBasis PapaBasis, int m)		// GET ALL OBJECT Angles
{
	if (m==0 || m==2) { // in 2D
	  if ((GetAsyncKeyState(VK_SHIFT) & 0x8000)&&ShiftMode == 0) if (n==CurObject) return;
	  if (DontCalcObj[n]) return;
	}
		  
      Vector3d v,vo, vs;		// current , old , start
	  float Xfi,Yfi,Zfi;

      if (Links[n].chCnt==0) return;

		int child = Links[n].children[0];

	    for(int ch=0;ch<Links[n].chCnt;ch++) { 
		   int Ochild = Links[n].children[ch];
		   if (Moved[Ochild]) child = Ochild;
		}

	   v = Links[child].v;

	   vs.x = oObj[child].ox - oObj[n].ox;
	   vs.y = oObj[child].oy - oObj[n].oy;
	   vs.z = oObj[child].oz - oObj[n].oz;

	   //ScaleVector(v, 1.f/Links[child].d);
	   ScaleVector(v, -1.f);
	   ScaleVector(vs, 1.f/Links[child].d);	// normalize vectors
/*
	   Xfi = Track[CurFrame][n].Xfi*pi/180.f;
	   Yfi = Track[CurFrame][n].Yfi*pi/180.f;
	   Zfi = Track[CurFrame][n].Zfi*pi/180.f;
*/
	   vo = GetVectorInGlobal(prevBasis[n],  vs);
	   if (m==0) CalcObjectAngles2D(n, PapaBasis, vo, v);
	   if (m==1) CalcObjectAngles3D_(n, PapaBasis, vo, v);
	   if (m==2) CalcObjectAngles3D_(n, PapaBasis, vo, v);	// 3d studio

	  for(int i=0;i<Links[n].chCnt;i++) 
		  ChangeObjectAngles(Links[n].children[i], Basis[n], m);
}

void InitIK()
{
	IKCtrlJoints = 1;

   Gravity = 30;
   TimeScale = 1;
   StartIKJoint = 0;
   ZeroMemory(locBase, OCount*4);
   ZeroMemory(flagFixPos, OCount*4);

   BasisesLoadIdentity(Basis);
   IdentityBasis = Basis[0];
   for(int o=0;o<OCount;o++) {
	   FillMemory(LInfo[o].restrict,3*4, 0);
	   LInfo[o].mass = 1;
	   af[o] = 1;
	   for(int a=0;a<3;a++) {
			LInfo[o].minFi[a] = 0.f*pi/180.f;	// -179.0
			LInfo[o].maxFi[a] = 0.f*pi/180.f;	// 179.0
	   }
   }

   FillMemory(LInfo[0].restrict,3*4, 0);
   if (OCount==0) return;
   CopyMemory(ObjRes, gObj, OCount*sizeof(TObj));
   BuildLinks3D(Links, ObjRes);	// build for sumdd;
   float sumdd = 0, sumM = 0;
   for(o=0;o<OCount;o++) {
		sumdd += Links[o].dd;
		sumM += LInfo[o].mass;
   }
   sumdd /= (float)OCount;
   sumM /= (float)OCount;

   LinksI = 128.0*128.0/(sumdd*sumM);

//   LInfo[0].minFi[2] = 0;
//   LInfo[0].maxFi[2] = pi/4.0;//2*pi/3.0;
}

void Debug()
{
	Vector3d vo,v;

	float Xfi,Yfi,Zfi;

	Xfi = 0*pi/180.f;
	Yfi = 120*pi/180.f;
	Zfi = 0*pi/180.f;

	CalcMatrix(Xfi, Yfi, Zfi);
	TransposeMatrix();

	Vector3d X,Y,Z;
	SET_VECTOR(X, M[0][0], M[0][1], M[0][2]);
	SET_VECTOR(Y, M[1][0], M[1][1], M[1][2]);
	SET_VECTOR(Z, M[2][0], M[2][1], M[2][2]);

	Xfi=0; Yfi=0; Zfi=0;
	CalcBasisAngles(X,Y,Z, Xfi, Yfi, Zfi, 0);

	Xfi = Xfi*180.f/pi;
	Yfi = Yfi*180.f/pi;
	Zfi = Zfi*180.f/pi;
	
	MessageBeep(-1);
}
  


