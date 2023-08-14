#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* mDevice)
{
	mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mFrameCommandAllocator.GetAddressOf()));
}
