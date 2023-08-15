#ifndef _CORE_MESH_GEOMETRY_H_
#define _CORE_MESH_GEOMETRY_H_
#include "..\CoreUtil.h"
#include "CoreSubMeshGeometry.h"
class CoreMeshGeometry {
public:
	std::string mName;
	
	ComPtr<ID3DBlob> mCpuVertexBuffer;
	ComPtr<ID3DBlob> mCpuIndexBuffer;
	ComPtr<ID3D12Resource> mGpuVertexBuffer;
	ComPtr<ID3D12Resource> mGpuIndexBuffer;
	ComPtr<ID3D12Resource> mGpuVertexBufferIntermediate;
	ComPtr<ID3D12Resource> mGpuIndexBufferIntermediate;

	UINT mVertexByteStride = 0;
	UINT mVertexBufferByteSize=0;
	UINT mIndexBufferByteSize = 0;
	DXGI_FORMAT mIndexBufferFormat = DXGI_FORMAT_R16_UINT;

	std::unordered_map<std::string, CoreSubMeshGeometry> mDrawArgs;

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()const;
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView()const;
};
#endif
