#pragma once

#include "Mathlib.h"

class CTransform
{
public:


    CTransform(glm::vec3 InPos = glm::vec3(0, 0, 0))
    {
        //		eye = glm::vec3(2.0f, 20.7f, 110.0f);
        SetPos(InPos);
        Rot = glm::quat(1,0,0,0);
    }

    void SetPos(glm::vec3 InPos)
    {
        eye = InPos;
    }

    void Move(glm::vec3 delta)
    {
        eye += delta;
    }

    glm::vec2 GetAbsRotation() const
    {
        glm::vec3 forward = GetForward();

        return glm::vec2(atan2f(forward.x, forward.z) + (float)PI, asinf(forward.y));
    }

    // relative
    // @param dx: positive rotates to the right, in radiants (0..2*PI is 360)
    // @param dy: positive rotates upwards, in radiants (0..2*PI is 360)
    void Rotate(float dx, float dy)
    {
        glm::vec2 w = GetAbsRotation();

        w.x += dx;
        w.y += dy;

        SetRotation(w);
    }

    // absolute
    void SetRotation(glm::vec2 w)
    {
        const float yborder = 0.00001f;

        // clamp top
        w.y = Max(w.y, -PI * 0.5f + yborder);
        // clamp bottom
        w.y = Min(w.y, PI * 0.5f - yborder);

        glm::vec3 euler(w.y, w.x, 0);
        Rot = glm::quat(euler);
//        Rot.SetRot(XMQuaternionRotationRollPitchYaw(w.y, w.x, 0));
    }
/*
    void DumpState(const char* Name) const
    {
        char str[256];

        sprintf_s(str, sizeof(str), "\n%s.SetPos(%ff, %ff, %ff);\n", Name, eye[0], eye[1], eye[2]);
        OutputDebugStringA(str);

        Rot.DumpState(Name);
    }
*/
    // camera center
    glm::vec3 GetPos() const { return eye; }


    // @return forward vector, is normalized
    glm::vec3 GetForward() const
    {
        return Rot * glm::vec3(0, 0, -1);
    }

    // @return up vector, is normalized
    glm::vec3 GetUp() const
    {
        return GetDirY();
    }

    // @return right vector, is normalized
    glm::vec3 GetDirX() const
    {
        return Rot * glm::vec3(1, 0, 0);
    }
    // @return up vector, is normalized
    glm::vec3 GetDirY() const
    {
        return Rot * glm::vec3(0, 1, 0);
    }
    // @return back vector, is normalized
    glm::vec3 GetDirZ() const
    {
        return Rot * glm::vec3(0, 0, 1);
    }

    glm::mat4 GetViewMatrix() const
    {
        glm::mat4  ret = glm::lookAtLH(GetPos(), (eye + GetForward()), GetUp());
        // can be optimized
//		XMMATRIX rot = XMMatrixRotationQuaternion(XMQuaternionConjugate(Rot.GetRot()));
//		XMMATRIX pos = XMMatrixTranslation(-eye.x, -eye.y, -eye.z);

        // this is the right order
//		return pos * rot;
        return ret;
    }

    CTransform& operator*=(const CTransform& rhs) { *this = *this * rhs; return *this; }

    // y = M * x
    // e.g. if this is n object matrix, b is in object space and the output is in world space
    glm::vec3 TransformPosition(const glm::vec3& x) const
    {
        return eye + Rot * x;
    }

    // multiply, b is applied first
    friend CTransform operator*(const CTransform& a, const CTransform& b)
    {
        CTransform Ret;

        // The result represents the rotation a followed by the rotation b to be consistent with XMMatrixMulplity concatenation
        Ret.Rot = a.Rot * b.Rot;
        Ret.eye = a.eye + a.Rot * b.eye;

        return Ret;
    }

    glm::quat Rot;

protected:
    // camera center
    glm::vec3 eye;
};



class CCamera : public CTransform
{
public:
	CCamera()
	{
		Fov4 = glm::vec4(1, 1, 1, 1);
	}

	void SetFov(glm::vec4 InFov4)
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
    glm::mat4 GetProjectionMatrix(float ZParamsX, float ZParamsY, float EyeRTSizeX, float EyeRTSizeY, float JitterOffsetX, float JitterOffsetY) const
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
  return glm::mat4();
#endif
		//			ovrMatrix4f p = ovrMatrix4f_Projection(LocalFov4, ZParamsY, ZParamsX, ovrProjection_FarClipAtInfinity | ovrProjection_LeftHanded);
	}

private:
  // float4(UpTan,DownTan,LeftTan,RightTan)
    glm::vec4 Fov4;
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
inline ovrVector3f To_ovrVector3f(const glm::vec3& In)
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