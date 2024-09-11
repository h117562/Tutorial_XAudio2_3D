#include "ApplicationClass.h"

ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_CameraClass = 0;
	m_uiManager = 0;
	m_TextClass = 0;
	m_SoundClass = 0;
	m_ShaderManager = 0;
	m_triangle = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, bool vsyncEnabled, bool fullScreen, float screenDepth, float screenNear)
{
	bool result;
	
	m_Direct3D = new D3DClass;
	if (!m_Direct3D)
	{
		return false;
	}

	result = m_Direct3D->Initialize(vsyncEnabled, hwnd, fullScreen, screenDepth, screenNear);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	m_ShaderManager = new ShaderManager;
	if (!m_ShaderManager)
	{
		return false;
	}

	result = m_ShaderManager->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the ShaderManager.", L"Error", MB_OK);
		return false;
	}

	m_TextClass = new TextClass;
	if (!m_TextClass)
	{
		return false;
	}

	result = m_TextClass->Initialize(m_Direct3D->GetSwapChain());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the TextClass.", L"Error", MB_OK);
		return false;
	}

	m_uiManager = new InterfaceManager;
	if (!m_uiManager)
	{
		return false;
	}

	result = m_uiManager->Initialize(m_Direct3D);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the UI Manager Class.", L"Error", MB_OK);
		return false;
	}

	m_triangle = new Triangle;
	if (!m_triangle)
	{
		return false;
	}

	result = m_triangle->Initialize(m_Direct3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the Triangle.", L"Error", MB_OK);
		return false;
	}

	m_CameraClass = new CameraClass;
	if (!m_CameraClass)
	{
		return false;
	}

	//�⺻ �� ��Ʈ���� �ʱ�ȭ
	m_CameraClass->SetBaseViewMatrix();


	m_SoundClass = new SoundClass;
	if (!m_SoundClass)
	{
		return false;
	}

	result = m_SoundClass->Initialize("..//data//monoTestFile.wav");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the SoundClass.", L"Error", MB_OK);
		return false;
	}

	//����� ���
	result = m_SoundClass->PlayAudio();
	if (!result)
	{
		MessageBox(hwnd, L"Could not play the Audio", L"Error", MB_OK);
		return false;
	}
	
	return result;
}

void ApplicationClass::Shutdown()
{

	if (m_SoundClass)
	{
		m_SoundClass->Shutdown();
		delete m_SoundClass;
		m_SoundClass = 0;
	}

	if (m_triangle)
	{
		m_triangle->Shutdown();
		delete m_triangle;
		m_triangle = 0;
	}

	if (m_uiManager)
	{
		m_uiManager->Shutdown();
		delete m_uiManager;
		m_uiManager = 0;
	}

	if (m_TextClass)
	{
		m_TextClass->Shutdown();
		delete m_TextClass;
		m_TextClass = 0;
	}

	if (m_CameraClass)
	{
		delete m_CameraClass;
		m_CameraClass = 0;
	}

	if (m_ShaderManager)
	{
		m_ShaderManager->Shutdown();
		delete m_ShaderManager;
		m_ShaderManager = 0;
	}
	
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	return;
}

bool ApplicationClass::Frame(HWND hwnd, InputClass* pInputClass, FrameTimer* pFrameTimer)
{
	bool result;

	//����� �Է� ó��
	HandleInput(pInputClass, pFrameTimer);

	//�׷��� ������
	Render(hwnd, pInputClass);

	//ī�޶��� ��ġ�� ȸ���� 3D ����� ��꿡 ���
	XMMATRIX inverseView;
	XMVECTOR pos, lookAt, up;

	//�� ��Ʈ������ ������� ����
	m_CameraClass->GetViewMatrix(inverseView);
	inverseView = XMMatrixInverse(0, inverseView);

	//���� ���� ����
	pos = inverseView.r[3];
	lookAt = inverseView.r[2];
	up = inverseView.r[1];

	//�Ҹ� ���ű� ������Ʈ
	m_SoundClass->UpdateListener(pos, lookAt, up);
	m_SoundClass->Frame();
	

	return true;
}

void ApplicationClass::HandleInput(InputClass* pInputClass, FrameTimer* pFrameTimer)
{
	bool state;
	float deltaX, deltaY, frameTime;

	frameTime = pFrameTimer->GetTime();
	m_CameraClass->SetFrameTime(frameTime);

	state = pInputClass->GetKeyPressed(DIK_A);
	m_CameraClass->MoveLeft(state);

	state = pInputClass->GetKeyPressed(DIK_S);
	m_CameraClass->MoveBackward(state);

	state = pInputClass->GetKeyPressed(DIK_D);
	m_CameraClass->MoveRight(state);

	state = pInputClass->GetKeyPressed(DIK_W);
	m_CameraClass->MoveForward(state);

	pInputClass->GetMouseDelta(deltaX, deltaY);
	m_CameraClass->UpdateRotation(deltaX, deltaY);

	return;
}


void ApplicationClass::Render(HWND hwnd, InputClass* pInputClass)
{
	XMMATRIX world, view, proj;

	m_CameraClass->Render();

	//3D RenderTarget �ʱ�ȭ(Ư�� �÷���)
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.2f, 1.0f);

	//2D RenderTarget �ʱ�ȭ
	m_TextClass->BeginDraw();

	m_Direct3D->GetWorldMatrix(world);
	m_Direct3D->GetProjectionMatrix(proj);
	m_CameraClass->GetViewMatrix(view);

	//�Ҹ� �߻��� 0,0,0�� ��ġ�� �ð������� ��Ÿ���� ���� �ﰢ���� ������
	{
		XMFLOAT3 direction, origin;
		m_CameraClass->GetPosition(direction);
		origin = XMFLOAT3(0.0f, 0.0f, 0.0f);

		//ī�޶� ���� �׻� �ٶ󺸰�
		float rotY;
		rotY = atan2f(origin.x - direction.x, origin.z - direction.z);

		//���� ȸ����Ŵ
		world = world * XMMatrixRotationY(rotY);

		//�ﰢ�� ������
		m_ShaderManager->GetColorShader()->Render(m_Direct3D->GetDeviceContext(), world, view, proj);
		m_triangle->Render(m_Direct3D->GetDeviceContext());
	}

	//UI ������
	m_uiManager->Frame(m_Direct3D, hwnd, m_ShaderManager, m_TextClass, m_CameraClass, pInputClass);

	m_TextClass->EndDraw();
	m_Direct3D->EndScene();

	return;
}