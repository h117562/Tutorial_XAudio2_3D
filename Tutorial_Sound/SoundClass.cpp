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

	//XAudio 초기화
	result = InitializeXAudio();
	if (!result)
	{
		return false;
	}

	//재생할 wav 사운드 파일 로드
	result = LoadSoundFile(filePath);
	if (!result)
	{
		return false;
	}

	//X3DAudio 초기화
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

	//COM 초기화
	result = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(result))
	{
		return false;
	}

	//XAudio2 생성
	result = ::XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(result))
	{
		return false;
	}

	//마스터링 보이스 생성
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

	//CreateFile을 사용하여 오디오 파일을 엽니다.
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

	//오디오 파일에서 'RIFF' 청크를 찾아 파일 형식을 확인합니다.
	//파일 타입이 fourccWAVE 또는 'XWMA'인지 확인합니다.
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


	//'fmt' 청크를 찾아 해당 내용을 WAVEFORMATEXTENSIBLE 구조체로 복사합니다.
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


	//'data' 청크를 찾아 해당 내용을 버퍼로 읽습니다.
	//오디오 버퍼를 fourccDATA 청크의 내용으로 채웁니다.
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

	buffer.AudioBytes = dwChunkSize;  //오디오 버퍼의 크기. Byte로
	buffer.pAudioData = m_dataBuffer;  //버퍼에 들어있는 오디오 데이터
	buffer.Flags = XAUDIO2_END_OF_STREAM; 	//이 버퍼 이후에 더 이상 데이터가 없다고 설정합니다.

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

	//소스, 마스터링 보이스의 채널수를 가져옴
	m_xAudio2SourceVoice->GetVoiceDetails(&m_sourceDetails);
	m_xAudio2MasteringVoice->GetVoiceDetails(&m_masteringDetails);

	//마스터링 보이스 채널 마스크를 가져옵니다.
	DWORD dwChannelMask;
	m_xAudio2MasteringVoice->GetChannelMask(&dwChannelMask);

	result = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_x3DInstance);
	if (FAILED(result))
	{
		return false;
	}

	//소리 수신기 설정
	ZeroMemory(&m_listener, sizeof(X3DAUDIO_LISTENER));
	m_listener.Position = { 0.0f, 0.0f, 0.0f };
	m_listener.OrientFront = { 0.0f, 0.0f, 1.0f };
	m_listener.OrientTop = { 0.0f, 1.0f, 0.0f };

	//소리 방출기 설정 
	ZeroMemory(&m_emitter, sizeof(X3DAUDIO_EMITTER));
	m_emitter.Position = { 0.0f, 0.0f, 0.0f };
	m_emitter.OrientFront = { 0.0f, 0.0f, 1.0f };
	m_emitter.OrientTop = { 0.0f, 1.0f, 0.0f };
	m_emitter.DopplerScaler = 1.0f;
	m_emitter.CurveDistanceScaler = 1.0f;
	m_emitter.ChannelCount = 1;


	//X3DAudioCalculate에서 계산된 결과를 반환하는 데 사용됨
	m_matrixCoefficients = new FLOAT32[m_masteringDetails.InputChannels * m_sourceDetails.InputChannels];
	ZeroMemory(&m_DSPSettings, sizeof(X3DAUDIO_DSP_SETTINGS));
	m_DSPSettings.SrcChannelCount = m_sourceDetails.InputChannels;
	m_DSPSettings.DstChannelCount = m_masteringDetails.InputChannels;
	m_DSPSettings.pMatrixCoefficients = m_matrixCoefficients;

	//음성에 대한 새 설정을 계산
	X3DAudioCalculate(m_x3DInstance, &m_listener, &m_emitter,
		X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
		&m_DSPSettings);

	//볼륨 및 피치 값을 원본 음성에 적용
	m_xAudio2SourceVoice->SetOutputMatrix(m_xAudio2MasteringVoice, m_DSPSettings.SrcChannelCount, m_DSPSettings.DstChannelCount, m_DSPSettings.pMatrixCoefficients);
	m_xAudio2SourceVoice->SetFrequencyRatio(m_DSPSettings.DopplerFactor);

	////계산된 반향 수준을 서브믹스 음성에 적용
	//m_xAudio2SourceVoice->SetOutputMatrix(m_xAudio2MasteringVoice, sourceDetails.InputChannels, m_DSPSettings.DstChannelCount, &m_DSPSettings.ReverbLevel);

	////계산된 로우 패스 필터 직접 계수를 원본 음성에 적용
	//XAUDIO2_FILTER_PARAMETERS FilterParameters = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * m_DSPSettings.LPFDirectCoefficient), 1.0f };
	//m_xAudio2SourceVoice->SetFilterParameters(&FilterParameters);

	return true;
}


bool SoundClass::PlayAudio()
{
	HRESULT result;

	//소스 보이스 재생
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

	//음성에 대한 새 설정을 계산
	X3DAudioCalculate(m_x3DInstance, &m_listener, &m_emitter,
		X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
		&m_DSPSettings);

	//볼륨 및 피치 값을 원본 음성에 적용
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