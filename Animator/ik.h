
#define SET_VECTOR(vec,vx,vy,vz) { vec.x=vx; vec.y=vy; vec.z=vz; }

typedef struct tagIKLink {
  Vector3d p;	 // this link calculated point
  float    d,dd; // next link info
  Vector3d v;
  int owner;
  int children[128];
  int chCnt;
} IKLink;
/*
typedef struct tagLinkResource {
	float minFi[3], maxFi[3];
	float mass;
	//int reserv[16];
} LinkResource;
*/

typedef struct tagLinkResource2 {
	int	  restrict[3];
	float minFi[3], maxFi[3];
	float mass;
	//int reserv[16];
} LinkResource2;


typedef struct tagLinkInfo {
	int	  restrict[3];
	float minFi[3], maxFi[3];
	float mass;
	Vector3d W;
} LinkInfo;

/*
typedef struct tagLinkInfo2 {
	float minFi[3], maxFi[3];
	float mass;
	Vector3d W,V, Wax;
	float Wt,SinWt,CosWt;
	float F[4];
} LinkInfo2;
*/
#define OrientationStruct(X,Y,Z,M,A)\
	union {\
		struct {\
		  Vector3d X,Y,Z;\
		};\
		float  M[3][3];\
		TBasis A;\
	};

typedef struct tagBasis {
	Vector3d X,Y,Z;
} TBasis;

_EXTORNOT float SphereR[256];

_EXTORNOT int StartIKJoint;			// Used For Ikmove base
_EXTORNOT int StartJoint;			// calculated joint for angels calculations
_EXTORNOT int      af[256];			// local base flag
_EXTORNOT int      locBase[256];	// local base flag

_EXTORNOT int      flagFixPos[256];		// fixate position
_EXTORNOT Vector3d vecFixPos[256];

_EXTORNOT IKLink   Links[256];		// generated links structures
_EXTORNOT LinkInfo LInfo[256];		// info about links
_EXTORNOT Vector3d AppForce[256];	// applyed force
_EXTORNOT float	   AbsAppForce[256];// abs applyed force
_EXTORNOT float	   absWa[256];
_EXTORNOT TObj     ObjRes[256];		// object as result calculatings
_EXTORNOT int      DontCalcObj[256];// it must be not processed (if trident children)
_EXTORNOT int	   Moved[256];		// 1 - if objecct was moved;
//_EXTORNOT TObj     startObj[256];

_EXTORNOT TBasis Basis[256];		// basis for adjustment start nodes to current
_EXTORNOT TBasis prevBasis[256];	// basis for adjustment prev nodes to current

_EXTORNOT TBasis IdentityBasis;

_EXTORNOT float Gravity;
_EXTORNOT int IKStarted;
_EXTORNOT int ReActivateIK;
_EXTORNOT Vector3d VCash;
_EXTORNOT float TimeScale;
_EXTORNOT float LinksI;

_EXTORNOT float DeltaT0;

_EXTORNOT int mode3d;
_EXTORNOT float DefAngleX, DefAngleY, DefAngleZ;

// debug
_EXTORNOT float Fi0[3],Fi1[3],Fi2[3], dXfi[2], a1, a2;
_EXTORNOT int algoritm;
// debug

void GetchildrenFor(IKLink *Links, TObj *ObjRes, int father);
void BuildLinks3D(IKLink *Links,TObj *ObjRes);

void UpdateObjPos(TObj ObjRes[], int n, Vector3d pos);
void UpdateObjPos(TObj *gObj, IKLink *Links, int n, Vector3d pos);

bool IK_RushObjects();

void DragLinks3DR(int n, Vector3d v);
void DragLinks2DR(int n, Vector3d v);
void CheckNodechildren3D(int exluden, int n, Vector3d newv);
void InitIK();
void CalcLinkedBodySpheresR();

void BuildLinks2D();
void CheckNodechildren2D(int n, int exluden, Vector3d newv);

float CheckAnglesRestriction(int n1, int n2, int n3, Vector3d &v);
void RotateObject(int no);
void CalcObjectAnglesByBasis(int n, TBasis PapaBasis, int check_state=0, int recalc_mind=0);
void UpdateObjPosFromBasis(TObj *Obj, int n);
void ChangeObjectAngles(int n,TBasis PapaBasis,int m);
void ChangeObjectAnglesE(int n, TBasis PapaBasis);		//ENGINE
void ChangeObjectAngles_Phys(int n,TBasis PapaBasis);

int RestrictAng(int n2, float &Xfi, float &Yfi, float &Zfi);
//void CalcBasisAngles_(int n, Vector3d X,Vector3d Y,Vector3d Z, float &newXfi, float &newYfi, float &newZfi);
int CalcBasisAngles_R2D(int n, Vector3d &X,Vector3d &Y,Vector3d &Z, float &newXfi, float &newYfi, float &newZfi);
int CalcBasisAngles_R3D(int n, Vector3d &X,Vector3d &Y,Vector3d &Z, float &newXfi, float &newYfi, float &newZfi, int m=0);
int CalcBasisAngles_R3D(int n, TBasis &b, float &newXfi, float &newYfi, float &newZfi, int m=0);

int CalcNewObjBasis3DR(int n1,int n2,int n3,Vector3d &oldV,Vector3d &newV);

void BasisesLoadIdentity(TBasis Basis[]);

void Debug();

void ProcessRealPhysic();
void RecalcGlobalBasisesFromAngles(TBasis Basis_[]);
void RecalcLocalBasisesFromAngles(TBasis Basis_[]);
void CheckAllRestrictions();
Vector3d AxisRotateVector(Vector3d Axis, float sinAng, float cosAng, Vector3d V);
TBasis AxisRotateBasis(Vector3d vv, float sina,float cosa, TBasis &Basis);

void ProcessSyncro();
void StartIKProcess();
void EndIKProcess();

/////////////////////////////IK3//////////////////////////

void UpdateLinks(IKLink *Links, TObj *Obj);
void MoveBaseJoint(TObj *Obj, int n, Vector3d &v);
void MoveJoint(int n, Vector3d &v);
void DragBase(Vector3d &v,int branch=-1);

void CalcChildrenBasisInLocal(int n, TBasis PapaBasis, TBasis Basis_[]);
void CalcChildrenBasisInGlobal(int n, TBasis PapaBasis, TBasis Basis_[]);

void CalcIKBasisesBy2Links(IKLink *Links, IKLink *Links2);