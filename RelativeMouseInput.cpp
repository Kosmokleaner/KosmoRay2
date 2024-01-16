#include "RelativeMouseInput.h"
#include <assert.h>

CRelativeMouseInput::CRelativeMouseInput()
	: UserCapture(0), UserData(0), ObservedWindow(0), bCaptured(false)
{
}

CRelativeMouseInput::~CRelativeMouseInput()
{
	Current.ButtonMask = MIB_None;
	UpdateCaptureState();
}

bool CRelativeMouseInput::IsCaptured(const char* User) const
{
	if(User)
	{
		return UserCapture == User;
	}
	else
	{
		return UserCapture != 0;
	}
}

void CRelativeMouseInput::Create(HWND Window)
{
	assert(Window);

	ObservedWindow = Window;
}

// to give the mouse some area to move
static POINT GetScreenCenter()
{
	POINT ret;

	ret.x = GetSystemMetrics(SM_CXSCREEN) / 2;
	ret.y = GetSystemMetrics(SM_CYSCREEN) / 2;

	return ret;
}

void CRelativeMouseInput::WndProc(HWND Wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(Wnd != ObservedWindow)
	{
		// Create missing or too early
		return;
	}

	switch(message)
	{
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MOUSEMOVE:
			{
				Current.ButtonMask = (MouseInputButtons)(wParam & MIB_All);
	//			UpdateCaptureState();

//				int2 GlobalPos(LOWORD(lParam), HIWORD(lParam));
			}
			break;
	}

	if(IsCaptured())
	{
		POINT NewScreenPos;

		GetCursorPos(&NewScreenPos);

		POINT Center = GetScreenCenter();
		
		int AbsX = NewScreenPos.x - Center.x;
		int AbsY = NewScreenPos.y - Center.y;

		Current.RelativeX += NewScreenPos.x - DragLastPos.x;
		Current.RelativeY += NewScreenPos.y - DragLastPos.y;

		// when the mouse is leaving the zone we move it back in
		// assuming a minimal screen resolution of 640x480 200 is a good size
		int Zone = 200;

		bool MoveCusror = false;

		if(AbsX < -Zone)
		{
			MoveCusror = true;
			AbsX += Zone;
		}
		else if(AbsX > Zone)
		{
			MoveCusror = true;
			AbsX -= Zone;
		}
		
		if(AbsY < -Zone)
		{
			MoveCusror = true;
			AbsY += Zone;
		}
		else if(AbsY > Zone)
		{
			MoveCusror = true;
			AbsY -= Zone;
		}

		DragLastPos.x = Center.x + AbsX;
		DragLastPos.y = Center.y + AbsY;

		if(MoveCusror)
		{
			SetCursorPos(DragLastPos.x, DragLastPos.y);
		}
	}
	else
	{	
		GetCursorPos((POINT*)&Current.AbsolutePos);
	}
}

bool CRelativeMouseInput::SetUserCapture(const char* User, void *Data)
{
	assert(User);

	if(UserCapture)
	{
		return false;
	}

	UserCapture = User;
	UserData = Data;

	UpdateCaptureState();

	return true;
}

void CRelativeMouseInput::ResetUserCapture(const char* User)
{
	if(UserCapture != User)
		return;

	UserCapture = 0;
	UserData = 0;

	UpdateCaptureState();
}

void CRelativeMouseInput::UpdateCaptureState()
{
	bool bNewCaptureCaptureState = UserCapture != 0;

	if(bNewCaptureCaptureState == bCaptured)
	{
		return;
	}

	if(!UserCapture)
	{
		int f =0;
	}

	bCaptured = bNewCaptureCaptureState;

	if(bNewCaptureCaptureState)
	{
		GetCursorPos((POINT*)&Current.AbsolutePos);

		ShowCursor(false);
		SetCapture(ObservedWindow);
		POINT Center = GetScreenCenter();
		SetCursorPos(Center.x, Center.y);
		DragLastPos = int2(Center.x, Center.y);

/*
		RECT rect;
		GetWindowRect(ObservedWindow, &rect);
		ClipCursor(&rect);
*/
	}
	else
	{
		// to disable WndProc
		HWND BackupObservedWindow = ObservedWindow;
		ObservedWindow = 0;

		ReleaseCapture();

		SetCursorPos(Current.AbsolutePos.x, Current.AbsolutePos.y);
		ShowCursor(true);
		Current.RelativeX = 0;
		Current.RelativeY = 0;

		ObservedWindow = BackupObservedWindow;
	}
}

