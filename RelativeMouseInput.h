#pragma once

#include "external/glm/vec2.hpp" // glm::ivec2
#include "windows.h" // HWND

//
// 03/24/2010
// Martin Mittring
//
// Dependencies:
// * none
//
// streamlined for Win32
// can be adjusted to fit MFC as well
// does not handle wacom tablets yet

class CRelativeMouseInput
{
public:
	// same numbers as windows MB_...
	enum MouseInputButtons
	{
		MIB_LButton = 0x01,
		MIB_RButton = 0x02,
		MIB_Shift   = 0x04,
		MIB_Control = 0x08,
		MIB_MButton = 0x10,
		//
		MIB_None    = 0x00,
		MIB_All     = 0x1f,
		MIB_InValid = -1,
	};

	struct MouseInputData
	{
		MouseInputData() :RelativeX(0), RelativeY(0), ButtonMask(MIB_InValid)
		{
			AbsolutePos.x = 0;
			AbsolutePos.y = 0;
		}

		bool IsValid() const
		{
			return ButtonMask != MIB_InValid;
		}

		// relative mouse movement, 0 when not captured
		int					RelativeX;
		// relative mouse movement, 0 when not captured
		int					RelativeY;
		// absolute mouse position on the screen (does not change when captured)
        glm::ivec2			AbsolutePos;
		// current button state
		MouseInputButtons	ButtonMask;
	};

	// -------------------------------------------------------

	// constructor
	CRelativeMouseInput();//MouseInputButtons InCaptureMask = MIB_RButton);
	// destructor
	~CRelativeMouseInput();

	void Create(HWND Wnd);

	// call before your windows message loop
	void WndProc(HWND Wnd, UINT message, WPARAM wParam, LPARAM lParam);

	const MouseInputButtons GetMouseButtons() const
	{	
		return Current.IsValid() ? Current.ButtonMask : MouseInputButtons(0);
	}

	// @param User must not be 0
	const MouseInputData ClaimMouseInputData(const char* User, void *Data = 0)
	{	
		assert(User);

		MouseInputData Ret = Current;

		if(IsCaptured(User) && UserData == Data)
		{
			ResetRelativeMovement();
			return Ret;
		}

		return MouseInputData();
	}

	// @param user 0: if capture at all, !=0 if capture by this user
	bool IsCaptured(const char* User = 0) const;

	// optional the mouse can be captured by calling this function
	// @param User must not be 0
	bool SetUserCapture(const char* User, void *Data = 0);

	// @param User must not be 0
	void ResetUserCapture(const char* User);
	
	void *					UserData;

    glm::ivec2 GetAbsPos() const { return Current.AbsolutePos;  }

private: // ---------------------------------------

	// capture if either user or Capture Mask is true
	void UpdateCaptureState();

	void ResetRelativeMovement()
	{
		Current.RelativeX = 0;
		Current.RelativeY = 0;
	}

	// --------------------------------------------

	// current button state and position
	MouseInputData Current;
	// absolute mouse position
    glm::ivec2 DragLastPos = glm::ivec2(0.0f, 0.0f);
	// 0 if not, name is used for debugging
	const char* UserCapture;
	
	HWND ObservedWindow;

	bool bCaptured;
};
