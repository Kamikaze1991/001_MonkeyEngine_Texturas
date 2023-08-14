#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* d3dDevice, int objectCount, int passCount)
{
	ExceptionFuse(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mFrameCommandAllocator.GetAddressOf())));
	ConstantBufferPass = std::make_unique<CoreUploadBuffer<PassConstant>>(d3dDevice, passCount, true);
	ConstantBufferObject = std::make_unique<CoreUploadBuffer<ObjectConstant>>(d3dDevice, objectCount, true);
}
