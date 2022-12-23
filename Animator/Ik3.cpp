//from ObjRes
#include "animator.h"
#include "math.h"

void UpdateLinks(IKLink *Links, TObj *Obj)
{
	for(int n=0;n<OCount;n++) 
    if (Moved[n]) 
	{
		int own = Links[n].owner;
		if (own==-1) continue;
		Links[n].v = SubVectors(Obj[own].pos, Obj[n].pos);
		//ScaleVector(Links[n].v, 1.f/Links[n].d);
		NormVector(Links[n].v, 1.f);
	}

}

void CalcChildrenBasisInLocal(int n, TBasis PapaBasis, TBasis Basis_[])
{
    Basis_[n].X = GetVectorInLocal(PapaBasis,  Basis_[n].X);
	Basis_[n].Y = GetVectorInLocal(PapaBasis,  Basis_[n].Y);
	Basis_[n].Z = GetVectorInLocal(PapaBasis,  Basis_[n].Z);
	for (int ch=0;ch<Links[n].chCnt;ch++) 
		CalcChildrenBasisInLocal(Links[n].children[ch], PapaBasis, Basis_);
}

void CalcChildrenBasisInGlobal(int n, TBasis PapaBasis, TBasis Basis_[])
{
    Basis_[n].X = GetVectorInGlobal(PapaBasis,  Basis_[n].X);
	Basis_[n].Y = GetVectorInGlobal(PapaBasis,  Basis_[n].Y);
	Basis_[n].Z = GetVectorInGlobal(PapaBasis,  Basis_[n].Z);
	for (int ch=0;ch<Links[n].chCnt;ch++) 
		CalcChildrenBasisInGlobal(Links[n].children[ch], PapaBasis, Basis_);
}

void CalcObjectAngles3D_3(int n, TBasis PapaBasis, Vector3d vo3d, Vector3d v3d)
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

	if (fabs(ang)<0.005) return;		// do not calc new XYZ Angles

	ScaleVector(vo3d, 1000.f);
	ScaleVector(v3d,  1000.f);
	MulVectorsVect(vo3d, v3d, vv);
	NormVector(vv, 1.f);

	TBasis B;			

	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInLocal(Links[n].children[o], 
														   Basis[n], Basis);

	Basis[n] = AxisRotateBasis(vv, sina,cosa, Basis[n]);
	B = GetBasisInLocal(PapaBasis, Basis[n]);

	// RESTRICTION !!!
	int r = CalcBasisAngles_R3D(n,B, Xfi,Yfi,Zfi);
	if (r) {
		 CalcMatrix(Xfi,Yfi,Zfi);
		 TransposeMatrix();		// get new restricted basis 
		 B =  *(TBasis*)&M;
		 Basis[n] = GetBasisInGlobal(PapaBasis, B);		// new Basis
		 //SET_VECTOR(LInfo[n].W, 0,0,0);
		 //SET_VECTOR(AppForce[n], 0,0,0);
		 //AbsAppForce[n] = 0;
	}
	for(o=0;o<Links[n].chCnt;o++) CalcChildrenBasisInGlobal(Links[n].children[o], 
														    Basis[n], Basis);
	Track[CurFrame][n].Xfi = (float)(Xfi*180.0/pi);
	Track[CurFrame][n].Yfi = (float)(Yfi*180.0/pi);
	Track[CurFrame][n].Zfi = (float)(Zfi*180.0/pi);
}


void CalcObjectAngelsByLinks(int n, TBasis &PapaBasis)		// GET ALL OBJECT AngleS
{ 
       Vector3d v,vo, vs;		// current , old , start
	   float Xfi,Yfi,Zfi;

       if (Links[n].chCnt==0) return;

	   int i, child = Links[n].children[0];
	   for(i=0;i<Links[n].chCnt;i++) {			// get moved bone
			int ch = Links[n].children[i];
			if (Moved[ch]) child = ch;
	   }
	   //if (!Moved[child]) return;

	   v = Links[child].v;

	   vs = SubVectors(oObj[child].pos, oObj[n].pos);	// start vector

	   ScaleVector(v, -1.f);

	   vo = GetVectorInGlobal(Basis[n],  vs);

	   NormVector(vo,  1.f);

	   CalcObjectAngles3D_3(n, PapaBasis, vo, v);	// 3d studio

	   for(i=0;i<Links[n].chCnt;i++)
		  CalcObjectAngelsByLinks(Links[n].children[i], Basis[n]);
}

// Branch:
// 0 - left
// 1 - right

void DragBase(Vector3d &v,int branch)
{
  ScaleVector(v, -1.0);
  ZeroMemory(Moved, OCount*4);

  BuildLinks3D(Links, gObj);

  StartJoint = 0;
  int n = 0;
  if (branch!=-1) n  = Links[0].children[branch];

  MoveBaseJoint(gObj, n, v);

  Moved[n] = 0;	// do not update link of main joint

  UpdateLinks(Links, gObj);

  //UpdateObjPos(gObj, Links, 0, gObj[0].pos);
  //if (StartJoint==-1) return;
  RecalcGlobalBasisesFromAngles(Basis);
  CalcObjectAngelsByLinks(0,IdentityBasis);
}

/*
void MoveJoint(int n, Vector3d &v)
{
  Moved[n] = 1;
  int own1,own2=-1;
  own1 = Links[n].owner;
  if (own1!=-1) own2 = Links[own1].owner;
  ObjRes[n].pos = AddVectors(mObj[n].pos, v);
  // if (own2==-1) { Track[CurFrame][own1].ddv = AddVectors(Track[CurFrame][own1].ddv, v); return; }
  // if (own1==-1) { Track[CurFrame][n].ddv    = AddVectors(Track[CurFrame][n].ddv, v); return; }
  if (own1==-1) return;
  if (own2==-1) return;

  // correction
  Vector3d cor_v = SubVectors(ObjRes[n].pos, mObj[own1].pos); // MOV: owner->n
  float cor_len = VectAbs(cor_v);
  ScaleVector(cor_v, (cor_len-Links[n].d)/cor_len);
  MoveJoint(own1, cor_v);
}
*/
#define Ga (9.8*128.0)

void MoveBaseJoint(TObj *Obj, int n, Vector3d &v)
{
  Moved[n] = 1;
  Obj[n].pos = AddVectors(Obj[n].pos, v);
  if ( Links[n].chCnt == 0 ) return;
  int own1 = Links[n].owner;
  if (n==0) {
	for(int ch=0;ch<Links[n].chCnt;ch++)
			   MoveBaseJoint(Obj, Links[n].children[ch], v); 
	return; }

  int child = Links[n].children[0];
  for(int ch=0;ch<Links[n].chCnt;ch++) {
  child = Links[n].children[ch];
  // correction
  Vector3d cor_v = SubVectors(Obj[n].pos, Obj[child].pos); // MOV: child->n
  float cor_len = VectAbs(cor_v);
  ScaleVector(cor_v, (cor_len - Links[child].d)/cor_len );
  MoveBaseJoint(Obj, child, cor_v);
  }
}