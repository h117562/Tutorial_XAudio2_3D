#include "Triangle.h"

Triangle::Triangle()
{
	m_triangle = 0;
	m_indices = 0;

	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}

Triangle::~Triangle()
{
}

bool Triangle::Initialize(ID3D11Device* pDevice)
{
	HRESULT result;

	m_triangle = new VertexColor[3];
	m_indices = new UINT[3];

	//�ﰢ���� ������ �����ϴ� �κ�
	//�÷��� ����������
	m_triangle[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
	m_triangle[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	m_triangle[1].position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_triangle[1].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	m_triangle[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_triangle[2].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	m_indices[0] = 0;
	m_indices[1] = 1;
	m_indices[2] = 2;

	//�ε����� ����
	m_indexCount = 3;

	//���� ���ۿ� ���� ������ �Է��Ѵ�.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexColor) * m_indexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = m_triangle;

	//Desc�� ������� ���۸� �����Ѵ�.
	result = pDevice->CreateBuffer(&vertexBufferDesc, &initData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//�ε��� ���ۿ� ���� ������ �Է��Ѵ�.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	initData.pSysMem = &m_indices[0];


	//Desc�� ������� ���۸� �����Ѵ�.
	result = pDevice->CreateBuffer(&indexBufferDesc, &initData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}


	return true;
}


void Triangle::Render(ID3D11DeviceContext* pDeviceContext)
{
	//��ǲ ��������� ���۸� Ȱ��ȭ�Ͽ� ������ �� �� �ֵ��� ����
	pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	pDeviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->DrawIndexed(m_indexCount, 0, 0);
}



void Triangle::Shutdown()
{
	//����, �ε��� ���� ����
	m_vertexBuffer->Release();
	m_indexBuffer->Release();
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}