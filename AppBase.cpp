#include "AppBase.h"

// windows
#include <windowsx.h>

// d3d
#include <directxcolors.h>

// std
#include <cassert>
#include <vector>
#include <iostream>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mWindow is valid
	return AppBase::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

AppBase* AppBase::mApp = nullptr;

AppBase::AppBase(HINSTANCE instance)
	: mInstance(instance)
	, mViewport()
{
	assert(mApp == nullptr);
	mApp = this;
}

AppBase::~AppBase() {}

AppBase *AppBase::GetApp()
{
	return mApp;
}

bool AppBase::Init()
{
	if (!InitWindow())
	{
		return false;
	}

	if (!InitDirect3D())
	{
		return false;
	}

	OnResize();

	// default vertex shader
	{
		std::wstring path = L"../RenderToyD3D11/shaders/Default.hlsl";

		ComPtr<ID3DBlob> pCode = CompileShader(path, nullptr, "DefaultVS", ShaderTarget::VS);

		ThrowIfFailed(mDevice->CreateVertexShader(pCode->GetBufferPointer(),
												  pCode->GetBufferSize(),
												  nullptr,
												  &mDefaultVS));

		NameResource(mDefaultVS.Get(), "DefaultVS");

		// input layout
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> desc =
			{
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};

			ThrowIfFailed(mDevice->CreateInputLayout(desc.data(),
													 UINT(desc.size()),
													 pCode->GetBufferPointer(),
													 pCode->GetBufferSize(),
													 &mInputLayout));

			NameResource(mInputLayout.Get(), "DefaultVS_InputLayout");
		}
	}

	// default pixel shader
	{
		std::wstring path = L"../RenderToyD3D11/shaders/Default.hlsl";

		ComPtr<ID3DBlob> pCode = CompileShader(path, nullptr, "DefaultPS", ShaderTarget::PS);

		ThrowIfFailed(mDevice->CreatePixelShader(pCode->GetBufferPointer(),
												 pCode->GetBufferSize(),
												 nullptr,
												 &mDefaultPS));

		NameResource(mDefaultPS.Get(), "DefaultPS");
	}

	// fullscreen vertex shader
	{
		std::wstring path = L"../RenderToyD3D11/shaders/Fullscreen.hlsl";

		ComPtr<ID3DBlob> pCode = CompileShader(path, nullptr, "FullscreenVS", ShaderTarget::VS);

		ThrowIfFailed(mDevice->CreateVertexShader(pCode->GetBufferPointer(),
												  pCode->GetBufferSize(),
												  nullptr,
												  &mFullscreenVS));

		NameResource(mFullscreenVS.Get(), "FullscreenVS");
	}

	// main pass CB
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(MainPassCB);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		ThrowIfFailed(mDevice->CreateBuffer(&desc, nullptr, &mMainPassCB));

		NameResource(mMainPassCB.Get(), "MainPassCB");
	}

	// sampler states
	{
		D3D11_SAMPLER_DESC desc;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		//desc.BorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = +FLT_MAX;

		ThrowIfFailed(mDevice->CreateSamplerState(&desc, &mSamplerLinearWrap));

		NameResource(mSamplerLinearWrap.Get(), "SamplerLinearWrap");
	}

	mMeshManager.Init(mDevice, mContext);
	mMaterialManager.Init(mDevice, mContext);
	mObjectManager.Init(mDevice, mContext);
	mTextureManager.Init(mDevice, mContext);

	return true;
}

bool AppBase::InitWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = HBRUSH(GetStockObject(NULL_BRUSH));
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT rect = {0, 0, LONG(mWindowWidth), LONG(mWindowHeight)};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	mWindow = CreateWindow(L"MainWnd",
						   mWindowName.c_str(),
						   WS_OVERLAPPEDWINDOW,
						   CW_USEDEFAULT,
						   CW_USEDEFAULT,
						   width,
						   height,
						   0,
						   0,
						   mInstance,
						   0);
	if (!mWindow)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mWindow, SW_SHOW);
	UpdateWindow(mWindow);

	return true;
}

bool AppBase::InitDirect3D()
{
	ComPtr<IDXGIFactory1> pFactory; // IDXGIFactory4
	ThrowIfFailed(CreateDXGIFactory1(__uuidof(IDXGIFactory1),
									 reinterpret_cast<void**>(pFactory.GetAddressOf())));

#if 1 // list GPUs
	IDXGIAdapter1* pAdapter;
	std::vector<ComPtr<IDXGIAdapter1>> pAdapters;

	for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		pAdapters.push_back(pAdapter);
	}

	for (const auto& pAdapter : pAdapters)
	{
		DXGI_ADAPTER_DESC desc;
		ThrowIfFailed(pAdapter->GetDesc(&desc));
		
		// display current GPU name on ImGui
	}
#endif

	// GPU name
	{
		ComPtr<IDXGIAdapter1> pAdapter;
		ThrowIfFailed(pFactory->EnumAdapters1(0, &pAdapter));

		DXGI_ADAPTER_DESC desc;
		ThrowIfFailed(pAdapter->GetDesc(&desc));

		mGPUName = desc.Description;
	}

	// device & context
	{
		UINT flags = 0;
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

		D3D_FEATURE_LEVEL featureLevel;

		ThrowIfFailed(D3D11CreateDevice(pAdapters[1].Get(),
										D3D_DRIVER_TYPE_UNKNOWN, // D3D_DRIVER_TYPE_HARDWARE,
										nullptr,
										flags,
										nullptr,
										0,
										D3D11_SDK_VERSION,
										&mDevice,
										&featureLevel,
										&mContext));

		assert(featureLevel >= D3D_FEATURE_LEVEL_11_0);
	}

	// swap chain
	{
		DXGI_SWAP_CHAIN_DESC desc;
		desc.BufferDesc.Width = mWindowWidth;
		desc.BufferDesc.Height = mWindowHeight;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = mSwapChainFormat;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = min(mSwapChainBufferCount, DXGI_MAX_SWAP_CHAIN_BUFFERS);
		desc.OutputWindow = mWindow;
		desc.Windowed = true;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = mSwapChainFlags;

		ThrowIfFailed(pFactory->CreateSwapChain(mDevice.Get(), &desc, &mSwapChain));
	}

//	// rasterizer states
//
//	// wireframe
//	{
//		D3D11_RASTERIZER_DESC desc;
//		desc.FillMode = D3D11_FILL_WIREFRAME;
//		desc.CullMode = D3D11_CULL_NONE;
//		desc.FrontCounterClockwise = false;
//		desc.DepthBias = 0;
//		desc.DepthBiasClamp = 0;
//		desc.SlopeScaledDepthBias = 0;
//		desc.DepthClipEnable = true;
//		desc.ScissorEnable = false;
//		desc.MultisampleEnable = false;
//		desc.AntialiasedLineEnable = false;
//
//		HR(mDevice->CreateRasterizerState(&desc, &mWireframeRS));
//	}
//
//	// no cull
//	{
//		D3D11_RASTERIZER_DESC desc;
//		desc.FillMode = D3D11_FILL_SOLID;
//		desc.CullMode = D3D11_CULL_NONE;
//		desc.FrontCounterClockwise = false;
//		desc.DepthBias = 0;
//		desc.DepthBiasClamp = 0;
//		desc.SlopeScaledDepthBias = 0;
//		desc.DepthClipEnable = true;
//		desc.ScissorEnable = false;
//		desc.MultisampleEnable = false;
//		desc.AntialiasedLineEnable = false;
//
//		HR(mDevice->CreateRasterizerState(&desc, &mNoCullRS));
//	}
//
//	// depth stencil states
//
//	// less equal
//	{
//		D3D11_DEPTH_STENCIL_DESC desc;
//		desc.DepthEnable = true;
//		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
//		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
//		desc.StencilEnable = false;
//		//desc.StencilReadMask = 0xff;
//		//desc.StencilWriteMask = 0xff;
//		//desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
//		//desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
//		//desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
//		//desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
//
//		HR(mDevice->CreateDepthStencilState(&desc, &mLessEqualDSS));
//	}
//
//	// equal
//	{
//		D3D11_DEPTH_STENCIL_DESC desc;
//		desc.DepthEnable = true;
//		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
//		desc.DepthFunc = D3D11_COMPARISON_EQUAL;
//		desc.StencilEnable = false;
//		//desc.StencilReadMask = 0xff;
//		//desc.StencilWriteMask = 0xff;
//		//desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
//		//desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
//		//desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
//		//desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
//		//desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
//
//		HR(mDevice->CreateDepthStencilState(&desc, &mEqualDSS));
//	}

	return true;
}

// void AppBase::CreateRTVAndDSVDescriptorHeaps()
// {
// 	// ImGUI SRV
// 	{
// 		D3D12_DESCRIPTOR_HEAP_DESC desc;
// 		desc.NumDescriptors = 1;
// 		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
// 		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
// 		desc.NodeMask = 0;

// 		ThrowIfFailed(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mSRVHeap.GetAddressOf())));
// 	}
// }

void AppBase::OnResize()
{
	static UINT prevWidth = 0;
	static UINT prevHeight = 0;

	// ImGui_ImplDX12_InvalidateDeviceObjects();

	// ImGui_ImplDX12_CreateDeviceObjects();

	if ((mWindowWidth == 0) || (mWindowHeight == 0) || ((mWindowWidth == prevWidth) && (mWindowHeight == prevHeight)))
	{
		// resize only if the new size is valid and different
		return;
	}

	prevWidth = mWindowWidth;
	prevHeight = mWindowHeight;

	// back buffer
	{
		mBackBufferRTV.Reset();

		ThrowIfFailed(mSwapChain->ResizeBuffers(min(mSwapChainBufferCount, DXGI_MAX_SWAP_CHAIN_BUFFERS),
												mWindowWidth,
												mWindowHeight,
												mSwapChainFormat,
												mSwapChainFlags));

		ComPtr<ID3D11Resource> pBackBuffer;
		ThrowIfFailed(mSwapChain->GetBuffer(0,
											__uuidof(ID3D11Resource),
											reinterpret_cast<void**>(pBackBuffer.GetAddressOf())));

		NameResource(pBackBuffer.Get(), "BackBuffer");

		ThrowIfFailed(mDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &mBackBufferRTV));

		NameResource(mBackBufferRTV.Get(), "BackBufferRTV");
	}

	// depth stencil buffer
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = mWindowWidth;
		desc.Height = mWindowHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		ComPtr<ID3D11Texture2D> pDepthStencilBuffer;
		ThrowIfFailed(mDevice->CreateTexture2D(&desc,
											   nullptr,
											   &pDepthStencilBuffer));

		NameResource(pDepthStencilBuffer.Get(), "DepthStencilBuffer");

		// depth stencil view
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC desc;
			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			desc.Flags = 0;
			desc.Texture2D.MipSlice = 0;

			mDepthStencilBufferDSV.Reset();
			ThrowIfFailed(mDevice->CreateDepthStencilView(pDepthStencilBuffer.Get(),
														  &desc,
														  &mDepthStencilBufferDSV));

			NameResource(mDepthStencilBufferDSV.Get(), "DepthStencilBufferDSV");
		}

		// shader resource view
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.MipLevels = 1;

			mDepthBufferSRV.Reset();
			ThrowIfFailed(mDevice->CreateShaderResourceView(pDepthStencilBuffer.Get(),
															&desc,
															&mDepthBufferSRV));

			NameResource(mDepthBufferSRV.Get(), "DepthBufferSVR");
		}
	}

	// viewport
	{
		mViewport.TopLeftX = 0;
		mViewport.TopLeftY = 0;
		mViewport.Width = FLOAT(mWindowWidth);
		mViewport.Height = FLOAT(mWindowHeight);
		mViewport.MinDepth = 0;
		mViewport.MaxDepth = 1;
	}

	mWindowAspectRatio = float(mWindowWidth) / float(mWindowHeight);

	mCamera.SetLens(0.25f * XM_PI, mWindowAspectRatio, 1.0f, 1000.0f);
}

// extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT AppBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// if (ImGui_ImplWin32_WndProcHandler(mWindow, msg, wParam, lParam))
	// {
	// 	return true;
	// }

	switch (msg)
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated
	// we pause the game when the window is deactivated and unpause it when it becomes active
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mIsAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mIsAppPaused = false;
			mTimer.Start();
		}
		return 0;
	}
	// WM_SIZE is sent when the user resizes the window
	case WM_SIZE:
	{
		// new client area dimensions
		mWindowWidth = LOWORD(lParam);
		mWindowHeight = HIWORD(lParam);

		if (mDevice)
		{
			switch (wParam)
			{

			case SIZE_MINIMIZED:
			{
				mIsAppPaused = true;
				mIsWindowMinimized = true;
				mIsWindowMaximized = false;
			}
			case SIZE_MAXIMIZED:
			{
				mIsAppPaused = false;
				mIsWindowMinimized = false;
				mIsWindowMaximized = true;
				OnResize();
			}
			case SIZE_RESTORED:
			{
				// restoring from minimized state?
				if (mIsWindowMinimized)
				{
					mIsAppPaused = false;
					mIsWindowMinimized = false;
					OnResize();
				}
				// restoring from maximized state?
				else if (mIsWindowMaximized)
				{
					mIsAppPaused = false;
					mIsWindowMaximized = false;
					OnResize();
				}
				else if (mIsWindowResizing)
				{
					// If user is dragging the resize bars, we do not resize
					// the buffers here because as the user continuously
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars. So instead, we reset after the user is
					// done resizing the window and releases the resize bars, which
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
			}
		}
		return 0;
	}
	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars
	case WM_ENTERSIZEMOVE:
	{
		mIsAppPaused = true;
		mIsWindowResizing = true;
		mTimer.Stop();
		return 0;
	}
	// WM_EXITSIZEMOVE is sent when the user releases the resize bars
	case WM_EXITSIZEMOVE:
	{
		mIsAppPaused = false;
		mIsWindowResizing = false;
		mTimer.Start();
		OnResize(); // we reset everything based on the new window dimensions
		return 0;
	}
	// WM_DESTROY is sent when the window is being destroyed
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	// WM_MENUCHAR is sent when a menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key
	case WM_MENUCHAR:
	{
		// don't beep when we alt-enter
		return MAKELRESULT(0, MNC_CLOSE);
	}
	// catch this message so to prevent the window from becoming too small
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO *)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO *)lParam)->ptMinTrackSize.y = 200;
		return 0;
	}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	{
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	case WM_KEYUP:
	{
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		return 0;
	}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void AppBase::OnMouseDown(WPARAM state, int x, int y)
{
	mLastMousePosition.x = x;
	mLastMousePosition.y = y;

	SetCapture(mWindow);
}

void AppBase::OnMouseUp(WPARAM state, int x, int y)
{
	ReleaseCapture();
}

void AppBase::OnMouseMove(WPARAM state, int x, int y)
{
	auto clamp = [](const float& x, const float& a, const float& b) -> float
	{
		return x < a ? a : (x > b ? b : x);
	};

	if ((state & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePosition.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePosition.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}

	mLastMousePosition.x = x;
	mLastMousePosition.y = y;
}

void AppBase::OnKeyboardEvent(const Timer& timer)
{
	const float dt = timer.GetDeltaTime();

	if ((GetAsyncKeyState('W') & 0x8000))
	{
		mCamera.Walk(+10.0f * dt);
	}
	if ((GetAsyncKeyState('S') & 0x8000))
	{
		mCamera.Walk(-10.0f * dt);
	}
	if ((GetAsyncKeyState('A') & 0x8000))
	{
		mCamera.Strafe(-10.0f * dt);
	}
	if ((GetAsyncKeyState('D') & 0x8000))
	{
		mCamera.Strafe(+10.0f * dt);
	}
}

int AppBase::Run()
{
	MSG msg = {0};

	mTimer.Reset();

	// IMGUI_CHECKVERSION();
	// ImGui::CreateContext();
	// ImGuiIO &io = ImGui::GetIO();
	// (void)io;
	// // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// ImGui::StyleColorsDark();
	// // ImGui::StyleColorsClassic();

	// ImGui_ImplWin32_Init(mWindow);
	// ImGui_ImplDX12_Init(mDevice.Get(),
	// 					SwapChainBufferSize,
	// 					mBackBufferFormat,
	// 					mSRVHeap.Get(),
	// 					mSRVHeap->GetCPUDescriptorHandleForHeapStart(),
	// 					mSRVHeap->GetGPUDescriptorHandleForHeapStart());

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer.Tick();

			if (!mIsAppPaused)
			{
				//CalculateFrameStats();
				Update(mTimer);
				Draw(mTimer);

				ThrowIfFailed(mSwapChain->Present(0, 0));
			}
			else
			{
				Sleep(100);
			}
		}
	}

	// ImGui_ImplDX12_Shutdown();
	// ImGui_ImplWin32_Shutdown();
	// ImGui::DestroyContext();

	return int(msg.wParam);
}

// void AppBase::CalculateFrameStats()
// {
// 	static int FrameCount = 0;
// 	static float TimeElapsed = 0.0f;

// 	FrameCount++;

// 	// compute averages over one second period
// 	if ((mTimer.GetTotalTime() - TimeElapsed) >= 1.0f)
// 	{
// 		const float fps = static_cast<float>(FrameCount); // fps = FrameCount / 1 sec
// 		const float mspf = 1000.0f / fps;				  // milliseconds per frame

// 		const std::wstring windowText = mWindowTitle +
// 										L"    fps: " + std::to_wstring(fps) +
// 										L"   mspf: " + std::to_wstring(mspf);

// 		SetWindowText(mWindow, windowText.c_str());

// 		// reset for next average
// 		FrameCount = 0;
// 		TimeElapsed += 1.0f;
// 	}
// }

void AppBase::UpdateMainPassCB(const Timer& timer)
{
	MainPassCB buffer;

	buffer.viewProj = mCamera.GetViewProjF();
	buffer.viewProjInv = mCamera.GetViewProjInvF();
	buffer.eyePosition = mCamera.GetPositionF();

	buffer.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	buffer.lights[0].direction = mLighting.GetLightDirection(0);
	buffer.lights[0].strength = { 0.9f, 0.9f, 0.7f };
	buffer.lights[1].direction = mLighting.GetLightDirection(1);
	buffer.lights[1].strength = { 0.4f, 0.4f, 0.4f };;
	buffer.lights[2].direction = mLighting.GetLightDirection(2);
	buffer.lights[2].strength = { 0.2f, 0.2f, 0.2f };
	
	mContext->UpdateSubresource(mMainPassCB.Get(), 0, nullptr, &buffer, 0, 0);
}

void AppBase::Update(const Timer& timer)
{
	OnKeyboardEvent(timer);

	mCamera.UpdateViewMatrix();

	mLighting.UpdateLights(timer);

	UpdateMainPassCB(timer);

	mMeshManager.UpdateBuffers();
	mMaterialManager.UpdateBuffer();
}