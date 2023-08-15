#include "CoreMeshGeometry.h"

D3D12_VERTEX_BUFFER_VIEW CoreMeshGeometry::GetVertexBufferView() const
{
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    vertexBufferView.BufferLocation = mGpuVertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = mVertexByteStride;
    vertexBufferView.SizeInBytes = mVertexBufferByteSize;
    return vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW CoreMeshGeometry::GetIndexBufferView() const
{
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    indexBufferView.BufferLocation = mGpuIndexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = mIndexBufferFormat;
    indexBufferView.SizeInBytes = mIndexBufferByteSize;
    return indexBufferView;
}
