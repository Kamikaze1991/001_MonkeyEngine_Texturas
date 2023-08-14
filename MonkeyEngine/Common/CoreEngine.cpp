#include "CoreEngine.h"

CoreEngine::~CoreEngine()
{
}

CoreEngine::CoreEngine(int width, int height, bool fullscreen):mClientWidth(width),mClientHeight(height),mFullsccreen(fullscreen)
{
	gMainHwnd = nullptr;
}

void CoreEngine::InitDirect3D(HWND mHwnd)
{
	mCoreGraphics = new CoreGraphics();
	mTimer = new CoreTimer();
	mCoreGraphics->InitDirect3D(mHwnd, mClientWidth, mClientHeight);
	gMainHwnd = mHwnd;
	OnInitialize();
}

void CoreEngine::ResetEngine()
{
	mCoreGraphics->OnReset(mClientWidth, mClientHeight);
	mCurrFrame = 0;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CoreEngine::GetEngineGraphicsCommandList()
{
	return mCoreGraphics->mGraphicsCommandList;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> CoreEngine::GetEngineCommandQueue()
{
	return mCoreGraphics->mCommandQueue;
}

Microsoft::WRL::ComPtr<IDXGISwapChain3> CoreEngine::GetEngineSwapChain()
{
	return mCoreGraphics->mSwapChain;
}

void CoreEngine::Loop()
{
	OnInitializeUi();
	OnUpdate();
	OnRender();
	mCurrFrame = (mCurrFrame + 1) % mFrameCount;
}

CoreGraphics* CoreEngine::GetCoreGraphics()
{
	return mCoreGraphics;
}

CoreTimer* CoreEngine::GetCoreTimer()
{
	return mTimer;
}

void CoreEngine::WindowRedimention(int width, int height)
{
	mClientWidth = width;
	mClientHeight = height;
}

