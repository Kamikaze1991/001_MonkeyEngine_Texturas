#include "CoreGraphics.h"

CoreGraphics::~CoreGraphics()
{
	FlushCommandQueue();

}

CoreGraphics::CoreGraphics()
{
	mScissorRect = {};
	mViewPort = {};
}

void CoreGraphics::InitDirect3D(HWND mHwnd, int clientWidth, int clientHeight)
{
#if defined(DEBUG)||defined(_DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> mDebug;
		ExceptionFuse(D3D12GetDebugInterface(IID_PPV_ARGS(mDebug.GetAddressOf())));
		mDebug->EnableDebugLayer();
	}
#endif
	ExceptionFuse(CreateDXGIFactory2(0, IID_PPV_ARGS(mFactory.GetAddressOf())));

	HRESULT hrCreateDevice = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(mDevice.GetAddressOf()));
	if (FAILED(hrCreateDevice)) {
		Microsoft::WRL::ComPtr<IDXGIAdapter> localAdapter;
		ExceptionFuse(mFactory->EnumWarpAdapter(IID_PPV_ARGS(localAdapter.GetAddressOf())));
		ExceptionFuse(D3D12CreateDevice(localAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(mDevice.GetAddressOf())));
	}

	mDsvHeapSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mRtvHeapSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mCbvSrvUavheapSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetAddressOf()));


	BuildCommandObjects();
	BuildSwapChain(mHwnd,clientWidth,clientHeight);
	BuildMainDescriptorHeaps();

	OnReset(clientWidth,clientHeight);
}



void CoreGraphics::FlushCommandQueue()
{
	mFenceCount++;
	mCommandQueue->Signal(mFence.Get(), mFenceCount);
	//ExceptionFuse(mCommandQueue->Signal(mFenceCount));

	if (mFence->GetCompletedValue() < mFenceCount) {
		HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (eventHandle == nullptr)
			return;
		mFence->SetEventOnCompletion(mFenceCount, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void CoreGraphics::FlushCommandQueue(UINT64 fenceValue)
{
	if (fenceValue != 0 && mFence->GetCompletedValue() < fenceValue) {
		HANDLE handleLocal = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (!handleLocal)return;
		mFence->SetEventOnCompletion(fenceValue, handleLocal);
		WaitForSingleObject(handleLocal, INFINITE);
		CloseHandle(handleLocal);
	}
}

void CoreGraphics::OnReset(int clientWidth, int clientHeight)
{
	if (!mDevice)
		return;
	FlushCommandQueue();
	//	mCommandAllocator->Reset();
	ExceptionFuse(mGraphicsCommandList->Reset(mCommandAllocator.Get(), nullptr));
	//reset buffers
	for (int i = 0; i < mFrameCount; i++)
		mRenderTargetBuffer[i].Reset();
	mDepthStencilBuffer.Reset();
	//resize swap chain
	DXGI_SWAP_CHAIN_DESC sd = {};
	mSwapChain->GetDesc(&sd);
	ExceptionFuse(mSwapChain->ResizeBuffers(mFrameCount, clientWidth, clientHeight, mSwapChainFormat, sd.Flags));
	//rebuild buffers
	CD3DX12_CPU_DESCRIPTOR_HANDLE mRtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < mFrameCount; i++) {
		ExceptionFuse(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mRenderTargetBuffer[i].GetAddressOf())));
		mDevice->CreateRenderTargetView(mRenderTargetBuffer[i].Get(), nullptr, mRtvHandle);
		mRtvHandle.Offset(1, mRtvHeapSize);
	}

	D3D12_CLEAR_VALUE clearValue;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;
	clearValue.Format = mDepthStencilFormat;
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC mDsvDesc = CD3DX12_RESOURCE_DESC::Tex2D(mDepthStencilFormat, clientWidth, clientHeight, 1, 0);
	mDsvDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	mDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&mDsvDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = {};
	dsvd.Flags = D3D12_DSV_FLAG_NONE;
	dsvd.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsvd.Format = mDepthStencilFormat;
	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvd, DepthStencilView());

	mGraphicsCommandList->Close();
	ID3D12CommandList* cmdList[] = { mGraphicsCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	FlushCommandQueue();
	mViewPort.Height = static_cast<float>(clientHeight);
	mViewPort.Width = static_cast<float>(clientWidth);
	mViewPort.MaxDepth = 1000.0f;
	mViewPort.MinDepth = 1.0f;
	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;

	mScissorRect = { 0,0,clientWidth,clientHeight };

}

D3D12_CPU_DESCRIPTOR_HANDLE CoreGraphics::CurrentBackBufferView(int currFrame) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), currFrame, mRtvHeapSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE CoreGraphics::DepthStencilView() const
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

/// <summary>
/// Create Commands Objects
/// </summary>
void CoreGraphics::BuildCommandObjects()
{
	ExceptionFuse(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocator.GetAddressOf())));
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ExceptionFuse(mDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(mCommandQueue.GetAddressOf())));
	ExceptionFuse(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(mGraphicsCommandList.GetAddressOf())));
	mGraphicsCommandList->Close();
}

/// <summary>
/// Swap chain initialization
/// </summary>
/// <param name="mHwnd">Windows handlet</param>
void CoreGraphics::BuildSwapChain(HWND mHwnd, int clientWidth, int clientHeight)
{
	mSwapChain.Reset();
	Microsoft::WRL::ComPtr<IDXGISwapChain1> localSwapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = mFrameCount;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Format = mSwapChainFormat;
	swapChainDesc.Height = clientHeight;
	swapChainDesc.Width = clientWidth;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	ExceptionFuse(mFactory->CreateSwapChainForHwnd(mCommandQueue.Get(), mHwnd, &swapChainDesc, nullptr, nullptr, &localSwapChain));
	ExceptionFuse(localSwapChain.As(&mSwapChain));
}


/// <summary>
/// initialize basic descriptor heaps
/// </summary>
void CoreGraphics::BuildMainDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = mFrameCount;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ExceptionFuse(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));
	ExceptionFuse(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}
