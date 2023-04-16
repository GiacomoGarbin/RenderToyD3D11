#pragma once

// windows
#include <wrl.h>
#include <comdef.h>
using Microsoft::WRL::ComPtr;

// d3d
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

// std
#include <string>
#include <sstream>

// 
#include "Camera.h"
#include "Lighting.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "ObjectManager.h"
#include "TextureManager.h"
#include "Timer.h"
#include "Utility.h"

// imgui
#define IMGUI _DEBUG

class AppBase
{
public:

    AppBase(HINSTANCE instance);
    AppBase(const AppBase&) = delete;
    AppBase& operator=(const AppBase&) = delete;
    ~AppBase();

    static AppBase* GetApp();
    LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    virtual bool Init();
    int Run();

    virtual void OnResize();
    virtual void Update(const Timer& timer);
    virtual void Draw(const Timer& timer) = 0;

protected:

    virtual void OnMouseDown(WPARAM state, int x, int y);
    virtual void OnMouseUp(WPARAM state, int x, int y);
    virtual void OnMouseMove(WPARAM state, int x, int y);
    virtual void OnKeyboardEvent(const Timer& timer);

    virtual void UpdateMainPassCB(const Timer& timer);


    ComPtr<ID3D11Device> mDevice;
    ComPtr<ID3D11DeviceContext> mContext;

    ComPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotation;

    ComPtr<ID3D11RenderTargetView> mBackBufferRTV;
    ComPtr<ID3D11DepthStencilView> mDepthStencilBufferDSV;
    ComPtr<ID3D11DepthStencilView> mDepthStencilBufferReadOnlyDSV;

    D3D11_VIEWPORT mViewport;

    Camera mCamera;

    ComPtr<ID3D11InputLayout> mInputLayout;

    D3D11_PRIMITIVE_TOPOLOGY mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // shaders
    ComPtr<ID3D11VertexShader> mDefaultVS;
    ComPtr<ID3D11PixelShader> mDefaultPS;
    ComPtr<ID3D11VertexShader> mFullscreenVS;

    struct MainPassCB
    {
        //XMFLOAT4X4 view = MathHelper::Identity4x4();
        //XMFLOAT4X4 ViewInverse = MathHelper::Identity4x4();
        //XMFLOAT4X4 proj = MathHelper::Identity4x4();
        //XMFLOAT4X4 ProjInverse = MathHelper::Identity4x4();
        XMFLOAT4X4 viewProj;
        XMFLOAT4X4 viewProjInv;
//        XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();
        XMFLOAT3 eyePosition = { 0.0f, 0.0f, 0.0f };
        float padding1;
//        XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
//        XMFLOAT2 RenderTargetSizeInverse = { 0.0f, 0.0f };
//        float NearPlane = 0.0f;
//        float FarPlane = 0.0f;
//        float DeltaTime = 0.0f;
//        float TotalTime = 0.0f;
//
        XMFLOAT4 ambientLight = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//
//#if IS_FOG_ENABLED
//        XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
//        float FogStart = 5.0f;
//        float FogRange = 150.0f;
//        XMFLOAT2 padding2;
//#endif // IS_FOG_ENABLED
//
        Lighting::Light lights[LIGHT_MAX_COUNT];
    };

    static_assert((sizeof(MainPassCB) % 16) == 0, "constant buffer size must be 16-byte aligned");

    ComPtr<ID3D11Buffer> mMainPassCB;

    MeshManager mMeshManager;
    MaterialManager mMaterialManager;
    ObjectManager mObjectManager;
    TextureManager mTextureManager;
    Lighting mLighting;

    ComPtr<ID3D11ShaderResourceView> mDepthBufferSRV;

    ComPtr<ID3D11RenderTargetView> mGBufferRTV;
    ComPtr<ID3D11ShaderResourceView> mGBufferSRV;

    ComPtr<ID3D11SamplerState> mSamplerLinearWrap;

    enum class TimestampQueryType
    {
        BeginFrame,

        RayTracedBegin,
        RayTracedShadows,
        RayTracedReflections,

        ImGuiBegin,
        ImGuiEnd,

        EndFrame,

        Count
    };

    void GPUProfilerTimestamp(const TimestampQueryType type);

private:

    bool InitWindow();
    bool InitDirect3D();
#if IMGUI
    void InitImGui();
    void CleanupImGui();

    //std::size_t mTimestampQueryCount = 3;
    std::vector<ComPtr<ID3D11Query>> mQueries;
    std::size_t mCurrQueryIndex = 0;
    std::size_t mCurrGetDataIndex = -1;

    bool GPUProfilerInit();
    void GPUProfilerShutdown();
    void GPUProfilerBegin();
    void GPUProfilerEnd();
    std::size_t GetTimestampQueryIndex(const TimestampQueryType type) const;
    std::size_t GetTimestampGetDataIndex(const TimestampQueryType type) const;

    void ShowPerfWindow();
#endif // IMGUI

    static AppBase* mApp;

    HINSTANCE mInstance = nullptr;
    HWND mWindow = nullptr;

    bool mIsAppPaused = false;
    bool mIsWindowMinimized = false;
    bool mIsWindowMaximized = false;
    bool mIsWindowResizing = false;

    Timer mTimer;

    POINT mLastMousePosition = { 0, 0 };

    std::wstring mWindowName = L"Window Name";
    UINT mWindowWidth = 800;
    UINT mWindowHeight = 600;
    float mWindowAspectRatio = float(mWindowWidth) / float(mWindowHeight);

    UINT mCurrGPUIndex;
    std::wstring mCurrGPUName;
    bool mIsCurrGPUNvidia;

    UINT mSwapChainBufferCount = 2;
    DXGI_FORMAT mSwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT mSwapChainFlags = 0;


    ComPtr<IDXGISwapChain> mSwapChain;




    // default states
    ComPtr<ID3D11RasterizerState> mRasterizerState;
    ComPtr<ID3D11BlendState> mBlendState;
    ComPtr<ID3D11DepthStencilState> mDepthStencilState;

    // default samplers
    // textures



};