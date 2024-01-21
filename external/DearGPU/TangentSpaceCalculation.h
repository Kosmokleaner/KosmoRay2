//////////////////////////////////////////////////////////////////////
//
//	tangent space calculation for arbitrary triangles meshes
//
// (c) by Crytek GmbH 2003-2008. All rights reserved. 
// Permission to distribute for commercial and non-commercial purposes.
// This source code is provided "as it" without support and any warranty.
//
//	
//	File:TangentSpaceCalculation.h
//  Description: calculated the tangent space base vector for a given mesh
//  Dependencies: none
//  Documentation: ShaderX4 "triangle mesh tangent space calculation"
//
//  Usage:
//		implement the proxy class: CTriangleInputProxy
//		instance the proxy class: CTriangleInputProxy MyProxy(MyData);
//    instance the calculation helper: CTangentSpaceCalculation<MyProxy> MyTangent;
//		do the calculation: MyTangent.CalculateTangentSpace(MyProxy);
//		get the data back: MyTangent.GetTriangleIndices(), MyTangent.GetBase()
//
//	History:
//	- 12/07/2002: Created by Martin Mittring as part of CryEngine
//  - 08/18/2003: MM improved stability (no illegal floats) with bad input data
//  - 08/19/2003: MM added check for input data problems (DebugMesh() is deactivated by default)
//  - 10/02/2003: MM removed rundundant code
//  - 10/01/2004: MM added errorcodes (NAN texture coordinates)
//  - 05/21/2005: MM made proxy interface typesafe
//  - 04/24/2008: MM updated copyright - this file should be usable for everyone
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>									// STL vector<> 
#include <map>										// STL map<,,> multimap<>

// derive your proxy from this class
// not virtual to save the call overhead
struct ITriangleInputProxy
{
/*
	//! not virtual to save the call overhead
	//! /return 0..
	unsigned int GetTriangleCount() const;

	//! not virtual to save the call overhead
	//! /param indwTriNo 0..
	//! /param outdwPos
	//! /param outdwNorm
	//! /param outdwUV
	void GetTriangleIndices( const unsigned int indwTriNo, unsigned int outdwPos[3], unsigned int outdwNorm[3], unsigned int outdwUV[3] ) const;

	//! not virtual to save the call overhead
	//! /param indwPos 0..
	//! /param outfPos
	void GetPos( const unsigned int indwPos, float outfPos[3] ) const;

	//! not virtual to save the call overhead
	//! /param indwPos 0..
	//! /param outfUV 
	void GetUV( const unsigned int indwPos, float outfUV[2] ) const;
*/
};
// ---------------------------------------------------------------------------------------------------------------


// InputProxy - derive from ITriangleInputProxy
template <class InputProxy>
class CTangentSpaceCalculation
{
public:

	// IN ------------------------------------------------

	//! /param inInput - the normals are only used as smoothing input - we calculate the normals ourself
	//! \return 0=no error, 1=texture coordinates include NAN
	unsigned int CalculateTangentSpace( const InputProxy &inInput );

	// OUT -----------------------------------------------

	//! /return the number of base vectors that are stored 0..
	size_t GetBaseCount();

	//!
	//! /param indwTriNo 0..
	//! /param outdwBase 
	void GetTriangleBaseIndices( const unsigned int indwTriNo, unsigned int outdwBase[3] );
	
	//! returns a orthogonal base (perpendicular and normalized)
	//! /param indwPos 0..
	//! /param outU in object/worldspace
	//! /param outV in object/worldspace
	//! /param outN in object/worldspace
	void GetBase( const unsigned int indwPos, float *outU, float *outV, float *outN );


private:	// -----------------------------------------------------------------

	class CVec2
	{
	public:
		float x,y;

		CVec2(){}
		CVec2(float fXval, float fYval) { x=fXval; y=fYval; }
		friend CVec2 operator - (const CVec2 &vec1, const CVec2 &vec2) { return CVec2(vec1.x-vec2.x, vec1.y-vec2.y); }
		operator float * () { return (float *)this; }
	};

	class CVec3
	{
	public:
		float x,y,z;

		CVec3(){}
		CVec3(float fXval, float fYval, float fZval) { x=fXval; y=fYval; z=fZval; }
		friend CVec3 operator - (const CVec3 &vec1, const CVec3 &vec2) { return CVec3(vec1.x-vec2.x, vec1.y-vec2.y, vec1.z-vec2.z); }
		friend CVec3 operator - (const CVec3 &vec1) { return CVec3(-vec1.x, -vec1.y, -vec1.z); }
		friend CVec3 operator + (const CVec3 &vec1, const CVec3 &vec2) { return CVec3(vec1.x+vec2.x, vec1.y+vec2.y, vec1.z+vec2.z); }
		friend CVec3 operator * (const CVec3 &vec1, const float fVal)	{ return CVec3(vec1.x*fVal, vec1.y*fVal, vec1.z*fVal); }
		friend float operator * (const CVec3 &vec1, const CVec3 &vec2)	{ return vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z; }
		operator float * () { return (float *)this; }
		void Negate() { x=-x;y=-y;z=-z; }		
		friend CVec3 normalize( const CVec3 &vec ) { CVec3 ret; float fLen=length(vec); if(fLen<0.00001f)return vec; fLen=1.0f/fLen;ret.x=vec.x*fLen;ret.y=vec.y*fLen;ret.z=vec.z*fLen;return ret; }
		friend CVec3 cross( const CVec3 &vec1, const CVec3 &vec2 ) { return CVec3(vec1.y*vec2.z-vec1.z*vec2.y, vec1.z*vec2.x-vec1.x*vec2.z, vec1.x*vec2.y-vec1.y*vec2.x); }
		friend float length( const CVec3 &invA ) { return (float)sqrt(invA.x*invA.x+invA.y*invA.y+invA.z*invA.z); }
		friend float CalcAngleBetween( const CVec3 &invA, const CVec3 &invB )
		{
			float LengthQ=length(invA)*length(invB);

			if(LengthQ<0.0001f)LengthQ=0.0001f;			// to prevent division by zero

			float f=(invA*invB)/LengthQ;

			if(f>1.0f)f=1.0f;												// acos need input in the range [-1..1]
				else if(f<-1.0f)f=-1.0f;							//

			float fRet=(float)acos(f);								// cosf is not avaiable on every plattform
			
			return fRet;
		}
		friend bool IsZero( const CVec3 &invA ) { return invA.x==0.0f && invA.y==0.0f && invA.z==0.0f; }
		friend bool IsNormalized( const CVec3 &invA ) { float f=length(invA);return f>=0.95f && f<=1.05f; }
	};

	class CBaseIndex
	{
	public:
		unsigned int m_dwPosNo;				//!< 0..
		unsigned int m_dwNormNo;				//!< 0..
	};

	// helper to get order for CVertexLoadHelper
	struct CBaseIndexOrder
	{
		bool operator() ( const CBaseIndex &a, const CBaseIndex &b ) const
		{
			// first sort by position
			if(a.m_dwPosNo<b.m_dwPosNo)return true;
			if(a.m_dwPosNo>b.m_dwPosNo)return false;

			// then by normal
			if(a.m_dwNormNo<b.m_dwNormNo)return true;
			if(a.m_dwNormNo>b.m_dwNormNo)return false;

			return false;
		}
	};


	class CBase33
	{
	public:

		CBase33() { }
		CBase33(CVec3 Uval, CVec3 Vval, CVec3 Nval) { u=Uval; v=Vval; n=Nval; }

		CVec3 u;								//!<
		CVec3 v;								//!<
		CVec3 n;								//!< is part of the tangent base but can be used also as vertex normal
	};

	class CTriBaseIndex
	{
	public:
		unsigned int p[3];							//!< index in m_BaseVectors 
	};

	// output data -----------------------------------------------------------------------------------

	std::vector<CTriBaseIndex>						m_TriBaseAssigment;							//!< [0..dwTriangleCount]
	std::vector<CBase33>									m_BaseVectors;									//!< [0..] generated output data

	//! /param indwPosNo
	//! /param indwNormNo
	CBase33 &GetBase( std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder> &inMap, const unsigned int indwPosNo, const unsigned int indwNormNo );

private: // ----------------------------------------------------------------------------------------

	//! creates, copies or returns a reference
	//! /param inMap
	//! /param indwPosNo
	//! /param indwNormNo
	//! /param inU weighted
	//! /param inV weighted
	//! /param inN normalized
	unsigned int AddUV2Base( std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder> &inMap, const unsigned int indwPosNo, const unsigned int indwNormNo, const CVec3 &inU, const CVec3 &inV, const CVec3 &inNormN );

	//! /param inMap
	//! /param indwPosNo
	//! /param indwNormNo
	//! /param inNormal weighted normal
	void AddNormal2Base( std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder> &inMap, const unsigned int indwPosNo, const unsigned int indwNormNo, const CVec3 &inNormal );

	void DebugMesh( const InputProxy &inInput ) const;
};




// ---------------------------------------------------------------------------------------------------------------

template <class InputProxy>
void CTangentSpaceCalculation<InputProxy>::DebugMesh( const InputProxy &inInput ) const
{
	unsigned int dwTriCount=inInput.GetTriangleCount();

	// search for polygons that use the same indices (input data problems)
	for(unsigned int a=0;a<dwTriCount;a++)
	{
		unsigned int dwAPos[3],dwANorm[3],dwAUV[3];

		inInput.GetTriangleIndices(a,dwAPos,dwANorm,dwAUV);

		for(unsigned int b=a+1;b<dwTriCount;b++)
		{
			unsigned int dwBPos[3],dwBNorm[3],dwBUV[3];

			inInput.GetTriangleIndices(b,dwBPos,dwBNorm,dwBUV);

			assert(!(  dwAPos[0]==dwBPos[0] && dwAPos[1]==dwBPos[1] && dwAPos[2]==dwBPos[2] ));
			assert(!(  dwAPos[1]==dwBPos[0] && dwAPos[2]==dwBPos[1] && dwAPos[0]==dwBPos[2] ));
			assert(!(  dwAPos[2]==dwBPos[0] && dwAPos[0]==dwBPos[1] && dwAPos[1]==dwBPos[2] ));

			assert(!(  dwAPos[1]==dwBPos[0] && dwAPos[0]==dwBPos[1] && dwAPos[2]==dwBPos[2] ));
			assert(!(  dwAPos[2]==dwBPos[0] && dwAPos[1]==dwBPos[1] && dwAPos[0]==dwBPos[2] ));
			assert(!(  dwAPos[0]==dwBPos[0] && dwAPos[2]==dwBPos[1] && dwAPos[1]==dwBPos[2] ));
		}
	}
}




template <class InputProxy>
unsigned int CTangentSpaceCalculation<InputProxy>::CalculateTangentSpace( const InputProxy &inInput )
{
	unsigned int dwTriCount=inInput.GetTriangleCount();

	bool bTextureCoordinatesBroken=false;		// not a number in texture coordinates

	// clear result
	m_BaseVectors.clear();
	m_TriBaseAssigment.clear();
	m_TriBaseAssigment.reserve(dwTriCount);
	assert(m_BaseVectors.empty());
	assert(m_TriBaseAssigment.empty());

	std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder>		mBaseMap;					// second=index into m_BaseVectors, generated output data
	std::vector<CBase33> vTriangleBase;																					// base vectors per triangle

//	DebugMesh(inInput);		// only for debugging - slow

	// calculate the base vectors per triangle -------------------------------------------
	{
		for(unsigned int i=0;i<dwTriCount;i++)
		{
			// get data from caller ---------------------------
			unsigned int dwPos[3],dwNorm[3],dwUV[3];

			inInput.GetTriangleIndices(i,dwPos,dwNorm,dwUV);

			CVec3 vPos[3];
			CVec2 vUV[3];

			for(int e=0;e<3;e++)
			{
				inInput.GetPos(dwPos[e],vPos[e]);
				inInput.GetUV(dwUV[e],vUV[e]);
			}

			// calculate tangent vectors ---------------------------

			CVec3 vA=vPos[1]-vPos[0];
			CVec3 vB=vPos[2]-vPos[0];

			float fDeltaU1=vUV[1].x-vUV[0].x;
			float fDeltaU2=vUV[2].x-vUV[0].x;
			float fDeltaV1=vUV[1].y-vUV[0].y;
			float fDeltaV2=vUV[2].y-vUV[0].y;

			float div	=(fDeltaU1*fDeltaV2-fDeltaU2*fDeltaV1);

			if(_isnan(div))
			{
				bTextureCoordinatesBroken=true;div=0.0f;
			}


			CVec3 vU,vV,vN=normalize(cross(vA,vB));

			if(div!=0.0)
			{
				//	2D triangle area = (u1*v2-u2*v1)/2
				// weight the tangent vectors by the UV triangles area size (fix problems with base UV assignment)
				float fAreaMul2=fabsf(fDeltaU1*fDeltaV2-fDeltaU2*fDeltaV1);

				float a = fDeltaV2/div;
				float b	= -fDeltaV1/div;
				float c = -fDeltaU2/div;
				float d	= fDeltaU1/div;

				vU=normalize(vA*a+vB*b)*fAreaMul2;
				vV=normalize(vA*c+vB*d)*fAreaMul2;
			}
			else
      {
        vU=CVec3(1,0,0);vV=CVec3(0,1,0);
      }

			vTriangleBase.push_back(CBase33(vU,vV,vN));
		}
	}


	// distribute the normals to the vertices
	{
		// we create a new tangent base for every vertex index that has a different normal (later we split further for mirrored use)
		// and sum the base vectors (weighted by angle and mirrored if necessary)
		for(unsigned int i=0;i<dwTriCount;i++)
		{
			unsigned int e;

			// get data from caller ---------------------------
			unsigned int dwPos[3],dwNorm[3],dwUV[3];

			inInput.GetTriangleIndices(i,dwPos,dwNorm,dwUV);
			CBase33 TriBase=vTriangleBase[i];
			CVec3 vPos[3];

			for(e=0;e<3;e++) inInput.GetPos(dwPos[e],vPos[e]);

			// for each triangle corner
			for(e=0;e<3;e++)	
			{
				float fWeight=CalcAngleBetween( vPos[(e+2)%3]-vPos[e],vPos[(e+1)%3]-vPos[e] );			// weight by angle to fix the L-Shape problem

				if(fWeight<=0.0f)
					fWeight=0.0001f;

				AddNormal2Base(mBaseMap,dwPos[e],dwNorm[e],TriBase.n*fWeight);
			}
		}
	}

	// distribute the uv vectors to the vertices
	{
		// we create a new tangent base for every vertex index that has a different normal
		// if the base vectors does'nt fit we split as well
		for(unsigned int i=0;i<dwTriCount;i++)
		{
			unsigned int e;

			// get data from caller ---------------------------
			unsigned int dwPos[3],dwNorm[3],dwUV[3];

			CTriBaseIndex Indx;
			inInput.GetTriangleIndices(i,dwPos,dwNorm,dwUV);
			CBase33 TriBase=vTriangleBase[i];
			CVec3 vPos[3];

			for(e=0;e<3;e++) 
				inInput.GetPos(dwPos[e],vPos[e]);

			// for each triangle corner
			for(e=0;e<3;e++)	
			{
				float fWeight=CalcAngleBetween( vPos[(e+2)%3]-vPos[e],vPos[(e+1)%3]-vPos[e] );			// weight by angle to fix the L-Shape problem

				Indx.p[e]=AddUV2Base(mBaseMap,dwPos[e],dwNorm[e],TriBase.u*fWeight,TriBase.v*fWeight,normalize(TriBase.n));
			}

			m_TriBaseAssigment.push_back(Indx);
		}
	}


	// adjust the base vectors per vertex -------------------------------------------
	{
		std::vector<CBase33>::template iterator it;
		
		for(it=m_BaseVectors.begin();it!=m_BaseVectors.end();++it)
		{
			CBase33 &ref=(*it);

			// rotate u and v in n plane
			{
				CVec3 vUout,vVout,vNout;

				vNout=normalize(ref.n);

				vUout = ref.u - vNout * (vNout*ref.u);						// project u in n plane
				vVout = ref.v - vNout * (vNout*ref.v);						// project v in n plane

				ref.u=normalize(vUout);ref.v=normalize(vVout);ref.n=vNout;
				
				//assert(ref.u.x>=-1 && ref.u.x<=1);
				//assert(ref.u.y>=-1 && ref.u.y<=1);
				//assert(ref.u.z>=-1 && ref.u.z<=1);
				//assert(ref.v.x>=-1 && ref.v.x<=1);
				//assert(ref.v.y>=-1 && ref.v.y<=1);
				//assert(ref.v.z>=-1 && ref.v.z<=1);
				//assert(ref.n.x>=-1 && ref.n.x<=1);
				//assert(ref.n.y>=-1 && ref.n.y<=1);
				//assert(ref.n.z>=-1 && ref.n.z<=1);
			}
		}
	}

	return bTextureCoordinatesBroken?1:0;		// 0=no error
}



template <class InputProxy>
unsigned int CTangentSpaceCalculation<InputProxy>::AddUV2Base( std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder> &inMap,
	const unsigned int indwPosNo, const unsigned int indwNormNo, const CVec3 &inU, const CVec3 &inV, const CVec3 &inNormN )
{
	// no mesh is perfect
//	assert(IsNormalized(inNormN));

	CBaseIndex Indx;

	Indx.m_dwPosNo=indwPosNo;
	Indx.m_dwNormNo=indwNormNo;

	std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder>::template iterator iFind,iFindEnd;
		
	iFind = inMap.lower_bound(Indx);

	assert(iFind!=inMap.end());

	CVec3 vNormal=m_BaseVectors[(*iFind).second].n;

	iFindEnd = inMap.upper_bound(Indx);

	unsigned int dwBaseUVIndex=0xffffffff;													// init with not found

	bool bParity=(cross(inU,inV)*inNormN>0.0f);

	for(;iFind!=iFindEnd;++iFind)
	{
		CBase33 &refFound=m_BaseVectors[(*iFind).second];

		if(!IsZero(refFound.u)) 
		{
			bool bParityRef=(cross(refFound.u,refFound.v)*refFound.n>0.0f);
			bool bParityCheck=(bParityRef==bParity);

			if(!bParityCheck)
				continue;

			CVec3 vrefUV = refFound.u+refFound.v;

			// project down to the plane (plane normal = inNormN)
			CVec3 vRotHalf = vrefUV - inNormN*(inNormN*vrefUV);

//			bool bHalfAngleCheck=normalize(inU+inV) * normalize(vRotHalf) > cos(fThreshold);
			bool bHalfAngleCheck = (inU+inV) * vRotHalf > 0.0f;		// length doesn't matter so normalize is not needed

			if(!bHalfAngleCheck)
				continue;
		}

		dwBaseUVIndex=(*iFind).second;break;
	}

	if(dwBaseUVIndex==0xffffffff)														// not found
	{	
		// otherwise create a new base

		CBase33 Base( CVec3(0,0,0), CVec3(0,0,0), vNormal );

		dwBaseUVIndex = (uint32)m_BaseVectors.size();

		inMap.insert( std::pair<CBaseIndex,unsigned int>(Indx,dwBaseUVIndex) );
		m_BaseVectors.push_back(Base);
	}

	CBase33 &refBaseUV=m_BaseVectors[dwBaseUVIndex];

	refBaseUV.u=refBaseUV.u+inU;
	refBaseUV.v=refBaseUV.v+inV;

	//no mesh is perfect 
	if(inU.x!=0.0f || inU.y!=0.0f || inU.z!=0.0f)
		assert(refBaseUV.u.x!=0.0f || refBaseUV.u.y!=0.0f || refBaseUV.u.z!=0.0f);
	// no mesh is perfect
	if(inV.x!=0.0f || inV.y!=0.0f || inV.z!=0.0f)
		assert(refBaseUV.v.x!=0.0f || refBaseUV.v.y!=0.0f || refBaseUV.v.z!=0.0f);

	return dwBaseUVIndex;
}




template <class InputProxy>
void CTangentSpaceCalculation<InputProxy>::AddNormal2Base( std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder> &inMap,
	const unsigned int indwPosNo, const unsigned int indwNormNo, const CVec3 &inNormal  )
{
	CBaseIndex Indx;

	Indx.m_dwPosNo=indwPosNo;
	Indx.m_dwNormNo=indwNormNo;

	std::multimap<CBaseIndex,unsigned int,CBaseIndexOrder>::template iterator iFind = inMap.find(Indx);

	unsigned int dwBaseNIndex;

	if(iFind!=inMap.end())																// found
	{
		// resuse the existing one

		dwBaseNIndex=(*iFind).second;
	}
	else
	{
		// otherwise create a new base

		CBase33 Base( CVec3(0,0,0), CVec3(0,0,0), CVec3(0,0,0) );

		dwBaseNIndex = (uint32)m_BaseVectors.size();
		inMap.insert( std::pair<CBaseIndex,unsigned int>(Indx,dwBaseNIndex) );
		m_BaseVectors.push_back(Base);
	}

	CBase33 &refBaseN=m_BaseVectors[dwBaseNIndex];

	refBaseN.n=refBaseN.n+inNormal;
}



template <class InputProxy>
void CTangentSpaceCalculation<InputProxy>::GetBase( const unsigned int indwPos, float *outU, float *outV, float *outN )
{
	CBase33 &base=m_BaseVectors[indwPos];

	outU[0]=base.u.x;
	outV[0]=base.v.x;
	outN[0]=base.n.x;
	outU[1]=base.u.y;
	outV[1]=base.v.y;
	outN[1]=base.n.y;
	outU[2]=base.u.z;
	outV[2]=base.v.z;
	outN[2]=base.n.z;
}


template <class InputProxy>
void CTangentSpaceCalculation<InputProxy>::GetTriangleBaseIndices( const unsigned int indwTriNo, unsigned int outdwBase[3] )
{
	assert(indwTriNo<m_TriBaseAssigment.size());
	CTriBaseIndex &indx=m_TriBaseAssigment[indwTriNo];

	for(unsigned int i=0;i<3;i++)
		outdwBase[i]=indx.p[i];
}


template <class InputProxy> 
size_t CTangentSpaceCalculation<InputProxy>::GetBaseCount()
{
	return m_BaseVectors.size();
}
