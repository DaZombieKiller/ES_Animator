#include "Animator.h"
#include "math.h"


Vector3d ZAroundRotate(Vector3d V,float sinA, float cosA)
{
  Vector3d Vres;
  Vres.x = V.x*cosA - V.y*sinA;
  Vres.y = V.x*sinA + V.y*cosA;
  Vres.z = V.z;
  return Vres;
}


Vector3d YAroundRotate(Vector3d V,float sinA, float cosA)
{
  Vector3d Vres;
  Vres.x = V.z*sinA + V.x*cosA;
  Vres.z = V.z*cosA - V.x*sinA;
  Vres.y = V.y;
  return Vres;
}

Vector3d XAroundRotate(Vector3d V,float sinA, float cosA)
{
  Vector3d Vres;
  Vres.x = V.x;
  Vres.y = V.y*cosA - V.z*sinA;
  Vres.z = V.y*sinA + V.z*cosA;
  return Vres;
}

/*
Vector3d YAroundRotate(Vector3d V,float sinA, float cosA)
{
  Vector3d Vres;
  Vres.x = V.x*cosA - V.z*sinA;
  Vres.z = V.x*sinA + V.z*cosA;
  Vres.y = V.y;
  return Vres;
}
*/


void ScaleVector(Vector3d& v, float Scale)
{  
  v.x*=Scale; 
  v.y*=Scale; 
  v.z*=Scale;   
}

Vector3d ScaledVector(Vector3d v, float Scale)
{    
  v.x*=Scale; 
  v.y*=Scale; 
  v.z*=Scale;   
  return v;
}


void MulVectorsScal(Vector3d& v1,Vector3d& v2, float& r)
{
  r = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

void MulVectorsVect(Vector3d& v1, Vector3d& v2, Vector3d& r )
{
  r.x= v1.y*v2.z - v2.y*v1.z;
  r.y=-v1.x*v2.z + v2.x*v1.z;
  r.z= v1.x*v2.y - v2.x*v1.y;
}

void RotatePoint(Vector3d &p)
{
  float x = p.x-acx; 
  float y = p.y-acy; 
  float z = p.z-acz;

  p.x = x*M[0][0] + y*M[0][1] + z*M[0][2] + acx;
  p.y = x*M[1][0] + y*M[1][1] + z*M[1][2] + acy;
  p.z = x*M[2][0] + y*M[2][1] + z*M[2][2] + acz;
}

Vector3d RotateVector(Vector3d v)
{
  Vector3d vres;
  vres.x = v.x*M[0][0] + v.y*M[0][1] + v.z*M[0][2];
  vres.y = v.x*M[1][0] + v.y*M[1][1] + v.z*M[1][2];
  vres.z = v.x*M[2][0] + v.y*M[2][1] + v.z*M[2][2];
  return vres;
}

Vector3d RotateVectorQuat(Vector3d v, float q[4])
{
    float x = q[0] * 2.f;
    float y = q[1] * 2.f;
    float z = q[2] * 2.f;
    float xx = q[0] * x;
    float yy = q[1] * y;
    float zz = q[2] * z;
    float xy = q[0] * y;
    float xz = q[0] * z;
    float yz = q[1] * z;
    float wx = q[3] * x;
    float wy = q[3] * y;
    float wz = q[3] * z;

    Vector3d vres;
    vres.x = (1.f - (yy + zz)) * v.x + (xy - wz) * v.y + (xz + wy) * v.z;
    vres.y = (xy + wz) * v.x + (1.f - (xx + zz)) * v.y + (yz - wx) * v.z;
    vres.z = (xz - wy) * v.x + (yz + wx) * v.y + (1.f - (xx + yy)) * v.z;
    return vres;
}

void TransposeMatrix()
{
	TMatrix tempM;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			tempM[j][i] = M[i][j];
 CopyMemory(M, tempM, sizeof(TMatrix));
}


Vector3d GetVectorInLocal(Vector3d Xax,Vector3d Yax,Vector3d Zax, Vector3d v)
{
 Vector3d vres;
 vres.x = v.x*Xax.x + v.y*Xax.y + v.z*Xax.z;	// to rotor_sys
 vres.y = v.x*Yax.x + v.y*Yax.y + v.z*Yax.z;	// PointCoord * Cos
 vres.z = v.x*Zax.x + v.y*Zax.y + v.z*Zax.z;
 return vres;
}

Vector3d GetVectorInGlobal(Vector3d Xax,Vector3d Yax,Vector3d Zax, Vector3d v)
{
 Vector3d vres;
 vres.x = v.x*Xax.x + v.y*Yax.x + v.z*Zax.x;
 vres.y = v.x*Xax.y + v.y*Yax.y + v.z*Zax.y;
 vres.z = v.x*Xax.z + v.y*Yax.z + v.z*Zax.z;
 return vres;
}
Vector3d GetVectorInLocal(TBasis &A, Vector3d v)					// by using matrix
{
 Vector3d vres;
 vres.x = v.x*A.X.x + v.y*A.X.y + v.z*A.X.z;	// to rotor_sys
 vres.y = v.x*A.Y.x + v.y*A.Y.y + v.z*A.Y.z;	// PointCoord * Cos
 vres.z = v.x*A.Z.x + v.y*A.Z.y + v.z*A.Z.z;
 //v = vres;
 return vres;
}

Vector3d GetVectorInGlobal(TBasis &A, Vector3d v)
{
 Vector3d vres;
/*vres.x = v.x*M[0][0] + v.y*M[1][0] + v.z*M[2][0];
 vres.y = v.x*M[0][1] + v.y*M[1][1] + v.z*M[2][1];
 vres.z = v.x*M[0][2] + v.y*M[1][2] + v.z*M[2][2];*/
 vres.x = v.x*A.X.x + v.y*A.Y.x + v.z*A.Z.x;
 vres.y = v.x*A.X.y + v.y*A.Y.y + v.z*A.Z.y;
 vres.z = v.x*A.X.z + v.y*A.Y.z + v.z*A.Z.z;
 //v = vres;
 return vres;
}

TBasis GetBasisInLocal(TBasis &PapaBasis, TBasis &Basis)
{
 TBasis B;
 B.X = GetVectorInLocal(PapaBasis, Basis.X);
 B.Y = GetVectorInLocal(PapaBasis, Basis.Y);
 B.Z = GetVectorInLocal(PapaBasis, Basis.Z);
 return B;
}

TBasis GetBasisInGlobal(TBasis &PapaBasis, TBasis &Basis)
{
 TBasis B;
 B.X = GetVectorInGlobal(PapaBasis, Basis.X);
 B.Y = GetVectorInGlobal(PapaBasis, Basis.Y);
 B.Z = GetVectorInGlobal(PapaBasis, Basis.Z);
 return B;
}

float AngleDifferenceS(float a, float b)
{
 a-=b; 
 if (a > pi) a = -(2*pi - a);
 if (a <-pi) a =-(-2*pi - a);
 return a;
}


TBasis AxisRotateBasis(Vector3d vv, float sina,float cosa, TBasis &Basis)
{
 TBasis B;
 B.X = AxisRotateVector(vv, sina,cosa, Basis.X);
 B.Y = AxisRotateVector(vv, sina,cosa, Basis.Y);
 B.Z = AxisRotateVector(vv, sina,cosa, Basis.Z);
 return B;
}


Vector3d AddVectors(Vector3d v1, Vector3d v2)
{
 Vector3d vres;
 vres.x = v1.x + v2.x;
 vres.y = v1.y + v2.y;
 vres.z = v1.z + v2.z;
 return vres;
}

Vector3d SubVectors(Vector3d v1, Vector3d v2)
{
 Vector3d vres;
 vres.x = v1.x - v2.x;
 vres.y = v1.y - v2.y;
 vres.z = v1.z - v2.z;
 return vres;
}

void NormVector(Vector3d& v, float Scale)
{
  float n;
  n=v.x*v.x + v.y*v.y + v.z*v.z;
  if (n<0.0000000000000000000001f) n=0.0000000000000000000001f;
  //
  n=Scale / (float)sqrt(n);
  v.x=v.x*n; 
  v.y=v.y*n;  
  v.z=v.z*n;
}

float VectAbs(Vector3d v)
{
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

float SubAngles(float Ang1, float Ang2)
{
   while (Ang1>2*pi) Ang1-=2*pi;
   while (Ang1<0) Ang1+=2*pi;

   while (Ang2>2*pi) Ang2-=2*pi;
   while (Ang2<0) Ang2+=2*pi;

   float dAlpha = Ang1 - Ang2;			// Ang1 <- Ang2 Angle
   while (dAlpha < -pi) dAlpha+=2*pi;	// clip dAlpha to {-pi..pi}
   while (dAlpha > pi)  dAlpha-=2*pi;

   return dAlpha;
}

void ClipAnglePI(float &Ang)
{
 while (Ang < -pi) Ang+=2*pi;
 while (Ang > pi)  Ang-=2*pi;
}

void ClipAngle2PI(float &Ang)
{
   while (Ang>2*pi) Ang-=2*pi;
   while (Ang<0) Ang+=2*pi;
}
