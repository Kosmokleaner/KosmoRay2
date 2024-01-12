#pragma once

#include "Mathlib.h"

// quaternion, left handed
class CQuat
{
public:
	// default constructor
	CQuat()
	{
		// quaternion identity
		//		Rot = XMVectorSet(0, 0, 0, 1);
		//		Rot = XMQuaternionRotationRollPitchYaw(0, 0, 0);
		//		Rot = XMQuaternionRotationRollPitchYaw(PI/2, 0, 0);
		//		Rot = XMQuaternionRotationRollPitchYaw(-PI / 2, 0, 0);
		//		Rot = XMQuaternionRotationRollPitchYaw(PI, 0, 0);
		//		Rot = XMQuaternionRotationRollPitchYaw(0, PI / 2, 0);
		//		Rot = XMQuaternionRotationRollPitchYaw(0, 0, PI / 2);
		//		Rot = XMQuaternionRotationRollPitchYaw(0, 0, -PI / 2);
		//		Rot = XMQuaternionRotationRollPitchYaw(0, 0, 0);
		Rot = XMQuaternionIdentity();
	}

	// constructor
	CQuat(XMVECTOR In)
	{
		Rot = In;
	}

	// set quaternion
	void SetRot(XMVECTOR InRot)
	{
		Rot = InRot;
	}
	// @param pitch: positive looks up
	// @param yaw: positive looks right
	// @param roll: positive rolls clockwise
	void SetPitchYawRoll(float Pitch, float Yaw, float Roll)
	{
		// XMQuaternionRotationRollPitchYaw is badly named as it's actually (Pitch,Yaw,Roll)
		Rot = XMQuaternionRotationRollPitchYaw(Pitch, Yaw, Roll);
	}

	void DumpState(const char* Name) const
	{
		char str[256];

		sprintf_s(str, sizeof(str), "%s.SetRot(%ff, %ff, %ff, %ff);\n", Name, Rot.m128_f32[0], Rot.m128_f32[1], Rot.m128_f32[2], Rot.m128_f32[3]);
		OutputDebugStringA(str);
	}

	XMVECTOR GetRot() const
	{
		return Rot;
	}

	CQuat& operator*=(const CQuat& rhs) { *this = *this * rhs; return *this; }

	// multiply quaternions
	friend CQuat operator*(const CQuat& a, const CQuat& b)
	{
		CQuat Ret;
		
		// The result represents the rotation a followed by the rotation b to be consistent with XMMatrixMulplity concatenation
		Ret.Rot = XMQuaternionMultiply(a.Rot, b.Rot);

		return Ret;
	}
	friend float3 operator*(const float3& a, const CQuat& b)
	{
		XMVECTOR A = XMVectorSet(a.x, a.y, a.z, 0);
		
		float3 Ret = ToFloat3(XMVector3Rotate(A, b.Rot));

		return Ret;
	}

	// y = M * x
	float3 Transform(const float3& x) const
	{
		return ToFloat3(XMVector3Rotate(x.GetXMVECTOR(), Rot));
	}

protected:
	// quaternion, only used if bUseAngles==false
	XMVECTOR Rot;
};

// left handed
class CTransform
{
public:
	CTransform(float3 InPos = float3(0, 0, 0))
	{
//		eye = float3(2.0f, 20.7f, 110.0f);
		SetPos(InPos);
	}

	void SetPos(float3 InPos)
	{
		eye = InPos;
	}

	void Move(float3 delta)
	{
		eye += delta;
	}

	float2 GetAbsRotation() const
	{
		float3 forward = GetForward();

		return float2(atan2f(forward.x, forward.z) + (float)PI, asinf(forward.y));
	}

	// relative
	// @param dx: positive rotates to the right, in radiants (0..2*PI is 360)
	// @param dy: positive rotates upwards, in radiants (0..2*PI is 360)
	void Rotate(float dx, float dy)
	{
		float2 w = GetAbsRotation();

		w.x += dx;
		w.y += dy;

		SetRotation(w);
	}

	// absolute
	void SetRotation(float2 w)
	{
		const float yborder = 0.00001f;

		// clamp top
		w.y = Max(w.y, -PI * 0.5f + yborder);
		// clamp bottom
		w.y = Min(w.y, PI * 0.5f - yborder);

		Rot.SetRot(XMQuaternionRotationRollPitchYaw(w.y, w.x, 0));
	}

	void DumpState(const char* Name) const
	{
		char str[256];

		sprintf_s(str, sizeof(str), "\n%s.SetPos(%ff, %ff, %ff);\n", Name, eye[0], eye[1], eye[2]);
		OutputDebugStringA(str);

		Rot.DumpState(Name);
	}

	// camera center
	float3 GetPos() const { return eye; }


	// @return forward vector, is normalized
	float3 GetForward() const
	{
		return float3(0,0,-1) * Rot;
	}

	// @return up vector, is normalized
	float3 GetUp() const
	{
		return GetDirY();
	}

	// @return right vector, is normalized
	float3 GetDirX() const
	{
		return float3(1, 0, 0) * Rot;
	}
	// @return up vector, is normalized
	float3 GetDirY() const
	{
		return float3(0, 1, 0) * Rot;
	}
	// @return back vector, is normalized
	float3 GetDirZ() const
	{
		return float3(0, 0, 1) * Rot;
	}

	XMMATRIX GetViewMatrix() const
	{
		return XMMatrixLookAtLH(GetPos().GetXMVECTOR(), (eye + GetForward()).GetXMVECTOR(), GetUp().GetXMVECTOR());
		// can be optimized
//		XMMATRIX rot = XMMatrixRotationQuaternion(XMQuaternionConjugate(Rot.GetRot()));
//		XMMATRIX pos = XMMatrixTranslation(-eye.x, -eye.y, -eye.z);

		// this is the right order
//		return pos * rot;
	}

	CTransform& operator*=(const CTransform& rhs) { *this = *this * rhs; return *this; }

	// y = M * x
	// e.g. if this is n object matrix, b is in object space and the output is in world space
	float3 TransformPosition(const float3& x) const
	{
		return eye + Rot.Transform(x);
	}

	// multiply, b is applied first
	friend CTransform operator*(const CTransform& a, const CTransform& b)
	{
		CTransform Ret;

		// The result represents the rotation a followed by the rotation b to be consistent with XMMatrixMulplity concatenation
		Ret.Rot = a.Rot * b.Rot;
		Ret.eye = a.eye + ToFloat3(XMVector3Rotate(b.eye.GetXMVECTOR(), a.Rot.GetRot()));

		return Ret;
	}

	CQuat Rot;

protected:
	// camera center
	float3 eye;
};


class CCamera : public CTransform
{
public:
	CCamera()
	{
		Fov4 = XMFLOAT4(1, 1, 1, 1);
	}

	void SetFov(XMFLOAT4 InFov4)
	{
		Fov4 = InFov4;
	}

	float GetVerticalFov() const
	{
//		ovrFovPort LocalFov4;

//	LocalFov4.UpTan = Fov4.x;
//		LocalFov4.DownTan = Fov4.y;
//		LocalFov4.LeftTan = Fov4.z;
//		LocalFov4.RightTan = Fov4.w;

		// correct?
		return atanf(Fov4.x) + atanf(Fov4.y);
	}

	// @param ZParamsX zNear
	// @param ZParamsY zFar
	XMMATRIX GetProjectionMatrix(float ZParamsX, float ZParamsY, float EyeRTSizeX, float EyeRTSizeY, float JitterOffsetX, float JitterOffsetY) const
	{
//		ovrFovPort LocalFov4;

//		LocalFov4.UpTan = Fov4.x;
//		LocalFov4.DownTan = Fov4.y;
//		LocalFov4.LeftTan = Fov4.z;
//		LocalFov4.RightTan = Fov4.w;

		// Notes:
		// ovrProjection_FarLessThanNear makes a difference 
		// changing the order of znear and far does make a difference, why? What is right?
		// by default it's right handed
		// when using right handed the sensor data needs to be reinterpreted
		// using ovrProjection_LeftHanded means the sensor data needs to be flipped as well: https://developer3.oculus.com/documentation/pcsdk/latest/concepts/dg-sensor
		// ovr_SubmitFrame RenderPose still needs to get the handed sensor data (not clearly mentioned in the SDK docs)

		//			ovrMatrix4f p = ovrMatrix4f_Projection(LocalFov4, ZParamsX, ZParamsY, ovrProjection_FarLessThanNear | ovrProjection_LeftHanded);	// ok but <1m is wrong
		//			ovrMatrix4f p = ovrMatrix4f_Projection(LocalFov4, ZParamsY, ZParamsX, ovrProjection_FarLessThanNear | ovrProjection_LeftHanded);	// garbage
		//			ovrMatrix4f p = ovrMatrix4f_Projection(LocalFov4, ZParamsY, ZParamsX, ovrProjection_LeftHanded);		// same as not exchanged and ovrProjection_FarLessThanNear	???

#if G_OCULUS_VR
#if G_INVERSE_ZBUFFER
		ovrMatrix4f p = ovrMatrix4f_Projection(LocalFov4, ZParamsX, ZParamsY, ovrProjection_FarLessThanNear | ovrProjection_LeftHanded);
#else
		ovrMatrix4f p = ovrMatrix4f_Projection(LocalFov4, ZParamsX, ZParamsY, ovrProjection_LeftHanded);
#endif
    return ConvertToXM(p);
#else
  // todo
  assert(0);
  return XMMATRIX();
#endif
		//			ovrMatrix4f p = ovrMatrix4f_Projection(LocalFov4, ZParamsY, ZParamsX, ovrProjection_FarClipAtInfinity | ovrProjection_LeftHanded);
	}

private:
  // float4(UpTan,DownTan,LeftTan,RightTan)
	XMFLOAT4 Fov4;
};

#if G_OCULUS_VR
// for Oculus
inline ovrQuatf To_ovrQuatf(const CQuat& In)
{
	ovrQuatf Ret;

	Ret.x = XMVectorGetX(In.GetRot());
	Ret.y = XMVectorGetY(In.GetRot());
	Ret.z = XMVectorGetZ(In.GetRot());
	Ret.w = XMVectorGetW(In.GetRot());

	return Ret;
}

// for Oculus
inline ovrVector3f To_ovrVector3f(const float3& In)
{
	ovrVector3f Ret;

	Ret.x = In.x;
	Ret.y = In.y;
	Ret.z = In.z;

	return Ret;
}

// for Oculus
inline ovrPosef To_ovrPosef(const CTransform& In)
{
	ovrPosef Ret;

	Ret.Orientation = To_ovrQuatf(In.Rot);
	Ret.Position = To_ovrVector3f(In.GetPos());

	return Ret;
}
#endif