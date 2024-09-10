#include "InputClass.h"

InputClass::InputClass()
{
	m_directInput = 0;
	m_keyboardDevice = 0;
	m_mouseDevice = 0;
	m_sensitivity = 0.1f;

	ZeroMemory(m_currKeyState, sizeof(m_currKeyState));
	ZeroMemory(m_prevKeyState, sizeof(m_prevKeyState));
	ZeroMemory(m_prevMouseState, sizeof(m_prevMouseState));
}

InputClass::~InputClass()
{
}

bool InputClass::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	HRESULT result;

	//DirectInput ����
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, nullptr);
	if (FAILED(result))
	{
		return false;
	}

	//Ű����� Device ����
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboardDevice, nullptr);
	if (FAILED(result))
	{
		return false;
	}

	//������ ������ Ű���� �������� ����
	result = m_keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	//���� ���� ����
	result = m_keyboardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	//Ű���� ��ġ ȹ��
	result = m_keyboardDevice->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	//���콺�� Device ����
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouseDevice, nullptr);
	if (FAILED(result))
	{
		return false;
	}

	//������ ������ ���콺 �������� ����
	result = m_mouseDevice->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(result))
	{
		return false;
	}

	//���� ���� ����
	result = m_mouseDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	//���콺 ��ġ ȹ��
	result = m_mouseDevice->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool InputClass::Frame()
{
	bool result;

	//���콺 State�� ������Ʈ
	result = ReadMouse();
	if (!result)
	{
		return false;
	}

	//Ű���� State�� ������Ʈ
	result = ReadKeyboard();
	if (!result)
	{
		return false;
	}

	return true;
}

void InputClass::Shutdown()
{
	//���콺 DirectDevice ����
	if (m_mouseDevice)
	{
		m_mouseDevice->Unacquire();
		m_mouseDevice->Release();
		m_mouseDevice = NULL;
	}

	//Ű���� DirectDevice ����
	if (m_keyboardDevice)
	{
		m_keyboardDevice->Unacquire();
		m_keyboardDevice->Release();
		m_keyboardDevice = NULL;
	}

	//DirectInput ����
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = NULL;
	}

	return;
}

bool InputClass::ReadKeyboard()
{
	HRESULT result;

	//Ű���� ��ġ�� ���¸� ������
	result = m_keyboardDevice->GetDeviceState(sizeof(m_currKeyState), (LPVOID)&m_currKeyState);
	if (FAILED(result))
	{
		//DirectInputDevice�� ������ ������ ��� ��ġ�� �ٽ� �޾ƿ��� �κ�
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboardDevice->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool InputClass::ReadMouse()
{
	HRESULT result;

	//���콺 ��ġ�� ���¸� ������
	result = m_mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE2), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{

		//DirectInputDevice�� ������ ������ ��� ��ġ�� �ٽ� �޾ƿ��� �κ�
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouseDevice->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

//���콺�� ��ǥ�� ��� �Լ�
void InputClass::GetMousePosition(long& posX, long& posY)
{
	POINT pos;

	GetCursorPos(&pos);

	posX = pos.x;
	posY = pos.y;

	return;
}

//����ȭ�� ���콺�� ��ǥ�� ��� �Լ�
void InputClass::GetNormalizedMousePosition(float& posX, float& posY)
{
	return;
}

//���콺�� �������� ��� �Լ�
void InputClass::GetMouseDelta(float& posX, float& posY)
{
	posX = m_mouseState.lX * m_sensitivity;
	posY = m_mouseState.lY * m_sensitivity;

	return;
}

//Ű�� ������ true�� �����ϴ� �Լ�
bool InputClass::GetKeyPressed(const unsigned char keyCode)
{
	if (KEYDOWN(m_currKeyState, keyCode))
	{
		return true;
	}

	return false;
}

//Ű�� ���ȴٰ� ���� �� true�� �����ϴ� �Լ�
bool InputClass::GetKeyPressedAndRelease(const unsigned char keyCode)
{
	if (!KEYDOWN(m_currKeyState, keyCode))
	{
		if (KEYDOWN(m_prevKeyState, keyCode))
		{
			m_prevKeyState[keyCode] = 0x00;
			return true;
		}
	}
	else
	{
		m_prevKeyState[keyCode] = 0x80;
	}

	return false;
}

//Ű�� ���ٰ� ������ �� true�� �����ϴ� �Լ�
bool InputClass::GetKeyReleasedAndPress(const unsigned char keyCode)
{
	if (KEYDOWN(m_currKeyState, keyCode))
	{
		if (!KEYDOWN(m_prevKeyState, keyCode))
		{
			m_prevKeyState[keyCode] = 0x80;
			return true;
		}
	}
	else
	{
		m_prevKeyState[keyCode] = 0x00;
	}

	return false;
}

//���콺 ���� ��ư�� ���ȴ��� üũ�ϴ� �Լ�
bool InputClass::GetLeftMouseButtonDown()
{
	if (m_mouseState.rgbButtons[0] & 0x80)
	{
		return true;
	}
	
	return false;
}

//���콺 ������ ��ư�� ���ȴ��� üũ�ϴ� �Լ�
bool InputClass::GetRightMouseButtonDown()
{
	if (m_mouseState.rgbButtons[1] & 0x80)
	{
		return true;
	}

	return false;
}

//���콺 ���� ��ư�� ���ȴ� ������ üũ�ϴ� �Լ�
bool InputClass::GetLeftMouseButtonUp()
{
	if (!KEYDOWN(m_mouseState.rgbButtons, 0))
	{
		if (m_prevMouseState[0])
		{
			m_prevMouseState[0] = false;
			return true;
		}
	}
	else
	{
		m_prevMouseState[0] = true;
	}

	return false;
}

//���콺 ������ ��ư�� ���ȴ� ������ üũ�ϴ� �Լ�
bool InputClass::GetRightMouseButtonUp()
{
	if (!KEYDOWN(m_mouseState.rgbButtons, 1))
	{
		if (m_prevMouseState[1])
		{
			m_prevMouseState[1] = false;
			return true;
		}
	}
	else
	{
		m_prevMouseState[1] = true;
	}

	return false;
}