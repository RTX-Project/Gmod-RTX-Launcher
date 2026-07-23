#pragma once
#include <windows.h>
#include <d3d11.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <string>

class VideoPlayer {
public:
    VideoPlayer(ID3D11Device* device, ID3D11DeviceContext* context);
    ~VideoPlayer();

    bool Open(const std::wstring& path);
    void Update(float deltaTime);
    void Rewind();
    ID3D11ShaderResourceView* GetShaderResourceView() const { return m_srv; }
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    void Close();

private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    
    IMFSourceReader* m_reader = nullptr;
    ID3D11Texture2D* m_texture = nullptr;
    ID3D11ShaderResourceView* m_srv = nullptr;
    
    UINT m_width = 0;
    UINT m_height = 0;
    LONGLONG m_frameDuration = 0; // in 100ns units
    
    float m_accumulator = 0.0f;
    bool m_isPlaying = false;
    
    void CreateTexture();
    bool ReadNextFrame();
};
