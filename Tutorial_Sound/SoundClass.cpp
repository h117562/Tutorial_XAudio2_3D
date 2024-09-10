#include "SoundClass.h"

SoundClass::SoundClass()
{
	m_xAudio2 = 0;
	m_xAudio2MasteringVoice = 0;
	m_xAudio2SourceVoice = 0;
	m_matrixCoefficients = 0;
	m_dataBuffer = 0;
}

SoundClass::SoundClass(const SoundClass& other)
{
}

SoundClass::~SoundClass()
{
}

bool SoundClass::Initialize(const char* filePath)
{
	bool result;

	//XAudio �ʱ�ȭ
	result = InitializeXAudio();
	if (!result)
	{
		return false;
	}

	//����� wav ���� ���� �ε�
	result = LoadSoundFile(filePath);
	if (!result)
	{
		return false;
	}

	//X3DAudio �ʱ�ȭ
	result = InitializeXAudio3D();
	if (!result)
	{
		return false;
	}
	
	return true;
}

bool SoundClass::InitializeXAudio()
{
	HRESULT result;

	//COM �ʱ�ȭ
	result = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(result))
	{
		return false;
	}

	//XAudio2 ����
	result = ::XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(result))
	{
		return false;
	}

	//�����͸� ���̽� ����
	result = m_xAudio2->CreateMasteringVoice(&m_xAudio2MasteringVoice);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


bool SoundClass::LoadSoundFile(const char* filePath)
{
	HRESULT result;

	WAVEFORMATEXTENSIBLE wfx;
	XAUDIO2_BUFFER buffer;

	ZeroMemory(&wfx, sizeof(WAVEFORMATEXTENSIBLE));
	ZeroMemory(&buffer, sizeof(XAUDIO2_BUFFER));

	//CreateFile�� ����Ͽ� ����� ������ ���ϴ�.
	HANDLE hFile = CreateFileA(
		filePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		return false;
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
	{
		return false;
	}


	DWORD dwChunkSize;
	DWORD dwChunkPosition;

	//����� ���Ͽ��� 'RIFF' ûũ�� ã�� ���� ������ Ȯ���մϴ�.
	//���� Ÿ���� fourccWAVE �Ǵ� 'XWMA'���� Ȯ���մϴ�.
	result = FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	if (FAILED(result))
	{
		return false;
	}

	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (FAILED(result))
	{
		return false;
	}

	if (filetype != fourccWAVE)
	{
		return false;
	}


	//'fmt' ûũ�� ã�� �ش� ������ WAVEFORMATEXTENSIBLE ����ü�� �����մϴ�.
	result = FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	if (FAILED(result))
	{
		return false;
	}

	result = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
	if (FAILED(result))
	{
		return false;
	}


	//'data' ûũ�� ã�� �ش� ������ ���۷� �н��ϴ�.
	//����� ���۸� fourccDATA ûũ�� �������� ä��ϴ�.
	result = FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	if (FAILED(result))
	{
		return false;
	}

	m_dataBuffer = new BYTE[dwChunkSize];

	result = ReadChunkData(hFile, m_dataBuffer, dwChunkSize, dwChunkPosition);
	if (FAILED(result))
	{
		return false;
	}

	buffer.AudioBytes = dwChunkSize;  //����� ������ ũ��. Byte��
	buffer.pAudioData = m_dataBuffer;  //���ۿ� ����ִ� ����� ������
	buffer.Flags = XAUDIO2_END_OF_STREAM; 	//�� ���� ���Ŀ� �� �̻� �����Ͱ� ���ٰ� �����մϴ�.

	result = m_xAudio2->CreateSourceVoice(&m_xAudio2SourceVoice, (WAVEFORMATEX*)&wfx);
	if (FAILED(result))
	{
		return false;
	}


	result = m_xAudio2SourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool SoundClass::InitializeXAudio3D()
{
	HRESULT result;

	//�ҽ�, �����͸� ���̽��� ä�μ��� ������
	m_xAudio2SourceVoice->GetVoiceDetails(&m_sourceDetails);
	m_xAudio2MasteringVoice->GetVoiceDetails(&m_masteringDetails);

	//�����͸� ���̽� ä�� ����ũ�� �����ɴϴ�.
	DWORD dwChannelMask;
	m_xAudio2MasteringVoice->GetChannelMask(&dwChannelMask);

	result = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_x3DInstance);
	if (FAILED(result))
	{
		return false;
	}

	//�Ҹ� ���ű� ����
	ZeroMemory(&m_listener, sizeof(X3DAUDIO_LISTENER));
	m_listener.Position = { 0.0f, 0.0f, 0.0f };
	m_listener.OrientFront = { 0.0f, 0.0f, 1.0f };
	m_listener.OrientTop = { 0.0f, 1.0f, 0.0f };

	//�Ҹ� ����� ���� 
	ZeroMemory(&m_emitter, sizeof(X3DAUDIO_EMITTER));
	m_emitter.Position = { 0.0f, 0.0f, 0.0f };
	m_emitter.OrientFront = { 0.0f, 0.0f, 1.0f };
	m_emitter.OrientTop = { 0.0f, 1.0f, 0.0f };
	m_emitter.DopplerScaler = 1.0f;
	m_emitter.CurveDistanceScaler = 1.0f;
	m_emitter.ChannelCount = 1;


	//X3DAudioCalculate���� ���� ����� ��ȯ�ϴ� �� ����
	m_matrixCoefficients = new FLOAT32[m_masteringDetails.InputChannels * m_sourceDetails.InputChannels];
	ZeroMemory(&m_DSPSettings, sizeof(X3DAUDIO_DSP_SETTINGS));
	m_DSPSettings.SrcChannelCount = m_sourceDetails.InputChannels;
	m_DSPSettings.DstChannelCount = m_masteringDetails.InputChannels;
	m_DSPSettings.pMatrixCoefficients = m_matrixCoefficients;

	//������ ���� �� ������ ���
	X3DAudioCalculate(m_x3DInstance, &m_listener, &m_emitter,
		X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
		&m_DSPSettings);

	//���� �� ��ġ ���� ���� ������ ����
	m_xAudio2SourceVoice->SetOutputMatrix(m_xAudio2MasteringVoice, m_DSPSettings.SrcChannelCount, m_DSPSettings.DstChannelCount, m_DSPSettings.pMatrixCoefficients);
	m_xAudio2SourceVoice->SetFrequencyRatio(m_DSPSettings.DopplerFactor);

	////���� ���� ������ ����ͽ� ������ ����
	//m_xAudio2SourceVoice->SetOutputMatrix(m_xAudio2MasteringVoice, sourceDetails.InputChannels, m_DSPSettings.DstChannelCount, &m_DSPSettings.ReverbLevel);

	////���� �ο� �н� ���� ���� ����� ���� ������ ����
	//XAUDIO2_FILTER_PARAMETERS FilterParameters = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * m_DSPSettings.LPFDirectCoefficient), 1.0f };
	//m_xAudio2SourceVoice->SetFilterParameters(&FilterParameters);

	return true;
}


bool SoundClass::PlayAudio()
{
	HRESULT result;

	//�ҽ� ���̽� ���
	result = m_xAudio2SourceVoice->Start(0, XAUDIO2_COMMIT_NOW);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void SoundClass::UpdateListener(const DirectX::XMVECTOR position, const DirectX::XMVECTOR lookAt, const DirectX::XMVECTOR up)
{
	m_listener.Position.x = position.m128_f32[0];
	m_listener.Position.y = position.m128_f32[1];
	m_listener.Position.z = position.m128_f32[2];

	m_listener.OrientFront.x = lookAt.m128_f32[0];
	m_listener.OrientFront.y = lookAt.m128_f32[1];
	m_listener.OrientFront.z = lookAt.m128_f32[2];

	m_listener.OrientTop.x = up.m128_f32[0];
	m_listener.OrientTop.y = up.m128_f32[1];
	m_listener.OrientTop.z = up.m128_f32[2];

	return;
}

void SoundClass::UpdateEmitter(const DirectX::XMVECTOR position, const DirectX::XMVECTOR lookAt, const DirectX::XMVECTOR up)
{
	m_emitter.Position.x = position.m128_f32[0];
	m_emitter.Position.y = position.m128_f32[1];
	m_emitter.Position.z = position.m128_f32[2];

	m_emitter.OrientFront.x = lookAt.m128_f32[0];
	m_emitter.OrientFront.y = lookAt.m128_f32[1];
	m_emitter.OrientFront.z = lookAt.m128_f32[2];

	m_emitter.OrientTop.x = up.m128_f32[0];
	m_emitter.OrientTop.y = up.m128_f32[1];
	m_emitter.OrientTop.z = up.m128_f32[2];

	return;
}

bool SoundClass::Frame()
{
	HRESULT result;

	//������ ���� �� ������ ���
	X3DAudioCalculate(m_x3DInstance, &m_listener, &m_emitter,
		X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
		&m_DSPSettings);

	//���� �� ��ġ ���� ���� ������ ����
	result = m_xAudio2SourceVoice->SetOutputMatrix(m_xAudio2MasteringVoice, m_sourceDetails.InputChannels, m_DSPSettings.DstChannelCount, m_DSPSettings.pMatrixCoefficients);
	if (FAILED(result))
	{
		return false;
	}

	result = m_xAudio2SourceVoice->SetFrequencyRatio(m_DSPSettings.DopplerFactor);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

HRESULT SoundClass::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType)
		{
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;
}

HRESULT SoundClass::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT result;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
		
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
	{
		result = HRESULT_FROM_WIN32(GetLastError());
	}
		
	return S_OK;
}

void SoundClass::Shutdown()
{

	if (m_dataBuffer)
	{
		delete[] m_dataBuffer;
		m_dataBuffer = 0;
	}

	if (m_matrixCoefficients)
	{
		delete[] m_matrixCoefficients;
		m_matrixCoefficients = 0;
	}

	if (m_xAudio2SourceVoice)
	{
		m_xAudio2SourceVoice->DestroyVoice();
		m_xAudio2SourceVoice = 0;
	}


	if (m_xAudio2MasteringVoice)
	{
		m_xAudio2MasteringVoice->DestroyVoice();
		m_xAudio2MasteringVoice = 0;
	}

	if (m_xAudio2)
	{
		m_xAudio2->Release();
		m_xAudio2 = 0;
	}

	// Uninitialize COM.
	::CoUninitialize();

	return;
}