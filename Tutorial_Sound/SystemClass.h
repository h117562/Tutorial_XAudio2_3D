#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#include "ApplicationClass.h"
#include "InputClass.h"
#include "FrameTimer.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();

private:
	LPCWSTR m_applicationName;	//���� ���α׷� �̸�
	HINSTANCE m_hinstance;		//�ν��Ͻ� �ڵ�
	HWND m_hwnd;				//������ �ڵ�

	ApplicationClass* m_applicationClass;
	InputClass* m_inputClass;
	FrameTimer* m_frameTimer;

};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* ApplicationHandle = 0;


#endif