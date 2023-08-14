#include "CoreUtil.h"

std::wstring CoreUtil::AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

UINT CoreUtil::CalcConstantBufferByteSize(UINT byteSize)
{
    return (byteSize + 255) & ~255;;
}

ComPtr<ID3D12Resource> CoreUtil::CreateDefaultBuffer(ComPtr<ID3D12GraphicsCommandList> &graphicsCommandList, ComPtr<ID3D12Device>& device, UINT64 byteSize, const void* initData, ComPtr<ID3D12Resource>& intermediateBuffer)
{
    ComPtr<ID3D12Resource> resultBuffer;
    D3D12_HEAP_PROPERTIES resultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC resultBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    
    ExceptionFuse(
        device->CreateCommittedResource(
            &resultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resultBufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(resultBuffer.GetAddressOf())

        )
    );
    D3D12_HEAP_PROPERTIES intermediateHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC intermediateBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ExceptionFuse(
        device->CreateCommittedResource(
            &intermediateHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &intermediateBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(intermediateBuffer.GetAddressOf()))
    );
    

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = (LONG_PTR)byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;
    D3D12_RESOURCE_BARRIER transitionScope = CD3DX12_RESOURCE_BARRIER::Transition(resultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    graphicsCommandList->ResourceBarrier(1, &transitionScope);
    UpdateSubresources<1>(graphicsCommandList.Get(), resultBuffer.Get(), intermediateBuffer.Get(), 0, 0, 1, &subResourceData);
    transitionScope = CD3DX12_RESOURCE_BARRIER::Transition(resultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    graphicsCommandList->ResourceBarrier(1, &transitionScope);

    return resultBuffer;
}
