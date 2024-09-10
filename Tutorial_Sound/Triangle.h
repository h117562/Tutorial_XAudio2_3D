#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include <d3dx11.h>

#include "GraphicsStructures.h"

class Triangle
{
public:
	Triangle();
	~Triangle();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

private:
	VertexColor* m_triangle;
	UINT* m_indices;
	UINT m_indexCount;

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	UINT stride = sizeof(VertexColor);
	UINT offset = 0;


};

#endif