#include "textclass.h"

TextClass::TextClass()
{
	m_d2dFactory = 0;
	m_dwFactory = 0;
	m_defaultFormat = 0;
	m_renderTarget = 0;
	m_defaultBrush = 0;
}

TextClass::~TextClass()
{

}

bool TextClass::Initialize(IDXGISwapChain* pSwapChain)
{
	HRESULT result;
	IDXGISurface* backBuffer;
	D2D1_RENDER_TARGET_PROPERTIES props;

	//D3D�� ����ü�� ���۸� ������
	result = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (FAILED(result))
	{
		return false;
	}

	//���� �ʱ�ȭ
	ZeroMemory(&props, sizeof(D2D1_RENDER_TARGET_PROPERTIES));

	//Direct2D ���� Ÿ�� ����
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.dpiX = 0.0f;//0 �⺻
	props.dpiY = 0.0f;//0 �⺻
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
	props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);

	//D2D ���丮 ���� (�̱� ������)
	result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory);
	if (FAILED(result))
	{
		return false;
	}

	//���� Ÿ�� ����
	result = m_d2dFactory->CreateDxgiSurfaceRenderTarget(
		backBuffer,
		&props,
		&m_renderTarget
	);

	if (FAILED(result))
	{
		return false;
	}

	//�� ���� ������ ����
	backBuffer->Release();
	backBuffer = 0;

	//Dwrite ���丮 ����
	result = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,	//�ش� ���丮�� �������� �ݸ����� ���ϴ� �÷���
		__uuidof(IDWriteFactory),	//���丮 �������̽��� �ĺ��ϴ� GUID
		reinterpret_cast<IUnknown**>(&m_dwFactory)//���丮�� ���� ������ �ּ�
	);

	if (FAILED(result))
	{
		return false;
	}

	//��Ʈ ����
	result = m_dwFactory->CreateTextFormat(
		L"���������� �׿� OTF",	//��Ʈ �̸�
		NULL,					//��Ʈ �÷��ǿ� ���� ������ �ּ� NULL�� �ý��� ��Ʈ �÷���
		DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL,//��Ʈ �β�
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,//��Ʈ ��Ÿ��
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,//��Ʈ ��Ʈ��ġ
		30.0f,					//��Ʈ ������
		L"ko",					//���� �̸� ex) KO, EN
		&m_defaultFormat		//�ؽ�Ʈ ����(IDWriteTextFormat)�� ���� ������ �ּҸ� ��ȯ�Ѵ�
	);

	if (FAILED(result))
	{
		return false;
	}

	//�⺻ �귯�� ���� (����)
	result = m_renderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f)),
		&m_defaultBrush
	);
	if (FAILED(result))
	{
		return false;
	}

	//�⺻ �ؽ�Ʈ ���� (����)
	result = m_defaultFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

//�ؽ�Ʈ ������ �⺻ ��Ʈ, �÷�
void TextClass::RenderText(CONST WCHAR* ptext, FLOAT x, FLOAT y, FLOAT width, FLOAT height)
{
	m_renderTarget->DrawTextW(
		ptext,
		wcslen(ptext),
		m_defaultFormat,
		D2D1::RectF(x, y, width, height),
		m_defaultBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL
	);
}

//�ؽ�Ʈ ������ ���� ��Ʈ, �÷�
void TextClass::RenderText(CONST WCHAR* ptext, FLOAT x, FLOAT y, FLOAT width, FLOAT height, IDWriteTextFormat* pformat, ID2D1SolidColorBrush* pbrush)
{
	m_renderTarget->DrawTextW(
		ptext,
		wcslen(ptext),
		pformat,
		D2D1::RectF(x, y, width, height),
		pbrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL
	);
}

//�ؽ�Ʈ ������ ���� �Լ�
void TextClass::BeginDraw()
{
	m_renderTarget->BeginDraw();
}

//�ؽ�Ʈ ������ ���� �Լ�
void TextClass::EndDraw()
{
	m_renderTarget->EndDraw();
}

void TextClass::Shutdown()
{
	if (m_defaultBrush)
	{
		m_defaultBrush->Release();
		m_defaultBrush = 0;
	}

	if (m_defaultFormat)
	{
		m_defaultFormat->Release();
		m_defaultFormat = 0;
	}

	if (m_renderTarget)
	{
		m_renderTarget->Release();
		m_renderTarget = 0;
	}

	if (m_dwFactory)
	{
		m_dwFactory->Release();
		m_dwFactory = 0;
	}

	if (m_d2dFactory)
	{
		m_d2dFactory->Release();
		m_d2dFactory = 0;
	}

}