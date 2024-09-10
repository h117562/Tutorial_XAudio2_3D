#include "FrameTimer.h"


FrameTimer::FrameTimer()
{
	m_frequency = 0;
	m_startTime = 0;
	m_frameTime = 0.0f;
	m_ticksPerMs = 0.0f;
}


FrameTimer::FrameTimer(const FrameTimer& other)
{
}


FrameTimer::~FrameTimer()
{
}

bool FrameTimer::Initialize()
{
	//���� ī���� ��(�ʴ� ����)�� ������
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if (m_frequency == 0)
	{
		return false;
	}

	//ms�� ���� ��ȯ
	m_ticksPerMs = (float)(m_frequency / 1000);

	//���� ī���� �󵵸� ����
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}

void FrameTimer::Frame()
{
	INT64 currentTime;
	INT64 elapsedTicks;


	//���� �� ����
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	//���� �󵵿� ������ ���� ���
	elapsedTicks = currentTime - m_startTime;

	//������ �ð� ���
	m_frameTime = (float)elapsedTicks / m_frequency;

	//���� ī���� �� �ʱ�ȭ
	m_startTime = currentTime;

	return;
}

float FrameTimer::GetTime()
{
	return m_frameTime;
}
