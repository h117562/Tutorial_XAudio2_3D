#ifndef _SOUNDCLASS_H_
#define _SOUNDCLASS_H_


#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

#pragma comment(lib, "xaudio2.lib")

#include <xaudio2.h>
#include <x3daudio.h>

class SoundClass
{
public:
	SoundClass();
	SoundClass(const SoundClass&);
	~SoundClass();

	bool Initialize(const char*);
	bool PlayAudio();
	void UpdateListener(const DirectX::XMVECTOR position, const DirectX::XMVECTOR lookAt, const DirectX::XMVECTOR up);
	void UpdateEmitter(const DirectX::XMVECTOR position, const DirectX::XMVECTOR lookAt, const DirectX::XMVECTOR up);
	bool Frame();
	void Shutdown();

private:
	bool InitializeXAudio();
	bool InitializeXAudio3D();
	bool LoadSoundFile(const char*);
	HRESULT FindChunk(HANDLE, DWORD, DWORD&, DWORD&);
	HRESULT ReadChunkData(HANDLE, void*, DWORD, DWORD);

private:
	IXAudio2* m_xAudio2;
	IXAudio2MasteringVoice* m_xAudio2MasteringVoice;
	IXAudio2SourceVoice* m_xAudio2SourceVoice;
	XAUDIO2_VOICE_DETAILS m_masteringDetails, m_sourceDetails;
	X3DAUDIO_HANDLE m_x3DInstance;
	X3DAUDIO_LISTENER m_listener;
	X3DAUDIO_EMITTER m_emitter;
	X3DAUDIO_DSP_SETTINGS m_DSPSettings;
	FLOAT32* m_matrixCoefficients;
	BYTE* m_dataBuffer;
};

#endif