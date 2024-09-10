#include "Systemclass.h"
#pragma comment(linker , "/entry:WinMainCRTStartup /subsystem:console")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	bool result;
	SystemClass* systemClass;

	//SystemClass ����
	systemClass = new SystemClass;
	if (!systemClass)
	{
		return 0;
	}

	//SystemClass ����
	result = systemClass->Initialize();
	if (result)
	{
		systemClass->Run();
	}

	//��� �ڿ� ��ȯ
	systemClass->Shutdown();
	delete systemClass;
	systemClass = 0;


	return 0;
}