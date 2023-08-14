#ifndef _CORE_GRAPHICS_H_
#define _CORE_GRAPHICS_H_
#include "CoreUtil.h"


class CoreGraphics {
public:
#pragma region Directx Main Variables
	Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
	Microsoft::WRL::ComPtr<IDXGIFactory4> mFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT mFenceCount = 0;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mGraphicsCommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	UINT mRtvHeapSize = 0;
	UINT mDsvHeapSize = 0;
	UINT mCbvSrvUavheapSize = 0;

	static const int mFrameCount = 2;
	Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargetBuffer[mFrameCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	D3D12_VIEWPORT mViewPort;
	D3D12_RECT mScissorRect;


	DXGI_FORMAT mSwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
#pragma endregion
	bool mFullsccreen = true;

public:
	~CoreGraphics();
	CoreGraphics();
	void InitDirect3D(HWND mHwnd, int clientWidth, int clientHeight);
	void OnReset(int clientWidth, int clientHeight);

	void FlushCommandQueue();
	void FlushCommandQueue(UINT64 fenceValue);
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(int currFrame)const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;
private:

	void BuildSwapChain(HWND mHwnd, int clientWidth, int clientHeight);
	void BuildCommandObjects();
	void BuildMainDescriptorHeaps();
};
#endif
