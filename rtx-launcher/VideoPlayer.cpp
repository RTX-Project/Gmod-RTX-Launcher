#include "VideoPlayer.h"
#include <iostream>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

template <class T> void SafeRelease(T **ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

VideoPlayer::VideoPlayer(ID3D11Device* device, ID3D11DeviceContext* context) 
    : m_device(device), m_context(context) {
    // Note: MFStartup is assumed to be called at the application start
}

VideoPlayer::~VideoPlayer() {
    Close();
}

bool VideoPlayer::Open(const std::wstring& path) {
    Close();

    IMFAttributes* attributes = nullptr;
    MFCreateAttributes(&attributes, 1);
    attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

    HRESULT hr = MFCreateSourceReaderFromURL(path.c_str(), attributes, &m_reader);
    SafeRelease(&attributes);

    if (FAILED(hr)) return false;

    // Deselect all streams first, then select video
    m_reader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
    m_reader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);

    IMFMediaType* partialType = nullptr;
    MFCreateMediaType(&partialType);
    partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    partialType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    hr = m_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, partialType);
    SafeRelease(&partialType);

    if (FAILED(hr)) return false;

    IMFMediaType* uncompressedVideoType = nullptr;
    hr = m_reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &uncompressedVideoType);
    if (SUCCEEDED(hr)) {
        MFGetAttributeSize(uncompressedVideoType, MF_MT_FRAME_SIZE, &m_width, &m_height);
        
        UINT32 num, den;
        if (SUCCEEDED(MFGetAttributeRatio(uncompressedVideoType, MF_MT_FRAME_RATE, &num, &den)) && num > 0) {
            m_frameDuration = (LONGLONG)(10000000.0 / ((double)num / den));
        } else {
            m_frameDuration = 333333; // ~30 fps default
        }
        SafeRelease(&uncompressedVideoType);
    } else {
        return false;
    }

    CreateTexture();
    m_isPlaying = true;
    m_accumulator = 0.0f;
    ReadNextFrame();

    return true;
}

void VideoPlayer::CreateTexture() {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_width;
    desc.Height = m_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM; 
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    m_device->CreateTexture2D(&desc, nullptr, &m_texture);
    
    if (m_texture) {
        m_device->CreateShaderResourceView(m_texture, nullptr, &m_srv);
    }
}

bool VideoPlayer::ReadNextFrame() {
    if (!m_reader || !m_texture) return false;

    IMFSample* sample = nullptr;
    DWORD streamIndex, flags;
    LONGLONG timestamp;
    
    HRESULT hr = m_reader->ReadSample(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0, &streamIndex, &flags, &timestamp, &sample
    );

    if (FAILED(hr)) return false;

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        PROPVARIANT var = {0};
        var.vt = VT_I8;
        var.hVal.QuadPart = 0;
        m_reader->SetCurrentPosition(GUID_NULL, var);
        SafeRelease(&sample);
        return ReadNextFrame(); // read again after seek
    }

    if (sample) {
        IMFMediaBuffer* buffer = nullptr;
        sample->ConvertToContiguousBuffer(&buffer);
        
        BYTE* data = nullptr;
        DWORD maxLength, currentLength;
        buffer->Lock(&data, &maxLength, &currentLength);

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_context->Map(m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            BYTE* dest = (BYTE*)mapped.pData;
            BYTE* src = data;
            int pitch = m_width * 4;
            
            // RGB32 in MF. Copy directly without flipping
            for (UINT y = 0; y < m_height; ++y) {
                memcpy(dest + y * mapped.RowPitch, src + y * pitch, pitch);
            }
            
            m_context->Unmap(m_texture, 0);
        }

        buffer->Unlock();
        SafeRelease(&buffer);
        SafeRelease(&sample);
        return true;
    }

    return false;
}

void VideoPlayer::Update(float deltaTime) {
    if (!m_isPlaying) return;

    m_accumulator += deltaTime;
    float frameTime = (float)m_frameDuration / 10000000.0f;

    while (m_accumulator >= frameTime) {
        m_accumulator -= frameTime;
        ReadNextFrame();
    }
}

void VideoPlayer::Rewind() {
    if (m_reader) {
        PROPVARIANT var = {0};
        var.vt = VT_I8;
        var.hVal.QuadPart = 0;
        m_reader->SetCurrentPosition(GUID_NULL, var);
        m_accumulator = 0.0f;
    }
}

void VideoPlayer::Close() {
    m_isPlaying = false;
    SafeRelease(&m_reader);
    SafeRelease(&m_srv);
    SafeRelease(&m_texture);
}
