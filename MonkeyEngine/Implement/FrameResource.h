#include "../Common/CoreUtil.h"
#include <DirectXMath.h>
#include "..\Common\\CoreUploadBuffer.h"
using namespace DirectX;
struct ObjectConstant {
	DirectX::XMFLOAT4X4 World;
};

struct PassConstant {
	XMFLOAT4X4 View;
	XMFLOAT4X4 InverseView;
	XMFLOAT4X4 Projection;
	XMFLOAT4X4 InverseProjection;
	XMFLOAT4X4 ViewProjection;
	XMFLOAT4X4 InverseViewProjection;
	XMFLOAT3 EyePosition = { 0.0f,0.0f,0.0f };
	float pad0;
	XMFLOAT2 RenderTargetSize = { 0.0f,0.0f };
	XMFLOAT2 InverseRenderTargetSize = { 0.0f,0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
};

struct Vertex {
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};

class FrameResource {
public:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mFrameCommandAllocator;
	UINT64 mFrameFenceCount=0;
	std::unique_ptr<CoreUploadBuffer<PassConstant>> ConstantBufferPass = nullptr;
	std::unique_ptr<CoreUploadBuffer<ObjectConstant>> ConstantBufferObject = nullptr;
	FrameResource(ID3D12Device* d3dDevice, int objectCount, int passCount);
};
