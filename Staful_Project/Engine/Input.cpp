#include "Input.h"
#include <string>

namespace Input
{
	LPDIRECTINPUT8   pDInput = nullptr;
	LPDIRECTINPUTDEVICE8 pKeyDevice = nullptr;
	BYTE keyState[256] = { 0 };
	BYTE prevKeyState[256];    //�O�t���[���ł̊e�L�[�̏��

	//�}�E�X
	LPDIRECTINPUTDEVICE8	pMouseDevice;
	DIMOUSESTATE			mouseState;
	DIMOUSESTATE			prevMouseState;
	XMFLOAT3				mousePosition;

	void Initialize(HWND hWnd)
	{
		DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&pDInput, nullptr);

		pDInput->CreateDevice(GUID_SysKeyboard, &pKeyDevice, nullptr); //pDInput��new����݂����Ȋ���
		pKeyDevice->SetDataFormat(&c_dfDIKeyboard); //�f�o�C�X�̎�ނ��w��
		pKeyDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

		pDInput->CreateDevice(GUID_SysMouse, &pMouseDevice, nullptr); //pDInput��new����݂����Ȋ���
		pMouseDevice->SetDataFormat(&c_dfDIMouse); //�f�o�C�X�̎�ނ��w��
		pMouseDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}

	void Update()
	{
		memcpy(prevKeyState, keyState, sizeof(BYTE) * 256);
		pKeyDevice->Acquire();
		pKeyDevice->GetDeviceState(sizeof(keyState), &keyState);

		//�}�E�X
		memcpy(&prevMouseState, &mouseState, sizeof(mouseState));
		pMouseDevice->Acquire();
		pMouseDevice->GetDeviceState(sizeof(mouseState), &mouseState);
	}

	bool IsKey(int keyCode)
	{
		if (keyState[keyCode] & 0x80)
		{
			return true;
		}
		return false;
	}
	bool IsKeyDown(int keyCode)
	{
		//���͉����ĂāA�O��͉����ĂȂ�
		if (keyState[keyCode] & 0x80 && (~prevKeyState[keyCode] & 0x80))
		{
			return true;
		}
		return false;
	}

	bool IsKeyUp(int keyCode)
	{
		if ((~keyState[keyCode] & 0x80) && prevKeyState[keyCode] & 0x80)
		{
			return true;
		}
		return false;
	}

	void Release()
	{
		SAFE_RELEASE(pDInput);
		SAFE_RELEASE(pKeyDevice);
	}
	bool IsMouseButton(int buttonCode)
	{
		if (mouseState.rgbButtons[buttonCode] & 0x80)
		{
			return true;
		}
		return false;
	}
	bool IsMouseButtonDown(int buttonCode)
	{
		if (mouseState.rgbButtons[buttonCode] & 0x80 && (~prevMouseState.rgbButtons[buttonCode] & 0x80))
		{
			return true;
		}
		return false;
	}
	bool IsMuoseButtonUp(int buttonCode)
	{
		if ((~mouseState.rgbButtons[buttonCode]) & 0x80 && prevMouseState.rgbButtons[buttonCode] & 0x80)
		{
			return true;
		}
		return false;
	}
	XMFLOAT3 GetMousePosition()
	{
		return mousePosition;
	}
	XMFLOAT3 GetMouseMove()
	{
		return XMFLOAT3((float)mouseState.lX,
						(float)mouseState.lY,
						(float)mouseState.lZ);
	}
	void SetMousePosition(int x, int y)
	{
		mousePosition.x = x;
		mousePosition.y = y;
		//std::string resStr = std::to_string((float)x) + ", " + std::to_string(y) + "\n";
		//OutputDebugString(resStr.c_str());
	}
}