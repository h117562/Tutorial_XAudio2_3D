#include "Systemclass.h"


SystemClass::SystemClass()
{
	m_applicationClass = 0;
	m_inputClass = 0;
	m_frameTimer = 0;
}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int screenwidth, screenheight;
	int posX, posY;
	bool result;

	//WndProc �Լ����� SystemClass�� �ν��Ͻ��� �����ϱ� ���� ���� ����
	ApplicationHandle = this;

	//���� ���α׷��� ���μ��� �ν��Ͻ� �ڵ��� ������
	m_hinstance = GetModuleHandle(NULL);

	//������ Ŭ���� �̸�
	m_applicationName = L"Tutorial Sound";

	//������ Ŭ���� ����
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	//������ Ŭ���� ���
	RegisterClassEx(&wc);

	//��ũ��ũ��
	screenwidth = 1200;
	screenheight = 800;

	//ȭ�� �߾ӿ� ����
	posX = (GetSystemMetrics(SM_CXSCREEN) - screenwidth) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - screenheight) / 2;


	//���α׷� ������ ����
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, WS_OVERLAPPEDWINDOW,
		posX, posY, screenwidth, screenheight, NULL, NULL, m_hinstance, NULL);

	//������ ��Ŀ�� ��������
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	
	m_frameTimer = new FrameTimer();
	if (!m_frameTimer)
	{
		return false;
	}

	//FrameTimer ����
	result = m_frameTimer->Initialize();
	if (!result)
	{
		return false;
	}

	m_inputClass = new InputClass();
	if (!m_inputClass)
	{
		return false;
	}

	//InputClass ����
	result = m_inputClass->Initialize(m_hinstance, m_hwnd);
	if (!result)
	{
		return false;
	}

	m_applicationClass = new ApplicationClass();
	if (!m_applicationClass)
	{
		return false;
	}

	result = m_applicationClass->Initialize(m_hinstance, m_hwnd, VSYNC_ENABLED, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		return false;
	}

	return true;
}


void SystemClass::Shutdown()
{

	if (m_applicationClass)
	{
		m_applicationClass->Shutdown();
		delete m_applicationClass;
		m_applicationClass = 0;
	}

	if (m_inputClass)
	{
		m_inputClass->Shutdown();
		delete m_inputClass;
		m_inputClass = 0;
	}

	if (m_frameTimer)
	{
		delete m_frameTimer;
		m_frameTimer = 0;
	}

	//������ �ڵ� �ı�
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	//��ϵ� ������ Ŭ������ ����
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	//SystemClass �ڵ� �����͸� NULL�� �ʱ�ȭ
	ApplicationHandle = NULL;

	return;
}


void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	//���� �ʱ�ȭ
	ZeroMemory(&msg, sizeof(MSG));


	done = false;
	while (!done)
	{
		//�޽��� ����Ȯ��
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//���� �޽��� Ȯ��
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			//���ᰡ �ƴϸ� ����Ŭ�ݺ�
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}

	}

	return;
}

bool SystemClass::Frame()
{
	bool result;

	m_frameTimer->Frame();

	result = m_inputClass->Frame();
	if (!result)
	{
		return false;
	}

	result = m_applicationClass->Frame(m_hwnd, m_inputClass, m_frameTimer);
	if (!result)
	{
		return false;
	}

	return true;
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}