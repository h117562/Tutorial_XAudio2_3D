#include "d3dclass.h"


D3DClass::D3DClass()
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
	m_rasterStateNoCulling = 0;
	m_rasterStateWireframe = 0;
	m_depthDisabledStencilState = 0;
	m_alphaEnableBlendingState = 0;
	m_alphaDisableBlendingState = 0;
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}


bool D3DClass::Initialize(bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	size_t stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	D3D11_BLEND_DESC blendStateDescription;

	//VSync ���� 
	m_vsync_enabled = vsync;

	//���ΰ�ħ ��
	numerator = 60;		//����
	denominator = 1;	//�и�


	//������ â ũ�⸦ ����
	RECT rect;

	GetClientRect(hwnd, &rect);
	m_screenSize.x = static_cast<unsigned int>(rect.right - rect.left);
	m_screenSize.y = static_cast<unsigned int>(rect.bottom - rect.top);

	//�׷��� �������̽� ���丮 ����
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	//�׷��� �������̽��� ����� ����
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	//����� ���(Output)�� ����(Enumerate)��(�����ϴ�)
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	//����� ��¿��� DXGI_FORMAT_..._UNORM ���Ŀ� �´� ���÷��� ��� ������ ������
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	//�� ��ǻ���� ��� ���÷��� ��� ����Ʈ�� ����
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	//�Ʊ��� �۾��� �ݺ��Ͽ� �̹��� ���÷��� ��� ����Ʈ�� Structure�� ä����
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	//��� ���÷��� ��带 Ȯ���Ͽ� ���� ȭ�� ũ�⿡ �´� ��带 ã��, �ش� ���÷����� ���ΰ�ħ �󵵸� ������
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == m_screenSize.x)
		{
			if (displayModeList[i].Height == m_screenSize.y)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}


	//�׷��� ī�� ������ ������
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	//�׷��� ī�� �޸𸮸� ������
	m_videoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	//�׷��� ī�� �̸� ����
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	//�׷���ī�忡�� �ʿ��� ������ �� ������� �ڿ� ��ȯ����
	delete[] displayModeList;
	displayModeList = 0;

	adapterOutput->Release();
	adapterOutput = 0;

	adapter->Release();
	adapter = 0;

	factory->Release();
	factory = 0;


	//���� �ʱ�ȭ
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//���� ü�� ����
	swapChainDesc.BufferCount = 1;						//�� ���� ����
	swapChainDesc.BufferDesc.Width = m_screenSize.x;		//�� ���� �ʺ�
	swapChainDesc.BufferDesc.Height = m_screenSize.y;		//�� ���� ����
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//�� ���� ����

	//���ΰ�ħ �� ����
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//�� ���� ��� ��� ����
	swapChainDesc.OutputWindow = hwnd;	//����� â�� Hwnd �ڵ� 
	swapChainDesc.SampleDesc.Count = 1;	//���� ���� ����(1�� ���� ���ø��� ��) ex)���� ���ø�, ��Ƽ ���ø�
	swapChainDesc.SampleDesc.Quality = 0;//ǰ�� ���� ����(0�� ���� ���ø��� �� ��)

	//��üȭ�� ����
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_0;

	//Direct Device ����
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}

	//�� ���� ������ ��������
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	//���� Ÿ�� �� ����
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	//�� ���� ������ ����
	backBufferPtr->Release();
	backBufferPtr = 0;

	//���� �ʱ�ȭ
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	//���� ���� ����
	depthBufferDesc.Width = m_screenSize.x;	//�ʺ�
	depthBufferDesc.Height = m_screenSize.y;	//����
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	//���� ���� ����
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//���� �ʱ�ȭ
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	//����, ���ٽ� ����
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	//�ո� �ȼ��� ��� �۾��� ���ٽ�
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//�޸� �ȼ��� ��� �۾��� ���ٽ�
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//���� ���ٽ� State ����
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	//�ٷ� ������ State�� ����
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	//���� �ʱ�ȭ
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	//���� ���ٽ� �� ����
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//���� ���ٽ� �� ����
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	//���� Ÿ�� ��� ���� ���ٽ� �並 ��� ���� ������ ���ο� ���ε�
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	
	//�⺻ �����Ͷ����� ���� 
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;//������ �ո鸸 �������� �׸� (�޸��� �����Ѵٴ� �ǹ�)
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	//�����Ͷ����� ���� ����
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	//������ ������ ���� ����
	m_deviceContext->RSSetState(m_rasterState);

	//���̾� ������ ��� ����
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	//���̾� ������ ��� ���� ����
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterStateWireframe);
	if (FAILED(result))
	{
		return false;
	}

	//DirectX�� Ŭ�� ���� ��ǥ�� ������ ��� ������ ������ �� �ֵ��� ����Ʈ�� ������
	viewport.Width = static_cast<float>(m_screenSize.x);
	viewport.Height = static_cast<float>(m_screenSize.y);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	//����Ʈ ����
	m_deviceContext->RSSetViewports(1, &viewport);

	//Projection ��� ����
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = static_cast<float>(m_screenSize.x) / static_cast<float>(m_screenSize.y);

	//������ �ϱ� ���� Perspective Projection ��� ����
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	//���� ��� �ʱ�ȭ
	m_worldMatrix = XMMatrixIdentity();

	//2D �������� ���� ���ٰ��� ���� Orthographic Projection(���� ����) ��� ����
	m_orthoMatrix = XMMatrixOrthographicLH(static_cast<float>(m_screenSize.x), static_cast<float>(m_screenSize.y), screenNear, screenDepth);

	//2D �������� ���� ���� ���ٽ� ����
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//2D�� ���� ���ٽ� ���� ����
	result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
	if (FAILED(result))
	{
		return false;
	}

	//���� �ʱ�ȭ
	ZeroMemory(&blendStateDescription, sizeof(blendStateDescription));

	//���� ���� ����(���� ����)
	blendStateDescription.AlphaToCoverageEnable = false;
	blendStateDescription.IndependentBlendEnable = false;
	blendStateDescription.RenderTarget[0].BlendEnable = true;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;		
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;				//SourceBlend ���� �ȼ��� ���� ����
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;		//DestinationBlend ������ �׷��� �ȼ��鿡 ���� ����
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	


	//���� ���� ���� ����
	result = m_device->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState);
	if (FAILED(result))
	{
		return false;
	}

	//��� ���� �� ���� ����
	blendStateDescription.RenderTarget[0].BlendEnable = false;

	//���� ���� ����
	result = m_device->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void D3DClass::Shutdown()
{
	//�����ϱ� �� â���� �����ϰų� ���� ü���� �����ϸ� ���� �߻�
	//if (m_swapChain)
	//{
	//	m_swapChain->SetFullscreenState(false, NULL);
	//}

	if (m_alphaDisableBlendingState)
	{
		m_alphaDisableBlendingState->Release();
		m_alphaDisableBlendingState = 0;
	}

	if (m_alphaEnableBlendingState)
	{
		m_alphaEnableBlendingState->Release();
		m_alphaEnableBlendingState = 0;
	}

	if (m_depthDisabledStencilState)
	{
		m_depthDisabledStencilState->Release();
		m_depthDisabledStencilState = 0;
	}

	if (m_rasterStateWireframe)
	{
		m_rasterStateWireframe->Release();
		m_rasterStateWireframe = 0;
	}

	if (m_rasterStateNoCulling)
	{
		m_rasterStateNoCulling->Release();
		m_rasterStateNoCulling = 0;
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	//���۸� ä�� ������ ����
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	//�� ���� �۱�
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	//���� ���� �۱�
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void D3DClass::EndScene()
{
	//�� ���۸� ȭ�鿡 ǥ��
	if (m_vsync_enabled)
	{
		//ȭ�� ���� ��ħ �� ����
		m_swapChain->Present(1, 0);
	}
	else
	{
		//�غ� �Ǵ´�� ȭ�鿡 �ִ��� ���� ǥ��
		m_swapChain->Present(0, 0);
	}

	return;
}


ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

IDXGISwapChain* D3DClass::GetSwapChain()
{
	return m_swapChain;
}

void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}


void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}

void D3DClass::GetScreenSize(XMFLOAT2& screenSize)
{
	screenSize = m_screenSize;
	return;
}


void D3DClass::TurnZBufferOn()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
	return;
}


void D3DClass::TurnZBufferOff()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	return;
}


void D3DClass::EnableAlphaBlending()
{
	float blendFactor[4];

	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor, 0xffffffff);

	return;
}


void D3DClass::DisableAlphaBlending()
{
	float blendFactor[4];

	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState, blendFactor, 0xffffffff);

	return;
}


void D3DClass::TurnOnCulling()
{
	//�ø� ��� ����
	m_deviceContext->RSSetState(m_rasterState);

	return;
}


void D3DClass::TurnOffCulling()
{
	//�ø� ��� ����
	m_deviceContext->RSSetState(m_rasterStateNoCulling);

	return;
}


void D3DClass::EnableWireframe()
{
	//���̾� ������ ��� ����
	m_deviceContext->RSSetState(m_rasterStateWireframe);

	return;
}


void D3DClass::DisableWireframe()
{
	//���̾� ������ ��� ����
	m_deviceContext->RSSetState(m_rasterState);

	return;
}