
Vector3d ZAroundRotate(Vector3d V,float sinA, float cosA);
Vector3d YAroundRotate(Vector3d V,float sinA, float cosA);
Vector3d XAroundRotate(Vector3d V,float sinA, float cosA);
void ScaleVector(Vector3d& v, float Scale);
Vector3d ScaledVector(Vector3d v, float Scale);
void MulVectorsScal(Vector3d& v1,Vector3d& v2, float& r);
void MulVectorsVect(Vector3d& v1, Vector3d& v2, Vector3d& r );
void RotatePoint(Vector3d &p);
Vector3d RotateVector(Vector3d v);
void TransposeMatrix();
Vector3d GetVectorInLocal(Vector3d Xax,Vector3d Yax,Vector3d Zax, Vector3d v);
Vector3d GetVectorInGlobal(Vector3d Xax,Vector3d Yax,Vector3d Zax, Vector3d v);
TBasis GetBasisInGlobal(TBasis &PapaBasis, TBasis &Basis);
TBasis GetBasisInLocal(TBasis &PapaBasis, TBasis &Basis);
Vector3d GetVectorInLocal(TBasis &A, Vector3d v);					// by using matrix
Vector3d GetVectorInGlobal(TBasis &A, Vector3d v);
Vector3d AddVectors(Vector3d v1, Vector3d v2);
Vector3d SubVectors(Vector3d v1, Vector3d v2);
void NormVector(Vector3d& v, float Scale);
float VectAbs(Vector3d v);

float AngleDifferenceS(float a, float b);
Vector3d AxisRotateVector(Vector3d Axis, float sinAng, float cosAng, Vector3d V);
float    SubAngles(float Ang1, float Ang2);
void	 ClipAnglePI(float &Ang);
void ClipAngle2PI(float &Ang);